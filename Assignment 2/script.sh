#!/bin/sh
python3 server_part1.py & 
sleep 1
python3 client_part1.py & 
wait
