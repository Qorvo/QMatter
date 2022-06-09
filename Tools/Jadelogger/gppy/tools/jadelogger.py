#!/usr/bin/env python3
'''
  jadelogger.py <connection> <options>
  - connection:
      tcp/udp:
          'host:port'        - TCP connection to 'host' on 'port'
          'host:port:type'   - choose the type of connection using tcp/udp
      serial:
          'COMx:baudrate'    - serial connection to COM# with baudrate
                               baudrate optional - defaults to 57600
      usb: 'usb:path'       - targetted HID usb device (use COM for usb2serial connections)
                             empty path gives a list of possible HID devices
      pc: 'pc:host:baudrate' - TCP connection to platform controller
                               connected to UART of device
      testbed: 'testbed'     - log all the available COM# which are mbed (QPG6095 Evaluation boards)
  - options:
      'raw' - prints without any SYN protocol
      'ble' - uses the BLE-UART protocol
      'file=<file_prefix>'- route output to a logfile - <file_prefix>_YYMMDD_HHMMSS.txt or log_YYMMDD_HHMMSS.txt when nothing specified
      'reconnect' - keeps trying to connect untill successful. Useful in case a USB connection is only available after some delay.
  Examples:

  To use jadelogger on COM1 - 57600 baudrate:
      'jadelogger.py COM1'
  To use jadelogger on COM1 - 57600 baudrate in combination with BLE:
      'jadelogger.py COM1 ble'
  To use jadelogger on a USB HID device - unique path ab3245ef:
      'jadelogger.py usb:ab3245ef'
  To use jadelogger on a network board:
      'jadelogger.py db05pc100:9190'
  To use jadelogger as logger.py:
      'jadelogger.py db05pc100:9240:udp raw'
  To use jadelogger with platform controller - baudrate 115200:
      'jadelogger.py pc:db09pc62:115200'
  To use jadelogger with platform controller - baudrate 57600 - PLATFCTRL setting:
      'jadelogger.py pc'
  To use jadelogger within the test environment:
      'jadelogger.py testbed'
'''
import sys
import os
import time
import datetime

# do the regular path stuff - drop down to v... to get to testEnv
try:
    sys.path.append(os.path.realpath(os.path.join(os.path.dirname(__file__),"..","..")))
    sys.path.append(os.path.realpath(os.path.join(os.path.dirname(__file__),"..")))
except NameError:
    # We are the main py2exe script, not a module
    pass

def formatParse(logPacket):
    """ Parse gpLog format string.

    Format string and parameters to format
    Example in code:
        GP_LOG_SYSTEM_PRINTF("Some numbers:%u, %lu", 0, 100, 256);
    Incoming string:
        "Some numbers:%u, %lu", /0, |100, 0|, |1, 1, 0, 0|

    :param logPacket: incoming gpLog string
    :type logPacket: str
    """
    formatstring = logPacket[0:logPacket.index(0)]
    formatstring = "".join(map(lambda x: "%c" % x, formatstring))

    # Get parameters
    parameters = logPacket[logPacket.index(0)+1:]
    #print (parameters)
    #print (logPacket)
    params = []
    specifierInd = formatstring.find("%")
    while(len(parameters)):
        if specifierInd != -1 and len(parameters) > 1:
            param = parameters.pop(0)
            param+= parameters.pop(0) << 8

            spec = formatstring[specifierInd:specifierInd+2]
            if spec == '%l' and len(parameters) > 1:
                param+= parameters.pop(0) << 16
                param+= parameters.pop(0) << 24
                spec = formatstring[specifierInd:specifierInd+3]
            #print (spec)

            #Create negative complement number in python for signed prints if needed
            if spec == '%d' or spec == '%i':
                if param & (1 << 15):
                    param-= (1 << 16)
            if spec == '%li':
                if param & (1 << 31):
                    param-= (1 << 32)

            params.append(param)
        specifierInd = formatstring.find("%", specifierInd+1)
        #print (specifierInd)
    formatparams = []
    for i in range(len(params)):
        formatparams.append("params[%i]" % i)
    formatparams = ",".join(formatparams)
    #print (formatparams)
    #print (formatstring)
    try:
        if len(params):
            formatted = eval("'''"+formatstring+"'''") % (eval(formatparams))
        else:
            formatted = eval("'''"+formatstring+"'''")
    except Exception:
        formatted = formatstring + "\n [%s]" % params

    return formatted

