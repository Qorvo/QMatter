import sys
import os
import traceback
import threading
import time
import six          # needed for heartbeat to run on Python2 & 3
import socket
import select

import serial
# print(f"=================================== {serial.__file__}")  # this breaks jadelogger
from serial import Serial

try:
    import Queue as queue
except ImportError:
    import queue

# do the regular path stuff - drop down to v... to get to testEnv
# envPath = os.path.join(os.path.dirname(__file__), "..", "..")
# sys.path.append(envPath)
# from testEnv.scripts.gpAPI_2 import ProcessThread   # noqa


envPath = os.path.join(os.path.dirname(__file__), "..", "..")
sys.path.append(envPath)

from testEnv.scripts.gpAPI_2 import ProcessThread   # noqa


def crc16(crc, byte):
    crc = crc ^ byte
    for bit in range(0, 8):
        if (crc & 0x0001) == 0x0001:
            crc = ((crc >> 1) ^ 0xA001)
        else:
            crc = crc >> 1
    return crc


class gpMsg(object):
    def __init__(self, moduleId, data):
        self.moduleId = moduleId
        self.data = data

    def __str__(self):
        s = "%02X [%s]" % (self.moduleId, ",".join(list(map(lambda x: "%02X" % x, self.data))))
        return s


############################
# Protocol Connector types #
############################
class genericConnector(object):
    def __init__(self):
        self.verbose = False
        self.name = "GENERIC"

        self.IsStarted = False
        self.thread = ProcessThread(self._Read, name=self.__class__.__name__)

        self.indicateCallbacks = []
        self.writeCallbacks = []

        # Allow alternate protocol parser
        self.protocol = None
        # Debug
        self.droppedBuffer = []

    def _ParseProtocol(self, data):
        raise RuntimeError("Need some parsing implementation")

    def _Write(self, data):
        raise RuntimeError("Need some write implementation")

    def _Read(self):
        raise RuntimeError("Need some read implementation")

    def _Stop(self):
        raise RuntimeError("Need some stop implementation")

    def log(self, s):
        if self.verbose:
            print("%s:%s" % (self.name, str(s)))

    def __str__(self):
        s = "%s" % self.name
        return s


# ### No Protocol ###

class rawConnector(genericConnector):
    def __init__(self):
        genericConnector.__init__(self)
        self.protocol_buffer = []

    def Start(self):
        if not self.IsStarted:
            self.IsStarted = True
            self._ResetProtocol()
            self.thread.Start()
        else:
            self.log("Already running")

        return True

    def Stop(self):
        if self.IsStarted:
            self.thread.Stop()
            while(not self.thread.IsDone()):
                pass
            self.IsStarted = False
            self._ResetProtocol()
            self._Stop()
        else:
            self.log("Already stopped")

        return True

    def WriteData(self, moduleId, data):
        """ Write raw data over connector.

        Function will prepending the module Id as 3 digit decimal number
        according to no SYN implementation of gpCom

        Keeps an option to send fully raw message using moduleId 0

        :param moduleId: module Id to send message to. Use 0 to send a full raw message
        :type moduleId: digit
        :param data: raw byte input
        :type data: list
        """
        if moduleId == 0:
            message = data[:]
        else:
            message = list(map(ord, "%03u " % moduleId)) + data
        # Terminator expected to be part of input
        # message += map(ord, ['\r', '\n'])

        self._Write(message)

        for write_cb in self.writeCallbacks:
            write_cb(moduleId, data)

    def _ParseProtocol(self, data):
        """ Parse data

            Expecting string data terminated by \r\n
            Full string can be chopped in multiple _ParseProtocol calls
        """
        # Eliminate non ASCII printable chars
        self.protocol_buffer += data

        logstring = "".join(['%c' % ch for ch in self.protocol_buffer])

        # Capture every line
        while '\n' in logstring:
            endIndex = logstring.find('\n')

            # Notify any subscribed callbacks
            for indCb in self.indicateCallbacks:
                try:
                    indCb(0, map(ord, logstring[:endIndex]))
                except Exception:
                    traceback.print_exc()

            # Scroll to remainder - skipping \n of printed part
            logstring = logstring[endIndex + 1:]

        # Keep remainder, waiting for next \r\n
        self.protocol_buffer = list(map(ord, logstring))

    def _ResetProtocol(self):
        self.protocol_buffer = []


# ### BLE stub ###
gpHci_CommandPacket = 0x01
gpHci_DataPacket = 0x02
gpHci_SyncDataPacket = 0x03
gpHci_EventPacket = 0x04


