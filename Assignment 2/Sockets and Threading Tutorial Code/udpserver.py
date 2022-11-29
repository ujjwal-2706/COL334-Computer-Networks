# -*- coding: utf-8 -*-
"""
Created on Thu Sep  8 09:36:52 2022

@author: shast
"""

import socket


localIP     = "127.0.0.1"
localPort   = 20001
bufferSize  = 1024




UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
UDPServerSocket.bind((localIP, localPort))

data_dict = {1:"Packet 1 data",2:"Packet 2 data",3:"Packet 3 data",4:"Packet 4 data",5:"Packet 5 data"}

print("UDP server up and listening")


while(True):

    bytesAddressPair = UDPServerSocket.recvfrom(bufferSize)
    message = int(bytesAddressPair[0])
    address = bytesAddressPair[1]

    clientMsg = "Packet request from Client:{}".format(message)
    clientIP  = "Client IP Address:{}".format(address)
    
    msgFromServer = data_dict[message]
    bytesToSend   = str.encode(msgFromServer)
    
    print(clientMsg)
    print(clientIP)

    UDPServerSocket.sendto(bytesToSend, address)
