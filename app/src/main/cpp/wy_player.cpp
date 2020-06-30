#include <jni.h>
#include <string>
#include "WwyPlayer.h"
#include "JavaCallHelper.h"
#include <android/native_window_jni.h>

extern "C" {
#include "ffmpeg/include/libavutil/avutil.h"
}

JavaVM *javaVM = 0;
JavaCallHelper *javaCallHelper = 0;
WwyPlayer *ffmpeg = 0;
ANativeWindow *window = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;//静态初始化mutex

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

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVM = vm;
    return JNI_VERSION_1_4;
}


//1，data;2，linesize；3，width; 4， height
void renderFrame(uint8_t *src_data, int src_lineSize, int width, int height) {
    pthread_mutex_lock(&mutex);
    if (!window) {
        pthread_mutex_unlock(&mutex);
        return;
    }
    ANativeWindow_setBuffersGeometry(window, width,
                                     height,
                                     WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer window_buffer;
    if (ANativeWindow_lock(window, &window_buffer, 0)) {
        ANativeWindow_release(window);
        window = 0;
        pthread_mutex_unlock(&mutex);
        return;
    }
    //把buffer中的数据进行赋值（修改）
    uint8_t *dst_data = static_cast<uint8_t *>(window_buffer.bits);
    int dst_lineSize = window_buffer.stride * 4;//ARGB
    //逐行拷贝
    for (int i = 0; i < window_buffer.height; ++i) {
        memcpy(dst_data + i * dst_lineSize, src_data + i * src_lineSize, dst_lineSize);
    }
    ANativeWindow_unlockAndPost(window);
    pthread_mutex_unlock(&mutex);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_ffmpegdemo1_WwyPlayer_prepareNative(JNIEnv *env,jobject instance,jstring data_source){

     const char * dataSource = env->GetStringUTFChars(data_source,0);

    javaCallHelper = new JavaCallHelper(javaVM, env, instance);
    ffmpeg = new WwyPlayer(javaCallHelper, const_cast<char *>(dataSource));
    ffmpeg->setRenderCallback(renderFrame);
    ffmpeg->prepare();

    env->ReleaseStringUTFChars(data_source, dataSource);

 }

 extern "C" JNIEXPORT void JNICALL
 Java_com_example_ffmpegdemo1_WwyPlayer_setSurfaceNative(JNIEnv *env,jobject instance,jobject surface){
     pthread_mutex_lock(&mutex);
     //先释放之前的显示窗口
     if (window) {
         ANativeWindow_release(window);
         window = 0;
     }
     //创建新的窗口用于视频显示
     window = ANativeWindow_fromSurface(env, surface);
     pthread_mutex_unlock(&mutex);
 }

extern "C" JNIEXPORT void JNICALL
Java_com_example_ffmpegdemo1_WwyPlayer_startNative(JNIEnv *env,jobject thiz){
    if (ffmpeg) {
        ffmpeg->start();
    }
 }

 extern "C" JNIEXPORT void JNICALL
 Java_com_example_ffmpegdemo1_WwyPlayer_stopNative(JNIEnv *env,jobject thiz){
     if (ffmpeg) {
         ffmpeg->stop();
     }
 }

 extern "C" JNIEXPORT void JNICALL
 Java_com_example_ffmpegdemo1_WwyPlayer_releaseNative(JNIEnv *env,jobject thiz){
     pthread_mutex_lock(&mutex);
     if (window) {
         //把老的释放
         ANativeWindow_release(window);
         window = 0;
     }
     pthread_mutex_unlock(&mutex);
     DELETE(ffmpeg); // 移除中转站
 }

 extern "C" JNIEXPORT jint JNICALL
 Java_com_example_ffmpegdemo1_WwyPlayer_getDurationNative(JNIEnv *env, jobject thiz){
     if (ffmpeg) {  // ffmpeg == kevinPlayer
         return ffmpeg->getDuration();
     }
     return 0;
 }

 extern "C" JNIEXPORT void JNICALL
 Java_com_example_ffmpegdemo1_WwyPlayer_seekToNative(JNIEnv *env, jobject thiz, jint playProgress){
     if (ffmpeg) {
         ffmpeg->seekTo(playProgress); // 异步线程
     }
 }
