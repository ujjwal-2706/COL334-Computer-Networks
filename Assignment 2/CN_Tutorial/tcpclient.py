# -*- coding: utf-8 -*-
"""
Created on Thu Sep  8 14:26:26 2022

@author: shast
"""


from socket import *

serverName = "127.0.0.1"
serverPort = 20001

TCPClientSocket = socket(AF_INET, SOCK_STREAM)

TCPClientSocket.connect((serverName,serverPort))

for i in range(1,6):

    TCPClientSocket.send(str(i).encode())
    server_message = TCPClientSocket.recv(1024)
    print('From Server: ', server_message.decode())

TCPClientSocket.send(str(-1).encode())
TCPClientSocket.close()