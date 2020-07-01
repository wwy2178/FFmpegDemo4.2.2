# FFmpegDemo4.2.2

V1.0 

1、本Demo 是在Linux 服务器使用FFmpeg4.2.2 、 Android NDK17 、 librtmp 混合编译的arm64-v8a的版本。然后讲编译之后的结果导入到Android项目中，
通过相应的native代码控制和Java代码相互调用在Android端的SurfaceView 空间上实现播放的rtmp直播流的过程。
2、本demo只适用安装在arm64-v8a的设备上。
3、第一版只是打开应用，进去就开始自动开始进行处理播放。根据网络环境有延时。
4、如果有人想了解其他CPU 架构的混合编译，可以移步：https://blog.csdn.net/I123456789T/article/details/107024244  和  https://blog.csdn.net/I123456789T/article/details/104298910
前面的一个是介绍arm64-v8a和armeabi-v7a两种的混合编译。后面的一个单独介绍了armeabi-v7a的混合编译。编译完之后，导入到项目中，路径配置好后，native层代码都是一样的，都可以实现播放。
但是项目的cpu架构一定要标注清楚。否则会有未知的问题出现。
