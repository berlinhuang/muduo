//
// Created by zhouyang on 16-9-23.
//

#ifndef SOCKS4A_SOCKS4_SERVER_H
#define SOCKS4A_SOCKS4_SERVER_H
#include "Resolve.h"
#include "tunnel.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <map>

using namespace muduo;
using namespace muduo::net;

class socks4_server {
public:

    socks4_server(EventLoop *loop,InetAddress serverAddr,InetAddress nameServer,string name);

    void start(){
        server_.start();
        resolver_.start();
    }

private:

    void onConnection(const TcpConnectionPtr &conn);


    void onMessage(const TcpConnectionPtr &conn,Buffer *buf,Timestamp);


    void onResolve(const TcpConnectionPtr &conn, Buffer * buf,const char * where, char ver, char cmd, in_port_t port, InetAddress address);

    TcpServer server_;
    Resolver resolver_;
    EventLoop * loop_;
    std::map<string,TunnelPtr> g_tunnels;
};


#endif //SOCKS4A_SOCKS4_SERVER_H
