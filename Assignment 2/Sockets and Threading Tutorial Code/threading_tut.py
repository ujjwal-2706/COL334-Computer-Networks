import sys
import os
import socket
import hashlib
import threading
import time


counter = 0
lock = threading.Lock()

def fetch(threadNumber):
    # global counter
    global lock
    while True:
        with lock:
            # counter+=1
            print(f'thread {threadNumber} updated counter to {counter}')
            # if counter>=100:
                # return
        for i in range(100):
            j=0




threads =[]
startTime = time.time()
noOfThreads = 3

for i in range(noOfThreads):   # for connection 1
    x = threading.Thread(target=fetch, args=(i,))
    threads.append(x)
    x.start()
    
endTime = time.time()

hash = hashlib.md5(open("./A2_small_file.txt", 'r').read().encode()).hexdigest()
print("md5 sum:",hash)
