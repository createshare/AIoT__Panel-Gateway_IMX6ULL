# sqlite_demo 使用说明  

**测试说明**  
* 数据库介绍  
    * 默认创建数据库名为 rtsmart.db  
    * 默认创建表 userinfo(username text, password text, role text ,ctime text)  
    * 默认创建表 syslog(ctime text, log_type text, level text, operation text ,detail text)  
* sqlite_demo 参数支持以下功能  
    * --adduser     增加一组数据  
    * --deluser     删除一组数据  
    * --login       用户登录检测  
    * --chpass      修改用户密码  
    * --resetpass   强制修改某个用户密码，密码为"888888"的md5加密字符串  
    * --getuserinfo 输出 userinfo 表内的数据  
    * --getsyslog   输出 syslog 表内的数据  

* 测试命令  
    * ./sqlite_demo.elf --adduser zhangsan zhangsan123 admin  
        * 如果 userinfo 表内有 zhangsan 这个用户，则不会添加这组数据，若无，则向表内插入该组数据，并向 syslog 表记录日志  
    * ./sqlite_demo.elf --deluser zhangsan  
        * 删除 userinfo 表中的某个用户数据(这里没做表中是否有该用户的检测)  
    * ./sqlite_demo.elf --login zhangsan zhangsan123  
        * 在 userinfo 表内对 zhangsan 这个用户做检测是否存在，存在并判断密码是否正确,正确并记录日志  
    * ./sqlite_demo.elf --chpass zhangsan zhangsan123 zhangsan456  
        * 在 userinfo 表内对 zhangsan 这个用户做检测是否存在，存在并判断密码是否正确，正确的话再修改密码并记录日志  
    * ./sqlite_demo.elf --resetpass zhangsan  
        * 在 useronfo 表内 强行重置 zhangsan 这个用户的密码，设置的密码为"888888"的md5加密字符串, 并记录日志  
    * ./sqlite_demo.elf --getuserinfo  
        * 输出 userinfo 表内的数据  
    * ./sqlite_demo.elf --getsyslog  
        * 输出 syslog 表内的数据  

