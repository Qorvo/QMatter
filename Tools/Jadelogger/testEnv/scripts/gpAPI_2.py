import time

import os
import sys
import threading
import struct
import traceback
import six

from datetime import datetime
try:
    import Queue as queue
except ImportError:
    import queue

# Add path up to Env/vXXX/
sys.path.append(os.path.realpath(os.path.join(os.path.dirname(__file__), "..", "..")))


# --------------------------
# Helper functions
# --------------------------

def ascii2hex(s):
    return ''.join(['%02x' % ord(c) for c in s])  # the %02 pads when needed


def str2ascii(string):
    return [ord(c) for c in string]


def bytesashex(a):
    s = "["
    if isinstance(a, list):
        s += " ".join(list(map(lambda x: "%02X" % (x), a)))
    else:
        s += "%02X" % a
    s = s + "]"
    return s


def numBytesinint(inp):
    ''' Count amount of bytes required to represent integer

    :param inp: integer number
    :type inp: int
    :return amount: Amount of byte required to hold the integer
    :type amount: int
    '''
    # extra 0x00 at beginning not counted
    num_bytes = 1
    if isinstance(inp, list):
        raise TypeError("List not expected")

    while (inp > pow(256, num_bytes)):
        num_bytes += 1

    return num_bytes


def int2Bytelist(inp, numBytes):
    ret = []
    if inp < 0:
        raise ValueError("too small")

    data_int = inp
    remaining = numBytes
    while remaining > 0:
        block_size = min(8, remaining)
        packed = struct.pack("<Q", data_int & 0xFFFFFFFFFFFFFFFF)[:block_size]
        # struct.pack returns str in python and bytes object in python3 !
        # So for python3 we do not need to convert to int !
        if six.PY2:
            ret.extend([ord(x) for x in packed])
        else:
            ret.extend(packed)
        remaining -= block_size
        data_int = data_int >> 64

    return ret


def list2int(lst):
    n = 0
    x = range(0, len(lst))
    if six.PY2:
        x.reverse()
    else:
        x = reversed(x)
    for i in x:
        n = n * 256
        n = n + lst[i]
    return n


def getFromDict(paramValue, dictionary):
    if paramValue in dictionary:
        formatted = "0x%X" % paramValue + " - '" + dictionary[paramValue] + "'"
    else:
        formatted = "0x%X" % paramValue + " - 'Unknown'"
    return formatted


def convertHexParams(parameter):
    if parameter is None:
        return None
    # return the list, if parameter is a list
    if parameter == "[]":
        parameter = []
    if (isinstance(parameter, list)):
        return parameter

    if parameter is False:
        return 0
    if parameter is True:
        return 1

    parameter = str(parameter)
    # Convert any hex parameters

    if len(parameter) == 0:
        return 0
    if parameter.isdigit():
        return int(parameter)
    elif (parameter[0] == "-" and (parameter.strip("-")).isdigit()):
        return int(parameter)
    elif (parameter.startswith("-0x") or parameter.startswith("-0X")):
        parameter = parameter[3:]
        reversed_parameter = ''
        while (parameter != ''):
            reversed_parameter = parameter[0:2] + reversed_parameter
            parameter = parameter[2:]
        return int(reversed_parameter, 16)
    elif (parameter.find("0x") != -1 or parameter.find("0X") != -1):
        return int(parameter, 16)
    elif parameter.startswith('"') and parameter.endswith('"'):
        # String to be converted to ASCII
        return list(map(ord, parameter[1:-1]))
    elif parameter.startswith('[') and parameter.endswith(']'):
        p = parameter[1:-1]
        return [int(x, 16) for x in [p[i:i + 2] for i in range(0, len(p), 2)]]
    else:
        raise ValueError(
            '''Wrong parameter input - only 0x and dec and strings between "" and hexbuf between [] - got:'%s' '''
            % parameter)
        # return 0


def valuePrint(name, value, cmdDef=None):
    formatted = ""
    # API specific formatting
    if name == "AckResult":
        formatted = "0x%02X - '%s'" % (value, gpAPI.AckStatus[value])
    elif name == "cmdId" and cmdDef is not None:
        formatted = "0x%02X - '%s'" % (value, cmdDef.cmdName)
    # Generic formatting
    elif isinstance(value, str):
        formatted = "%s" % value
    elif isinstance(value, list):  # List of integers/other values
        formatted = "l:"
        formvalues = []
        for val in value:
            if str(val).isdigit():
                formvalues.append("%02X" % int(str(val)))
            elif isinstance(val, list):
                for v in val:
                    if str(v).isdigit():
                        formvalues.append("%02X" % int(str(v)))
                    else:
                        formvalues.append("%s" % val)
            else:
                formvalues.append("%s" % val)
        formatted += ",".join(formvalues)
    elif str(value).isdigit():  # Single digit
        formatted = "0x%02X - %d" % (value, value)
    elif isinstance(value, bool):
        formatted = str(value)

    return formatted


# ------------------------
# General API class
# ------------------------


