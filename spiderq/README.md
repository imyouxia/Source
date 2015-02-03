What is spiderq?
================
Spiderq is a Web Spider to crawl webpage(html) by Qteqpid. The performance depends on your server configuration and network. I will continue maintain it and list some TODOs at the end of this file. More people are welcome to join!


Building spiderq
================
Spiderq can be compiled and used on Centos 5.8 .
It is as simple as:

    % make
    % make install

Then you will get an executable file named spider. After configurating spiderq.conf, run program:

    % ./spider

For more informations, see Makefile.


Contact
================
For any question, just contact me at any time. Enjoy!
mailto: qteqpid<glloveyp@163.com>
blog: http://hi.baidu.com/qteqpid_pku


TODO
===============
@线程池
@信号处理
@网页内容排重
@同一ip间隔抓取
@层次结构存储网页
@是否遵守robots.txt
@支持更新抓取，不重复抓
@定义对外api和html类，方便用户自定义处理html，动态加载方式