def gpLog(data):
    """ Parse a gLog printf indication

    :param data: incoming gpCom level data
    :type data: str
    """

    # Parse Log header
    # 0x02 | module ID | endianness | reserved | count | time0 | time1 | time2 | time3 |
    logPacket =data[:]
    if logPacket.pop(0) != 0x02:
        print ("Not a printf!")
        return [0, 0, 0, 0, ""]

    compId = logPacket.pop(0)
    logPacket.pop(0) # 0x2 - big endian - not used currently
    logPacket.pop(0) # reserved byte
    cnt = logPacket.pop(0)
    time = logPacket[0] +  \
           (logPacket[1] << 8) + \
           (logPacket[2] << 16) + \
           (logPacket[3] << 24)

    # Format formatstring
    logPacket = logPacket[4:]
    formatted = formatParse(logPacket)

    return [compId, cnt, time, formatted]

class jadelogger(object):
    '''
    Class to handle GP logging
    - peakconnector = connection to accept logging from
    - logfile = send logging to file iso stdout
    - .LogStringIndication - add functions to perform further handling
    '''
    def __init__(self, peakconnector, logfile=None):
        self.peakconnector = peakconnector
        #File to log to
        self.logfile = logfile

        #Functions to be called when logging comes in
        self.LogStringIndication = []

        #Output handle - stdout/file
        self.out = None
        print ("== Connecting to %s - logfile: %s ==" % (peakconnector, logfile))

        self.starttime = time.time()

    def HoldLog(self):
        pass

    def Start(self):
        if self.logfile is not None:
            # Open file
            self.out = open(self.logfile, 'w')
        else:
            self.out = sys.stdout
        if not self.peakconnector.IsStarted:
            self.peakconnector.Start()
        self.peakconnector.indicateCallbacks.append(self._LoggingSeen)
        self.peakconnector.writeCallbacks.append(self._TxSeen)

    def Stop(self):
        if self.peakconnector is not None:
            if self._LoggingSeen in self.peakconnector.indicateCallbacks:
                self.peakconnector.indicateCallbacks.remove(self._LoggingSeen)
            if self._TxSeen in self.peakconnector.writeCallbacks:
                self.peakconnector.writeCallbacks.remove(self._TxSeen)

        if self.out is not None and self.logfile is not None:
            # Close file
            self.out.close()
        self.out = None

    def _getTime(self):
        x= datetime.datetime.now()
        t="%02u-%02u-%02u-%02u:%02u:%02u.%03u " % (x.year, x.month, x.day, x.hour, x.minute, x.second, x.microsecond/1000)
        return t

    def _LoggingSeen(self, moduleId, data):
        """ Parse incoming data from connector used with this logger.
        Non-logging data will be logged as raw hex values.
        Registered with connector as indication callback.

        :param moduleId: Id of SW component that is sending the data.
        :type moduleId: digit
        :param data: list of incoming bytes, to be interpreted as logging
        :type data: list
        """
        rxTime = time.time() - self.starttime

        if moduleId == 0xA and len(data) >= 2 and data[0] == 0x02: # gpCom parsing
            #Overflow occurred
            log = "!Overflow:%d mess missed" % data[1]
        elif moduleId == 0xB: # gpLog parsing
            # Logging output
            [compId, cnt, rxTime, log] = gpLog(data)

            rxTime = (rxTime*32.0)/1000000
            moduleId = compId
            log = " " + log
        elif moduleId == 0x0: # Raw connector - no module Id present
            # Raw ASCII output
            # Convert any non-printable characters with repr()
            log = " " + repr("".join(['%c' % ch for ch in data])).strip("'")

            # Control characters made explicit by repr() call
            # Strip again for actual print
            if "\\r" in log:
                log = log.replace("\\r", "", 1)
        else:
            # Raw hex data output
            log = "<[" + ",".join(map(lambda x: "%02X" % x, data)) + "]"

        logging = "%02X %04.6f%s" % (moduleId, rxTime, log)

        output = self._getTime() + " " + logging.strip("\r") +"\n"

        self.out.write(output)
        self.out.flush()

        # Signal any registered log monitors
        for ind in self.LogStringIndication:
            ind(output)

    def _TxSeen(self, moduleId, data):
        """ Log any data sent over connector.
        Registered with connector as write callback.

        :param moduleId: Id of SW component that data is sent to.
        :type moduleId: digit
        :param data: list of outgoing bytes, to be logged as raw hex bytes
        :type data: list
        """
        txTime = time.time() - self.starttime
        logging = "%02X %04.6f>[%s]" % (moduleId, txTime, ",".join(map(lambda x: "%02X" % x, data)))

        output = self._getTime() + " " + logging+"\n"

        self.out.write(output)
        self.out.flush()

        # Signal any registered log monitors
        for ind in self.LogStringIndication:
            ind(output)

    def handleInput(self, inputCommand):
        """ Command line input handler

        :param inputCommand: commandline input for jadelogger
        :type inputCommand: str
        """
        handleInput(inputCommand, self.peakconnector, self)

    def __del__(self):
        self.Stop()
        self.peakconnector = None


