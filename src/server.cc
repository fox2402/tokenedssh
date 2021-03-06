#include "server.h"

#include <iostream>
#include <system_error>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

#include <netinet/ip.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <wordexp.h>

namespace
{
  const int max_events = 50;
  void init_hints(struct addrinfo* hints)
  {
      memset(hints, 0, sizeof(struct addrinfo));
      hints->ai_family = AF_UNSPEC;
      hints->ai_socktype = SOCK_STREAM;
      hints->ai_flags = AI_PASSIVE;
      hints->ai_protocol = 0;
      hints->ai_canonname = NULL;
      hints->ai_addr = NULL;
      hints->ai_next = NULL;
  }

  void setnonblocking(int fd)
  {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
      throw std::system_error(errno, std::system_category());
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
      throw std::system_error(errno, std::system_category());
  }

}

Server::Server(std::string srcport, std::string srcstr)
{
  port = srcport;
  config_file = srcstr;
  buffer = new char[1024];
  parse_file();
  ep_socket = epoll_create1(0);
  bind_socket();

  if (ep_socket == -1)
    throw std::system_error(errno, std::system_category());

}

Server::~Server()
{
  delete[] buffer;
  close(server_socket);
  close(ep_socket);
}

void Server::parse_file()
{
  wordexp_t word;
  wordexp(config_file.c_str(), &word, 0);
  std::ifstream infile;
  infile.open(word.we_wordv[0]);
  wordfree(&word);
  std::string line;
  while (std::getline(infile, line))
  {
    std::stringstream sstr;
    sstr.str(line);
    std::string word1;
    std::string word2;
    std::getline(sstr, word1, ' ');
    std::getline(sstr, word2, ' ');
    login_token_map[word1] = word2;
    used_login[word1] = false;
  }
}


void Server::bind_socket()
{
  struct addrinfo hints;
  struct addrinfo* result;
  struct addrinfo* rp;
  int sfd;
  init_hints(&hints);

  int s = getaddrinfo(NULL, port.c_str(), &hints, &result);
  if (s != 0)
  {
    throw std::system_error(errno, std::system_category(), "cannot retrieve\
addr");
  }
  for (rp = result; rp != NULL; rp = rp->ai_next)
  {
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd == -1)
      continue;
    if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
      break;
    close(sfd);
  }
  freeaddrinfo(result);
  if (rp == NULL)
  {
    throw std::system_error(0, std::system_category(), "Failed to find an\
addr");
  }
  server_socket = sfd;

  std::cout << "Socket is up and binded" << std::endl;
}

void Server::server_loop()
{
  struct epoll_event events[max_events];
  struct epoll_event ev;
  for(;;)
  {
    int nfds = epoll_wait(ep_socket, events, max_events, -1);
    if (nfds == -1)
      throw std::system_error(errno, std::system_category());
    for (int i = 0; i < nfds; i++)
    {
      if (events[i].data.fd == server_socket)
      {
        int client = accept(server_socket, NULL, NULL);
        std::cout << "Accepted one client" << std::endl;
        if (client == -1)
          throw std::system_error(errno, std::system_category());
        setnonblocking(client);
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = client;
        if (epoll_ctl(ep_socket, EPOLL_CTL_ADD, client, &ev) == -1)
          throw std::system_error(errno, std::system_category(),
            "can't add to epoll");
        logged_client[client] = false;
      }
      else
      {
          std::cout << "Reading one client" << std::endl;
          read_size = read(events[i].data.fd, buffer, 1024);
          std::cout << "Managing one client" << std::endl;
          if (read_size == 0)
          {
            std::cout << "Lost one client" << std::endl;
            if (epoll_ctl(ep_socket, EPOLL_CTL_DEL, events[i].data.fd, NULL) == -1)
              throw std::system_error(errno, std::system_category(),
              "can't del from epoll");
      
            logged_client.erase(logged_client.find(events[i].data.fd));
            client_login.erase(client_login.find(events[i].data.fd));
          }
          manage_req(events[i].data.fd);
          memset(buffer, 0, 1024);
      }
    }
  }
}

void Server::begin_listen()
{
  int i;
  if ((i = listen(server_socket, 50)) == -1)
    throw std::system_error(errno, std::system_category(), "cannot listen");
  std::cout << "Listening..." << std::endl;
  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = server_socket;
  if (epoll_ctl(ep_socket, EPOLL_CTL_ADD, server_socket, &ev) == -1)
    throw std::system_error(errno, std::system_category(), "can't add to epoll");
  server_loop();
}

void Server::manage_req(int client)
{
  std::string s(buffer, 3);
  if (!s.compare("LOG"))
  {
    manage_LOG(client);
    return;
  }
  if (!s.compare("SSH"))
  {
    manage_SSH(client);
    return;
  }
}

void Server::manage_LOG(int client)
{
  if (read_size > 3)
  {
    std::string str(buffer + 3, read_size - 3);
    std::cout << str << std::endl;
    std::stringstream sstr;
    sstr.str(str);
    std::string word1;
    std::string word2;
    std::getline(sstr, word1, ' ');
    std::getline(sstr, word2, ' ');
    try 
    {
      if (login_token_map[word1] == word2 && !used_login[word1])
      {
        std::cout << "YES" << std::endl;
        write(client, "YES", 3);
        logged_client[client] = true;
        client_login[client] = word1;
      }
      else
      {
        std::cout << "Not found" << std::endl;
        write(client, "NOP", 3);
      }
    }
    catch (...)
    {
      std::cout << "Catched" << std::endl;
      write(client, "NOP", 3);
    }
  }
}

void Server::manage_SSH(int client)
{
  std::cout << "read: " << read_size << std::endl;
  std::cout << "buffer: " << buffer << std::endl;
  std::string str(buffer + 3, read_size - 3);
  std::ofstream out;
  try
  {
    wordexp_t word;
    wordexp("~/.ssh/authorized_keys", &word, 0);
    out.open(word.we_wordv[0], std::ios::out | std::ios::app);
    wordfree(&word);
    out.write(str.c_str(), read_size - 3);
    out.flush();
    out.close();
  }
  catch (const std::exception& e)
  {
    std::cout << "error: " << e.what() << std::endl; 
    write(client, "NOP", 3);
  }
  write(client, "YES", 3);
  used_login[client_login[client]] = true;
}


void Server::print_config()
{
  for (const auto& it : login_token_map)
  {
    std::cout << it.first << " " << it.second << std::endl;
  }
}
