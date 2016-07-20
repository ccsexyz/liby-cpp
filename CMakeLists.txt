cmake_minimum_required(VERSION 3.5)
project(liby_cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        src/Socket.cpp
        src/Socket.h
        src/Resolver.cpp
        src/Resolver.h
        src/FileDescriptor.cpp
        src/FileDescriptor.h
        src/util.cpp
        src/util.h
        src/TimerMixin.cpp
        src/TimerMixin.h
        src/Channel.cpp
        src/Channel.h
        src/Poller.cpp
        src/Poller.h
        src/PollerEpoll.cpp
        src/PollerEpoll.h
        src/EventLoop.cpp
        src/EventLoop.h
        src/Logger.cpp
        src/Logger.h
        src/BlockingQueue.h
        src/Liby.h
        src/EventQueue.cpp
        src/EventQueue.h
        src/TimerQueue.cpp
        src/TimerQueue.h
        src/BinaryHeap.h
        src/Buffer.h
        src/Buffer.cpp
        src/Endpoint.cpp
        src/Endpoint.h
        src/EventLoopGroup.cpp
        src/EventLoopGroup.h
        src/Connection.h
        src/Connection.cpp
        src/TcpClient.cpp
        src/TcpClient.h
        src/TcpServer.cpp
        src/TcpServer.h
        src/UdpSocket.cpp
        src/UdpSocket.h
        src/UdpConnection.cpp
        src/UdpConnection.h
        src/PollerPoll.cpp
        src/PollerPoll.h
        src/PollerSelect.cpp
        src/PollerSelect.h
        src/PollerKevent.cpp
        src/PollerKevent.h
        src/http/HttpServer.cpp
        src/http/HttpServer.h
        src/http/HttpClient.cpp
        src/http/HttpClient.h
        src/http/HttpDef.cpp
        src/http/HttpDef.h)

set(ECHO_FILES examples/echo.cpp ${SOURCE_FILES})
set(ECHO_CLIENT_FILES examples/echo_client.cpp ${SOURCE_FILES})
set(TCP_RELAYER_FILES examples/tcp_relayer.cpp ${SOURCE_FILES})
set(TCP_PROXY_FILES examples/tcp_proxy.cpp ${SOURCE_FILES})
set(DAYTIME_FILES examples/daytime.cpp ${SOURCE_FILES})
set(DISCARD_FILES examples/discard.cpp ${SOURCE_FILES})
set(CHAT_FILES examples/chat.cpp ${SOURCE_FILES})
set(ALL_FILES examples/all.cpp ${SOURCE_FILES})
set(UDP_HELLO_FILES examples/udp_hello.cpp ${SOURCE_FILES})
set(UDP_CLIENT_FILES examples/udp_client.cpp ${SOURCE_FILES})
set(HTTPD_FILES examples/httpd.cpp ${SOURCE_FILES})
set(HTTPD_CLIENT_TEST_FILES examples/http_client_test.cpp ${SOURCE_FILES})


add_library(_cpp SHARED ${SOURCE_FILES})

add_executable(echo ${ECHO_FILES})
target_link_libraries(echo pthread)
add_executable(echo_client ${ECHO_CLIENT_FILES})
target_link_libraries(echo_client pthread)
add_executable(tcp_relayer ${TCP_RELAYER_FILES})
target_link_libraries(tcp_relayer pthread)
add_executable(tcp_proxy ${TCP_PROXY_FILES})
target_link_libraries(tcp_proxy pthread)
add_executable(daytime ${DAYTIME_FILES})
target_link_libraries(daytime pthread)
add_executable(discard ${DISCARD_FILES})
target_link_libraries(discard pthread)
add_executable(chat ${CHAT_FILES})
target_link_libraries(chat pthread)
add_executable(all_ ${ALL_FILES})
target_link_libraries(all_ pthread)
add_executable(udp_hello ${UDP_HELLO_FILES})
target_link_libraries(udp_hello pthread)
add_executable(udp_client ${UDP_CLIENT_FILES})
target_link_libraries(udp_client pthread)
add_executable(httpd ${HTTPD_FILES})
target_link_libraries(httpd pthread)
add_executable(httpd_client_test ${HTTPD_CLIENT_TEST_FILES})
target_link_libraries(httpd_client_test pthread)