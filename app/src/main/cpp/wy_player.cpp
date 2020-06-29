#include <jni.h>
#include <string>

extern "C" {
#include "ffmpeg/include/libavutil/avutil.h"
}

JavaVM *javaVm = 0;


/**
 * 获取ffmpeg当前的版本
 */
 const char *getFFmpegVer(){
    return av_version_info();
 }

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_ffmpegdemo1_WwyPlayer_getFFmpegVersion(JNIEnv *env, jobject thiz) {
    return env->NewStringUTF(getFFmpegVer());
}
