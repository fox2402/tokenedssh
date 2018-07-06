#pragma once


#include <map>
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
    void parse_file();
    void bind_socket();
    
    void server_loop();

    void manage_req(int client);
    void manage_LOG(int client);
    void manage_SSH(int client);

    std::map<std::string, std::string> login_token_map;
    std::map<int, bool> logged_client;
    std::map<std::string, bool> used_login;
    std::map<int, std::string> client_login;
    int               server_socket;
    int               ep_socket;
    
    char*             buffer;
    ssize_t           read_size;
    ssize_t           write_size{0};
    


    std::string       port;
    std::string       config_file;
};