###### Application part ######

def convertHexParams(parameter):
    # return the list, if parameter is a list
    if parameter == "[]":
        parameter = []
    if (isinstance(parameter,list)):
        return parameter

    parameter=str(parameter)
    #Convert any hex parameters
    if parameter.isdigit():
        return int(parameter)
    elif (parameter[0] == "-" and (parameter.strip("-")).isdigit()):
        return int(parameter)
    elif(parameter.startswith("-0x") or parameter.startswith("-0X")):
        parameter = parameter[3:]
        reversedParameter = ''
        while (parameter != ''):
            reversedParameter = parameter[0:2] + reversedParameter
            parameter = parameter[2:]
        return int(reversedParameter,16)
    elif(parameter.find("0x") != -1 or parameter.find("0X") != -1):
        return int(parameter,16)
    else:
        raise Exception("Wrong parameter input - only 0x and dec - got:'%s'" % parameter)

    return 0


def handleInput(command, peak, logger):
    commandhelp = '''
============================
 h/help/info - this help
 q/quit/exit - quit logger
 start - start log parsing
 stop  - stop log parsing (can be used to pause stream)
== basic write ==
 w [moduleID] [data] - Write data to a SW module
                        - data space separated, '0xAA' for Hex
                          normal '11' for decimal
 wa [moduleID] [chars] - Write ascii chars to certain moduleID - terminal emulation
 wr [chars]            - Write ascii chars - without SYN encapsulation
 wble [opcode] [data] - Write data to HCI prefixed with an OpCode
== link testing (dedicated program needed on other side) ==
 test [# bytes]      - Writes X bytes to moduleID 0xAA (link testing)
 stress              - Write an incremental X bytes up to 100 to moduleID 0xCC
============================
'''
#Parse input command
    if command[0] == 'info' or command[0] == 'h' or command[0] == 'help':
        print (commandhelp)
    elif command[0] == 'w':
        data = []
        for comm in command[1:]:
            data.append(convertHexParams(comm))
        peak.WriteData(data[0], data[1:])
    elif command[0] == 'wa':
        moduleId = convertHexParams(command[1])
        cmdString = " ".join(command[2:])
        cmdString+= "\r"

        message = map(ord, cmdString)
        peak.WriteData(moduleId, message)
    elif command[0] == 'wr':
        cmdString = " ".join(command[1:])
        cmdString+= "\r"

        message = map(ord, cmdString)
        # Using raw write function - no SYN header/footer
        peak._Write(message)
    #Stress the connection
    elif command[0] == 'test':
        #Send X bytes over the a certain device
        data = range(int(command[1]))
        data = [byte % 0xFF for byte in data]
        peak.WriteData(0xAA, data)
    elif command[0] == 'stress':
        data = [0xAA, 0xBB, 0x0]
        if len(command) > 1:
            stresstime = int(command[1])
        else:
            stresstime = 2.0 #seconds
        now = time.time()
        while(time.time() < (now+stresstime)):
            peak.WriteData(0xCC, data)
            data[2] = (data[2]+1) % 0xFF
            data.append(data[2])
            if len(data) == 100:
                data = data[:3]
    elif command[0] == 'wble':
        data = []
        for comm in command[1:]:
            data.append(convertHexParams(comm))
        #0x9C is gpHci ModuleId
        peak.WriteData(0x9C, data) # opcode(2) len(1) data(var) example (len 0 - no data)  command line: "wble 0x03 0x0c 0"
    elif command[0] == "start":
        logger.Start()
    elif command[0] == "stop":
        logger.Stop()
    elif command[0] == 'q' or command[0] == 'quit' or command[0] == 'exit':
        logger.Stop()
        peak.Stop()
        peak.ClearChannel()
        sys.exit()
    else:
        print ("== Command unknown ==")

