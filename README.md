CStudy

======

这是我用于 C 自学的案例。

它们的内容分别是

showip：显示本地 IP 信息

beej：beej 网络教程里公用的代码

conn001：Socket 连接示例 1

stream001：TCP 运用示例 1

datagram001：UDP 运用示例 1

select001：select() 运用示例 1

tinyhttp001：通过 select() 实现了最简单的 Http Server，示例 1

lua001：Lua 虚拟机最简单内嵌示例

st001：基于 state-threads 的简单 Http Server 示例

IOCP001/IOCP001：基于 IOCP 的最简单 TCP 示例，只会接收客户端消息并打印在控制台当中。这个示例是从教程里抄下来的。

IOCP001/IOCP002：在前示例的基础上做了调整，修改了数据结构，并完成了 echo 回显的功能。

IOCP001/IOCP003：按自己的理解重新编写了 Echo 服务，重新制订了数据结构，把原来不相关的 PER_SESSION_CONTEXT PER_IO_CONTEXT 关联到一起。
