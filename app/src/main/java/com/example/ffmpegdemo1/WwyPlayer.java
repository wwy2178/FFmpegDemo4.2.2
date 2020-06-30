package com.example.ffmpegdemo1;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class WwyPlayer implements SurfaceHolder.Callback {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("wy_player");
    }

    private OnPreparedListener onPreparedListener; // 消息告诉MainActivity
    private OnProgressListener onProgressListener;
    private OnErrorListener onErrorListener;

    private SurfaceHolder surfaceHolder; // 为了 渲染 屏幕 帧数据

    public WwyPlayer() {
    }

    // 媒体流的源 (文件路径 、 直播地址（要集成X264才行）)
    private String dataSource;

    public void setDataSource(String dataSource) {
        this.dataSource = dataSource;
    }

    /**
     * 准备工作（解封装-其实就是把.mp4给解出来）
     * MainActivity打开后，最先调用的 函数
     */
    public void prepare() {
        prepareNative(dataSource);
    }

    /**
     * 开始播放
     */
    public void start() {
        startNative();
    }

    /**
     * 停止播放
     */
    public void stop() {
        stopNative();
    }

    /**
     * 获取总的播放时长
     * @return
     */
    public int getDuration(){
        return getDurationNative();
    }

    /**
     * 播放进度跳转
     * @param playProgress
     */
    public void seekTo(final int playProgress) {
        new Thread(){
            @Override
            public void run() {
                seekToNative(playProgress); // 异步线程
            }
        }.start();
    }

    /**
     * 资源释放
     */
    public void release() {
        surfaceHolder.removeCallback(this);
        releaseNative();
    }

    /**
     * 给JNI 方式调用的方法，准备成功
     */
    public void onPrepared() {
        if (null != onPreparedListener) {
            this.onPreparedListener.onPrepared();
        }
    }

    public void setOnError(OnErrorListener listener){
        if (onErrorListener != null){
            this.onErrorListener = listener;
        }
    }

    /**
     * 给JNI 方式回调的方法（进行错误的回调）
     */
    public void onError(int errorCode) {
        if (null != onPreparedListener) {
            String errorText = null;

            switch (errorCode) {
                case SurPlayFlags.FFMPEG_ALLOC_CODEC_CONTEXT_FAIL:
                    errorText = "无法根据解码器创建上下文";
                    break;
                case SurPlayFlags.FFMPEG_CAN_NOT_FIND_STREAMS:
                    errorText = "找不到媒体流信息";
                    break;
                case SurPlayFlags.FFMPEG_CAN_NOT_OPEN_URL:
                    errorText = "打不开媒体数据源";
                    break;
                case SurPlayFlags.FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL:
                    errorText = "根据流信息 配置上下文参数失败";
                    break;
                case SurPlayFlags.FFMPEG_FIND_DECODER_FAIL:
                    errorText = "找不到解码器";
                    break;
                case SurPlayFlags.FFMPEG_NOMEDIA:
                    errorText = "没有音视频";
                    break;
                case SurPlayFlags.FFMPEG_READ_PACKETS_FAIL:
                    errorText = "读取媒体数据包失败";
                    break;
                default:
                    errorText = "未知错误，自己去检测你的垃圾代码...";
                    break;
            }
            this.onPreparedListener.onError(errorText);
        }
    }

    /**
     * MainActivity设置监听，就可以回调到MainActivity的方法，进行UI的操作了
     * @param onPreparedListener
     */
    public void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        this.onPreparedListener = onPreparedListener;
    }

    /**
     * 消息告诉MainActivity
     */
    interface OnPreparedListener {
        void onPrepared();
        void onError(String errorText);
    }

    public interface OnProgressListener {
        void onProgress(int progress);
    }

    public interface OnErrorListener {
        void onError(int errorCode);
    }

//    /**
//     * 供native反射调用
//     * 表示出错了
//     */
//    public void onError(int errorCode) {
//        if (null != onErrorListener) {
//            onErrorListener.onError(errorCode);
//        }
//    }

    // native层传递上来的 进度值
    public void onProgress(int progress) {
        if (null != onProgressListener) {
            onProgressListener.onProgress(progress);
        }
    }

    public void setOnProgressListener(OnProgressListener onProgressListener) {
        this.onProgressListener = onProgressListener;
    }

    /**
     * 全部都是native函数
     */
    public native void prepareNative(String dataSource);
    public native void startNative();
    public native void stopNative();
    public native void releaseNative();
    private native int getDurationNative();
    private native void seekToNative(int playProgress);
    public native String getFFmpegVersion();
    private native void setSurfaceNative(Surface surface);

    // ##############  SurfaceView 相关的

    public void setSurfaceView(SurfaceView surfaceView) {
        if (null != this.surfaceHolder) {
            this.surfaceHolder.removeCallback(this);
        }
        this.surfaceHolder = surfaceView.getHolder();
        this.surfaceHolder.addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }
    // Surface创建 回调
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        // SufaceView 的 surfaceHolder Surface对象 给Native（Native好去渲染屏幕）
//        Surface surface = surfaceHolder.getSurface();
//        setSurfaceNative(surface);
        setSurfaceNative(holder.getSurface());
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }
}
