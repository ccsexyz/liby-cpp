#!/bin/bash
ccflags=$1

build() {
    g++ -g $ccflags -o $1".run" "../examples/"$1".cpp" *.o -std=c++14 -lpthread
}

g++ -g $ccflags -c ../src/*.cpp -std=c++14
g++ -g $ccflags -c ../src/http/*.cpp -std=c++14

build all
build daytime
build echo
build echo_client
build tcp_proxy
build udp_client
build chat
build discard
build tcp_relayer
build udp_hello
build httpd