class bleConnector(genericConnector):
    def __init__(self):
        genericConnector.__init__(self)
        self.name = "BLE"
        self.packettype = gpHci_EventPacket  # just an initial value

    def Start(self):
        print("starting...")
        if not self.IsStarted:
            self._ResetProtocol()
            self._ResetPacket()
            self.IsStarted = True
            self.thread.Start()
        else:
            self.log("Already running")

        return True

    def Stop(self):
        if self.IsStarted:
            self.thread.Stop()
            while(not self.thread.IsDone()):
                pass
            self.IsStarted = False
            self._ResetProtocol()
            self._ResetPacket()
            self._Stop()
        else:
            self.log("Already stopped")

        return True

    def WriteData(self, moduleId, data):
        # data is a fully populated HCI frame . . . no need to chop it up again : just pass it on
        # some printing for debug purpose
        #        if (data[0] == gpHci_CommandPacket):
        #            opCode = data[1:2]
        #            length = data[3]
        #
        #            for ele in opCode:
        #                print " HCI Command Opcode %x  " % ele
        #            print " HCI Command Len= %d " % length
        #            #print " ModuleId %d " % moduleId
        #            for ele in data[4:]:
        #                print " data %d  " % ele
        #
        #        if (data[0] == gpHci_DataPacket):
        #            ConnId = data[1:2] #including the PacketBoundary Flag
        #            length = data[3:4]
        #
        #            print " HCI Data to hciConnectionId %d" %( ConnId[0] + ConnId[1]*256)
        #            print " HCI Data Len= %d " % (Length[0] + Length[1]*256)
        #            #print " ModuleId %d " % moduleId
        #            for ele in data[5:]:
        #                print " data %d  " % ele
        # end of print stuff

        # just send the raw HCI formatted data
        self._Write(data)

    def _ParseProtocol(self, data):
        # if self.protocol:
        #    if self.protocol(data):
        #        return
        # print " bleConnector _ParseProtocol len %d Protstate %d" % len(data), self.protocolState

        while(len(data)):
            byte = data.pop(0)
            # print "Rx 0x%x " % byte
            # self.log(byte)

            if self.protocolState == 0:
                # self.log("H %02X %d" % (byte, self.protocolCount))
                if self.protocolCount == 0:
                    if ((byte == gpHci_EventPacket) or (byte == gpHci_DataPacket)):
                        # Reset packet parser state mpachine : we'e received a packet delimiter
                        # print "start of frame detected: %d"%byte
                        self._ResetPacket()

                        if len(self.droppedBuffer):
                            self.log("!!Dr %d:[%s]!!" % (len(self.droppedBuffer),
                                                         ",".join(map(lambda x: "%02X" % x, self.droppedBuffer))))
                        self.droppedBuffer = []

                        self.protocolCount += 1
                        self.packettype = byte
                        self.packetData.append(byte)
                    else:
                        # we're out of synch with the controller - try to resynch
                        # print "*** out of synch ***"
                        if byte != 0:
                            self.droppedBuffer.append(byte)
                            self.log("Dropping @ S 0x%02X - %c" % (byte, byte))
                        # self.droppedBuffer.append(byte)
                        self._ResetProtocol()
                elif self.protocolCount == 1:
                    # Now we should have received:
                    # - for a gpHci_EventPacket: Event Type
                    # - for a gpHci_DataPacket: ConnId_LSB
                    # ... just store the received byte
                    self.protocolCount += 1
                    self.packetData.append(byte)
                elif self.protocolCount == 2:
                    # Now we should have received:
                    # - for gpHci_EventPacket: Event Length : got to self.protocolState = 1
                    # - for gpHci_DataPacket: ConnId_MSB
                    if(self.packettype == gpHci_EventPacket):
                        self.protocolState = 1
                        self.packetLength = byte
                        # self.protocolCount +=1
                        self.packetData.append(byte)
                    if(self.packettype == gpHci_DataPacket):
                        self.protocolCount += 1
                        self.packetData.append(byte)
                elif self.protocolCount == 3:
                    # here we're only dealing with a gpHci_DataPacket : receive Length_LSB
                    self.packetLength = byte
                    self.protocolCount += 1
                    self.packetData.append(byte)
                elif self.protocolCount == 4:
                    # here we're only dealing with a gpHci_DataPacket : receive Length_MSB
                    self.packetLength += (byte * 256)
                    self.protocolCount += 1
                    self.packetData.append(byte)
                    # could add a length sanity check
                    self.protocolState = 1
            elif self.protocolState == 1:  # data
                # print "D %02X %d %d" % (byte, protocolCount, packetLength)
                if self.packetLength:
                    # print "Len: %d %02X" % (packetLength, packetModuleID)
                    self.packetData.append(byte)
                    self.packetLength -= 1
                if self.packetLength == 0:
                    self.protocolState += 1
                    self.protocolCount = 0

                    # Finish off
                    for indCb in self.indicateCallbacks:
                        try:
                            # print "calling %s" % indCb
                            indCb(self.packetModuleID, self.packetData)
                        except Exception:
                            traceback.print_exc()
                    # print "%02X [%s]" % (packetModuleID, ",".join(map(lambda x: "%02X" % x, packetData)))
                    # Start again
                    self._ResetProtocol()

    def _ResetProtocol(self):
        self.protocolCount = 0
        self.protocolState = 0
        self.EventCode = 0

    def _ResetPacket(self):
        self.packetLength = 0
        # small issue: the ModuleID is set as a magic value - it is actually defined in the gpHciAPI.py file
        self.packetModuleID = 156
        self.packetData = []


