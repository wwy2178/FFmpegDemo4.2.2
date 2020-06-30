package com.example.ffmpegdemo1;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;
import com.scwang.smart.refresh.header.TwoLevelHeader;
import com.scwang.smart.refresh.header.listener.OnTwoLevelListener;
import com.scwang.smart.refresh.layout.api.RefreshLayout;
import com.scwang.smart.refresh.layout.constant.RefreshState;
import com.scwang.smart.refresh.layout.simple.SimpleMultiListener;

import java.io.File;

public class MainActivity extends AppCompatActivity implements SeekBar.OnSeekBarChangeListener {


    private TwoLevelHeader header;
    private RefreshLayout refreshLayout;
    // 下面都是媒体流资源
//    private final static String PATH = Environment.getExternalStorageDirectory() + File.separator + "demo.mp4";
     private final static String PATH = "rtmp://58.200.131.2:1935/livetv/hunantv";

    private WwyPlayer wwyPlayer;

    private SurfaceView surfaceView;
    private SeekBar seekBar;//进度条-与播放总时长挂钩
    private boolean isTouch ;
    private boolean isSeek ;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        surfaceView = findViewById(R.id.surface_view);
        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
        seekBar = findViewById(R.id.seekBar);
//        tv.setText("FFmpeg 当前版本：" + stringFromJNI());

        refreshLayout = (RefreshLayout)findViewById(R.id.refreshLayout);
        header = findViewById(R.id.header);

        refreshLayout.setOnMultiListener(new SimpleMultiListener(){
            @Override
            public void onLoadMore(@NonNull RefreshLayout refreshLayout) {
                Toast.makeText(getApplication(),"上拉加载",Toast.LENGTH_SHORT).show();
                refreshLayout.finishLoadMore(2000);
            }

            @Override
            public void onRefresh(@NonNull RefreshLayout refreshLayout) {
                Toast.makeText(getApplication(),"下拉刷新", Toast.LENGTH_SHORT).show();
                refreshLayout.finishRefresh(2000);
            }

            @Override
            public void onStateChanged(@NonNull RefreshLayout refreshLayout, @NonNull RefreshState oldState, @NonNull RefreshState newState) {
                if (oldState == RefreshState.TwoLevel){
                    findViewById(R.id.second_floor_content).animate().alpha(0).setDuration(1000);
                }
            }
        });
        header.setOnTwoLevelListener(new OnTwoLevelListener() {
            @Override
            public boolean onTwoLevel(@NonNull RefreshLayout refreshLayout) {
                Toast.makeText(getApplication(),"打开二楼",Toast.LENGTH_SHORT).show();
                findViewById(R.id.second_floor_content).animate().alpha(1).setDuration(2000);
                return true;
            }
        });

        wwyPlayer = new WwyPlayer();
        wwyPlayer.setSurfaceView(surfaceView);
        try {
            tv.setText("FFmpeg" + wwyPlayer.getFFmpegVersion());
        }catch (Exception e){
            tv.setText("FFmpeg:" + e.getMessage());
        }

        File file = new File(PATH);

        if (file.exists()){
            Log.e("MainActivity", "---1-> 不存在" );
        }else {
            Log.e("MainActivity", "----2-> 存在");
        }

        wwyPlayer.setDataSource(
//                file.getAbsolutePath();
                PATH
        );
        wwyPlayer.setOnPreparedListener(new WwyPlayer.OnPreparedListener() {
            @Override
            public void onPrepared() {
                int duration = wwyPlayer.getDuration();
                //如果是直播，duration是0
                //不为0，可以显示seekbar
                if (duration != 0) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            seekBar.setVisibility(View.VISIBLE);
                        }
                    });
                }
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Log.e("MainActivity", "开始播放");
                        Toast.makeText(MainActivity.this, "开始播放！", Toast.LENGTH_SHORT).show();
                    }
                });
                //播放 调用到native去
                //start play
                wwyPlayer.start();
            }

            @Override
            public void onError(String errorText) {
                Log.e("MainActivity", "onError: " + errorText);
            }
        });

        wwyPlayer.setOnProgressListener(new WwyPlayer.OnProgressListener() {
            @Override
            public void onProgress(final int progress) {
                //progress: 当前的播放进度
                Log.e("MainActivity", "progress: " + progress);
                //duration

                //非人为干预进度条，让进度条自然的正常播放
                if (!isTouch){
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            int duration = wwyPlayer.getDuration();
                            Log.e("MainActivity", "duration: " + duration);
                            if (duration != 0) {
                                if(isSeek){
                                    isSeek = false;
                                    return;
                                }
                                seekBar.setProgress(progress * 100 / duration);
                            }
                        }
                    });
                }
            }
        });
        seekBar.setOnSeekBarChangeListener(this);

//        Toast.makeText(this, "FFmpeg" + wwyPlayer.getFFmpegVersion(), Toast.LENGTH_SHORT).show();
//        wwyPlayer.setDataSource(PATH);
//        wwyPlayer.setOnPreparedListener(new WwyPlayer.OnPreparedListener() {
//            @Override
//            public void onPrepared() {
//                runOnUiThread(new Runnable() {
//                    @Override
//                    public void run() {
//                        new AlertDialog.Builder(MainActivity.this)
//                                .setTitle("UI")
//                                .setMessage("准备好了，开始播放 ...")
//                                .setPositiveButton("老夫知道了", null)
//                                .show();
//                    }
//                });
//                // 准备成功之后，开始播放 视频 音频
//                wwyPlayer.start();
//            }
//
//            @Override
//            public void onError(final String errorText) {
//                runOnUiThread(new Runnable() {
//                    @Override
//                    public void run() {
//                        new AlertDialog.Builder(MainActivity.this)
//                                .setTitle("Error")
//                                .setMessage("已经发生错误，请查阅:" + errorText)
//                                .setPositiveButton("我来个去，什么情况", null)
//                                .show();
//                    }
//                });
//            }
//        });
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        isTouch = true; // 用户 手触摸到了 拖动条
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        isSeek = true;
        isTouch = false;
        //获取seekbar的当前进度（百分比）
        int seekBarProgress = seekBar.getProgress();
        //将seekbar的进度转换成真实的播放进度
        int duration = wwyPlayer.getDuration();
        int playProgress = seekBarProgress * duration / 100; // 把拖动条的值，变成播放的进度时长
        //将播放进度传给底层 ffmpeg

        // playProgress == 时长 native只认识时长，不认识拖动值

        //seek 的核心思路2
        // 手动拖动进度条，要能跳到指定的播放进度  av_seek_frame
        wwyPlayer.seekTo(playProgress);
    }

    @Override
    protected void onResume() {
        super.onResume();
        wwyPlayer.prepare();
    }

    @Override
    protected void onStop() {
        super.onStop();
        wwyPlayer.stop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        wwyPlayer.release();
    }
}
