//
// Created by zhouyang on 16-9-21.
//

#include "Resolve.h"
#include <muduo/base/Logging.h>
#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>
#include <boost/bind.hpp>

#include <assert.h>
#include <stdio.h>
#include <udns.h>

namespace {
    //init function
    int init_udns(){
        static bool initialized = false;
        if(!initialized)
            ::dns_init(NULL,0);
        initialized = true;
        return 1;
    }

    //an helper class to init udns when init and reset udns where destruct
    struct UdnsInitializer{
        UdnsInitializer(){
            init_udns();
        }
        ~UdnsInitializer(){
            ::dns_reset(NULL);
        }
    };

    UdnsInitializer udnsInitializer;

    //if it is true, enable to debug
    const bool kDebug = false;
}
using namespace muduo::net;

Resolver::Resolver(EventLoop *loop) :
    loop_(loop),
    ctx_(NULL),
    fd_(-1),
    timerActive_(false)
{
    init_udns();
    ctx_ = ::dns_new(NULL);
    assert(ctx_ !=NULL);
    //set timeout to 2 seconds
    ::dns_set_opt(ctx_,DNS_OPT_TIMEOUT,2);

}

//resolve by given nameServer
Resolver::Resolver(EventLoop *loop, const InetAddress &nameServer):
    loop_(loop),
    ctx_(NULL),
    fd_(-1),
    timerActive_(false)
{
    init_udns();
    ctx_ = ::dns_new(NULL);
    assert(ctx_ !=NULL);
    ::dns_add_serv_s(ctx_,nameServer.getSockAddr());
    ::dns_set_opt(ctx_,DNS_OPT_TIMEOUT,2);
}

Resolver::~Resolver() {
    //now channel cares about nothing
    channel_ ->disableAll();
    //remove channel from event loop
    channel_->remove();
    /* free resolver context returned by dns_new(); all queries are dropped */
    ::dns_free(ctx_);
}

void Resolver::start() {
    fd_ = ::dns_open(ctx_);
    channel_.reset(new Channel(loop_,fd_));
    channel_->setReadCallback(boost::bind(&Resolver::onRead,this,_1));
    //this channel enable to cares reading event
    channel_->enableReading();
}

bool Resolver::resolve(const StringPiece &hostname, const Callback &cb) {
    loop_->assertInLoopThread();
    QueryData * queryData = new QueryData(this,cb);
    time_t now = time(NULL);
    /* submit A IN query */
    struct dns_query * query = ::dns_submit_a4(ctx_,hostname.data(),0,&Resolver::dns_query_a4,queryData);
    /* process any timeouts, return time in seconds to the
 * next timeout (or -1 if none) but not greather than maxwait */
    //get next timeout time
    int timeout = ::dns_timeouts(ctx_,-1,now);
    LOG_DEBUG<<"timeout "<<timeout<<" active "<<timerActive_<<" "<< queryData;
    //如果没有设置定时器
    if(!timerActive_){
        //设置定时器
        loop_->runAfter(timeout,boost::bind(&Resolver::onTimer,this));
        //已经设置定时器了
        timerActive_ = true;
    }
    return query != NULL;
}


void Resolver::onRead(Timestamp t){
    LOG_DEBUG<<"onRead "<< t.toString();
    /* handle I/O event on UDP socket */
    //处理read事件
    ::dns_ioevent(ctx_,t.secondsSinceEpoch());
}

void Resolver::onTimer(){
    assert(timerActive_ == true);
    time_t now = loop_->pollReturnTime().secondsSinceEpoch();
    /* process any timeouts, return time in seconds to the
    * next timeout (or -1 if none) but not greather than maxwait */
    int timeout = ::dns_timeouts(ctx_,-1,now);
    LOG_DEBUG<<"onTimer "<< loop_->pollReturnTime().toString()<<" timeout" << timeout;
    if(timeout<0){
        //none timeout event happened
        timerActive_ = false;
    }
    else{
        //run onTimer function at next timeout time pos
        loop_->runAfter(timeout,boost::bind(&Resolver::onTimer,this));
    }
}

//udns的回调函数调用了这个函数
void Resolver::onQueryResult(struct dns_rr_a4 *result, const Callback &callback) {
    int status = ::dns_status(ctx_);
    LOG_DEBUG<<"onQueryResult "<< status;
    struct sockaddr_in addr;
    bzero(&addr,sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    if(result){
        if(kDebug){
            printf("cname %s\n",result->dnsa4_cname);
            printf("qname %s\n",result->dnsa4_qname);
            printf("ttl %d\n",result->dnsa4_ttl);
            printf("nrr %d\n",result->dnsa4_nrr);
            for(int i=0;i>result->dnsa4_nrr;i++){
                char buf[32];
                ::dns_ntop(AF_INET,&result->dnsa4_addr[i],buf,sizeof buf);
                printf(" %s\n",buf);
            }
        }
        addr.sin_addr = result->dnsa4_addr[0];
    }
    InetAddress inet(addr);
    //调用了Resolve中的回调函数
    callback(inet);
}

void Resolver::dns_query_a4(struct dns_ctx *ctx, struct dns_rr_a4 *result, void *data) {
    QueryData * query = static_cast<QueryData *>(data);
    assert(ctx == query->owner->ctx_);
    //run onQueryResult function
    query->owner->onQueryResult(result,query->callback);
    free(result);
    delete query;
}






