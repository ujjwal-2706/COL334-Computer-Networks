import socket
import threading
import hashlib
buffer_size = 2048
total_clients = 100
DISTRIBUTION = 0
FILE_DATA = {}
COMPLETE = 0
DONE = False
TCP_server_ports = []
for index in range(1,total_clients+1):
    TCP_server_ports.append(index*10 + 1672)
client_TCP_ports = []
for index in range(1,total_clients + 1):
    client_TCP_ports.append(index*10 + 1318)
client_TCP_sockets = []
for index in range(total_clients):
    tcp_socket = socket.socket(family=socket.AF_INET,type= socket.SOCK_STREAM)
    tcp_socket.bind(('127.0.0.1',client_TCP_ports[index]))
    tcp_socket.listen(total_clients)
    client_TCP_sockets.append(tcp_socket)
UDP_client_address = []
for i in range(total_clients):
    UDP_client_address.append(("127.0.0.1",i*10 + 2607))
udp_ports_server = []
for index in range(1,total_clients+1):
    udp_ports_server.append(index*10 + 1315)

udp_socket_list = []
for index in range(total_clients):
    udp_socket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
    udp_socket.bind(UDP_client_address[index])
    udp_socket_list.append(udp_socket)
#this function will break the server message into chunk id and chunk data part
def break_message(reply_server):
    break_point = 0
    while reply_server[break_point] != ' ':
        break_point += 1
    chunk_id = int(reply_server[1:break_point])
    chunk_value = reply_server[break_point+1:]
    return (chunk_id,chunk_value)
#this function call will fetch the initial data chunks over TCP
def fetch_data(client_id):
    TCPClientSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    TCPClientSocket.connect(('127.0.0.1',TCP_server_ports[client_id]))
    chunk_data = []
    id_map = {}
    while True:
        msg = 'y'
        msg = str.encode(msg)
        TCPClientSocket.send(msg)
        reply = TCPClientSocket.recv(buffer_size).decode()
        if reply == '#':
            break
        chunk_id,chunk_value = break_message(reply)
        chunk_data.append(chunk_value)
        id_map[chunk_id] = chunk_value
    print("chunk data is: ",client_id," ",len(''.join(chunk_data)))
    return (chunk_data,id_map)

def cache_update(client_id,local_client_data):
    tcp_socket = client_TCP_sockets[client_id]
    connection,addr_server = tcp_socket.accept()
    chunk_id = int(connection.recv(buffer_size).decode())
    # print("cache update : ",client_id," ",chunk_id)
    if chunk_id in local_client_data:
        value = local_client_data[chunk_id]
        new_string = '$' + str(chunk_id) + ' ' + value
        connection.send(str.encode(new_string))
    else:
        connection.send(str.encode('#'))
    connection.close()

def client_chunk_request(client_id,chunk_id,local_data_client):
    # if chunk_id not in local_data_client:
    udp_socket = udp_socket_list[client_id]
    server_addr = ('127.0.0.1',udp_ports_server[client_id])
    data = str.encode(str(chunk_id))
    udp_socket.sendto(data,server_addr)
    TCPClientSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    TCPClientSocket.connect(('127.0.0.1',TCP_server_ports[client_id]))
    msg_received = TCPClientSocket.recv(buffer_size)
    chunk_id,chunk_value = break_message(msg_received.decode())
    local_data_client[chunk_id] = chunk_value 

def checkComplete(local_data_client):
    chunk_id = 0
    result = []
    while chunk_id in local_data_client:
        result.append(local_data_client[chunk_id])
        chunk_id += 1
    final_string = ''.join(result)
    if hashlib.md5(final_string.encode('utf-8')).hexdigest() == "9f9d1c257fe1733f6095a8336372616e":
        print("Hurray !")
        return True
    else:
        return False
lock = threading.Lock()
def thread_function_udp(thread_number):
    global lock
    global DISTRIBUTION
    global FILE_DATA
    global COMPLETE
    global DONE
    local_data_file = (fetch_data(thread_number))[1]
    chunk_id = 0
    with lock :
        DISTRIBUTION += 1
        FILE_DATA[thread_number] = local_data_file
    # if 1 in FILE_DATA:
    #     print("file: " ,FILE_DATA[1])
    while not checkComplete(local_data_file):
        client_chunk_request(thread_number,chunk_id,local_data_file)
        chunk_id += 1
    with lock:
        COMPLETE += 1
        print(COMPLETE," value")
        if COMPLETE == total_clients:
            DONE = True

def thread_function_tcp(thread_number):
    global lock
    global DISTRIBUTION
    global total_clients
    global FILE_DATA
    global COMPLETE
    global DONE
    while DISTRIBUTION < total_clients:
        val = 0
    # print("Distribution done")
    while not DONE:
        # print("file is woh : ",len(FILE_DATA))
        local_file = FILE_DATA[thread_number - total_clients]
        # print(local_file)
        cache_update(thread_number - total_clients,local_file)
        # print("loop: ",COMPLETE)
    
threads = []
for i in range(2*total_clients):
    if i < total_clients:
        thread = threading.Thread(target=thread_function_udp,args=(i,))
        threads.append(thread)
        thread.start()
    else:
        thread = threading.Thread(target=thread_function_tcp,args=(i,))
        threads.append(thread)
        thread.start()