class peakConnector(genericConnector):
    def __init__(self, wakeup=False):
        genericConnector.__init__(self)
        self.name = "PEAK"
        # Add Wake up using extra pre-amble byte
        self.wakeup = wakeup

    def Start(self):
        if not self.IsStarted:
            self._ResetProtocol()
            self._ResetPacket()
            self.IsStarted = True
            self.thread.Start()
        else:
            self.log("Already running")

        return True

    def Stop(self):
        if self.IsStarted:
            self.thread.Stop()
            try:
                while(not self.thread.IsDone()):
                    pass
            except Exception as e:
                print("Ignoring exception %s" % str(e))
            self.IsStarted = False
            self._ResetProtocol()
            self._ResetPacket()
            self._Stop()
        else:
            self.log("Already stopped")

        return True

    def WriteData(self, moduleId, data):
        if len(data) >= (1 << 12):  # 8 + 4 bit length field
            raise RuntimeError("Packet too long %d >= %d" % (len(data), (1 << 12)))
        # SYN|len|res|modId|data
        # SYN = 0x53, 0x59, 0x4E

        if self.wakeup:
            # wakeup on RX
            # Send pre-amble byte first, wait and let the chip wake-up, then send the actual data (eg: wa 0x94 dbgu_app)
            preamble = [0xA5]
            self._Write(preamble)
            time.sleep(0.002)

        lenLSB = len(data) & 0xFF
        lenMSB = (len(data) >> 8) & 0x0F  # Only using lower 4 bits

        message = []
        message += [ord('S'), ord('Y'), ord('N'), lenLSB, lenMSB, moduleId]
        message += data

        crc = 0
        for byte in message:
            crc = self._updateCRC(crc, byte)
        message += [crc & 0xFF, (crc >> 8) & 0xFF]

        # self.log(message)
        self._Write(message)

        for writeCb in self.writeCallbacks:
            writeCb(moduleId, data)

    def _ParseSynProtocol(self, data):
        while(len(data)):
            byte = data.pop(0)
            # self.log("%02X - %c" % (byte, byte))

            if self.protocolState == 0:
                # self.log("H %02X %d" % (byte, self.protocolCount))
                if self.protocolCount == 0:
                    if(byte == ord('S')):  # gpCom_ProtocolHeaderS
                        # Reset packet
                        self._ResetPacket()

                        if len(self.droppedBuffer):
                            self.log("!!Dr %d:[%s]!!" % (len(self.droppedBuffer),
                                                         ",".join(["%02X" % x for x in self.droppedBuffer])))
                        self.droppedBuffer = []

                        # print "S seen"
                        self.protocolCount += 1
                    else:
                        if byte != 0:
                            self.droppedBuffer.append(byte)
                            self.log("Dropping @ S 0x%02X - %c" % (byte, byte))
                        # self.droppedBuffer.append(byte)
                        self._ResetProtocol()
                        continue
                elif self.protocolCount == 1:  # gpCom_ProtocolHeaderY
                    if(byte == ord('Y')):
                        # print "Y seen"
                        self.protocolCount += 1
                    else:
                        self.log("Dropping @ Y 0x%02X - %c" % (byte, byte))
                        self._ResetProtocol()
                        continue
                elif self.protocolCount == 2:  # gpCom_ProtocolHeaderN
                    if(byte == ord('N')):
                        # print "N seen"
                        self.protocolCount += 1
                    else:
                        self.log("Dropping @ N 0x%02X - %c" % (byte, byte))
                        self._ResetProtocol()
                        continue
                elif self.protocolCount == 3:  # gpCom_ProtocolHeaderLength
                    self.packetLength = byte
                    self.protocolCount += 1
                elif self.protocolCount == 4:  # gpCom_ProtocolHeaderFrameControl
                    self.packetLength += ((byte & 0x0F) << 8)  # top 4 bits length
                    # self.log("Len: %d" % self.packetLength)
                    self.protocolCount += 1
                elif self.protocolCount == 5:  # gpCom_ProtocolHeaderModule
                    self.packetModuleID = byte
                    # self.log("ID: %02X" % self.packetModuleID)
                    self.protocolCount = 0
                    self.protocolState += 1
                else:
                    self._ResetProtocol()
                    continue
                # Update CRC calculation
                self.crcCalculated = self._updateCRC(self.crcCalculated, byte)
            elif self.protocolState == 1:  # data
                # print "D %02X %d %d" % (byte, self.protocolCount, self.packetLength)
                if self.packetLength:
                    # print "Len: %d %02X" % (self.packetLength, self.packetModuleID)
                    self.packetData.append(byte)
                    self.packetLength -= 1
                if self.packetLength == 0:
                    self.protocolState += 1
                    self.protocolCount = 0
                # Update CRC calculation
                self.crcCalculated = self._updateCRC(self.crcCalculated, byte)
            elif self.protocolState == 2:  # CRC
                # print "C %02X %d" % (byte, self.protocolCount)
                # Skip 2 bytes
                if self.protocolCount == 0:
                    self.protocolCount += 1
                    if byte != (self.crcCalculated & 0xFF):
                        self.log("CRC fail (LSB) %02X <> %04X" % (byte, self.crcCalculated))
                        self.log("%02X [%s]" % (self.packetModuleID, ",".join(["%02X" % x for x in self.packetData])))
                        self.log("   [%s]" % (",".join(["%c" % x for x in self.packetData])))
                        self._ResetProtocol()
                        continue
                elif self.protocolCount == 1:
                    self.protocolCount += 1
                    if byte != ((self.crcCalculated >> 8) & 0xFF):
                        self.log("CRC fail (MSB) %02X <> %04X" % (byte, self.crcCalculated))
                        self.log("%02X [%s]" % (self.packetModuleID, ",".join(["%02X" % x for x in self.packetData])))
                        self._ResetProtocol()
                        continue

                if self.protocolCount == 2:
                    # Finish off
                    # print "%02X [%s]" % (self.packetModuleID, ",".join(map(lambda x: "%02X" % x, self.packetData)))
                    for indCb in self.indicateCallbacks:
                        try:
                            # print "calling indCb ", indCb
                            indCb(self.packetModuleID, self.packetData)
                        except Exception:
                            traceback.print_exc()
                    # Start again
                    self._ResetProtocol()
                    continue

    def _ParseProtocol(self, data):
        if self.protocol:
            if self.protocol(data):
                return

        self._ParseSynProtocol(data)

    def _ResetProtocol(self):
        self.protocolCount = 0
        self.protocolState = 0
        self.crcCalculated = 0

        self._ResetPacket()

    def _updateCRC(self, crc, byte):
        crc = crc16(crc, byte)
        return crc

    def _ResetPacket(self):
        self.packetLength = 0
        self.packetModuleID = 0xFF
        self.packetData = []

########################
# HW Connection types
########################


