#include <jni.h>
#include <string>
#include <android/log.h>
#include "com_example_zjf_ffmpeg_FFmpeg.h"
extern "C" {
#include <libavutil/log.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#define TAG "FFmpegAddBgm"
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR, TAG, FORMAT, ##__VA_ARGS__);
#define LOGD(FORMAT,...) __android_log_print(ANDROID_LOG_DEBUG, TAG, FORMAT, ##__VA_ARGS__);
};

/*JNIEXPORT jint JNICALL Java_com_example_zjf_ffmpeg_FFmpeg_addBgm
        (JNIEnv *env, jobject obj, jstring input_video, jstring input_music, jstring output_video){
    AVOutputFormat *ofmt = NULL;
    //Input AVFormatContext and Output AVFormatContext
    AVFormatContext *ifmt_ctx_v = NULL, *ifmt_ctx_a = NULL,*ofmt_ctx = NULL;
    AVPacket pkt;
    int ret, i;
    int videoindex_v=-1,videoindex_out=-1;
    int audioindex_a=-1,audioindex_out=-1;
    int frame_index=0;
    int64_t cur_pts_v=0,cur_pts_a=0;


    const char *in_filename_v =  env->GetStringUTFChars(input_video,NULL);
    const char *in_filename_a =  env->GetStringUTFChars(input_music,NULL);

    const char *out_filename = env->GetStringUTFChars(output_video,NULL);;//Output file URL
    av_register_all();
    //Input
    if ((ret = avformat_open_input(&ifmt_ctx_v, in_filename_v, 0, 0)) < 0) {//打开输入的视频文件
        LOGE( "Could not open input file.");
        goto end;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx_v, 0)) < 0) {//获取视频文件信息
        LOGE( "Failed to retrieve input stream information");
        goto end;
    }

    if ((ret = avformat_open_input(&ifmt_ctx_a, in_filename_a, 0, 0)) < 0) {//打开输入的音频文件
        LOGE( "Could not open input file.");
        goto end;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx_a, 0)) < 0) {//获取音频文件信息
        LOGE( "Failed to retrieve input stream information");
        goto end;
    }
    LOGE("===========Input Information==========\n");
    av_dump_format(ifmt_ctx_v, 0, in_filename_v, 0);
    av_dump_format(ifmt_ctx_a, 0, in_filename_a, 0);
    LOGE("======================================\n");
    //Output
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);//初始化输出码流的AVFormatContext。
    if (!ofmt_ctx) {
        LOGE( "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        return -1;
    }
    ofmt = ofmt_ctx->oformat;

    //从输入的AVStream中获取一个输出的out_stream
    for (i = 0; i < ifmt_ctx_v->nb_streams; i++) {
        if(ifmt_ctx_v->streams[i]->codecpar->codec_type ==AVMEDIA_TYPE_VIDEO){
            AVStream *in_stream = ifmt_ctx_v->streams[i];

            //创建输出流通道AVStream
            AVCodecContext *pCodecCtx = avcodec_alloc_context3(NULL);
            if (pCodecCtx == NULL)
            {
                printf("Could not allocate AVCodecContext\n");
                return -1;
            }
            avcodec_parameters_to_context(pCodecCtx, in_stream->codecpar);

            AVStream *out_stream = avformat_new_stream(ofmt_ctx,pCodecCtx->codec);

            videoindex_v=i;
            if (!out_stream) {
                LOGE( "Failed allocating output stream\n");
                ret = AVERROR_UNKNOWN;
                break;
            }
            videoindex_out=out_stream->index;
            //Copy the settings of AVCodecContext
            if (ret = avcodec_parameters_from_context(out_stream->codecpar, pCodecCtx) < 0) {
                printf("Failed to copy codec context to out_stream codecpar context\n");
                goto end;
            }

            pCodecCtx->codec_tag = 0;
            if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
            break;
        }
    }

    for (i = 0; i < ifmt_ctx_a->nb_streams; i++) {
        //Create output AVStream according to input AVStream
        //if(ifmt_ctx_a->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO){
        if(ifmt_ctx_a->streams[i]->codecpar->codec_type ==AVMEDIA_TYPE_AUDIO){
            AVStream *in_stream = ifmt_ctx_a->streams[i];
            //创建输出流通道AVStream
            AVCodecContext *pCodecCtx = avcodec_alloc_context3(NULL);
            if (pCodecCtx == NULL)
            {
                printf("Could not allocate AVCodecContext\n");
                return -1;
            }
            avcodec_parameters_to_context(pCodecCtx, in_stream->codecpar);

            AVStream *out_stream = avformat_new_stream(ofmt_ctx,pCodecCtx->codec);
            audioindex_a=i;
            if (!out_stream) {
                LOGE( "Failed allocating output stream\n");
                ret = AVERROR_UNKNOWN;
                goto end;
            }
            audioindex_out=out_stream->index;
            //Copy the settings of AVCodecContext
            if (ret = avcodec_parameters_from_context(out_stream->codecpar, pCodecCtx) < 0) {
                printf("Failed to copy codec context to out_stream codecpar context\n");
                goto end;
            }

            pCodecCtx->codec_tag = 0;
            if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
            break;
        }
    }

    LOGE("==========Output Information==========\n");
    av_dump_format(ofmt_ctx, 0, out_filename, 1);
    LOGE("======================================\n");
    //Open output file
    if (!(ofmt->flags & AVFMT_NOFILE)) {
        if (avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE) < 0) {//打开输出文件。
            LOGE( "Could not open output file '%s'", out_filename);
            return -1;
        }
    }
    //Write file header
    if (avformat_write_header(ofmt_ctx, NULL) < 0) {
        LOGE( "Error occurred when opening output file\n");
        return -1;
    }


    //FIX
#if USE_H264BSF
    AVBitStreamFilterContext* h264bsfc =  av_bitstream_filter_init("h264_mp4toannexb");
#endif
#if USE_AACBSF
    AVBitStreamFilterContext* aacbsfc =  av_bitstream_filter_init("aac_adtstoasc");
#endif

    while (1) {
        AVFormatContext *ifmt_ctx;
        int stream_index=0;
        AVStream *in_stream, *out_stream;

        //Get an AVPacket .   av_compare_ts是比较时间戳用的。通过该函数可以决定该写入视频还是音频。
        if(av_compare_ts(cur_pts_v,ifmt_ctx_v->streams[videoindex_v]->time_base,cur_pts_a,ifmt_ctx_a->streams[audioindex_a]->time_base) <= 0){
            ifmt_ctx=ifmt_ctx_v;
            stream_index=videoindex_out;

            if(av_read_frame(ifmt_ctx, &pkt) >= 0){
                do{
                    in_stream  = ifmt_ctx->streams[pkt.stream_index];
                    out_stream = ofmt_ctx->streams[stream_index];

                    if(pkt.stream_index==videoindex_v){
                        //FIX：No PTS (Example: Raw H.264) H.264裸流没有PTS，因此必须手动写入PTS
                        //Simple Write PTS
                        if(pkt.pts==AV_NOPTS_VALUE){
                            //Write PTS
                            AVRational time_base1=in_stream->time_base;
                            //Duration between 2 frames (us)
                            int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(in_stream->r_frame_rate);
                            //Parameters
                            pkt.pts=(double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
                            pkt.dts=pkt.pts;
                            pkt.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
                            frame_index++;
                        }

                        cur_pts_v=pkt.pts;
                        break;
                    }
                }while(av_read_frame(ifmt_ctx, &pkt) >= 0);
            }else{
                break;
            }
        }else{
            LOGE("写音频数据");
            ifmt_ctx=ifmt_ctx_a;
            stream_index=audioindex_out;
            if(av_read_frame(ifmt_ctx, &pkt) >= 0){
                do{
                    in_stream  = ifmt_ctx->streams[pkt.stream_index];
                    out_stream = ofmt_ctx->streams[stream_index];

                    if(pkt.stream_index==audioindex_a){

                        //FIX：No PTS
                        //Simple Write PTS
                        if(pkt.pts == AV_NOPTS_VALUE){

                            //Write PTS
                            AVRational time_base1=in_stream->time_base;
                            //Duration between 2 frames (us)
                            int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(in_stream->r_frame_rate);
                            //Parameters
                            pkt.pts=(double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
                            pkt.dts=pkt.pts;
                            pkt.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
                            frame_index++;
                        }
                        cur_pts_a = pkt.pts;
                        LOGE("cur_pts_a === %lld",cur_pts_a);
                        break;
                    }
                }while(av_read_frame(ifmt_ctx, &pkt) >= 0);
            }else{
                break;
            }

        }

        //FIX:Bitstream Filter
#if USE_H264BSF
        av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
#if USE_AACBSF
        av_bitstream_filter_filter(aacbsfc, out_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif

        LOGE("pkt.pts = %lld",pkt.pts);
        //Convert PTS/DTS
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        LOGE("pkt.pts == %lld",pkt.pts);
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        pkt.stream_index=stream_index;

        LOGE("Write 1 Packet. size:%5d\tpts:%lld\n",pkt.size,pkt.pts);
        //Write AVPacket 音频或视频裸流
        if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
            LOGE( "Error muxing packet\n");
            break;
        }
        av_packet_unref(&pkt);
        //av_free_packet(&pkt);
    }
    //Write file trailer
    av_write_trailer(ofmt_ctx);

#if USE_H264BSF
    av_bitstream_filter_close(h264bsfc);
#endif
#if USE_AACBSF
    av_bitstream_filter_close(aacbsfc);
#endif

    end:
    avformat_close_input(&ifmt_ctx_v);
    avformat_close_input(&ifmt_ctx_a);
    *//* close output *//*
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    if (ret < 0 && ret != AVERROR_EOF) {
        LOGE( "Error occurred.\n");
        return -1;
    }
    return 0;
}*/


