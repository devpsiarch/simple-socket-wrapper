#pragma once

#include <string>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>
#include <stdexcept>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <utility>

namespace mysocket {
class socket {
public:
    socket(int file_desc) : fd(file_desc) , active(false){}
    socket() : fd(-1) , active(false){}
    ~socket(){this->close();}

    socket(const socket&other) = delete;
    socket& operator=(const socket&other) = delete;

    socket(socket&&other) noexcept : fd(std::exchange(other.fd,-1)), active(other.active.exchange(false)){}
    socket& operator=(socket&&other) noexcept {
        if(this != &other){
            this->close();
            this->fd = std::exchange(other.fd,-1);
            this->active = other.active.exchange(false);
        }
        return (*this);
    }
                                                                                                                          void close() {                                                                                                            if(fd >= 0){                                                                                                              ::shutdown(fd,SHUT_RDWR);
            ::close(fd);
            this->fd = -1;
        }
        this->active = false;
    }
    bool isActive(void){return this->active;}
    //* socket api implientations *//
    void listen(const char*host,int port,int backlog = 10){
        if(port < 0) throw std::runtime_error("invalid port.\n");
        this->fd = ::socket(AF_INET,SOCK_STREAM,0);
        if(this->fd < 0) throw std::runtime_error("socket() failed.\n");
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(host);
        addr.sin_port = htons(port);

        int opt = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        if (::bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0)
            throw std::runtime_error("bind() failed");
        if (::listen(fd, backlog) < 0)
            throw std::runtime_error("listen() failed");
        active = true;
    }
    void connect(const std::string&host,int port){
        if(port < 0) throw std::runtime_error("invalid port\n");
        this->fd = ::socket(AF_INET,SOCK_STREAM,0);
        if(this->fd < 0) throw std::runtime_error("socket() failed.\n");
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        if(inet_pton(AF_INET,host.c_str(),&addr.sin_addr) <= 0){
            throw std::runtime_error("Invalid address\n");
        }
        if(::connect(this->fd,(sockaddr*)&addr,sizeof(addr)) < 0){
            throw std::runtime_error("connect() failed\n");
        }
        this->active = true;
    }
    socket accept(void){
        sockaddr_in client_addr{};
        socklen_t len = sizeof(client_addr);
        int client_fd = ::accept(fd, (sockaddr*)&client_addr, &len);
        if (client_fd < 0) throw std::runtime_error("accept() failed");
        return socket(client_fd);
    }
    std::string receive(void){
        char temp_buffer[4024];
        ssize_t bytes_read = ::recv(fd,&temp_buffer,4024,0);
        std::string msg(temp_buffer, temp_buffer + bytes_read);
        return msg;
    }
    ssize_t send(const std::string&msg){
        std::lock_guard<std::mutex> lock(locker);
        return ::send(fd, msg.data(), msg.size(), 0);
    }
    void handle(const std::string&msg){
        (void)(msg);
    }
    /*Suplimentary functionality*/
    void print_socket_info(){
        sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        if (getpeername(this->fd, (sockaddr*)&client_addr, &addr_len) == -1) {
            perror("getpeername failed");
            return;
        }

        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
        int port = ntohs(client_addr.sin_port);

        std::cout << "Client IP: " << ip_str << ", Port: " << port << '\n';
    }
private:
    int fd;
    std::atomic<bool> active;
    std::mutex locker;
};
};