class gpAPI_Generic(object):
    Ack_CmdId = 0xFE
    AckStatus_Succes = 0x00
    AckStatus_UnknownCommand = 0x01
    AckStatus_UnsupportedCommand = 0x02
    AckStatus_WrongParameterLength = 0x03
    AckStatus_ExecutionFailed = 0x04
    AckStatus_Busy = 0x05
    AckStatus = {0x00: "Success",
                 0x01: "UnknownCommand",
                 0x02: "UnsupportedCommand",
                 0x03: "WrongParameterLength",
                 0x04: "ExecutionFailed",
                 0x05: "Busy"
                 }
    AckTimeout = 10  # 10 seconds

    def __init__(self, cmdDictionary=None, formatParseSpecial=None, processCallback=None, parList=None, name="",
                 ignoreWaitFunc=False):
        if cmdDictionary is None:
            cmdDictionary = {"gpAPIInfo": [0x0, name], 0: ["Nothing", []]}
        self.verbose = False
        self.cmdDictionary = cmdDictionary
        self.ignoreWaitFunc = ignoreWaitFunc

        if name == "":
            self.name = self.cmdDictionary["gpAPIInfo"][1]
        else:
            self.name = name
        self.moduleID = self.cmdDictionary["gpAPIInfo"][0]

        # Try to find out if API is acked - if we don't find a reference assume non-acked version
        try:
            self.isAcked = self.cmdDictionary["gpAPIInfo"][2]
        except Exception:
            self.isAcked = False

        self.connParList = parList or []

        self.commChannel = None

        # Extra processing function
        # Callback function to handle return values - only for Acked API's
        self.processCallback = processCallback

        # Extra formatting function
        self.log(formatParseSpecial)
        if formatParseSpecial:
            self.formatParseSpecial = formatParseSpecial
        else:
            self.formatParseSpecial = [None, None]

        # API event messages storage
        self.events = queue.Queue()
        self.acks = queue.Queue()

        # Thread to poll message commChannel
        self.inputThread = ProcessThread(self._processEvent, name=self.name + "thr")

    # Incoming messages

    def _processEvent(self):
        if self.commChannel is None:
            self.inputThread.StopRequest = True
            return

        msg = self.commChannel.getMsg()

        if (msg is not None):
            confirm_indication = None

            try:
                cmd_id = msg[0]
                # Handle Acks
                if ((cmd_id == gpAPI.Ack_CmdId) and self.isAcked):
                    # Expect WaitForAck is pending or will be pending
                    self.acks.put(msg)
                    return None

                cmd_def = gpAPICmdDef(cmd_id, self.cmdDictionary, parRaw=msg[1:])
                # Check indication / confirm - De-serialize parameters
                confirm_indication = gpAPIConfirmIndication(msg, cmd_def, self.formatParseSpecial)
                self.log(confirm_indication)

                if (cmd_id == 0x62):  # MacDispatcher_cbDriverResetIndication_CmdId
                    msg2 = self.commChannel.getMsg()
                    if msg2 is not None:
                        cmd_id2 = msg2[0]

                        if ((cmd_id2 == gpAPI.Ack_CmdId) and self.isAcked):
                            # Expect WaitForAck is still pending after the cbIndication
                            self.acks.put(msg2)
                            self.log("ACK received: %x, %s, %s \n" % (cmd_id2, self.isAcked, msg2))
                            # DO NOT RETURN, process the earlier received indication

                # Do further processing on the event
                self._indicate(confirm_indication)

                self.log(confirm_indication)

                # Store event for later processing
                self.events.put(confirm_indication)

            except Exception:
                self.log("Ignoring exception!")
                traceback.print_exc()

    def _indicate(self, ind):
        '''Give possibility to call a function after sending the acknowledge

        :param ind: Received Indication for this API
        :type ind: gpAPIConfirmIndication class
        '''

        if (ind.cmdId == gpAPI.Ack_CmdId):
            # Ack normally catched when request was waiting for it
            self.log(ind)

        if (self.processCallback is not None):
            self.processCallback(self, ind)

    def getEvents(self, timeout=0, WaitF=None):
        ''' Get a list of events objects collected by API so far

        :param timeout: Time to wait for new incoming events.
                        - 0 - no wait done
                        - None - infinite wait
                        - > 0 - will wait for 'timeout' seconds
        :type timeout: digit
        :param waitF: Function called during the polling for messages.
        :type waitF: Function callback - callback(time_remaining in us)
        '''
        block = (timeout is None) or (timeout > 0)
        try:
            if WaitF is None or self.ignoreWaitFunc is True:
                ev = self.events.get(block=block, timeout=timeout)
            else:
                elapsed_wait_time = 0
                while timeout * 1000000 > elapsed_wait_time and self.events.empty():
                    elapsed_wait_time = elapsed_wait_time + WaitF(timeout * 1000000 - elapsed_wait_time)
                ev = self.events.get(block=False)
            return [ev]
        except queue.Empty:
            return []

    # API requests
    def Ack(self, cmdDef, returnValues, status=AckStatus_Succes):
        if not (self.isAcked):
            return
        # Construct Ack message parameterList
        ack_message = [status, cmdDef.cmdId]
        ack_message.extend(returnValues)

        # Get Ack definition
        ack_def = cmdDef.ackCmdDef
        ack_def.setValues(parList=ack_message)

        # Serialize parameter List according to definition
        message = ack_def.marshallCmd()
        if self.commChannel is not None:
            self.commChannel.sendMsg(message)

    def Request(self, cmdId, parList, noAck=False, waitF=None):
        # Format input
        # Convert Hex to normal int
        par_list = list(map(convertHexParams, parList))

        cmd_def = gpAPICmdDef(cmdId, self.cmdDictionary, parList=par_list, formatting=self.formatParseSpecial[0])

        # Serialize parameter List according to definition
        message = cmd_def.marshallCmd()

        self.log("\n|%s\n|[%s]" % (cmd_def.cmdName, ",".join(list(map(lambda x: "%02X" % x, message)))))
        if self.commChannel is not None:
            self.commChannel.sendMsg(message)

        ack = None
        if not noAck:
            # Wait for ack:
            ack = self.waitForAck(cmd_def, waitF=waitF)

            # Fill in results definition
            # request.cmd_def.setValues(retList = ack.parList[2:])

        return ack

    def waitForAck(self, cmdDef, timeout=None, waitF=None):
        if not (self.isAcked):
            return None

        # Check if API is running - need it for receiving ACK
        if (not self.inputThread.thr.is_alive()):
            raise RuntimeError("Can't receive Ack without running thread")

        # Add timeout
        if timeout is None:
            timeout = gpAPI.AckTimeout

        try:
            if waitF is None:
                msg = self.acks.get(block=True, timeout=timeout)
            else:
                if self.ignoreWaitFunc is True:
                    msg = self.acks.get(block=True)
                else:
                    elapsed_wait_time = 0
                    while timeout * 1000000 > elapsed_wait_time and self.acks.empty():
                        elapsed_wait_time = elapsed_wait_time + waitF(timeout * 1000000 - elapsed_wait_time)
                    msg = self.acks.get(block=True)
        except queue.Empty:
            raise TimeoutError("Timeout %d sec - No ack received for:\n%s" % (timeout, cmdDef))

        # We can only process the correct ACK frame at this time. Did the sender generate an unexpected ack?
        assert cmdDef.cmdId == msg[2], "Received ACK for a wrong command id, expected %s got %s" % (cmdDef.cmdId, msg)

        ack_def = cmdDef.ackCmdDef
        # De-serialize parameters
        # Waiting command overwritten with Ack information
        received_ack = gpAPIConfirmIndication(msg, ack_def, self.formatParseSpecial)

        # Check Ack status
        if received_ack.parList[0] != gpAPI.AckStatus_Succes:
            ack_result = received_ack.cmdDef.parValues["AckResult"]
            ack_string = "Unknown!"
            if ack_result in gpAPI.AckStatus:
                ack_string = gpAPI.AckStatus[ack_result]

            self.log("\n!'%s' Ack failed - result:'%s'" % (cmdDef.cmdName, ack_string))

        return received_ack

    def isRequest(self, cmdId):
        cmd_def = gpAPICmdDef(cmdId, self.cmdDictionary)
        return cmd_def.isRequest or "Unknown"

    def Test(self, waitTime=0.5):
        # Test existence API - force unknown command Ack
        if not (self.isAcked):
            # Non Acked API, not possible to test connection - assume it exists
            return True

        test_cmd = gpAPICmdDef(0xFF, {0xFF: ['Test_Cmd', 0, [], [], True]})
        if self.commChannel is not None:
            self.commChannel.sendMsg([0xFF])

        verbose = self.verbose
        self.verbose = False

        # Wait for ack:
        exists = False
        try:
            ack = self.waitForAck(test_cmd, waitTime)
            if ack.cmdDef.parValues["AckResult"] == gpAPI.AckStatus_UnknownCommand:
                exists = True
        except Exception:
            pass
        finally:
            self.verbose = verbose

        return exists

    def log(self, s):
        if self.verbose:
            print(str(datetime.now()) + " " + self.name + ": " + str(s))
            sys.stdout.flush()


