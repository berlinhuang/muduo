//
// Created by zhouyang on 16-9-21.
//

#ifndef SOCKS4A_TUNNEL_H
#define SOCKS4A_TUNNEL_H

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/TcpServer.h>

#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
//class tunnel
class Tunnel:public boost::enable_shared_from_this<Tunnel>,boost::noncopyable{
public:
                                                                //forward server address and connection between client and socks server
    Tunnel(muduo::net::EventLoop *loop,const muduo::net::InetAddress &serverAddr,const muduo::net::TcpConnectionPtr &serverConn):
        client_(loop,serverAddr,serverConn->name()),
        serverConn_(serverConn) {

        LOG_INFO<<"Tunnel "<< serverConn->peerAddress().toIpPort()<< "<->"<<serverAddr.toIpPort();
        
    }
    ~Tunnel(){
        LOG_INFO<<"~Tunnel";
    }
    void setup(){
        //set connection callback function
        client_.setConnectionCallback(boost::bind(&Tunnel::onClientConnection,shared_from_this(),_1));
        //set message callback function
        client_.setMessageCallback(boost::bind(&Tunnel::onClientMessage,shared_from_this(),_1,_2,_3));
        //set high water mark callback function
        serverConn_->setHighWaterMarkCallback( boost::bind(&Tunnel::onHighWaterMarkWeak, boost::weak_ptr<Tunnel>(shared_from_this()), _1, _2),
                                               10*1024*1024);

    }
    //close all connection, and set default callback function as callback function
    void teardown(){
        client_.setConnectionCallback(muduo::net::defaultConnectionCallback);
        client_.setMessageCallback(muduo::net::defaultMessageCallback);
        if(serverConn_){
            //reset context to null
            serverConn_->setContext(boost::any());
            //shutdown the connection between client and socks server
            serverConn_->shutdown();
        }
    }
    //connect to forward server
    void connect(){
        client_.connect();
    }
    //disconnect from forward server
    void disconnect(){
        client_.disconnect();
    }

    void onClientConnection(const muduo::net::TcpConnectionPtr &conn){
        LOG_DEBUG<<(conn->connected()?"up":"down");
        if(conn->connected()){
            conn->setTcpNoDelay(true);
            conn->setHighWaterMarkCallback(
                    boost::bind(&Tunnel::onHighWaterMarkWeak,boost::weak_ptr<Tunnel>(shared_from_this()),_1,_2),10*1024*1024
            );
            serverConn_ ->setContext(conn);
            //如果socks server 有信息要转到下一跳服务器，传给服务器
            if(serverConn_->inputBuffer()->readableBytes()>0){
                conn->send(serverConn_->inputBuffer());
            }
        }
            //remote server close the connection
        else{
            teardown();
        }
    }
    void onClientMessage(const muduo::net::TcpConnectionPtr &conn,muduo::net::Buffer * buf,muduo::Timestamp){
        LOG_DEBUG<<conn->name() << " "<< buf->readableBytes();
        //如果连接有效，将下一条服务器返回的消息直接返回给client
        if(serverConn_){
            //向socks server send message
            serverConn_->send(buf);
        }
            //big error! connection between socks server is closed!
        else{
            //retrieve all from buffer
            buf->retrieveAll();
            //abort
            abort();
        }
    }
    //if 传送的数据量达到bytesToSent, close the connection
    void onHighWaterMark(const muduo::net::TcpConnectionPtr &conn,size_t bytesToSent){
        LOG_INFO<<"onHighWaterMark "<<conn->name() << " bytes "<<bytesToSent;
        disconnect();
    }
    //这里使用的是弱引用，避免因为采用　shared_ptr 引用计数不能清零，连接不能及时释放
    static void onHighWaterMarkWeak(const boost::weak_ptr<Tunnel> &wkTunnel,const muduo::net::TcpConnectionPtr &conn,size_t bytesToSent){
        boost::shared_ptr<Tunnel> tunnel = wkTunnel.lock();
        //提升成功，代表连接仍然有效
        if(tunnel){
            tunnel->onHighWaterMark(conn,bytesToSent);
        }
    }
private:
    muduo::net::TcpClient client_;
    muduo::net::TcpConnectionPtr serverConn_;
};
typedef boost::shared_ptr<Tunnel> TunnelPtr;
#endif //SOCKS4A_TUNNEL_H
