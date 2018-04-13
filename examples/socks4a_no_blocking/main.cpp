//
// Created by zhouyang on 16-9-23.
//

#include "socks4_server.h"
#include <muduo/base/Logging.h>

#include <muduo/net/EventLoop.h>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;


int main(int argc,char **argv){
    if(argc<2){
        fprintf(stderr,"Usage: %s <listen_port>\n",argv[0]);
    }
    else{
        LOG_INFO<<"pid = "<<getpid();
        uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
        InetAddress listenAddr(port);
        InetAddress nameServer("114.114.114.114",0);
        EventLoop loop;
        socks4_server server(&loop,listenAddr,nameServer,"socks4a");
        server.start();
        loop.loop();
    }


    return 0;

}