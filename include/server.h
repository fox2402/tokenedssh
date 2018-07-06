#pragma once

#include <vector>
#include <stdexcept>
#include <string>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

class Server
{
  public:
    Server(std::string port, std::string srcstr = "config");
    ~Server();  
    
    void begin_listen();

    
  private:
    void bind_socket();
    
    void server_loop();

    void manage_req(int client);
    
    std::vector<int>  clients_list;
    int               server_socket;
    int               ep_socket;
    
    char*             buffer;
    ssize_t           read_size;
    ssize_t           write_size{0};
    
    std::string       port;
    std::string       config_file;
};
