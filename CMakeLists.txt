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
        src/TcpServer.h)

set(LIBY_CPP_FILES src/main.cpp ${SOURCE_FILES})
set(ECHO_FILES examples/echo.cpp ${SOURCE_FILES})
set(ECHO_CLIENT_FILES examples/echo_client.cpp ${SOURCE_FILES})
set(TCP_RELAYER_FILES examples/tcp_relayer.cpp ${SOURCE_FILES})
set(TCP_PROXY_FILES examples/tcp_proxy.cpp ${SOURCE_FILES})
set(DAYTIME_FILES examples/daytime.cpp ${SOURCE_FILES})
set(DISCARD_FILES examples/discard.cpp ${SOURCE_FILES})
set(CHAT_FILES examples/chat.cpp ${SOURCE_FILES})
set(ALL_FILES examples/all.cpp ${SOURCE_FILES})


add_library(_cpp SHARED ${SOURCE_FILES})

add_executable(liby_cpp ${LIBY_CPP_FILES})
target_link_libraries(liby_cpp pthread)
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

