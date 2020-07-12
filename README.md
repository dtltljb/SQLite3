## 编译说明

使用cmake构建, 项目根目录下已包含CMakeLists.txt文件，
由于每台电脑上的编译器路径不尽相同，CMakeLists.txt文件中没有指定编译器路径，
需要额外的CMAKE_TOOLCHAIN_FILE来指定.示例如下: 

* 1). 在项目路径下新建build目录并进入build目录,新建 CMakeLists.txt 文件，内容如下:
```
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_SYSROOT /opt/host/arm-buildroot-linux-gnueabihf/sysroot)
set(tools /opt/host)
set(CMAKE_C_COMPILER ${tools}/bin/arm-linux-gcc)
set(CMAKE_CXX_COMPILER ${tools}/bin/arm-linux-g++)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMKAE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```
* 注意: 将上面的CMAKE_SYSROOT和tools路径替换成本机实际路径  

* 2). 使用命令cmake -DCMAKE_TOOLCHAIN_FILE=path_to_your_cmake_toolchain_file ../配置项目
* 3). 配置成功,make编译


## 第三方库说明

以下C++ 第三方库,可以在github 上搜索 clone 到指定 third_party 路径即可编译此工程
include_directories(third_party/spdlog/include)				///> log 
include_directories(third_party/json/single_include)        ///> C++ json prase
include_directories(third_party/concurrentqueue)            ///> IPC 并发队列 



## ubuntu 调试环境说明
在项目工程路径:/qt_proj/qt_pro/robot-qt.pro 项目文件中,有本地库添加list,注意本地库相对路径需要一致.


unix:!macx: LIBS += -L$$PWD/../../../../../../usr/local/curls/lib/ -lcurl

INCLUDEPATH += $$PWD/../../../../../../usr/local/curls/include
DEPENDPATH += $$PWD/../../../../../../usr/local/curls/include

unix:!macx: LIBS += -L$$PWD/../../../../../../usr/local/lib/ -levent

INCLUDEPATH += $$PWD/../../../../../../usr/local/include
DEPENDPATH += $$PWD/../../../../../../usr/local/include

unix:!macx: LIBS += -L$$PWD/../../../../../../usr/local/lib/ -levent_pthreads

INCLUDEPATH += $$PWD/../../../../../../usr/local/include
DEPENDPATH += $$PWD/../../../../../../usr/local/include



unix:!macx: LIBS += -L$$PWD/../../../../../../usr/local/lib/ -lrt

INCLUDEPATH += $$PWD/../../../../../../usr/local/include
DEPENDPATH += $$PWD/../../../../../../usr/local/include

unix:!macx: PRE_TARGETDEPS += $$PWD/../../../../../../usr/local/lib/librt.a

unix:!macx: LIBS += -L$$PWD/../../../../../../usr/lib/x86_64-linux-gnu/ -lsqlite3

INCLUDEPATH += $$PWD/../../../../../../usr/lib/x86_64-linux-gnu
DEPENDPATH += $$PWD/../../../../../../usr/lib/x86_64-linux-gnu