class usbConnection(object):
    HIDReadStatus = {
        0: "Success",
        1: "WaitTimedOut",
        2: "WaitFail",
        3: "NoDataRead",
        4: "ReadError",
        5: "NotConnected"
    }
    USB_VID = 0x1DB0
    USB_PID = 0x5000

    def __init__(self, path=None):
        # sleep shorter to assure that we poll the USB fast enough to void losing data
        self.thread.sleeptime = 0.00005

        self.name = "USB"
        if sys.platform.find("win") != -1:
            # replacement HID dll ? -https://pypi.python.org/pypi/pywinusb/
            # Get DLL
            sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'dll'))

            import clr
            clr.AddReference("HIDLibrary")
            from HIDLibrary import HidDevices
        elif sys.platform.find("linux") != -1:
            raise RuntimeError("No USB HID possible yet")

        devices = HidDevices.Enumerate(usbConnection.USB_VID, [usbConnection.USB_PID])
        if len(devices) == 0:
            raise RuntimeError("No HID device found")

        if path is None:
            self.device = devices[0]
        else:
            self.device = None
            for dev in devices:
                uniquePart = dev.DevicePath.split('#')[2]
                # Match with wanted path
                if uniquePart.find(path) != -1:
                    self.log("Path %s found: %s" % (path, uniquePart))
                    if self.device is not None:
                        raise RuntimeError("2 devices found, be more unique in path-name\n%s matches:\n- %s\n- %s" %
                                           (path, uniquePart, self.device.DevicePath.split('#')[2]))
                    self.device = dev
            if self.device is None:
                raise RuntimeError("Path '%s' not found in devicelist:\n%s" % (path, "".join(
                    list(map(lambda x: "Dev: %s\n%s\n" % (x.DevicePath.split('#')[2], x), devices)))))
        self.log(self.device)

    def _Read(self):
        deviceData = self.device.Read()

        status = deviceData.Status
        if status == deviceData.ReadStatus.Success:
            data = []
            data.extend(deviceData.Data)
            self._IndicateCallback(data[1:])
        elif status == deviceData.ReadStatus.WaitTimedOut:
            pass
        else:
            self.log("Read fail:%s" % usbConnection.HIDReadStatus[status])

    def _Write(self, data):
        # Split in multiple HID packets
        self.log("[" + ",".join(["%02X" % byte for byte in data]) + "]")
        if len(data) > 64:
            while len(data) > 64:
                hidPacket = data[:64]
                self.device.Write(hidPacket)
                data = data[64:]

        self.device.Write(data)

    def ClearChannel(self):
        pass


class serialConnection(object):
    # def __init__(self, port="COM1", baudrate=57600):
    def __init__(self, port="COM3", baudrate=57600):
        self.comport = port
        self.baudrate = int(baudrate)

        self.name = self.comport
        self.__open()
        self.reinit_lock = threading.Lock()  # Protection for _Read, _Write calls
        self.empty_read_count = 0

    def __open(self, printexc=True):
        try:
            # https://pypi.python.org/pypi/pyserial
            import serial
        except ImportError:
            raise RuntimeError("Install pyserial from https://pypi.python.org/pypi/pyserial for serial comm")

        try:
            self.port = Serial(self.comport, baudrate=self.baudrate, timeout=0.1)
        except Exception:
            # raise RuntimeError("connection fail: %s %s" % (self.comport, str(self.baudrate)))
            import serial.tools.list_ports
            portlist = serial.tools.list_ports.comports()
            ports = []
            for port in portlist:
                ports.append("%s\n-- %s\n-- %s" % (port[0], port[1], port[2]))
            if printexc:
                traceback.print_exc()
            raise RuntimeError("connection fail: %s %s\nAvailable COM ports:\n%s" %
                               (self.comport, str(self.baudrate), "\n".join(ports)))

    def _Read(self):
        try:
            # Timeout set in init
            rx = self.port.read(64)
            if six.PY3:
                rx = rx.decode("latin-1")
            if(len(rx)):
                # self.log(rx)
                # self.log("["+",".join(map(lambda x: "%02X" % ord(x),rx))+"]")
                self._IndicateCallback(list(map(ord, rx)))
                self.empty_read_count = 0
            else:
                self.empty_read_count = self.empty_read_count + 1
                # Do not continuously reconnect.
                # Only reconnect after 50 empty _Read calls (about 5 secs).
                # Continuous reconnects make it also impossible to ctrl-C the jadelogger process
                # as we would be spending most of the time in the try-catch statement below.
                if self.empty_read_count > 50:
                    self.empty_read_count = 0
                    # vima: disabled print, this print is polluting the log-output.
                    # print "reconnecting", self.comport
                    # if there's no traffic, then close and re-open the port
                    # this is to detect virtual COM port disconnections.
                    # on WSL the COM ports are always visible as /dev/ttySx (even if there's no device attached)
                    # => so you need to open the port to check if there is still an active device present.
                    # if not, we keep trying to re-open untill the device is alive again
                    self.reinit_lock.acquire()
                    opened = False
                    while not opened:
                        try:
                            self._Stop()
                            self.__open(printexc=False)
                            opened = True
                        except Exception:
                            self.log("Reconnect failed!")
                            time.sleep(1)
                    self.reinit_lock.release()
        except Exception:
            traceback.print_exc(limit=1)
            self.log("Problem with serial: Reconnecting to serial - 1s retries")

            opened = False
            while not opened:
                try:
                    self.__open(printexc=False)
                    self.log("Reconnect done!")
                    opened = True
                except Exception:
                    time.sleep(1)

    def _Write(self, data):
        packet = ""
        for byte in data:
            packet += "%c" % byte

        # make sure we don't write while another thread is re-initing this connection
        self.reinit_lock.acquire()
        self.port.write(six.b(packet))
        self.reinit_lock.release()

    def _IndicateCallback(data):
        pass

    def _Stop(self):
        if self.port:
            self.port.close()
            self.port = None

    def ClearChannel(self):
        pass

    def __del__(self):
        self._Stop()


