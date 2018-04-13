# -*- coding:utf-8 -*- 
# author：zhouyang

import socket
import thread
import time

# tcp relay 监听的端口地址
listen_port = 3007
# 要转发的服务器地址
connect_addr = ("localhost", 2007)
# 每发送一个字节就sleep下述时间
sleep_per_byte = 0.001


# 函数,用于转发数据
# 该函数既可以用于将数据从A发送给C,也可以用于将数据从C发送给A
def forward(source, destination):
    source_addr = source.getpeername()
    while True:
        data = source.recv(4096)
        if data:
            for i in data:
                destination.sendall(i)
                time.sleep(sleep_per_byte)
        else:
            print "disconnect", source_addr
            destination.shutdown(socket.SHUT_WR)
            break


serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serversocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
serversocket.bind(('', listen_port))
serversocket.listen(5)

while True:
    (clientsocket, address) = serversocket.accept()
    print 'accepted', address
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # 连接要转发数据包的服务器
    sock.connect(connect_addr)
    print 'connected', sock.getpeername()
    # 将数据从客户端转发给服务器
    thread.start_new_thread(forward, (clientsocket, sock))
    # 将数据从服务器转发给客户端
    thread.start_new_thread(forward, (sock, clientsocket))
