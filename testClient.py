#!/usr/bin/env python3

import socket
import sys

HOST = 'localhost'  # The server's hostname or IP address
PORT = 8888        # The port used by the server

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    for line in sys.stdin:
        s.sendall(line.encode())
        data = s.recv(1024)
        print('Received', repr(data))

