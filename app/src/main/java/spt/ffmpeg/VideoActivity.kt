package spt.ffmpeg

import android.graphics.PixelFormat
import android.os.Bundle
import android.support.v7.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_video.*
import java.io.File

/**
 * @author YangJ
 */
class VideoActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_video)
        initView()
    }

    private fun initView() {
        val holder = surfaceView.holder
        holder.setFormat(PixelFormat.RGBA_8888)
        surfaceView.setOnClickListener {
            // 开始播放
            Thread(Runnable {
                FFmpegTools.getInstance().playVideoByPath(
                        File(externalCacheDir, "wx.mp4").absolutePath, holder.surface)
            }).start()
        }
    }
}
