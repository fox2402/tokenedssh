#! /usr/bin/python3

import socket

host = "localhost"
port = 16555

socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
socket.connect((host, port))

while(True):
  login = input("Enter Login:")
  token = input("Enter Token:")
  
  glob = login + " " + token
  socket.send(("LOG" + glob).encode("utf-8"))
  print("sended")
  data, addr = socket.recvfrom(1024)
  if (data.decode("utf-8") == "YES"):
    break
  print("Login/token invalid or already used")

close(socket)