class gpAPI(gpAPI_Generic):
    def __init__(self, cmdDictionary, formatParseSpecial=None, processCallback=None, parList=None,
                 peakConnector=None, name="", ignoreWaitFunc=False):
        gpAPI_Generic.__init__(self, cmdDictionary, formatParseSpecial, processCallback, parList, name,
                               ignoreWaitFunc=False)

        self.peakConnector = peakConnector

        # Establish peakConnector ourselves
        if (peakConnector is None) and (len(self.connParList) > 0):
            import gppy.tools.peakconnect
            self.peakConnector = gppy.tools.peakconnect.ConnectAndStart(parList[0])
            self.ownPeakConnector = True
        else:
            # Providing a peakConnector later on
            self.ownPeakConnector = False

    def ReplacePeakConnector(self, p):
        self.peakConnector = p
        if self.commChannel is not None:
            self.commChannel.Cleanup()
        self.commChannel = peakChannel(self.moduleID, p)

    # Thread functions

    def startInput(self):
        if self.peakConnector is None:
            raise RuntimeError("%s: No peakConnection yet - not starting %s" % (self.name, self.name))

        if self.commChannel is None:
            self.commChannel = peakChannel(self.moduleID, self.peakConnector)
            self.log("Starting Input & commChannel:%s" % self.commChannel)

        # commChannel started - input processing can start
        if self.inputThread.IsDone():
            self.inputThread.Start()

    def stopInput(self):
        self.log("Stopping Input and commChannel")
        self.inputThread.Stop()
        if (self.commChannel):
            self.commChannel.Cleanup()
            self.commChannel = None

        if (self.ownPeakConnector):
            if (self.peakConnector):
                self.peakConnector.Stop()
                self.peakConnector = None

    def __str__(self):
        formatted = ""
        formatted += "API        : 0x%02X - " % (self.moduleID) + self.name + "\n"
        if (self.ownPeakConnector):
            formatted += "Connection : " + str(self.connParList) + "\n"
        else:
            formatted += "Connector  : " + str(self.peakConnector) + "\n"
        formatted += "Formatting : " + str(self.formatParseSpecial) + "\n"
        formatted += "Processing : " + str(self.processCallback) + "\n"

        return formatted

    def __del__(self):
        self.stopInput()


