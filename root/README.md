

TinyWebServer
===============
Linux下C/C++轻量级Web服务器

* 使用 **线程池 + 非阻塞socket + epoll(ET和LT均实现) + 事件处理(Reactor和模拟Proactor均实现)** 的并发模型
* 使用**状态机**解析HTTP请求报文，支持解析**GET和POST**请求
* 访问服务器数据库实现web端用户**注册、登录**功能，可以请求服务器**图片和视频文件**
* 实现**同步/异步日志系统**，记录服务器运行状态
* 基于升序链表实现定时器，关闭超时的非活动连接
* 经Webbench压力测试可以实现**上万的并发连接**数据交换




写在前面
----
* 本项目参考了游双的《Linux高性能服务器编程》和微信公众号“两猿社”的《Web服务器项目详解》




项目框架
-------------
![frame](F:\Zzz\tmp\25\5.1\TinyWebServer-master\root\frame.jpg)

运行环境：
----------
> * Ubuntu18.04

> * Mysql 8.0.33   gcc 7.5.0   g++ 7.5.0

> * 安装MySQL客户端开发库 libmysqlclient-dev



快速运行
------------
* 安装配置Mysql
	* 安装Mysql，配置账号密码为：root/123456
	
	* 安装MySQL客户端开发库：
	
	  sudo apt update          
	
	  sudo apt install libmysqlclient-dev -y
	
* 创建数据库和用户账号表格

    ```C++
    // 建立yourdb库
    create database tinyWebServer;
    
    // 创建user表
    USE tinyWebServer;
    CREATE TABLE user(
        username char(50) NULL,
        passwd char(50) NULL
    )ENGINE=InnoDB CHARSET=utf8mb4;
    
    // 添加数据
    INSERT INTO user(username, passwd) VALUES('admin', 'admin');
    ```

* 修改main.cpp中的数据库初始化信息

    ```C++
    //数据库登录名,密码,库名
    string user = "root";
    string passwd = "123456";
    string databasename = "tinyWebServer";
    ```

* build

    ```C++
    sh ./build.sh
    ```

* 启动server

    ```C++
    ./server
    ```

* 浏览器端

    ```C++
    ip:9006
    ```



个性化运行
------

```C++
./server [-p port] [-l LOGWrite] [-m TRIGMode] [-o OPT_LINGER] [-s sql_num] [-t thread_num] [-c close_log] [-a actor_model]
```

温馨提示:以上参数不是非必须，不用全部使用，根据个人情况搭配选用即可.

* -p，自定义端口号
	* 默认9006
* -l，选择日志写入方式，默认同步写入
	* 0，同步写入
	* 1，异步写入
* -m，listenfd和connfd的模式组合，默认使用LT + LT
	* 0，表示使用LT + LT
	* 1，表示使用LT + ET
  * 2，表示使用ET + LT
  * 3，表示使用ET + ET
* -o，优雅关闭连接，默认不使用
	* 0，不使用
	* 1，使用
* -s，数据库连接数量
	* 默认为8
* -t，线程数量
	* 默认为8
* -c，关闭日志，默认打开
	* 0，打开日志
	* 1，关闭日志
* -a，选择反应堆模型，默认Proactor
	* 0，Proactor模型
	* 1，Reactor模型

测试示例命令与含义

```C++
./server -p 9007 -l 1 -m 0 -o 1 -s 10 -t 10 -c 1 -a 1
```

- [x] 端口9007
- [x] 异步写入日志
- [x] 使用LT + LT组合
- [x] 使用优雅关闭连接
- [x] 数据库连接池内有10条连接
- [x] 线程池内有10条线程
- [x] 关闭日志
- [x] Reactor反应堆模型



压力测试
-------------

测试机配置：vmware workstation ubuntu18.04虚拟机，6核CPU 8GB内存

在**<u>关闭日志后</u>**，使用Webbench对服务器进行压力测试，对listenfd和connfd分别采用ET和LT模式，均可实现上万的并发连接，下面列出的是两者组合后的测试结果：

> * Proactor，LT + LT，21434 QPS

![image-20250503163531237](C:\Users\23级硕士-张梦杰\AppData\Roaming\Typora\typora-user-images\image-20250503163531237.png)

> * Proactor，LT + ET，20896 QPS

![image-20250503163658395](C:\Users\23级硕士-张梦杰\AppData\Roaming\Typora\typora-user-images\image-20250503163658395.png)

> * Proactor，ET + LT，22735 QPS

![image-20250503163801943](C:\Users\23级硕士-张梦杰\AppData\Roaming\Typora\typora-user-images\image-20250503163801943.png)

> * Proactor，ET + ET，23068 QPS

![image-20250503163857988](C:\Users\23级硕士-张梦杰\AppData\Roaming\Typora\typora-user-images\image-20250503163857988.png)

> * Reactor，LT + ET，13429 QPS

![image-20250503164055365](C:\Users\23级硕士-张梦杰\AppData\Roaming\Typora\typora-user-images\image-20250503164055365.png)

> * 并发连接总数：10000
> * 访问服务器时间：10s
> * 所有访问均成功



## 测试地址

本项目已部署在云服务器，测试地址：

http://http://8.152.208.228:9006/



## 联系我

Email: 724603054@qq.com



