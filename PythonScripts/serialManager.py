from threading import Thread
from time import sleep
from serial import Serial


class SerialCom(Thread):
    def __init__(self, callback, end_char='$', serial_port='/dev/ttyS0', baud_rate=9600):
        self.terminate = False
        self.callback = callback
        self.end_char = end_char
        self.in_buffer = ''
        self.serial_port = serial_port
        self.baud_rate = baud_rate
        self.ser = Serial(self.serial_port, self.baud_rate)
        Thread.__init__(self, daemon=True)

    def run(self):
        while True:
            in_byte = self.ser.read().decode('utf-8')
            if in_byte == self.end_char:
                self.callback(self.in_buffer)
                self.in_buffer = ''
            else:
                self.in_buffer += in_byte


    def send_msg(self, msg: str):
        if not msg.endswith(self.end_char):
            msg += self.end_char
        self.ser.write(bytes(msg, 'utf-8'))