class socketConnection(object):
    def __init__(self, path=None, host=None, port=None, type=None):

        self.path = path
        self.host = host
        self.port = port
        self.type = type
        # Keep a local buffer of all data - to avoid TCP nacking and buffer overflows on transmitter side
        self.rx_buffer = []

        self.socket = None
        try:
            self._Connect()
        except Exception:
            traceback.print_exc()
            raise RuntimeError("connection fail %s:%s %s" % (self.host, self.port, self.path))

    def _Connect(self):
        if self.path is not None:
            self.name = "%s" % (self.path)
            self.socket = self._ConnectUnix(self.path)
        elif self.host is not None and self.port is not None:
            self.name = "%s:%s" % (self.host, self.port)
            if self.type is None or self.type.lower() == "tcp":
                self.name += ":tcp"
                self.socket = self._ConnectTcp(self.host, self.port)
            elif self.type.lower() == "tcpserver":
                self.name += ":tcpserver"
                self.socket = self._AcceptTcp(self.host, self.port)
            elif self.type.lower() == "udp":
                self.name += ":udp"
                self.socket = self._ConnectMulticast(self.host, self.port)
            else:
                raise RuntimeError("Unknown type for %s - %s" % (self.name, self.type))
        else:
            raise RuntimeError("Need something, host:port<:udp/tcp> or path")
        self.socket.setblocking(False)

    def ReConnect(self):
        self._Stop()
        self._Connect()

    def _ConnectTcp(self, host, port, connectionTimeout=10):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)  # Disable Nagle
        s.settimeout(connectionTimeout)
        print("host %s port %s" % (host, port))
        s.connect((host, port))
        s.settimeout(None)
        # print "_ConnectTcp finished with ",s
        return s

    def _AcceptTcp(self, host, port, connectionTimeout=2):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind((host, port))
        s.listen(1)
        conn, addr = s.accept()
        print('Connected by', addr)

        conn.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)  # Disable Nagle

        conn.settimeout(None)

        return conn

    def _ConnectUnix(self, path, connectionTimeout=2):
        s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

        s.settimeout(connectionTimeout)
        s.connect(path)
        s.settimeout(None)
        return s

    def _ConnectMulticast(self, host, port):
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        source_addr = socket.gethostbyname(host)
        if sys.platform == "win32" or sys.platform == "cygwin":
            mcast_join = self._mcast_join_windows
        elif 'linux' in sys.platform:
            mcast_join = self._mcast_join_linux
        else:
            raise RuntimeError("unknown platform %s", sys.platform)

        s.bind(('', port))

        # a proper SSM address: 232.x.y.z
        mcast_join(s, '232.92.40.52', source_addr)

        return s

    def _mcast_join_linux(self, s, multicast_addr, source_addr, if_addr='0.0.0.0'):
        # on linux, we can't use connect(device,0),
        # and not even connect(device, 9240).
        # if we try to connect, all traffic gets dropped.
        # It is unclear why this does not work.
        # however source-specific multicast works.

        # this setsockopt cmdid not defined in python.
        if not hasattr(socket, 'IP_ADD_SOURCE_MEMBERSHIP'):
            setattr(socket, 'IP_ADD_SOURCE_MEMBERSHIP', 39)

        # cmd needs a ip_mreq_source structure.
        # imr_multiaddr,imr_interfac, imr_sourceaddr
        imr = (socket.inet_pton(socket.AF_INET, multicast_addr)
               + socket.inet_pton(socket.AF_INET, if_addr)
               + socket.inet_pton(socket.AF_INET, source_addr)
               )

        s.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_SOURCE_MEMBERSHIP, imr)
        # s.connect((source_addr, 9240)) #any src port

    def _mcast_join_windows(self, s, multicast_addr, source_addr, if_addr='0.0.0.0'):
        import struct
        # i can't get source-specific multicast working on windows.
        # it reports: socket.error: (22, 'Invalid argument')
        # this error is WSAEINVAL
        # should be present on winXP, see table at bottom of
        # http://msdn.microsoft.com/en-us/library/ms738586%28VS.85%29.aspx
        #
        # however connect works, as opposed to linux.

        # we don't use source specific
        # but we found out the correct number (ws2ipdef.h)
        if not hasattr(socket, 'IP_ADD_SOURCE_MEMBERSHIP'):
            setattr(socket, 'IP_ADD_SOURCE_MEMBERSHIP', 15)

        mreq = struct.pack("=4s4s", socket.inet_aton(multicast_addr),
                           socket.inet_aton(if_addr))
        s.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

        # this line avoids being flood with the logging of ALL boards
        if source_addr != '0.0.0.0':  # INADDR_ANY
            s.connect((source_addr, 0))  # any source port

    def _Read(self):

        readable, writeable, exceptional = select.select([self.socket], [], [], 0)  # timeout of 0 means just a 'peek'
        for s in readable:
            try:
                # Append received bytes to buffer (use large value to be able to effciently receive any TCP packet size)
                self.rx_buffer += s.recv(65536)
            except Exception:
                # ignore closed socket
                pass

        # Distribute data to framework when there are still bytes in the buffer
        if(len(self.rx_buffer)):
            # Pass data to the test framework in chunks (to avoid long delays before reading again)
            chunk_size = min(2000, len(self.rx_buffer))

            # Get the chunk and update the buffered data
            rx = self.rx_buffer[0:chunk_size]
            self.rx_buffer = self.rx_buffer[chunk_size:]
            if six.PY2:
                # Python2 socket.recv returns string iso of bytes object, we need to tranform it here
                self._IndicateCallback(map(ord, rx))
            else:
                self._IndicateCallback(rx)

    def _Write(self, data):
        packet = ""
        for byte in data:
            packet += "%c" % byte
        self.socket.send(six.b(packet))

    def _IndicateCallback(data):
        pass

    def _Stop(self):
        if self.socket:
            self.socket.shutdown(socket.SHUT_RDWR)
            self.socket.close()
            self.socket = None

    def ClearChannel(self):
        pass

    def __del__(self):
        self._Stop()


