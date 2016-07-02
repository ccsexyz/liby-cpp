##Liby CPP

###基本用法

```c++
#include "Liby.h"

using namespace Liby;

int main() {
    EventLoopGroup group;
    auto echo_server = group.creatTcpServer("localhost", "9377");
    echo_server->onRead([](Connection &conn) { conn.send(conn.read()); });
    group.run();
    return 0;
}
```

####设置工作线程数和IO复用接口
```c++
    EventLoopGroup group(8, "POLL");
```

####主动关闭连接
```c++
    echo_server->onRead([](Connection &conn) {
        conn.destroy();
    });
```

####设置IO循环终止条件
```c++
    // 默认为无限循环
    group.run([]->bool{
        return false;
    });
```
###设置定时器
```c++
    // 指定绝对时刻
    conn.runAt(Timestamp(sec, usec), []{});
    // 指定相对时间
    conn.runAfter(Timestamp(sec, usec), []{});
    // 指定循环触发的定时器
    conn.runEvery(Timestamp(sec, usec), []{});
    // 取消定时器
    // 与conn绑定的定时器在conn析构或调用destroy时自动取消
    conn.cancelTimer(timerId);
```
