package spt.ffmpeg

import android.view.Surface

/**
 * Created by YangJ on 2019/4/1.
 */
class FFmpegTools {

    private constructor()

    /**
     * 获取FFmpeg的配置信息
     */
    external fun configuration(): String

    /**
     * 将输入的视频数据解码成YUV像素数据
     */
    external fun decoder(inputPath: String, outputPath: String): Int

    /**
     * 播放本地视频
     */
    external fun playVideoByPath(path: String, surface: Surface)

    private object SingletonHolder {
        val INSTANCE = FFmpegTools()
    }

    companion object {

        @JvmStatic
        fun getInstance(): FFmpegTools {
            return SingletonHolder.INSTANCE
        }

        init {
            System.loadLibrary("avcodec-56")
            System.loadLibrary("avdevice-56")
            System.loadLibrary("avformat-56")
            System.loadLibrary("avutil-54")
            System.loadLibrary("avfilter-5")
            System.loadLibrary("postproc-53")
            System.loadLibrary("swresample-1")
            System.loadLibrary("swscale-3")
            //
            System.loadLibrary("native-lib")
        }
    }

}