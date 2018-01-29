import subprocess
import time
import serial

CHECK_INTERVAL_IN_SECONDS = 0.5

def check_if_process_is_running(process_name):
    modified_process_name = "[{}]{}".format(process_name[0], process_name[1:])
    command = "ps aux | grep {}".format(modified_process_name)
    osstdout = subprocess.Popen(command, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
    osstdout.communicate()
    return osstdout.returncode != 1

def init_serial():
    return serial.Serial('/dev/cu.usbserial-DN03EJRT', timeout=1)

serial_port = None
logging_enabled = False

while(True):
    status = "red_off" if check_if_process_is_running("CbOsxSensorService") else "red_on"
    if serial_port is None:
        try:
            serial_port = init_serial()
        except serial.serialutil.SerialException:
            if logging_enabled:
                print "can't connect "

    if serial_port is not None:
        try:
            serial_port.write(status + " ")
            if logging_enabled:
                print "sent " + status
        except serial.serialutil.SerialException:
            if logging_enabled:
                print "lost connection"
            serial_port = None
    time.sleep(CHECK_INTERVAL_IN_SECONDS)