import traceback
def mainJadeLogger():
    from testEnv.scripts.gpAPI_2 import keyInput
    import peakconnect

    if len(sys.argv) < 2:
        print ("Need a connection specification")
        print (sys.argv)
        print (__doc__)
        return

#FIXME - start using option parser
    protocol=peakconnect.PEAK
    #print sys.argv[1]
    if len(sys.argv) > 2:
        if "ble" in sys.argv[1:]:
            protocol = peakconnect.BLE
        if "raw" in sys.argv[1:]:
            protocol = peakconnect.RAW

    wait_for_connection_success = False
    for arg in sys.argv:
        if arg.find("reconnect") != -1:
            wait_for_connection_success = True

    if wait_for_connection_success:
        peak = None
        while peak is None:
            try:
                peak = peakconnect.ConnectAndStart(sys.argv[1], protocol=protocol)
            except Exception:
                time.sleep(1)
    else:
        peak = peakconnect.ConnectAndStart(sys.argv[1], protocol=protocol)

    peak.verbose = True

    # Add log-file
    logfileprefix = "log"

    for arg in sys.argv:
        if arg.find("file") != -1:
            file_props = arg.split("=")
            if len(file_props) == 2:
                logfileprefix = file_props[1]

            x= datetime.datetime.now()
            logfile = "%s_%02u%02u%02u_%02u%02u%02u.txt" % (logfileprefix, x.year, x.month, x.day, x.hour, x.minute, x.second)

            loggerfile = jadelogger(peak, logfile)
            loggerfile.Start()
        if arg.find("wakeup") != -1:
            #Activate pre-amble addition
            print ("Adding wakeup pre-amble")
            peak.wakeup = True

    logger = jadelogger(peak, None)
    logger.Start()

    keyb = keyInput(True)

    try:
        while(1):
            command = keyb.processCommand()
            time.sleep(0.1)

            if(len(command) != 0):
                logger.handleInput(command)
    except Exception:
        traceback.print_exc()
        # Stop all loggers and connectors gracefully
        # Can still be interrupted by fast consecutive CTRL+C
        for arg in sys.argv:
            if arg.find("file") != -1:
                loggerfile.Stop()
        logger.Stop()
        peak.Stop()


if __name__ == "__main__":
    mainJadeLogger()
