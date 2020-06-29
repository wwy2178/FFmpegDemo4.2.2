package com.example.ffmpegdemo1;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.os.Environment;
import android.view.SurfaceView;
import android.widget.TextView;
import android.widget.Toast;
import com.scwang.smart.refresh.header.TwoLevelHeader;
import com.scwang.smart.refresh.header.listener.OnTwoLevelListener;
import com.scwang.smart.refresh.layout.api.RefreshLayout;
import com.scwang.smart.refresh.layout.constant.RefreshState;
import com.scwang.smart.refresh.layout.simple.SimpleMultiListener;

import java.io.File;

public class MainActivity extends AppCompatActivity {


    private TwoLevelHeader header;
    private RefreshLayout refreshLayout;
    // 下面都是媒体流资源
    private final static String PATH = Environment.getExternalStorageDirectory() + File.separator + "demo.mp4";
    // private final static String PATH = "rtmp://58.200.131.2:1935/livetv/hunantv";

    private WwyPlayer wwyPlayer;

    private SurfaceView surfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        surfaceView = findViewById(R.id.surface_view);
        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
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

}
