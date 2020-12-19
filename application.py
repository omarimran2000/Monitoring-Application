# Using PySerial to implent the monitoring application. Two threads monitor the input from the 
# user and the state of the board.
# Code for serialization is based off PySerial documentation https://pyserial.readthedocs.io/en/latest/shortintro.html
# Code for threading is based off Python threading library https://docs.python.org/3/library/threading.html

import serial 
import threading
import time

def keyboard_input(): 
    """
    Function to take a keyboard input to change the state of the board. 
    """
    while 1:
        inp = input()

        if(inp=="c" or inp=="C"): #if user wants to close the application 
            ser.close()
            break

        elif(inp=="n" or inp=="N" or inp=="p" or inp=="P"):
            ser.write(str.encode(inp))

def board_output():
    """
    Function to see if the board has any output waiting 
    """
    while 1: 
        try:
            time.sleep(.2)                            #add delay
            output = "" 

            bits_waiting = ser.in_waiting

            if bits_waiting>0:                                     #if there are any bits to print 
                for i in range(bits_waiting):                      #print out all the bits 
                    output += ser.read().decode()
                print(output)
        except:                                               #if serial port is closed due to other thread, then that means break the loop here 
            break                                   


#setup serial port using PySerial
ser = serial.Serial()
ser.braudrate = 9600
ser.port = 'COM5'
ser.open()

#start threads 

t1 = threading.Thread(target=keyboard_input)
t2 = threading.Thread(target=board_output)
t1.start()
t2.start()