class gpAPI_Linux(gpAPI_Generic):
    def __init__(self, cmdDictionary=None, formatParseSpecial=None, processCallback=None, parList=None, name="",
                 tcpConnector=None):
        # TCP connector
        self.tcpConnector = tcpConnector
        # will normally not be given as init argument, but will be assigned from gpTestWrapper.Connect()
        gpAPI_Generic.__init__(self, cmdDictionary, formatParseSpecial, processCallback, parList, name)
        self.pollRequestSent = 0

    def Request(self, cmdId, parList, waitF=None):
        if self.tcpConnector is None:
            raise RuntimeError("No tcpConnector defined")
        self.tcpConnector.sendmsg(str(cmdId))

    # Incoming messages
    def _processEvent(self):
        msg = self.commChannel.getMsg()
        if (msg is not None):
            if ('No data available' not in msg):
                self.events.put(msg)
                self.pollRequestSent = 0
        else:
            if (self.pollRequestSent == 0):
                self.pollRequestSent = 1
                self._poll()
        return None

    def _pollSocket(self):
        s = self.tcpConnector.sock
        s.setblocking(False)
        import select

        readable, _, _ = select.select([s], [], [], 0.5)
        for s in readable:
            msg = s.recv(1024)
            self.commChannel._HandleReceived(self.moduleID, msg)

    def startInput(self):
        self.log('gpAPI_Linux: startInput')
        if self.commChannel is None:
            self.commChannel = tcpChannel(self.moduleID, self.tcpConnector)
        # thread socket listening for incoming messages
        self.pollingThread = ProcessThread(self._pollSocket, name=self.name + "_listeningThread")

        self.pollingThread.Start()

    def stopInput(self):
        self.log('gpAPI_Linux: stopInput')
        self.pollingThread.Stop()
        if (self.commChannel):
            self.commChannel.Cleanup()
            self.commChannel = None

    def _poll(self):
        self.tcpConnector.sendmsg("poll")

    def Ack(self, cmdDef, returnValues, status=gpAPI_Generic.AckStatus_Succes):
        self.log('gpAPI_Linux: Ack')
        if not (self.isAcked):
            return


class gpAPI_Tunnel(gpAPI):
    '''Provide a tunnel for another API.
       Tunnel API must have a Tx and Rx command formatted as [destination, length, isNull, pData]

       Other API can be added dynamically by AddDestination when now connections are made.
       To fix the destination - for test definition fi - use SetDestination()

       Incoming Rx is translated into the tunneled API.
       Requests with a destination are translated into the set API to tunnel.
       The command payload is encapsulated in the Tunnel API Tx command.
    '''

    def __init__(self, cmdDictionary, formatParseSpecial=None, processCallback=None, parList=None,
                 peakConnector=None, name="",
                 txRxCmdId=None, txCfmCmdId=None):
        '''
        :param txRxCmdId: list of cmd Id's for Tx and Rx of the tunnel API
        :type txRxCmdId: list
        '''
        gpAPI.__init__(self, cmdDictionary, formatParseSpecial, processCallback, parList, peakConnector, name)

        # Dictionary with { destination : [API dictionary, formatting] } pairs
        # For each destination ID a separate API can be selected
        self.destinations = {}

        self.txCmdId = txRxCmdId[0] if txRxCmdId is not None else None
        self.rxCmdId = txRxCmdId[1] if txRxCmdId is not None else None
        self.txCfmCmdId = txCfmCmdId

        # As long as destination is None, API works as a standard API
        self.defaultDestination = None

    def SetDefaultDestination(self, destination=None):
        if destination is not None and destination not in self.destinations:
            raise ValueError("Default destination '%s' is not set in destination possibilities\n - %s" % (
                destination, self.destinations.keys()))

        self.defaultDestination = destination

    def AddDestination(self, destination, tunnelDictionary, tunnelFormatting=None):
        self.destinations[destination] = [tunnelDictionary, tunnelFormatting]

    def RemoveDestination(self, destination):
        del self.destinations[destination]

    def _getTunnelDict(self, destination):
        if destination in self.destinations:
            tunnel_dictionary = self.destinations[destination][0]
            tunnel_formatting = self.destinations[destination][1]
        else:
            tunnel_dictionary = None
            tunnel_formatting = None

        return [tunnel_dictionary, tunnel_formatting]

    def _processEvent(self):
        '''Overrule to parse out tunneled messages'''
        if self.commChannel is None:
            self.inputThread.StopRequest = True
            return

        msg = self.commChannel.getMsg()

        if (msg is not None):
            confirm_indication = None

            try:
                cmd_id = msg[0]

                # Handle Acks - of tunneling API
                if ((cmd_id == gpAPI.Ack_CmdId) and self.isAcked):
                    # Expect WaitForAck is pending or will be pending
                    self.acks.put(msg)
                    return None

                cmd_def = gpAPICmdDef(cmd_id, self.cmdDictionary, parRaw=msg[1:])
                # Check indication / confirm - De-serialize parameters
                confirm_indication = gpAPIConfirmIndication(msg, cmd_def, self.formatParseSpecial)

                # Check on tunneled message
                if cmd_id == self.rxCmdId:
                    try:
                        destination = confirm_indication.parList[0]
                        if destination in self.destinations:
                            self.log("Tunneled from %s" % destination)
                        else:
                            self.log("Tunneled from unknown")
                        # Fixed values in DataIndication function
                        msg = confirm_indication.cmdDef.parRaw[4:]

                        cmd_id = msg[0]
                        # Handle Acks of tunneled API
                        if ((cmd_id == gpAPI.Ack_CmdId) and self.isAcked):
                            # Expect WaitForAck is pending or will be pending
                            self.acks.put(msg)
                            return None

                        [tunnel_dictionary, tunnel_formatting] = self._getTunnelDict(destination)

                        cmd_def = gpAPICmdDef(cmd_id, tunnel_dictionary, parRaw=msg[1:])
                        # Check indication / confirm - De-serialize parameters
                        confirm_indication = gpAPIConfirmIndication(msg, cmd_def, [tunnel_formatting, None])
                    except Exception:
                        self.log("Not a tunneled message")

                elif cmd_id == self.txCfmCmdId:
                    # Drop confirm
                    return None

                # Do processing
                self._indicate(confirm_indication)
                self.log(confirm_indication)

                # Store event for later processing
                self.events.put(confirm_indication)

            except Exception:
                self.log("Ignoring exception!")
                traceback.print_exc()

    def Request(self, cmdId, parList, noAck=False, destination=None, waitF=None):
        '''Overrule to create payload to be transported over tunneled messages'''
        par_list = list(map(convertHexParams, parList))

        if destination is None:
            destination = self.defaultDestination

        # Create payload to be tunneled
        if destination in self.destinations:
            self.log("Tunneling to %s" % destination)
            [tunnel_dictionary, tunnel_formatting] = self._getTunnelDict(destination)

            cmd_def_tunnel = gpAPICmdDef(cmdId, tunnel_dictionary, parList=par_list, formatting=tunnel_formatting)
            payload = cmd_def_tunnel.marshallCmd()
            par_list = [destination, len(payload), 0, payload]
            cmdId = self.txCmdId

        cmd_def = gpAPICmdDef(cmdId, self.cmdDictionary, parList=par_list, formatting=self.formatParseSpecial[0])

        message = cmd_def.marshallCmd()

        self.log("\n|%s\n|[%s]" % (cmd_def.cmdName, ",".join(list(map(lambda x: "%02X" % x, message)))))
        if self.commChannel is not None:
            self.commChannel.sendMsg(message)

        ack = None
        if not noAck:
            # Wait for ack:
            ack = self.waitForAck(cmd_def)

        if destination in self.destinations:
            # Wait for ack through tunnel
            ack = self.waitForAck(cmd_def_tunnel)
            ack.cmdDef.formatting = tunnel_formatting

        return ack


