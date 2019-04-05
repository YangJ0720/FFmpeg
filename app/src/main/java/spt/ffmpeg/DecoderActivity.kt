package spt.ffmpeg

import android.os.Bundle
import android.support.v7.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_decoder.*
import java.io.File

/**
 * @author YangJ 对视频文件进行转码操作
 */
class DecoderActivity : AppCompatActivity() {

    private lateinit var mInputPath: String
    private lateinit var mOutputPath: String

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_decoder)
        initData()
        initView()
    }

    private fun initData() {
        mInputPath = File(externalCacheDir, "wx.mp4").absolutePath
        mOutputPath = File(externalCacheDir, "wx_decoder.mp4").absolutePath
    }

    private fun initView() {
        tv_input_path.text = mInputPath
        tv_output_path.text = mOutputPath
        btn_decoder.setOnClickListener {
            FFmpegTools.getInstance().decoder(mInputPath, mOutputPath)
        }
    }
}
