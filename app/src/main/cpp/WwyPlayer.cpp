//
// Created by Administrator on 2020/6/29.
//

#include "WwyPlayer.h"


WwyPlayer::~WwyPlayer() {
    DELETE(dataSource);
    DELETE(javaCallHelper);
    pthread_mutex_destroy(&seekMutex);
}


WwyPlayer::WwyPlayer(JavaCallHelper *javaCallHelper, char *dataSource) {
    this->javaCallHelper = javaCallHelper;
    this->dataSource = new char[strlen(dataSource) + 1];
    strcpy(this->dataSource, dataSource);
    pthread_mutex_init(&seekMutex, 0);
}

/**
 * 准备线程pid_prepare真正执行的函数
 * @param args
 * @return
 */
void *task_prepare(void *args) {
    //打开输入
    WwyPlayer *ffmpeg = static_cast<WwyPlayer *>(args);
    //2 dataSource
    ffmpeg->_prepare();
    return 0;//一定一定一定要返回0！！！
}

void *task_start(void *args) {
    WwyPlayer *ffmpeg = static_cast<WwyPlayer *>(args);
    ffmpeg->_start();
    return 0;//一定一定一定要返回0！！！
}

//设置为友元函数
void *task_stop(void *args) {
    WwyPlayer *ffmpeg = static_cast<WwyPlayer *>(args);
    //要保证_prepare方法（子线程中）执行完再释放（在主线程）
    //pthread_join ：这里调用了后会阻塞主，可能引发ANR
    ffmpeg->isPlaying = 0; // 修改成false，音频和视频，的所有解码 和 播放 等等，全部停止了
    pthread_join(ffmpeg->pid_prepare, 0);//解决了：要保证_prepare方法（子线程中）执行完再释放（在主线程）的问题
//    //2 dataSource
//    ffmpeg->_prepare();

    if (ffmpeg->formatContext) {
        avformat_close_input(&ffmpeg->formatContext); // 关闭媒体格式上下文
        avformat_free_context(ffmpeg->formatContext); // 回收媒体格式上下文
        ffmpeg->formatContext = 0;
    }
    DELETE(ffmpeg->videoChannel);
    DELETE(ffmpeg->audioChannel);

    DELETE(ffmpeg);
    return 0;//一定一定一定要返回0！！！
}


void WwyPlayer::_prepare() {

    //0.5 AVFormatContext **ps
    formatContext = avformat_alloc_context();
    AVDictionary *dictionary = 0;
    av_dict_set(&dictionary, "timeout", "10000000", 0);//设置超时时间为10秒，这里的单位是微秒
    // 1, 打开媒体
    int ret = avformat_open_input(&formatContext, dataSource, 0, &dictionary);
    av_dict_free(&dictionary);
    if (ret) {
        LOGE("打开媒体失败：%s", av_err2str(ret));
        //失败 ，回调给java
        //        LOGE("打开媒体失败：%s", av_err2str(ret));
        ////        javaCallHelper jni 回调java方法
        ////        javaCallHelper->onError(ret);
        //          //可能java层需要根据errorCode来更新UI!
        //          //2019.8.12 作业自行尝试实现
        //TODO 作业:反射通知java
        // 1 反射Java 属性（成员/静态）
        // 2 反射Java 方法 （成员/静态）
        // 3 子线程反射
        if (javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL);
        }
        return;
    }
    //2 查找媒体中的流信息
    ret = avformat_find_stream_info(formatContext, 0);
    if (ret < 0) {
        LOGE("查找媒体中的流信息失败：%s", av_err2str(ret));
        //TODO 作业:反射通知java
        if (javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_FIND_STREAMS);
        }
        return;
    }
    duration = formatContext->duration / AV_TIME_BASE; // 最终是要拿到秒
    //这里的 i 就是后面 166行的 packet->stream_index
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        //3获取媒体流（音频或视频）
        AVStream *stream = formatContext->streams[i];
        //4获取编解码这段流的参数
        AVCodecParameters *codecParameters = stream->codecpar;
        //5 通过参数中的id（编解码的方式），来查找当前流的解码器
        AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
        if (!codec) {
            LOGE("查找当前流的解码器失败");
            //TODO 作业:反射通知java
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            }
            return;
        }
        //6 创建解码器上下文
        AVCodecContext *codecContext = avcodec_alloc_context3(codec);

        if (!codecContext) {//2019.8.14 add.
            LOGE("创建解码器上下文失败");
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            }
        }
        //7 设置解码器上下文的参数
        ret = avcodec_parameters_to_context(codecContext, codecParameters);
        if (ret < 0) {
            //TODO 作业:反射通知java
            LOGE("设置解码器上下文的参数失败：%s", av_err2str(ret));
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            }
            return;
        }
        //8 打开解码器
        ret = avcodec_open2(codecContext, codec, 0);
        if (ret) {
            //TODO 作业:反射通知java
            LOGE("打开解码器失败：%s", av_err2str(ret));
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
            }
            return;
        }
        AVRational time_base = stream->time_base;
        //判断流类型（音频还是视频？）
        if (codecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            //音频
            audioChannel = new AudioChannel(i, codecContext, time_base, javaCallHelper);
        } else if (codecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            //视频
            AVRational fram_rate = stream->avg_frame_rate;
//            int fps = fram_rate.num / fram_rate.den;
            int fps = av_q2d(fram_rate);

            videoChannel = new VideoChannel(i, codecContext, fps, time_base, javaCallHelper);
            videoChannel->setRenderCallback(renderCallback);
        }
    }//end for

    if (!audioChannel && !videoChannel) {
        //既没有音频也没有视频
        //TODO 作业:反射通知java
        LOGE("没有音视频");
        if (javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_NOMEDIA);
        }
        return;
    }

    //准备好了，反射通知java
    if (javaCallHelper) {
        javaCallHelper->onPrepared(THREAD_CHILD);
    }

}