# --------------------------
# Message definition
# --------------------------


class gpAPICmdDef(object):
    def __init__(self, cmdId, cmdDictionary, parList=None, retList=None, formatting=None, parRaw=None):
        self.cmdId = cmdId
        if self.cmdId in cmdDictionary:
            self.cmdName = cmdDictionary[cmdId][0]
            # Add parameter list
            self.parListByteSize = cmdDictionary[cmdId][1]
            self.parList = cmdDictionary[cmdId][2]
            self.numOfParams = len(self.parList)

            # Check for legacy API
            if isinstance(cmdDictionary[cmdId][3], list):
                # Add returnvalues information
                self.retList = cmdDictionary[cmdId][3]
                # Request/Indication
                self.isRequest = cmdDictionary[cmdId][4]
            else:
                # Add returnvalues information
                self.retList = []
                # Request/Indication
                self.isRequest = cmdDictionary[cmdId][3]

            self.returnByteSize = 0
            for return_val in self.retList:
                if isinstance(return_val, int):
                    self.returnByteSize += return_val[1]
                else:
                    self.returnByteSize = 0  # special cases detected
                    break

            # Construct specific Ack definition
            if (self.cmdId != gpAPI.Ack_CmdId):
                self.ackParList = [["AckResult", 1], ["cmdId", 1]]
                for return_val in self.retList:
                    if len(return_val) > 2:
                        # Length calculation function defined
                        size_func = return_val[2]
                        orig_size_param = return_val[1]
                    else:
                        # Try skipping returnvalues if Ack returns failure
                        size_func = self.sizeFuncAck(return_val[1])  # Creates lambda function + AckResult check
                        orig_size_param = 'AckResult'

                    return_val_ = [return_val[0], orig_size_param, size_func]
                    self.ackParList.append(return_val_)

                # Create definition of 1 Ack Cmd with newly created parameterlist
                self.ackDef = ["Ack_%s" % (self.cmdName), 0, self.ackParList, [], not (self.isRequest)]
                self.ackCmdDef = gpAPICmdDef(gpAPI.Ack_CmdId, {gpAPI.Ack_CmdId: self.ackDef})
        else:
            self.cmdName = "Unknown"
            self.parListByteSize = 0
            self.parList = []
            self.numOfParams = 0
            self.isRequest = None
            self.retList = []
            self.returnByteSize = 0
            print("Warning: %s received an unknown command id (0x%02x)" % (cmdDictionary["gpAPIInfo"][1], cmdId))

        self.formatting = formatting
        # Store any given values
        self.setValues(parList, retList)
        # Raw message
        self.parRaw = parRaw or []

    def sizeFuncAck(self, origSize):
        # Give size calculation function for Ack values -
        #    direct construction in loop always takes the value of the last iteration
        return lambda ack: (self.sizeWrongAck(ack, origSize))

    def sizeWrongAck(self, ackResult, origSize):
        # Don't try to parse return values if something went wrong
        return origSize if (ackResult == gpAPI.AckStatus_Succes) else 0

    def setValues(self, parList=None, retList=None):
        self.parValues = {}
        for i in range(len(self.parList)):
            if parList is not None:
                if (len(self.parList) != len(parList)):
                    raise ValueError("%s: Trying to set %d - exp:%d" % (self.cmdName, len(parList), len(self.parList)))
                self.parValues[self.parList[i][0]] = parList[i]
                if type(parList[i]) == float:
                    raise TypeError(
                        "Is float: %d %s. The '/' operation returned type int in python2 but in python3 this returns a float. Use '//' instead." % (i, repr(parList[i])))
            else:
                # Prepare dictionary, no values set
                self.parValues[self.parList[i][0]] = 0

        self.retValues = {}

        for i in range(len(self.retList)):
            if retList is not None:
                if len(self.retList) != len(retList):
                    raise RuntimeError("%s: Trying to set %d- exp:%d" % (self.cmdName, len(retList), len(self.retList)))
                self.retValues[self.retList[i][0]] = retList[i]
            else:
                # Prepare dictionary, no values set
                self.retValues[self.retList[i][0]] = 0

    def paramSize(self, index, returnSize=False):
        # Get size definition of parameterlist
        if returnSize:
            param_def = self.retList[index]
        else:
            param_def = self.parList[index]

        byte_size = param_def[1]

        gen_v2_prefix = "__PARAMS__."

        # Extra size calculation function is given
        if (len(param_def) > 2):
            # Format: ["name", "calculationParam", calculationFunction]
            size_function = param_def[2]
            size_parameter = param_def[1]

            # Size is somewhere in parameters
            if not str(size_parameter).isdigit():
                if gen_v2_prefix == size_parameter[:len(gen_v2_prefix)]:
                    size_parameter = size_parameter[len(gen_v2_prefix):]
                if size_parameter in self.parValues:
                    size_parameter = self.parValues[size_parameter]
                elif size_parameter in self.retValues:
                    size_parameter = self.retValues[size_parameter]

            byte_size = size_function(size_parameter)

        # Size is somewhere in parameters
        if not str(byte_size).isdigit():
            # Format: ["name", "lengthname"]
            # Name of length parameter is indicated by byte_size given

            if gen_v2_prefix == byte_size[:len(gen_v2_prefix)]:
                byte_size = byte_size[len(gen_v2_prefix):]

            if byte_size in self.parValues:
                byte_size = self.parValues[byte_size]
            elif byte_size in self.retValues:
                byte_size = self.retValues[byte_size]
            else:
                print("%s: value '%s' unknown" % (self.cmdName, byte_size))
                # return parameter might need parameter from request parameters to calculate it's length -
                # take all remaining bytes as length
                byte_size = 0xFF

        return byte_size

    def checkParameterList(self, parList):
        # Check parList length
        if len(parList) < self.numOfParams:
            print("!Parameter List too short %02d" % len(parList) + " != %02d" % self.numOfParams)
            return False

        if len(parList) > self.numOfParams:
            print("!Parameter List too long %02d" % len(parList) + " != %02d" % self.numOfParams)
            return False

        return True

    def marshallParList(self):
        ''' Create a serialized version of all parameters

        :return message: list of bytes to send over as serial stream
        :type message: list
        '''
        message = []

        for i, param in enumerate(self.parList):
            marshalled_parameter = []

            param_byte_size = self.paramSize(i)
            param_value = self.parValues[param[0]]
            # Split parameters larger then 1 byte into multiple bytes
            if param_value is None:
                continue
            elif param_value == "skip" or (
                    isinstance(param_value, list) and param_value and param_value[0] in ('store', 'get')):
                marshalled_parameter = param_byte_size * ["skip"]
            else:
                marshalled_parameter = self.sizeSplit(param_value, param_byte_size)

            # Add bytes to message
            message.extend(marshalled_parameter)

        return message

    def getParValueList(self):
        ''' Return values of all parameters set
            as a list

        :return param_values: list of parameter values
        :type param_values: list
        '''
        param_values = []

        for param in self.parList:
            param_value = self.parValues[param[0]]
            param_values.append(param_value)

        return param_values

    def marshallCmd(self):
        ''' Create a serialized version of the command

        :return message: list of bytes to send over as serial stream
        :type message: list
        '''
        # First byte = cmdId
        message = [self.cmdId]
        message.extend(self.marshallParList())

        return message

    def sizeSplit(self, parameter, parameterByteSize):
        ''' Split an input parameter into a list of bytes

        FIXME - function should only accept int or list.
        :param parameter: list or number
        :type parameter: unsigned int or list
        :param parameterByteSize: Known size of the parameter. 0 if size is unknown.
        :type parameterByteSize: int
        '''

        # Split up in byte-size pieces
        split_bytes = []

        # handle list
        if (isinstance(parameter, list)):
            if parameterByteSize != 0:
                if parameterByteSize != len(parameter):
                    raise RuntimeError("%s - %s: List of length:%d Expected:%d" % (
                        self.cmdName, parameter[0], len(parameter), parameterByteSize))
            return parameter

        # Unknown size
        if parameterByteSize == 0:
            # Calculate amount of bytes
            param_check = parameter

            if param_check == 0:
                # Parameter is zero -> size = 0
                parameterByteSize = 1
            else:
                while param_check != 0:  # Expect positive number (no sign extension)
                    param_check = param_check >> 8
                    parameterByteSize += 1

        # Size is known
        while parameterByteSize != 0:
            split_bytes.append(parameter & 0xFF)
            parameter = parameter >> 8
            parameterByteSize = parameterByteSize - 1

        if parameter > 0:
            print("!Parameter clipped %02X" % parameter)

        return split_bytes

    def __str__(self):
        return_format = " -- %s -- 0x%02X -- \n" % (self.cmdName, self.cmdId)
        for par in self.parList:
            formatted = ""
            if self.formatting:
                try:  # For backwards compatibility
                    formatted = self.formatting(par[0], self.parValues[par[0]], self)
                except Exception:
                    traceback.print_exc()
            if formatted == "":
                formatted = valuePrint(par[0], self.parValues[par[0]], self)
            return_format += " > %s - %s\n" % (par[0], formatted)
        for par in self.retList:
            formatted = ""
            if self.formatting:
                try:  # For backwards compatibility
                    formatted = self.formatting(par[0], self.retValues[par[0]], self)
                except Exception:
                    traceback.print_exc()
            if formatted == "":
                formatted = valuePrint(par[0], self.retValues[par[0]], self)
            return_format += " < %s - %s\n" % (par[0], formatted)
        return_format += " -- "

        return return_format


