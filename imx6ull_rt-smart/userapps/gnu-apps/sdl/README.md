## 说明 ##

* 移植SDL2最新的2.0.14版本。暂不支持directfb，没有实现窗口系统。默认视频驱动使用dummy，在qemu lcd屏上显示。可以绘制基本的线，三角形，矩形。可以实现texture绘制。
* 测试程序为test/testrtt.c

## 编译方法 ##
工作目录是 `gnu_apps/sdl`，编译sdl：

```bash
$ ./build_sdl.sh

# 编译测试代码：
$ ./build_test.sh
```
