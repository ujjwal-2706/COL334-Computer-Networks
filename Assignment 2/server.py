import socket
import threading
total_clients = 5

file = open('A2_small_file.txt','r')
data_file = file.read()
TOTAL_SIZE = len(data_file)
chunk_size = len(data_file) // total_clients
chunks = []
for i in range(total_clients):
    if i < total_clients-1 :
        chunks.append(data_file[i*chunk_size :(i+1)*chunk_size])
    else:
        chunks.append(data_file[i*chunk_size:])
file.close()

buffer_size = 2048
server_IP = "127.0.0.1"
udp_ports_server = []
for index in range(1,total_clients+1):
    udp_ports_server.append(index*10 + 1315)
tcp_ports = []
for index in range(1,total_clients+1):
    tcp_ports.append(index*10 + 1672)
tcp_sockets = []
for i in range(total_clients):
    tcp_socket = socket.socket(family=socket.AF_INET,type= socket.SOCK_STREAM)
    tcp_socket.bind((server_IP,tcp_ports[i]))
    tcp_socket.listen(total_clients)
    tcp_sockets.append(tcp_socket)

#ports where the clients will act as server
client_TCP_ports = []
for index in range(1,total_clients + 1):
    client_TCP_ports.append(index*10 + 1318)
UDP_client_address = []
for i in range(total_clients):
    UDP_client_address.append(("127.0.0.1",i*10 + 2607))

print("Server Up")
#sockets created
udp_socket_list = []
for i in range(total_clients):
    udp_socket = socket.socket(family=socket.AF_INET,type= socket.SOCK_DGRAM)
    port_val = udp_ports_server[i]
    udp_socket.bind((server_IP,port_val))
    udp_socket_list.append(udp_socket)
#map between packet id and string data
LRU_Cache = {}
recent_activity = 0
#function to handle a particular client 
#this will send initial chunks over TCP
#here we send only one time all the information for that chunk in 1024 bytes and then close it
def server_communication(client_id,chunks):
    tcp_socket = tcp_sockets[client_id]
    initial_chunk_id = 0
    for i in range(client_id):
        if len(chunks[i]) % 1000 == 0:
            initial_chunk_id += len(chunks[i])// 1000
        else:
            initial_chunk_id += (len(chunks[i])// 1000) + 1
    connection, addr_client = tcp_socket.accept()
    chunk_value = chunks[client_id]
    max_length = len(chunk_value)
    index = 0
    connection.recv(buffer_size).decode()
    connection.send(str.encode(str(TOTAL_SIZE//1000)))
    while index < len(chunks[client_id]):
        connection.recv(buffer_size).decode()
        packet_data = "$" + str(initial_chunk_id) + " " + chunk_value[index : min(index + 1000,max_length)]
        connection.send(str.encode(packet_data))
        index += 1000
        initial_chunk_id += 1
    connection.recv(buffer_size).decode()
    data = str.encode('#')
    connection.send(data)
    connection.close()

def cache_update(client_id,chunk_id):
    global recent_activity
    #server will ask the client with client_id for chunk_id (but we will require a new socket for that)
    id_packet = str.encode(str(chunk_id))
    udp_socket = udp_socket_list[client_id]
    udp_socket.sendto(id_packet,UDP_client_address[client_id])
    tcp_socket = tcp_sockets[client_id]
    connection,addr = tcp_socket.accept()
    packet_received = connection.recv(buffer_size).decode()
    if packet_received[0] == '$' and (chunk_id not in LRU_Cache):
        if len(LRU_Cache) == total_clients:
            min_key = -1
            for key in LRU_Cache:
                if min_key == -1:
                    min_key = key
                else:
                    if LRU_Cache[min_key][0] > LRU_Cache[key][0]:
                        min_key = key
            del LRU_Cache[min_key]
        LRU_Cache[chunk_id] = [recent_activity,packet_received]
        recent_activity += 1
    elif packet_received[0] == '$' :
        LRU_Cache[chunk_id][0] = recent_activity
        recent_activity += 1
    connection.close()

lock = threading.Lock()
def thread_function(thread_number):
    global lock
    global LRU_Cache
    server_communication(thread_number,chunks)
    client_id = thread_number
    while True:
        udp_socket = udp_socket_list[client_id]
        data_received = udp_socket.recvfrom(buffer_size)
        chunk_id = int((data_received[0]).decode())
        msg = ""
        for i in range(total_clients):
            with lock:
                cache_update(i,chunk_id)
        msg = str.encode(LRU_Cache[chunk_id][1])
        TCPClientSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        TCPClientSocket.connect(('127.0.0.1',client_TCP_ports[client_id]))
        TCPClientSocket.send(msg)

threads = []
for i in range(total_clients):
    thread = threading.Thread(target=thread_function,args=(i,))
    threads.append(thread)
    thread.start()

