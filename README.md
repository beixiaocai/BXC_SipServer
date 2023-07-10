# BXC_SipServer

* 作者：北小菜 
* 邮箱：bilibili_bxc@126.com
* QQ：1402990689
* 微信：bilibili_bxc
* 作者主页：http://www.any12345.com/user/4
* 作者-哔哩哔哩主页：https://space.bilibili.com/487906612
* 作者-头条西瓜主页：https://www.ixigua.com/home/4171970536803763

## 本项目的视频讲解
* [BXC_SipServer 哔哩哔哩视频讲解](https://www.bilibili.com/video/BV1Mv4y1d7Vy)
* [BXC_SipServer 头条西瓜视频讲解](https://www.ixigua.com/7247470596639064587?utm_source=xiguastudio)


## 介绍
1. 一个基于C++开发的国标GB28181流媒体信令服务器。
2. 采用BXC_SipServer+ZLMediaKit。可以搭建一个接收摄像头国标协议推流的国标流媒体服务，然后实现RTSP/RTMP/HTTP-FVL/HLS/WS/SRT等协议分发视频流。
3. BXC_SipServer负责信令模块，ZLMediaKit负责流媒体模块。
4. BXC_SipServer作为国标流媒体服务器的信令模块。用于接收摄像头的信令注册，注册完成后，
主动向摄像头发送Invite请求，摄像头收到Invite请求后， 返回Invite的确认。 服务端收到确认后，发送ACK请求，
摄像头收到ACK请求后，开始通过RTP传输ps流推流至ZLMediaKit的国标RTP Server。 ZLMediaKit作为国标流媒体服务器的流媒体模块，主要用于接收摄像头国标推流和其他协议的分发。
5. 补充说明一下，这只是我为了讲课而开发的demo级的信令服务器，部分信令功能并没有实现。但是基本功能是没问题了，对接摄像头是完全没问题的，
   对于学习国标流媒体信令服务器的新手，完全够用了，可以快速体验国标协议，通过wireshark抓包快速感受国标对接的流程。

## BXC_SipServer介绍
1. 基于osip和exosip开源库，开发的国标GB28181流媒体信令服务器
2. osip和exosip版本一定要对应，否则可能会出现不兼容的情况。我经常用的版本osip2-5.1.2和exosip2-5.1.2，
我准备了一份放到了百度云
* osip2-5.1.2百度云链接：https://pan.baidu.com/s/1P25xUSri3otwjfaPo5g9Zg 提取码：1234
* exosip2-5.1.2百度云链接：https://pan.baidu.com/s/1ClkPjDzc5VjSCthNAL0nLQ 提取码：1234
* windows系统编译还需要c-ares库

#### 附3个库的官方下载地址：
~~~
  osip:   http://ftp.twaren.net/Unix/NonGNU/osip/
  exosip: http://download.savannah.gnu.org/releases/exosip/
  c-ares: https://c-ares.haxx.se/
~~~


## ZLMediaKit 介绍
1. ZLMediaKit是一个国人开发的流媒体开源项目，非常的牛逼，具体我就不介绍了，大家可以自行了解
2. [ZLMediaKit](https://gitee.com/xia-chu/ZLMediaKit) 开源地址

## 快速开始

#### linxu系统编译运行
~~~

一，首先安装osip和exosip，建议按照上面的BXC_SipServer介绍下载我推荐的 osip2-5.1.2和exosip2-5.1.2

1. 编译安装 osip2-5.1.2
 cd osip2-5.1.2  
 ./configure
 make
 sudo make install
 
2. 编译安装 exosip2-5.1.2
 cd exosip2-5.1.2
 ./configure
 make
 sudo make install
 
二，开始安装 BXC_SipServer
1. 下载代码
 git clone https://gitee.com/Vanishi/BXC_SipServer.git
2. 编译
 cd BXC_SipServer
 mkdir build
 cd build
 cmake ..
 make 
3. 运行
 ./BXC_SipServer
 
 
~~~
### windows系统编译运行
~~~

一，osip和exosip编译到windows平台比较麻烦，我也是在编译过程中解决了多个报错，用了大半天时间，才编译出可用的版本

如果你在windows平台自行编译osip和exosip，还需要编译c-ares库。推荐使用 c-ares-1.16.0 配合 osip2-5.1.2 和 exosip2-5.1.2

我已经将上面3个库编译好放在了3rdparty，并提供了 vs2019/x64/Debug 和 vs2019/x64/Release

二，只需要使用vs2019打开 BXC_SipServer.sln
选择 x64/Debug 或 x64/Release就能直接运行，依赖库都配置了相对路径
 
~~~


## 常见问题

1. 常见报错 error while loading shared libraries: libXXX.so.X: cannot open shared object file: No such file [解决方法](https://blog.csdn.net/deeplan_1994/article/details/83927832)






