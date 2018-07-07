#! /usr/bin/python3

import socket
import os

host = "localhost"
port = 16555

socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
socket.connect((host, port))

while(True):
  login = input("Enter Login:")
  token = input("Enter Token:")
  
  glob = login + " " + token
  socket.send(("LOG" + glob).encode("utf-8"))
  data, addr = socket.recvfrom(1024)
  if (data.decode("utf-8") == "YES"):
    break
  print("Login/token invalid or already used")

with open(os.path.expanduser('~/.ssh/my_git_key_file.pub'), 'r') as f:
  stri = f.read()
  while True:
    socket.send(("SSH" + stri).encode("utf-8"))
    data, addr = socket.recvfrom(1024)
    if (data.decode("utf-8") == "YES"):
      break

socket.close()