/**
 * 播放准备
 * 可能是主线程
 * doc/samples/
 */
void WwyPlayer::prepare() {
    //可以直接来进行解码api调用吗？
    //xxxxxx。。。。不能！
    //文件：io流问题
    //直播：网络
//    pthread_create： 创建子线程

//pthread_create(pthread_t* __pthread_ptr, pthread_attr_t const* __attr, void* (*__start_routine)(void*), void*);
    pthread_create(&pid_prepare, 0, task_prepare, this);
}

/**
 * 开始播放
 */
void WwyPlayer::start() {
    isPlaying = 1;
    if (videoChannel) {
        videoChannel->setAudioChannel(audioChannel);
        videoChannel->start();
    }
    if (audioChannel) {
        audioChannel->start();
    }
    pthread_create(&pid_start, 0, task_start, this);
}

/**
 * 真正执行解码播放
 */
void WwyPlayer::_start() {
    while (isPlaying) {
        /**
         * 内存泄漏点1
         * 控制 视频 packets 队列
         */
        if (videoChannel && videoChannel->packets.size() > 100) {
            av_usleep(10 * 1000);
            continue;
        }
        //内存泄漏点1（控制 音频 packets 队列）
        if (audioChannel && audioChannel->packets.size() > 100) {
            av_usleep(10 * 1000);
            continue;
        }
        AVPacket *packet = av_packet_alloc();
        pthread_mutex_lock(&seekMutex);
        int ret = av_read_frame(formatContext, packet);
        pthread_mutex_unlock(&seekMutex);
        if (!ret) {
            //ret 为 0 表示成功
            //要判断流类型，是视频还是音频
            if (videoChannel && packet->stream_index == videoChannel->id) {
                //往视频编码数据包队列中添加数据
                videoChannel->packets.push(packet);
            } else if (audioChannel && packet->stream_index == audioChannel->id) {
                //往音频编码数据包队列中添加数据
                audioChannel->packets.push(packet);
            }
        } else if (ret == AVERROR_EOF) {
            //表示读完了
            //要考虑读完了，是否播完了的情况
            //如何判断有没有播放完？判断队列是否为空
            if (videoChannel->packets.empty() && videoChannel->frames.empty()
                && audioChannel->packets.empty() && audioChannel->frames.empty()
                    ) {
                //播放完了
                av_packet_free(&packet);
                break;
            }

        } else {
            //TODO 作业:反射通知java
            LOGE("读取音视频数据包失败");
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_READ_PACKETS_FAIL);
            }
            av_packet_free(&packet);
            break;
        }
    }//end while

    isPlaying = 0;
    //停止解码播放（音频和视频）
    videoChannel->stop();
    audioChannel->stop();