class platformConnection(socketConnection):
    '''
    Connection with a Log and Program board (DB-0X/LP01)
    Connects to UART of the STM32 connected to Q chip
    Supports 2 options depending on LnP SW version
    - Polled - a thread reads out the buffered UART on the STM32.
    - Direct - baudrate is configured once, normal TCP connection to 9190 is setup afterwards
    v3.0.38.0 and beyond support direct.
    '''

    def __init__(self, host, port, baudrate, pc_host, pc_port):
        # add required paths
        sys.path.append(os.path.join(envPath, "dig"))
        sys.path.append(os.path.join(envPath, "dig", "common", "sw"))
        sys.path.append(os.path.join(envPath, "dig", "common", "sw", "lnp"))

        self.baudrate = baudrate
        self.host = host
        self.port = port

        import controller
        import pclib
        self.dev = controller.create_ec_controller("%s:%u" % (pc_host, pc_port), False).pc

        # Add connector to dev context - used in regression framework
        self.dev.platformconnection = self

        # Check version to set up either direct or polled connection
        if pclib.system.check_version(self.dev, [3, 0, 37, 0]):
            self.polled = False
            self.__InitDirect()
        else:
            # raise RuntimeError("Need a LnP with SW >= 3.0.38.0 - support for UART streaming to 9190")
            self.polled = True
            self.__InitPolled()

        print("== Connected %s:%u baudrate:%s p:%s ==" % (self.host, port, self.baudrate, self.polled))

    # ####### Direct functionality #########
    def __InitDirect(self):
        self.SetBaudrate()

        # Open socket connection
        socketConnection.__init__(self, host=self.host, port=self.port)

    def SetBaudrate(self):
        import pclib
        pclib.stmio.enable_uart(self.dev, self.baudrate, reinit=False)

    # ####### Polled functionality #########
    def __InitPolled(self):
        self.lock = threading.Lock()  # Protection of Rx/Tx RPC calls and other dev users

        self.paused = 0
        self.initialised = False

        self.name = self.host

    def PausePolled(self):
        print("PlatformConnection paused: %u->%u" % (self.paused, self.paused + 1))
        self.paused += 1
        if self.paused == 1:
            # Only lock once
            self.lock.acquire()

    def ContinuePolled(self):
        if self.paused < 0:
            raise RuntimeError("Pause corrupted ?")

        if self.paused > 0:
            self.paused -= 1
        if self.paused == 0:
            print("PlatformConnection continuing: 1->%u" % self.paused)
            # Trigger re-init of connection after pause - UART settings might be erased
            self.initialised = False
            self.lock.release()
        else:
            print("PlatformConnection still paused: %u->%u" % (self.paused + 1, self.paused))

    def __Init(self):
        if not self.initialised:
            import pclib
            if self.__Lock():
                pclib.stmio.enable_uart(self.dev, self.baudrate, reinit=False)
                self.__Release()
                self.initialised = True

    def __Lock(self):
        import pclib

        # Python lock
        self.lock.acquire()
        locked = False

        # On board lock
        who = pclib.system.who_default()

        starttime = time.time()
        while(time.time() < (starttime + 5)) and not self.thread.StopRequest:
            try:
                pclib.system.acquire_lock(self.dev, who=who)
                locked = True
                break
            except Exception:
                traceback.print_exc()
                print("Waiting to lock again")
                time.sleep(1)

        if not locked:
            # If lock failed - board is probably re-programmed
            print("Deinit")
            self.initialised = False
            self.lock.release()

        return locked

    def __Release(self):
        import pclib
        pclib.system.release_lock(self.dev)
        try:
            self.lock.release()
        except threading.ThreadError:
            pass

    def _ReadPolled(self):
        rx = []

        if self.dev is None or self.thread.StopRequest or (self.paused > 0):
            return

        self.__Init()
        if self.__Lock():
            pending = self.dev.rpc.uart_get_rxbytes()
            # print pending
            if pending:
                rx = self.dev.rpc.uart_rx_bytes(pending)
            self.__Release()

        if(len(rx)):
            # print rx
            # print "["+",".join(map(lambda x: "%02X" % ord(x),rx))+"]"
            self._IndicateCallback(rx)

    def _WritePolled(self, data):
        if self.paused:
            raise RuntimeError("PeakConnector paused - Write not possible")

        self.__Init()
        if self.__Lock():
            self.dev.rpc.uart_tx_bytes(data)
            self.__Release()
        else:
            raise RuntimeError("Write: no lock to %s" % self.host)

    def _IndicateCallback(data):
        pass

    # ####### Common functionality #########
    def _Stop(self):
        if self.dev:
            import pclib
            pclib.stmio.disable_uart(self.dev)
            if self.polled:
                self.__Release()
            self.initialised = False
            # Don't cleanup channel here - peakconnector can still be used by test framework
            # self.dev.channel.Cleanup()
            # self.dev = None

        if not self.polled:
            socketConnection._Stop(self)

    def ClearChannel(self):
        self.dev.channel.Cleanup()
        self.dev = None

    def _Read(self):
        if self.polled:
            self._ReadPolled()
        else:
            socketConnection._Read(self)

    def _Write(self, data):
        if self.polled:
            self._WritePolled(data)
        else:
            socketConnection._Write(self, data)

    def Pause(self):
        if self.polled:
            self.PausePolled()
        else:
            # No actions for Direct
            pass

    def Continue(self, baudrate=None):
        if baudrate is not None:
            self.baudrate = baudrate
        if self.polled:
            self.ContinuePolled()
            pass
        else:
            self.SetBaudrate()

    def __del__(self):
        if self.dev:
            self.dev = None

###########################
# Peak Connector flavors
###########################


class peakConnectorSerial(peakConnector, serialConnection):
    def __init__(self, port="COM3", baudrate=57600):
        peakConnector.__init__(self)
        serialConnection.__init__(self, port, baudrate)

    def _Write(self, data):
        serialConnection._Write(self, data)

    def _Read(self):
        serialConnection._Read(self)

    def _Stop(self):
        serialConnection._Stop(self)

    def _IndicateCallback(self, data):
        peakConnector._ParseProtocol(self, data)