# --------------------------
# Incoming message API class
# --------------------------


class gpAPIConfirmIndication(object):
    def __init__(self, message, cmdDef, formatParseSpecial=None):
        '''
        :param formatParseSpecial: list of 2 processing functions,
                                   applied to each parameter when printing
        :type formatParseSpecial: list
        '''
        self.verbose = False
        self.message = message

        self.cmdId = cmdDef.cmdId
        self.parBytes = message[1:len(message)]
        self.cmdDef = cmdDef
        self.cmdDef.formatting = formatParseSpecial[0] if formatParseSpecial else None
        self.parseSpecial = formatParseSpecial[1] if formatParseSpecial else None

        self.parList = []
        # Perform checks
        if (self.checkParameterList()):
            # Parse message
            if (self.parseSpecial is not None):
                if not self.parseSpecial(self):
                    self.parseParamList()
            else:
                self.parseParamList()
        else:
            # Dump message
            self.dumpMessage()

    def checkParameterList(self):
        if self.cmdDef.parListByteSize == 0:  # No check possible
            return True

        # Check parBytes length
        if len(self.parBytes) < self.cmdDef.parListByteSize:
            print("!Too few bytes %02d" % len(self.parBytes) + " != %02d" % self.cmdDef.parListByteSize)
            return False

        if len(self.parBytes) > self.cmdDef.parListByteSize:
            print("!Too much bytes %02d" % len(self.parBytes) + " != %02d" % self.cmdDef.parListByteSize)
            return False

        return True

    def parseParamList(self):
        # Convert byte-stream into parameters
        self.parList = []
        counter = 0
        base_name = " "
        for ind, param in enumerate(self.cmdDef.parList):
            value = 0

            # Check if null pointer indicator received
            if not param[0].startswith(base_name):
                null_idx = str(param[0]).rfind("_IsNullPtr")
                if null_idx > 0:
                    base_name = param[0][:null_idx]
                else:
                    base_name = " "
            if base_name + "_IsNullPtr" in self.cmdDef.parValues.keys():
                is_null_pointer_ind = bool(self.cmdDef.parValues[base_name + "_IsNullPtr"])
            else:
                is_null_pointer_ind = False

            byte_size = self.cmdDef.paramSize(ind)

            # print ("-%s %i" % (param[0], byte_size))
            # Unknown size - parameter @ end of payload -> size = rest of payload
            if (byte_size == 0xFF):
                byte_size = len(self.parBytes) - counter

            # Start building up parameter value from received bytes
            if not is_null_pointer_ind:
                for i in range(byte_size):
                    if counter > (len(self.parBytes) - 1):
                        print("!Clipped '%s' %d - %d" % (param[0], counter - 1, len(self.parBytes)))
                        print("message was '[%s]' " % (" ".join(map(lambda x: "%02x" % x, self.message))))
                        break

                    if self.parBytes[counter] != "skip":
                        value = value + (self.parBytes[counter] << (8 * i))
                    counter += 1

            # Skip parsing data if null pointer indicator received for this data
            if not is_null_pointer_ind:
                # Store found value
                if (param[1] == 0):
                    # Store bytesize as well if length was undefined
                    self.parList.append([value, byte_size])
                    self.cmdDef.parValues[param[0]] = [value, byte_size]
                else:
                    self.parList.append(value)
                    self.cmdDef.parValues[param[0]] = value
            else:
                self.parList.append(None)
                self.cmdDef.parValues[param[0]] = None

    def __str__(self):
        # Copy parsed parameters to cmdDefinition
        self.cmdDef.setValues(parList=self.parList)

        return str(self.cmdDef)

    def dumpMessage(self):
        print("\n- " + self.cmdDef.cmdName)

        for byte in self.parBytes:
            print("-* %02X" % byte)
        print("Expected :")
        print(self.cmdDef)


