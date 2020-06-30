cmake_minimum_required(VERSION 3.4.1)

#找到包含的所有cpp文件,不包含.h文件
file(GLOB allCpp *.cpp)

#添加自己编写的 cpp源文件生成动态库
add_library(
        wy_player
        SHARED
        ${allCpp})

#找到系统中 NDK log库
find_library(
        android
        android
)

#引入ffmpeg头文件
include_directories(${CMAKE_SOURCE_DIR}/ffmpeg/include )

#添加ffmpeg 库
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/ffmpeg/libs/${CMAKE_ANDROID_ARCH_ABI}")

#指定 rtmp静态库文件目录
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/librtmp/libs/${CMAKE_ANDROID_ARCH_ABI}")


#最后开始链接库
target_link_libraries(
        wy_player
        -Wl,--start-group
        avcodec avfilter avformat avutil swresample swscale
        -Wl,--end-group
        log
        ${android}
        z
        OpenSLES
        rtmp
)