class peakConnectorUSB(peakConnector, usbConnection):
    def __init__(self, path=None):
        peakConnector.__init__(self)
        usbConnection.__init__(self, path)

    def _Write(self, data):
        usbConnection._Write(self, data)

    def _Read(self):
        usbConnection._Read(self)

    def _Stop(self):
        pass

    def _IndicateCallback(self, data):
        peakConnector._ParseProtocol(self, data)


class peakConnectorSocket(peakConnector, socketConnection):
    def __init__(self, path=None, host=None, port=None, type=None):
        peakConnector.__init__(self)
        socketConnection.__init__(self, path, host, port, type)

    def _Write(self, data):
        socketConnection._Write(self, data)

    def _Read(self):
        socketConnection._Read(self)

    def _Stop(self):
        socketConnection._Stop(self)

    def _IndicateCallback(self, data):
        peakConnector._ParseProtocol(self, data)


class peakConnectorPlatformController(peakConnector, platformConnection):
    def __init__(self, host, port, baudrate, pc_host, pc_port):
        peakConnector.__init__(self)
        platformConnection.__init__(self, host, port, baudrate, pc_host, pc_port)
        # dig rpc socket timeout is 12s (see Env\vlatest\dig\common\sw\lnp\channel.py).
        # We want to give it some time to hit its timeout.
        self.thread.jointimeout = 30.0

    def _Write(self, data):
        platformConnection._Write(self, data)

    def _Read(self):
        platformConnection._Read(self)

    def _Stop(self):
        platformConnection._Stop(self)

    def _IndicateCallback(self, data):
        peakConnector._ParseProtocol(self, data)


class peakConnectorSingleWriteDoubleRead(peakConnector):
    def __init__(self, rw_Connector, ro_Connector):
        peakConnector.__init__(self)
        self.__rw_Connector = rw_Connector
        self.__ro_Connector = ro_Connector
        self.__peakConnectorL = [rw_Connector]
        if ro_Connector is not None:
            self.__peakConnectorL.append(ro_Connector)
        self.__indicationQueue = queue.Queue()

        for peak in self.__peakConnectorL:
            peak.indicateCallbacks.append(self.indCb)

        # __rw_Connector.writeCallbacks = [self.writeCb]

    def _Write(self, data):
        # print "!!!! Data:", data
        # raise RuntimeError(str(data))
        # sys.stdout.flush()
        # self.__ro_Connector.initialised = False
        self.__rw_Connector._Write(data)

    def _Read(self):  # called by thread in a loop
        try:
            # Thread-safe
            indication = self.__indicationQueue.get(block=True, timeout=1)
            packetModuleID, packetData = indication
            for indCb in self.indicateCallbacks:
                try:
                    indCb(packetModuleID, packetData)
                except Exception:
                    traceback.print_exc()

            self.__indicationQueue.task_done()
        except queue.Queue_Empty:
            pass

    def _Stop(self):
        for peak in self.__peakConnectorL:
            peak.Stop()

    def _IndicateCallback(self, data):
        raise RuntimeError("not implemented")

    def indCb(self, packetModuleID, packetData):
        indication = (packetModuleID, packetData)
        # Thread-safe
        self.__indicationQueue.put(indication)


###########################
# BLE Connector flavors
###########################
class bleConnectorUSB(bleConnector, usbConnection):
    # Endpoints
    pass


class bleConnectorSerial(bleConnector, serialConnection):
    def __init__(self, port="COM1", baudrate=57600):
        bleConnector.__init__(self)
        serialConnection.__init__(self, port, baudrate)

    def _Write(self, data):
        serialConnection._Write(self, data)

    def _Read(self):
        serialConnection._Read(self)

    def _Stop(self):
        serialConnection._Stop(self)

    def _IndicateCallback(self, data):
        bleConnector._ParseProtocol(self, data)


class bleConnectorSocket(bleConnector, socketConnection):
    def __init__(self, path=None, host=None, port=None, type=None):
        bleConnector.__init__(self)
        socketConnection.__init__(self, path, host, port, type)

    def _Write(self, data):
        socketConnection._Write(self, data)

    def _Read(self):
        socketConnection._Read(self)

    def _Stop(self):
        socketConnection._Stop(self)

    def _IndicateCallback(self, data):
        bleConnector._ParseProtocol(self, data)


class bleConnectorPlatformController(bleConnector, platformConnection):
    def __init__(self, host, port, baudrate, pc_host, pc_port):
        bleConnector.__init__(self)
        platformConnection.__init__(self, host, port, baudrate, pc_host, pc_port)
        # dig rpc socket timeout is 12s (see Env\vlatest\dig\common\sw\lnp\channel.py).
        # We want to give it some time to hit its timeout.
        self.thread.jointimeout = 30.0

    def _Write(self, data):
        platformConnection._Write(self, data)

    def _Read(self):
        platformConnection._Read(self)

    def _Stop(self):
        platformConnection._Stop(self)

    def _IndicateCallback(self, data):
        bleConnector._ParseProtocol(self, data)

###########################
# RAW Connector flavors
###########################


class rawConnectorSerial(rawConnector, serialConnection):
    def __init__(self, port="COM3", baudrate=57600):
        rawConnector.__init__(self)
        serialConnection.__init__(self, port, baudrate)

    def _Write(self, data):
        serialConnection._Write(self, data)

    def _Read(self):
        serialConnection._Read(self)

    def _Stop(self):
        serialConnection._Stop(self)

    def _IndicateCallback(self, data):
        rawConnector._ParseProtocol(self, data)


