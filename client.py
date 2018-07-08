#! /usr/bin/python3

import socket
import os
import subprocess

f = input("Where do I create the ssh key?")
bashcommand = "ssh-keygen -t rsa -b 4096 -f " + os.path.expanduser(f)

print(bashcommand)

process = subprocess.Popen(bashcommand.split())
process.wait()

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

with open(os.path.expanduser(f + '.pub'), 'r') as f:
  stri = f.read()
  while True:
    socket.send(("SSH" + stri).encode("utf-8"))
    data, addr = socket.recvfrom(1024)
    if (data.decode("utf-8") == "YES"):
      break

socket.close()
