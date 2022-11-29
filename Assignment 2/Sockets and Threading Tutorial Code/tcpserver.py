# -*- coding: utf-8 -*-
"""
Created on Thu Sep  8 09:36:52 2022

@author: shast
"""

import socket


localIP     = "127.0.0.1"
localPort   = 20001
bufferSize  = 1024




TCPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM)
TCPServerSocket.bind((localIP, localPort))

data_dict = {1:"Packet 1 data",2:"Packet 2 data",3:"Packet 3 data",4:"Packet 4 data",5:"Packet 5 data"}


TCPServerSocket.listen(1)
print("TCP server up and listening")

while(True):
    
    connectionSocket, addr = TCPServerSocket.accept()
    
    while True:
        message = connectionSocket.recv(bufferSize).decode()
        message = int(message)
        
        if message != -1:
            connectionSocket.send(data_dict[message].encode())
        else: 
            print("connection closed with ",addr)
            connectionSocket.close()
            break
        