class rawConnectorPlatformController(rawConnector, platformConnection):
    def __init__(self, host, port, baudrate, pc_host, pc_port):
        rawConnector.__init__(self)
        platformConnection.__init__(self, host, port, baudrate, pc_host, pc_port)
        # dig rpc socket timeout is 12s (see Env\vlatest\dig\common\sw\lnp\channel.py).
        # We want to give it some time to hit its timeout.
        self.thread.jointimeout = 30.0

    def _Write(self, data):
        platformConnection._Write(self, data)

    def _Read(self):
        platformConnection._Read(self)

    def _Stop(self):
        platformConnection._Stop(self)

    def _IndicateCallback(self, data):
        rawConnector._ParseProtocol(self, data)


class rawConnectorSocket(rawConnector, socketConnection):
    def __init__(self, path=None, host=None, port=None, type=None):
        rawConnector.__init__(self)
        socketConnection.__init__(self, path, host, port, type)

    def _Write(self, data):
        socketConnection._Write(self, data)

    def _Read(self):
        socketConnection._Read(self)

    def _Stop(self):
        socketConnection._Stop(self)

    def _IndicateCallback(self, data):
        rawConnector._ParseProtocol(self, data)


PEAK = 0
BLE = 1
RAW = 2


def ConnectAndStart(conninfo, protocol=PEAK, start=True, pc_host=None, pc_port=9241):
    """ Returns a PeakConnector object based on commandline style type selection

    :param conninfo: contains info on connection to set up. Parameters given by : split list
        syntax dependent on different types:
        tcp/udp:
            'host:port'
            'host:port:type'
        serial:
            'COMx:baudrate'
            '/dev/ttySx:baudrate'
        pc:
            'pc:host:baudrate:port'
            'pc:host:baudrate'
            'pc::baudrate'
            'pc'
        usb:
            'usbt:path'
    :type conninfo: str
    :param protocol: Protocol to be parsed on connection - PeakProtocol, BLE or raw stream
    :type protocol: number
    :param start: 'True' if returned connector object needs to be started, default True
    :type start: bool, optional
    :param pc_host: Host name used for platform connection. Host name of the LogAndProgram board,
                    defaults to host name deduced from conninfo
    :type pc_host: str, optional
    :param pc_port: Port number used for platform connection. LogAndProgram SW commands listen to 9241 default.
    :type pc_port: digit, optional

    #FIXME - pc_host/pc_port connection specific parameters should be removed or encapsulated in existing conninfo.
    """
    protocol_names = {
        PEAK: "Peak",
        BLE: "Ble",
        RAW: "Raw",
    }
    connectors = {
        PEAK: {"Serial": peakConnectorSerial,
               "PlatformController": peakConnectorPlatformController,
               "USB": peakConnectorUSB,
               "Socket": peakConnectorSocket,
               },
        BLE: {"Serial": bleConnectorSerial,
              "PlatformController": bleConnectorPlatformController,
              "Socket": bleConnectorSocket,
              },
        RAW: {"Serial": rawConnectorSerial,
              "PlatformController": rawConnectorPlatformController,
              "Socket": rawConnectorSocket,
              },
    }

    if protocol not in connectors:
        raise RuntimeError("Unknown protocol %s - options %s" % (protocol, six.iterkeys(connectors)))

    parameters = conninfo.split(":")
    type_connection = parameters[0]

    print("== Connecting %s - %s ==" % (type_connection, protocol_names[protocol]))

    if any([conninfo.find(substr) != -1 for substr in ['COM', 'tty', 'pts', 'ptmx', '/dev/serial']]):
        port = type_connection
        if len(parameters) > 1:
            baudrate = parameters[1]
        else:
            baudrate = 57600
        if "Serial" not in connectors[protocol]:
            raise RuntimeError("Serial connector not supported in protocol %s" % protocol)
        peakConnector = connectors[protocol]["Serial"](port=port, baudrate=baudrate)
    elif type_connection.upper() == "PC":
        host = os.getenv("PLATFCTRL")
        baudrate = 57600
        if len(parameters) > 1:
            if parameters[1] != '':
                host = parameters[1]
        if len(parameters) > 2:
            baudrate = parameters[-1]
            baudrate = int(baudrate)
        if len(parameters) > 3:
            port = int(parameters[2])
        else:
            port = 9190
        if "PlatformController" not in connectors[protocol]:
            raise RuntimeError("PlatformController connector not supported in protocol %s" % protocol)
        peakConnector = connectors[protocol]["PlatformController"](
            host=host,
            port=port,
            baudrate=baudrate,
            pc_host=host if pc_host is None else pc_host,
            pc_port=pc_port)
    elif type_connection.upper() == 'USB':
        if len(parameters) > 1:
            path = parameters[1]
        else:
            path = None
        if "USB" not in connectors[protocol]:
            raise RuntimeError("USB connector not supported in protocol %s" % protocol)
        peakConnector = connectors[protocol]["USB"](path=path)
    else:
        host = type_connection
        port = 9190
        type = None
        if len(parameters) > 1:
            port = parameters[1]
            port = int(port)
        if len(parameters) > 2:
            type = parameters[2]
        if "Socket" not in connectors[protocol]:
            raise RuntimeError("Socket connector not supported in protocol %s" % protocol)
        peakConnector = connectors[protocol]["Socket"](host=host, port=port, type=type)

    if start:
        rc = peakConnector.Start()
        if not rc:
            raise RuntimeError("Start failed:%s" % peakConnector)

    return peakConnector

# ##### Testing part ######


def packetSeen(moduleId, data):
    print(gpMsg(moduleId, data))


def printPacket(moduleId, data):
    print("".join(["%c" % ch for ch in data]))


def readGPPackets():
    # peak = ConnectAndStart(sys.argv[1])
    # peak = peakConnectorUSB()
    # peak = peakConnectorSerial(port="COM1", baudrate=57600)
    # peak = peakConnectorSocket(host="localhost", port=3231)
    peak = rawConnectorSerial(port="/dev/ttyS5")
    peak.verbose = True

    peak.indicateCallbacks.append(printPacket)
    peak.Start()

    while(1):
        time.sleep(0.5)


if __name__ == "__main__":
    readGPPackets()