//    AVPacket *avPacket = av_packet_alloc();
//    AVFrame *avFrame = av_frame_alloc();
//    //放入队列
//    SafeQueue<AVPacket *> packets;
//    SafeQueue<AVFrame *> frames;
//
//    packets.clear();
//
//    av_packet_free(&avPacket);
//    av_frame_free(&avFrame);
//    函数指针？


}

void WwyPlayer::setRenderCallback(RenderCallback renderCallback) {
    this->renderCallback = renderCallback;
}

/**
 * 停止播放
 */
void WwyPlayer::stop() {
//    isPlaying = 0;
    javaCallHelper = 0;//prepare阻塞中停止了，还是会回调给java "准备好了"


    //既然在主线程会引发ANR，那么我们到子线程中去释放
    pthread_create(&pid_stop, 0, task_stop, this);//创建stop子线程

//    if (formatContext) {
//        avformat_close_input(&formatContext);
//        avformat_free_context(formatContext);
//        formatContext = 0;
//    }
//    if (videoChannel) {
//        videoChannel->stop();
//    }
//    if (audioChannel) {
//        audioChannel->stop();
//    }
}

int WwyPlayer::getDuration() const {
    return duration;
}

// 异步线程
// 控制 播放时长的，音频 和 视频 的 快进快退
void WwyPlayer::seekTo(int playProgress) {

    if (playProgress < 0 || playProgress > duration) {
        // JNICallback告诉上层，错误信息
        return;
    }
    if (!audioChannel && !videoChannel) {
        // JNICallback告诉上层，错误信息
        return;
    }
    if (!formatContext) {
        // JNICallback告诉上层，错误信息
        return;
    }
    //1,上下文
    //2，流索引，-1：表示选择的是默认流
    //3，要seek到的时间戳
    //4，seek的方式
    //AVSEEK_FLAG_BACKWARD： 表示seek到请求的时间戳之前的最靠近的一个关键帧
    //AVSEEK_FLAG_BYTE：基于字节位置seek
    //AVSEEK_FLAG_ANY：任意帧（可能不是关键帧，会花屏）
    //AVSEEK_FLAG_FRAME：基于帧数seek

    // 为什么要加互斥锁？ 我们用到了 媒体格式上下文formatContext，（音频通道，视频通道 都用到了formatContext）为了安全，所以加锁
    pthread_mutex_lock(&seekMutex);

    int ret = av_seek_frame(formatContext, -1, playProgress * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);

    if (ret < 0) {
        if (javaCallHelper) {
            // JNICallback告诉上层，错误信息
            javaCallHelper->onError(THREAD_CHILD, ret);
        }
        return;
    }

    /**
       用户在拖动的过程中，还没有松手的过程中，（正在解码，正在播放.....）

       上层手放开了，拖动值---》时长值 ---> native（重新播放：跳转到用户指定的位置播放）
     */
    if (audioChannel) {
        audioChannel->packets.setWork(0); // 全部停止 -- 不要在解码了，不要在播放了
        audioChannel->frames.setWork(0);  // 全部停止 -- 不要在解码了，不要在播放了
        audioChannel->packets.clear(); // 停止后 收尾动作
        audioChannel->frames.clear(); // 停止后 收尾动作
        //清除数据后，让队列重新工作
        audioChannel->packets.setWork(1); // 让队列里面的一切，开始工作....
        audioChannel->frames.setWork(1); // 让队列里面的一切，开始工作....
    }
    if (videoChannel) {
        videoChannel->packets.setWork(0); // 全部停止 -- 不要在解码了，不要在播放了
        videoChannel->frames.setWork(0); // 全部停止 -- 不要在解码了，不要在播放了
        videoChannel->packets.clear(); // 停止后 收尾动作
        videoChannel->frames.clear(); // 停止后 收尾动作
        //清除数据后，让队列重新工作
        videoChannel->packets.setWork(1); // 让队列里面的一切，开始工作....
        videoChannel->frames.setWork(1); // 让队列里面的一切，开始工作....
    }

    pthread_mutex_unlock(&seekMutex);


}