# ------------------------
# Connection wrapper class
# ------------------------

class commChannel(object):
    def __init__(self, moduleID, connector):
        self.verbose = False
        self.moduleID = moduleID
        self.connector = connector
        self.rx = queue.Queue()

    def Cleanup(self):
        pass

    def getMsg(self):
        if not self.rx.empty():
            msg = self.rx.get(False)
        else:
            msg = None
            return msg
        return msg

    def sendMsg(self, payload):
        raise NotImplementedError("Minimal implementation expected")

    def log(self, s):
        if self.verbose:
            print("cc:%02X: %s" % (self.moduleID, str(s)))

    def __str__(self):
        s = ""
        s += "CommChannel: Id:%d\n" % self.moduleID
        return s


class peakChannel(commChannel):
    def __init__(self, moduleID, peakConnector):
        commChannel.__init__(self, moduleID, peakConnector)

        # PeakConnector we can use for this commChannel
        if peakConnector is None:
            raise RuntimeError("No peakConnector given!")
        self.peakConnector = peakConnector
        self.peakConnector.indicateCallbacks.append(self._HandleReceived)

        # Start it if needed
        if not self.peakConnector.IsStarted:
            self.peakConnector.Start()
            assert (self.peakConnector.IsStarted)

    def Cleanup(self):
        # Remove our callback from the peakConnector
        self.peakConnector.indicateCallbacks.remove(self._HandleReceived)

    def _HandleReceived(self, moduleid, payload):  # PeakConnector cb
        if (moduleid != self.moduleID):
            self.log("ignoring data for other module %02X" % moduleid)
            return

        # Add message to rx list
        msg = []
        msg.extend(payload)

        # Add to rx list
        if not self.rx.empty():
            self.log("!Message buffered - Queue not empty when adding %s\n" % msg)

        self.rx.put(msg)

        self.log("Rx: %s" % (bytesashex(msg)))

    def sendMsg(self, payload):  # gpAPI
        self.log("Tx: %s" % (bytesashex(payload)))
        self.peakConnector.WriteData(self.moduleID, payload)


