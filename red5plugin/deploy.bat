@echo off

set base=D:\Software\Media\red5-server-1.0

@echo 这是测试用的部署工具，默认部署路径为 %base%
@echo 如果配置不正确请先配置，在进行部署！！！
@echo 如果配置正确，按任意键继续。

call %base%\red5-shutdown.bat
rm -rf %base%\webapps\videochat
cp -rf web %base%\webapps\videochat
call %base%\red5.bat

