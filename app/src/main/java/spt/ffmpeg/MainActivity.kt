package spt.ffmpeg

import android.content.Intent
import android.os.Bundle
import android.support.v7.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_main.*

/**
 * @author YangJ
 */
class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        initData()
        initView()
    }

    private fun initData() {

    }

    private fun initView() {
        tv_configuration.text = FFmpegTools.getInstance().configuration()
        //
        btn_video.setOnClickListener {
            startActivity(Intent(this, VideoActivity::class.java))
        }
        btn_decoder.setOnClickListener {
            startActivity(Intent(this, DecoderActivity::class.java))
        }
    }

}