class tcpChannel(commChannel):
    def __init__(self, moduleID, tcpConnector):
        commChannel.__init__(self, moduleID, tcpConnector)

        # TcpConnector we can use for this commChannel
        if tcpConnector is None:
            raise Exception("No tcpConnector given!")
        self.tcpConnector = tcpConnector

    def _HandleReceived(self, moduleid, msg):
        # If incoming messages on socket, _HandleReceived is called. Will add msg to the rx list
        if (moduleid != self.moduleID):
            self.log("ignoring data for other module %02X" % moduleid)
            return

        # Add to rx list
        self.lock.acquire()
        self.rx.append(msg)
        if len(self.rx) > 1:
            self.log("!Message buffered - Num buffered:%d" % len(self.rx))
        self.lock.release()

        self.log("Rx: %s" % (msg))
        return

    def sendMsg(self, payload):
        self.log("Tx: %s" % (bytesashex(payload)))
        self.tcpConnector.sendmsg(payload)


# ------------------------
# Key input handling class
# ------------------------


if (sys.platform.find("win") != -1 and sys.platform.find("cygwin") == -1):
    import msvcrt


class keyInput(object):
    ''' Simple Key Input handler wrapper.

        To be used in a separate thread to process key events.
        FIXME - CI-611 - to be replaced by more feature-rich python lib
    '''

    def __init__(self, threadUsed):
        self.threadUsed = threadUsed
        self.input = ""
        self.command = []

    def processKeyEvent(self):
        if self.threadUsed:
            # Blocking code
            self.input = sys.stdin.readline()
        else:
            if (sys.platform.find("win") != -1):
                # - Under Development - Unused when using 2 threads
                # Non-blocking code
                if msvcrt.kbhit():
                    key = msvcrt.getche()

                    # Handle special keys
                    if (self.specialKey(key) is False):
                        self.input += key
            else:
                raise RuntimeError("Not supported for platform %s" % sys.platform)

    def specialKey(self, key):
        print(ord(key))

        if ord(key) < 33:
            # Special keys
            if ord(key) == 8:
                # Backspace
                print("BackSpace")
                return True

            if ord(key) == 13:
                # Carriage Return
                return False

            if ord(key) == 32:
                # Space
                return False

            return True

        elif ord(key) == 72:
            print("KeyUp")

            # Key up
            self.keyUpCnt = 0
            # self.input = self.lastInputs[len(self.lastInput) - self.keyUpCnt]
            return True

        elif ord(key) == 80:
            print("KeyDown")

        return False

    def processCommand(self):
        # Process key
        self.processKeyEvent()

        # Check on newline
        if (self.input.find("\r") != -1) or (self.threadUsed):
            self.command = self.input.split()

            self.clearInput()

        # Return command
        command = self.command
        self.clearCommand()
        return command

    def clearInput(self):
        self.input = ""

    def clearCommand(self):
        self.command = []


# ---------------
# Threading class
# ---------------

class ProcessThread(object):
    ''' code running in a python thread '''

    def __init__(self, function, name=""):
        ''' Initializer for ProcessThread class

        :param function: Function to execute in this thread.
                         Function expected to have no required parameters.
                         func()
        :type function: function
        :param name: Name of the Thread
        :type name: str
        '''
        self.verbose = False
        self.jointimeout = 5.0
        self.sleeptime = 0.005

        self.function = function
        self.name = name
        self.thr = None

        self.StopIndication = True
        self.StopRequest = True
        self.exception = None

    def Start(self):
        if self.thr is None:
            self.daemon = False
            self.StopIndication = False  # TestDevice -> #Test: test has stopped
            self.StopRequest = False  # Test->TestDevice
            self.thr = threading.Thread(target=self._Execute)
            # by setting this thread as a daemon,
            # a keyboard interrupt (that only kills the main thread)
            # will kill the complete application
            self.thr.setDaemon(True)
            self.exception = None

            self.thr.start()
            self.log("Thread started")
        else:
            self.log("Thread already running")

    def Execute(self):
        if (self.function is not None):
            while (self.StopRequest is False):
                time.sleep(self.sleeptime)  # Don't overload the processor by adding some delay
                self.function()

    def _Execute(self):
        # apparently, an exception will not stop the test.
        # we need to catch it and pass to the main thread.

        try:
            self.Execute()
            self.log("ProcessThread exited normally")
        except Exception as e:
            if traceback is not None:
                traceback.print_exc()
            self.exception = e
            self.log("ProcessThread exited with exception")

        self.StopIndication = True

    def Stop(self):
        self.log("Stopping")
        if (self.thr is not None) and (self.thr.is_alive()):
            self.StopRequest = True
            self.thr.join(self.jointimeout)
            if self.thr.is_alive():
                print("%s:could not stop ProcessThread retrying!" % (self.name))
                self.thr.join(5 * self.jointimeout)
                if self.thr.is_alive():
                    self.exception = Exception("%s:could not stop ProcessThread!" % (self.name))
                    print("%s:could not stop ProcessThread!" % (self.name))
        else:
            # in case a process was killed or crashed
            self.StopIndication = True
        self.thr = None

    def IsDone(self):
        if self.exception is not None:
            raise RuntimeError("%s:ProcessThread has thrown an exception: %s" % (self.name, self.exception))
        return self.StopIndication

    def log(self, s):
        if self.verbose:
            print(self.name + ": " + str(s))
