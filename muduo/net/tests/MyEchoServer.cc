//
// Created by root on 17-8-14.
//
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>
#include <muduo/net/EventLoop.h>

#include <boost/bind.hpp>

#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

int numThreads = 0;

class MyEchoServer{
public:
    MyEchoServer(EventLoop * loop, const InetAddress& listenAddr)
            :loop_(loop),
             server_(loop, listenAddr,"MyEchoServer")
    {
        //conn
        server_.setConnectionCallback(
                boost::bind(&MyEchoServer::onConnection,this,_1)
        );
        //msg
        server_.setMessageCallback(
                boost::bind(&MyEchoServer::onMessage,this,_1,_2,_3)
        );
        //thread num
        server_.setThreadNum(numThreads);
    }

    void start()
    {
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        LOG_INFO<<conn->peerAddress().toIpPort()<<"-> "
                <<conn->localAddress().toIpPort()<< " is "
                <<(conn->connected()?"UP":"Down");
        conn->send("hello\n");
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
    {
        string msg(buf->retrieveAllAsString());
        LOG_INFO <<conn->name()<<" recv "<<msg.size()<<" bytes at "<< time.toString();
        if(msg=="exit\n")
        {
            conn->send("bye\n");
            conn->shutdown();
        }
        if(msg=="quit\n")
        {
            loop_->quit();
        }
        conn->send(msg);
    }


    EventLoop* loop_;
    TcpServer server_;
};

int main(int argc, char *argv[])
{
    LOG_INFO<<"pid="<<getpid()<<",tid="<<CurrentThread::tid();
    LOG_INFO<<"sizeof TcpConnection =  "<<sizeof(TcpConnection);
    if(argc> 1)
    {
        numThreads = atoi(argv[1]);
    }
    bool ipv6 = argc > 2;
    EventLoop loop;
    InetAddress listenAddr( 2000, false, ipv6);
    MyEchoServer myserver(&loop, listenAddr);
    myserver.start();
    loop.loop();
}