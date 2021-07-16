from serialManager import SerialCom

callback = lambda s: print(s)

com = SerialCom(callback)

com.start()

while True:
    user_in = input('Send: ')
    com.send_msg(user_in + com.end_char)
