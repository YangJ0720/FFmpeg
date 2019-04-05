#include <jni.h>
#include <string>
#include <zconf.h>
#include "android/log.h"
#include <android/native_window_jni.h>

static const char *TAG = "Aicare";
#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO,  TAG, fmt, ##args)
#define LOGE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, TAG, fmt, ##args)

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

#include "libswscale/swscale.h"
}

extern "C" {

    // 获取FFmpeg的配置信息
    JNIEXPORT jstring JNICALL Java_spt_ffmpeg_FFmpegTools_configuration(JNIEnv *env, jobject j_obj) {
        std::string hello = avcodec_configuration();
        return env->NewStringUTF(hello.c_str());
    }

    void custom_log(void *ptr, int level, const char* fmt, va_list vl){
        FILE *fp=fopen("/storage/emulated/0/av_log.txt","a+");
        if(fp){
            vfprintf(fp,fmt,vl);
            fflush(fp);
            fclose(fp);
        }
    }

    // 将输入的视频数据解码成YUV像素数据
    JNIEXPORT int JNICALL Java_spt_ffmpeg_FFmpegTools_decoder(JNIEnv *env, jobject j_obj,
            jstring j_input, jstring j_output) {


        return 0;
    }

    JNIEXPORT void JNICALL
    Java_spt_ffmpeg_FFmpegTools_playVideoByPath(JNIEnv *env, jobject j_obj, jstring j_path,
                                                 jobject j_surface) {
        // 将Java字符串转换为C字符串
        const char *path = env->GetStringUTFChars(j_path, JNI_FALSE);
        if (path == NULL) {
            LOGE("路径转换失败");
            return;
        }
        // 注册所有组件
        av_register_all();
        // 获取上下文对象
        AVFormatContext *avFormatContext = avformat_alloc_context();
        // 打开视频文件
        int error = avformat_open_input(&avFormatContext, path, NULL, NULL);
        LOGI("error = %d", error);
        if (error < 0) {
            LOGE("打开视频失败");
            char buffer[128];
            if (av_strerror(error, buffer, sizeof(buffer)) == 0) {
                LOGE("错误信息: %s", buffer);
            }
            return;
        }
        //
        if (avformat_find_stream_info(avFormatContext, NULL) < 0) {
            LOGE("读取视频流信息失败");
            return;
        }
        // 查找视频流
        int video_index = 0;
        for (int i = 0; i < avFormatContext->nb_streams; ++i) {
            // 音频流 -> AVMEDIA_TYPE_AUDIO
            // 字幕流 -> AVMEDIA_TYPE_SUBTITLE
            if (AVMEDIA_TYPE_VIDEO == avFormatContext->streams[i]->codec->codec_type) {
                // 找到视频流
                video_index = i;
                break;
            }
        }
        LOGI("video_index = %d", video_index);
        if (video_index == -1) {
            LOGE("没有找到视频流");
            return;
        }
        // 获取解码器上下文对象
        AVCodecContext *avCodecContext = avFormatContext->streams[video_index]->codec;
        // 获取解码器
        AVCodec *avCodec = avcodec_find_decoder(avCodecContext->codec_id);
        if (avCodec == NULL) {
            LOGE("没有找到对应的解码器");
            return;
        }
        // 打开解码器
        if (avcodec_open2(avCodecContext, avCodec, NULL) < 0) {
            LOGE("解码器开启失败");
            return;
        } else {
            LOGI("解码器开启成功");
        }
        // 申请AVPacket
        AVPacket *avPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
        av_init_packet(avPacket);
        LOGI("申请AVPacket");
        // 申请AVFrame
        AVFrame *avFrame = av_frame_alloc();
        AVFrame *rgb_frame = av_frame_alloc();
        LOGI("申请AVFrame");
        // 缓冲区
        uint8_t *out_buffer = (uint8_t *) av_malloc(avpicture_get_size(AV_PIX_FMT_RGBA,
                avCodecContext->width, avCodecContext->height));
        // 设置rgb_frame缓存区
        avpicture_fill((AVPicture *) rgb_frame, out_buffer, AV_PIX_FMT_RGBA,
                       avCodecContext->width, avCodecContext->height);
        LOGI("设置rgb_frame缓存区");
        // 初始化ANativeWindow
        ANativeWindow *aNativeWindow = ANativeWindow_fromSurface(env, j_surface);
        if (aNativeWindow == 0) {
            LOGE("ANativeWindow初始化失败");
            return;
        } else {
            LOGI("ANativeWindow初始化成功");
        }
        // 视频缓冲区
        ANativeWindow_Buffer native_out_buffer;
        LOGI("声明视频缓冲区ANativeWindow_Buffer");
        LOGI("avCodecContext->width = %d", avCodecContext->width);
        LOGI("avCodecContext->height = %d", avCodecContext->height);
        SwsContext* swsContext = sws_getContext(avCodecContext->width, avCodecContext->height,
                avCodecContext->pix_fmt, avCodecContext->width,
                avCodecContext->height, AV_PIX_FMT_RGBA, SWS_BICUBIC, NULL, NULL, NULL);
        LOGI("初始化SwsContext");
        // 开始解码
        int frameCount;
        LOGI("frameCount = %d", frameCount);
        while (av_read_frame(avFormatContext, avPacket) >= 0) {
            LOGI("解码 -> %d", avPacket->stream_index);
            // 判断是否为视频流
            if (avPacket->stream_index == video_index) {
                avcodec_decode_video2(avCodecContext, avFrame, &frameCount, avPacket);
                if (frameCount) {
                    LOGI("转换并绘制");
                    ANativeWindow_setBuffersGeometry(aNativeWindow, avCodecContext->width,
                            avCodecContext->height, WINDOW_FORMAT_RGBA_8888);
                    // 加锁
                    ANativeWindow_lock(aNativeWindow, &native_out_buffer, NULL);
                    // 转换为rgb格式
                    sws_scale(swsContext, (const uint8_t *const *) avFrame->data, avFrame->linesize,
                              0, avFrame->height, rgb_frame->data, rgb_frame->linesize);
                    uint8_t *dst = (uint8_t *) native_out_buffer.bits;
                    // 拿到一行有多少个字节 RGBA
                    int destStride = native_out_buffer.stride * 4;
                    // 像素数据的首地址
                    uint8_t *src = rgb_frame->data[0];
                    // 实际内存一行数量
                    int srcStride = rgb_frame->linesize[0];
                    for (int i = 0; i < avCodecContext->height; ++i) {
                        // 将rgb_frame中每一行的数据复制给NativeWindow
                        memcpy(dst + i * destStride,  src + i * srcStride, srcStride);
                    }
                    // 解锁
                    ANativeWindow_unlockAndPost(aNativeWindow);
                    // 休眠
                    usleep(1000 * 16);
                }
            }
            av_free_packet(avPacket);
        }
        // 释放资源
        LOGI("执行资源释放");
        sws_freeContext(swsContext);
        ANativeWindow_release(aNativeWindow);
        av_frame_free(&avFrame);
        av_frame_free(&rgb_frame);
        avcodec_close(avCodecContext);
        avformat_free_context(avFormatContext);
        env->ReleaseStringUTFChars(j_path, path);
    }

}
