//
// Created by root on 17-8-14.
//

#include <muduo/net/EventLoop.h>
#include <muduo/base/Thread.h>
#include <stdio.h>

using namespace muduo;

using namespace muduo::net;



void threadFunc()
{
    printf("threadFunc(): pid = %d, tid = %d\n",getpid(), CurrentThread::tid());
    EventLoop loop;
    loop.loop();
}


int main()
{
    printf("threadFunc(): pid = %d, tid = %d\n",getpid(), CurrentThread::tid());
    EventLoop loop;
    //Thread t(threadFunc);
    Thread t(threadFunc);
    t.start();
    loop.loop();
    t.join();//terminates
    return 0;
}