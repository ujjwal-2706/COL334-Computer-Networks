# -*- coding: utf-8 -*-
"""
Created on Thu Sep  8 09:35:09 2022

@author: shast
"""


import socket


serverAddressPort   = ("127.0.0.1", 20001)
bufferSize          = 1024

# Create a UDP socket at client side
UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
UDPClientSocket.bind(("127.0.0.1",65530))

for i in range(1,6):
    

    msgFromClient = str(i)
    bytesToSend   = str.encode(msgFromClient)
    
    UDPClientSocket.sendto(bytesToSend, serverAddressPort)
    msgFromServer = UDPClientSocket.recvfrom(bufferSize)
    
    msg = "Message from Server {}".format(msgFromServer[0])
    
    print(msg)