JNIEXPORT jint JNICALL Java_com_example_zjf_ffmpeg_FFmpeg_addBgm
        (JNIEnv *env, jobject obj, jstring input_video, jstring input_music, jstring output_file){
    AVOutputFormat *pOutputFmt = NULL;
    AVFormatContext *pVideoInFmtCxt = NULL;//输入的视频的FormatContext
    AVFormatContext *pAudioInFmtCxt = NULL;//输入的音频的FormatContext
    AVFormatContext *pOutFmtCxt = NULL;//输出的音视频的FormatContext
    AVPacket pkt;
    int ret, i;
    int videoindex_v = -1,videoindex_out = -1;
    int audioindex_a = -1,audioindex_out = -1;
    int frame_index = 0;
    int64_t cur_pts_v = 0;//视频的pts
    int64_t cur_pts_a = 0;//音频的pts

    char errorbuf[1024] = { 0 };

    const char *pVideoInFileName = env->GetStringUTFChars(input_video,NULL);
    const char *pAudioInFileName = env ->GetStringUTFChars(input_music,NULL);
    const char *pVideoOutFilePath = env->GetStringUTFChars(output_file,NULL);

    LOGD("pVideoOutFilePath === %s",pVideoOutFilePath);
    av_register_all();

    //打开输入的视频文件
    if ((ret = avformat_open_input(&pVideoInFmtCxt,pVideoInFileName,NULL,NULL)) < 0) {
        LOGE( "Could not open input video file.");
        goto end;
    }
    //获取视频文件信息
    if ((ret = avformat_find_stream_info(pVideoInFmtCxt,NULL)) < 0) {
        LOGE( "Failed to retrieve input video stream information");
        goto end;
    }

    //打开输入的音频文件
    if ((ret = avformat_open_input(&pAudioInFmtCxt,pAudioInFileName,NULL,NULL)) < 0) {
        LOGE( "Could not open input audio file.");
        goto end;
    }
    //获取音频文件信息
    if ((ret = avformat_find_stream_info(pAudioInFmtCxt,NULL)) < 0) {
        LOGE( "Failed to retrieve input stream information");
        goto end;
    }

    LOGE("===========Input Information==========\n");
    av_dump_format(pVideoInFmtCxt, 0, pVideoInFileName, 0);
    av_dump_format(pAudioInFmtCxt, 0, pAudioInFileName, 0);
    LOGE("======================================\n");

    //初始化输出码流的AVFormatContext
    avformat_alloc_output_context2(&pOutFmtCxt,NULL,NULL,pVideoOutFilePath);
    if (!pOutFmtCxt) {
        LOGE( "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        return -1;
    }

    //输出格式赋值
    pOutputFmt = pOutFmtCxt->oformat;

    //从输入的AVStream中获取一个输出的out_stream，视频输出流
    for (i = 0;i < pVideoInFmtCxt->nb_streams;i++){
        //根据输入的视频流创建一个输出的视频流
        if (pVideoInFmtCxt->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            AVStream *in_stream = pVideoInFmtCxt->streams[i];
            //创建输出流通道AVStream
            AVCodecContext *pCodecCtx = avcodec_alloc_context3(NULL);
            if (pCodecCtx == NULL)
            {
                printf("Could not allocate AVCodecContext\n");
                return -1;
            }
            avcodec_parameters_to_context(pCodecCtx, in_stream->codecpar);

            AVStream *out_stream = avformat_new_stream(pOutFmtCxt,pCodecCtx->codec);
            videoindex_v = i;
            if (!out_stream) {
                LOGE( "Failed allocating output stream\n");
                ret = AVERROR_UNKNOWN;
                break;
            }
            videoindex_out = out_stream->index;

            //Copy the settings of AVCodecContext
            if ((ret = avcodec_parameters_from_context(out_stream->codecpar, pCodecCtx)) < 0) {
                printf("Failed to copy codec context to out_stream codecpar context\n");
                goto end;
            }

            pCodecCtx->codec_tag = 0;
            if (pOutFmtCxt->oformat->flags & AVFMT_GLOBALHEADER)
                pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
            break;
        }
    }

    //从输入的AVStream中获取一个输出的out_stream，音频输出流
    for (i = 0;i < pAudioInFmtCxt->nb_streams;i++){
        if (pAudioInFmtCxt->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
            AVStream *in_stream = pAudioInFmtCxt->streams[i];
            //创建输出流通道AVStream
            AVCodecContext *pCodecCtx = avcodec_alloc_context3(NULL);
            if (pCodecCtx == NULL)
            {
                printf("Could not allocate AVCodecContext\n");
                return -1;
            }
            avcodec_parameters_to_context(pCodecCtx, in_stream->codecpar);
            AVStream *out_stream = avformat_new_stream(pOutFmtCxt,pCodecCtx->codec);

            audioindex_a = i;
            if (!out_stream) {
                LOGE( "Failed allocating output stream\n");
                ret = AVERROR_UNKNOWN;
                goto end;
            }
            audioindex_out = out_stream->index;

            if ((ret = avcodec_parameters_from_context(out_stream->codecpar, pCodecCtx)) < 0) {
                printf("Failed to copy codec context to out_stream codecpar context\n");
                goto end;
            }

            pCodecCtx->codec_tag = 0;
            if (pOutFmtCxt->oformat->flags & AVFMT_GLOBALHEADER)
                pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
            break;
        }
    }

    LOGE("==========Output Information==========\n");
    av_dump_format(pOutFmtCxt, 0, pVideoOutFilePath, 1);
    LOGE("======================================\n");

    //打开输出文件
    if (!(pOutputFmt->flags & AVFMT_NOFILE)) {
        if (avio_open(&pOutFmtCxt->pb, pVideoOutFilePath, AVIO_FLAG_WRITE) < 0) {//打开输出文件。
            LOGE( "Could not open output file '%s'", pVideoOutFilePath);
            return -1;
        }
    }

    //写文件头
    if (avformat_write_header(pOutFmtCxt, NULL) < 0) {
        LOGE( "Error occurred when opening output file\n");
        return -1;
    }

    //FIX
#if USE_H264BSF
    AVBitStreamFilterContext* h264bsfc =  av_bitstream_filter_init("h264_mp4toannexb");
#endif
#if USE_AACBSF
    AVBitStreamFilterContext* aacbsfc =  av_bitstream_filter_init("aac_adtstoasc");
#endif

    while(1){
        AVFormatContext *pInFmtCtx;
        int stream_index = 0;
        AVStream *in_stream,*out_stream;
        // av_compare_ts是比较时间戳用的。通过该函数可以决定该写入视频还是音频
        if (av_compare_ts(cur_pts_v,pVideoInFmtCxt->streams[videoindex_v]->time_base,
                    cur_pts_a,pAudioInFmtCxt->streams[audioindex_a]->time_base) <= 0) {
            LOGE("写视频数据");
            pInFmtCtx = pVideoInFmtCxt;//视频
            //这里要赋值了，注意注意
            stream_index = videoindex_out;

            if (av_read_frame(pInFmtCtx,&pkt) >= 0) {//读取流
                do {
                    in_stream  = pInFmtCtx->streams[pkt.stream_index];
                    out_stream = pOutFmtCxt->streams[stream_index];

                    if(pkt.stream_index == videoindex_v){
                        // H.264裸流没有PTS，因此必须手动写入PTS
                        if(pkt.pts == AV_NOPTS_VALUE){
                            //写PTS
                            AVRational time_base1 = in_stream->time_base;
                            //Duration between 2 frames (us)
                            int64_t calc_duration = (double)AV_TIME_BASE/av_q2d(in_stream->r_frame_rate);
                            //Parameters
                            pkt.pts = (double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
                            pkt.dts=pkt.pts;
                            pkt.duration = (double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
                            frame_index++;
                        }
                        cur_pts_v = pkt.pts;
                        LOGE("cur_pts_v === %lld",cur_pts_v);
                        break;
                    }
                }while (av_read_frame(pInFmtCtx,&pkt) >= 0);
            } else {
                break;
            }
        } else {
            LOGE("写音频数据");
            pInFmtCtx = pAudioInFmtCxt;
            stream_index = audioindex_out;
            if(av_read_frame(pInFmtCtx, &pkt) >= 0){
                do{
                    in_stream  = pInFmtCtx->streams[pkt.stream_index];
                    out_stream = pOutFmtCxt->streams[stream_index];

                    if(pkt.stream_index == audioindex_a){
                        //FIX：No PTS
                        //Simple Write PTS
                        if(pkt.pts==AV_NOPTS_VALUE){
                            //Write PTS
                            AVRational time_base1=in_stream->time_base;
                            //Duration between 2 frames (us)
                            int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(in_stream->r_frame_rate);
                            //Parameters
                            pkt.pts = (double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
                            pkt.dts = pkt.pts;
                            pkt.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
                            frame_index++;
                        }
                        cur_pts_a = pkt.pts;
                        LOGE("cur_pts_a === %lld",cur_pts_a);
                        break;
                    }
                }while(av_read_frame(pInFmtCtx, &pkt) >= 0);
            } else {
                break;
            }
        }

        //FIX:Bitstream Filter
#if USE_H264BSF
        av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
#if USE_AACBSF
        av_bitstream_filter_filter(aacbsfc, out_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
        LOGE("pkt.pts = %lld ",pkt.pts);
        //Convert PTS/DTS
        pkt.pts = av_rescale_q_rnd(pkt.pts,in_stream->time_base,out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        LOGE("pkt.pts == %lld",pkt.pts);
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration,in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        pkt.stream_index = stream_index;
        LOGE("Write 1 Packet. size:%5d\tpts:%lld\n",pkt.size,pkt.pts);
        //Write AVPacket 音频或视频裸流
        if ((ret = av_interleaved_write_frame(pOutFmtCxt, &pkt)) < 0) {
            av_strerror(ret,errorbuf, sizeof(errorbuf));
            LOGE("ERROR : %s",errorbuf);
            LOGE( "Error muxing packet error code === %d\n",ret);
            av_packet_unref(&pkt);
            break;
        }
        av_packet_unref(&pkt);
    }

    //写文件尾
    av_write_trailer(pOutFmtCxt);

#if USE_H264BSF
    av_bitstream_filter_close(h264bsfc);
#endif
#if USE_AACBSF
    av_bitstream_filter_close(aacbsfc);
#endif

    end:
    avformat_close_input(&pVideoInFmtCxt);
    avformat_close_input(&pAudioInFmtCxt);
    if (pOutFmtCxt && !(pOutputFmt->flags & AVFMT_NOFILE))
        avio_close(pOutFmtCxt->pb);
    avformat_free_context(pOutFmtCxt);
    if (ret < 0 && ret != AVERROR_EOF) {
        LOGE( "Error occurred.\n");
        return -1;
    }
    return 0;
}
