//
// Created by Administrator on 2020/6/29.
//

#ifndef FFMPEGDEMO1_WWYPLAYER_H
#define FFMPEGDEMO1_WWYPLAYER_H

#include "JavaCallHelper.h"
#include "AudioChannel.h"
#include "VideoChannel.h"
#include "macro.h"
#include <cstring>
#include <pthread.h>

extern "C" {
#include "ffmpeg/include/libavformat/avformat.h"
};

class WwyPlayer {
    friend void *task_stop(void *args);
public:
    WwyPlayer(JavaCallHelper *javaCallHelper, char *dataSource);

    WwyPlayer();

    virtual ~WwyPlayer();

    void prepare();

    void _prepare();

    void start();

    void _start();

    void setRenderCallback(RenderCallback renderCallback);

    void stop();

private:
    JavaCallHelper *javaCallHelper = 0;
    AudioChannel *audioChannel = 0;
    VideoChannel *videoChannel = 0;
    char *dataSource;
    pthread_t pid_prepare;
    pthread_t pid_start;
    pthread_t pid_stop;
    bool isPlaying;
    AVFormatContext *formatContext = 0;
    RenderCallback renderCallback;
    int duration;  // 总时长
    pthread_mutex_t seekMutex;
public:
    void setDuration(int duration);

    int getDuration() const;
//总播放时长

    void seekTo(int i);
};


#endif //FFMPEGDEMO1_WWYPLAYER_H
