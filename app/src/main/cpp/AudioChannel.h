//
// Created by Administrator on 2020/6/30.
//

#ifndef FFMPEGDEMO1_AUDIOCHANNEL_H
#define FFMPEGDEMO1_AUDIOCHANNEL_H

#include "BaseChannel.h"


extern "C" {
#include "ffmpeg/include/libswresample/swresample.h"
};

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

class AudioChannel : public BaseChannel{
public:
    AudioChannel(int id, AVCodecContext *codecContext, AVRational time_base,
                 JavaCallHelper *javaCallHelper);

    ~AudioChannel();

    void start();

    void stop();

    void audio_decode();

    void audio_play();

    int getPCM();

    uint8_t *out_buffers = 0;
    int out_channels;
    int out_sampleSize;
    int out_sampleRate;
    int out_buffers_size;

private:
    SwrContext *swrContext = 0;
    pthread_t pid_audio_decode;
    pthread_t pid_audio_play;
    //引擎
    SLObjectItf engineObject = 0;
    //引擎接口
    SLEngineItf engineInterface = 0;
    //混音器
    SLObjectItf outputMixObject = 0;
    //播放器
    SLObjectItf bqPlayerObject = 0;
    //播放器接口
    SLPlayItf bqPlayerPlay = 0;
    //播放器队列接口
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = 0;


};


#endif //FFMPEGDEMO1_AUDIOCHANNEL_H
