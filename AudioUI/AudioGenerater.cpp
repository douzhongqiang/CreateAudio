#include "AudioGenerater.h"
#include <QDebug>

AudioGenerater::AudioGenerater(QObject *parent)
	:QObject(parent)
{
	av_register_all();
	avcodec_register_all();
}

AudioGenerater::~AudioGenerater()
{

}

void AudioGenerater::generateAudioFile(AudioFormatType formatType, QString fileName, QByteArray pcmData)
{
	AVCodecID codecID = AV_CODEC_ID_PCM_S16LE;
	if (formatType == AudioFormat_MP3)
		codecID = AV_CODEC_ID_MP3;

	// 查找Codec
	m_AudioCodec = avcodec_find_encoder(codecID);
	if (m_AudioCodec == nullptr)
		return;

	// 创建编码器上下文
	m_AudioCodecContext = avcodec_alloc_context3(m_AudioCodec);
	if (m_AudioCodecContext == nullptr)
		return;

	// 设置参数
	m_AudioCodecContext->bit_rate = 64000;
	m_AudioCodecContext->sample_rate = 44100;
	if (formatType == AudioFormat_WAV)
		m_AudioCodecContext->sample_fmt = AV_SAMPLE_FMT_S16;
	else
		m_AudioCodecContext->sample_fmt = AV_SAMPLE_FMT_S16P;
	m_AudioCodecContext->channels = 2;
	m_AudioCodecContext->channel_layout = av_get_default_channel_layout(2);
	m_AudioCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	// 打开编码器
	int result = avcodec_open2(m_AudioCodecContext, m_AudioCodec, nullptr);
	if (result < 0)	
		goto end;

	// 创建封装
	if (!initFormat(fileName))
		goto end;

	// 写入文件头
	result = avformat_write_header(m_FormatContext, nullptr);
	if (result < 0)
		goto end;
	
	// 创建Frame
	AVFrame *frame = av_frame_alloc();
	if (frame == nullptr)
		goto end;

	int nb_samples = 0;
	if (m_AudioCodecContext->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
		nb_samples = 10000;
	else
		nb_samples = m_AudioCodecContext->frame_size;

	// 设置Frame的参数
	frame->nb_samples = nb_samples;
	frame->format = m_AudioCodecContext->sample_fmt;
	frame->channel_layout = m_AudioCodecContext->channel_layout;

	// 申请数据内存
	result = av_frame_get_buffer(frame, 0);
	if (result < 0)
	{
		av_frame_free(&frame);
		goto end;
	}

	// 设置Frame为可写
	result = av_frame_make_writable(frame);
	if (result < 0)
	{
		av_frame_free(&frame);
		goto end;
	}

	int perFrameDataSize = frame->linesize[0];
	if (formatType == AudioFormat_MP3)
		perFrameDataSize *= 2;
	int count = pcmData.size() / perFrameDataSize;
	bool needAddOne = false;
	if (pcmData.size() % perFrameDataSize != 0)
	{
		count++;
		needAddOne = true;
	}

	int frameCount = 0;
	for (int i = 0; i < count; ++i)
	{
		// 创建Packet
		AVPacket *pkt = av_packet_alloc();
		if (pkt == nullptr)
			goto end;
		av_init_packet(pkt);

		if (i == count - 1)
			perFrameDataSize = pcmData.size() % perFrameDataSize;

		// 设置数据
		if (formatType == AudioFormat_WAV)
		{
			// 合成WAV文件
			memset(frame->data[0], 0, perFrameDataSize);
			memcpy(frame->data[0], &(pcmData.data()[perFrameDataSize * i]), perFrameDataSize);
		}
		else
		{
			memset(frame->data[0], 0, frame->linesize[0]);
			memset(frame->data[1], 0, frame->linesize[0]);
#if 1
			// 合成MP3文件
			int channelLayout = av_get_default_channel_layout(2);

			// 格式转换 S16->S16P
			SwrContext *swrContext = swr_alloc_set_opts(nullptr, channelLayout, AV_SAMPLE_FMT_S16P, 44100, \
				channelLayout, AV_SAMPLE_FMT_S16, 44100, 0, nullptr);
			swr_init(swrContext);

			uchar *pSrcData[1] = {0};
			pSrcData[0] = (uchar*)&(pcmData.data()[perFrameDataSize * i]);

			swr_convert(swrContext, frame->data, frame->nb_samples, \
				(const uint8_t **)pSrcData, frame->nb_samples);

			swr_free(&swrContext);
			AVRational rational;
			rational.den = m_AudioCodecContext->sample_rate;
			rational.num = 1;
			//frame->pts = av_rescale_q(0, rational, m_AudioCodecContext->time_base);
#endif
		}

		frame->pts = frameCount++;
		// 发送Frame
		result = avcodec_send_frame(m_AudioCodecContext, frame);
		if (result < 0)
			continue;

		// 接收编码后的Packet
		result = avcodec_receive_packet(m_AudioCodecContext, pkt);
		if (result < 0)
		{
			av_packet_free(&pkt);
			continue;
		}

		// 写入文件
		av_packet_rescale_ts(pkt, m_AudioCodecContext->time_base, m_FormatContext->streams[0]->time_base);
		pkt->stream_index = 0;
		result = av_interleaved_write_frame(m_FormatContext, pkt);
		if (result < 0)
			continue;

		av_packet_free(&pkt);
	}

	// 写入文件尾
	av_write_trailer(m_FormatContext);
	// 关闭文件IO
	avio_closep(&m_FormatContext->pb);
	// 释放Frame内存
	av_frame_free(&frame);
end:
	avcodec_free_context(&m_AudioCodecContext);
	if (m_FormatContext != nullptr)
		avformat_free_context(m_FormatContext);
}

bool AudioGenerater::initFormat(QString audioFileName)
{
	// 创建输出 Format 上下文
	int result = avformat_alloc_output_context2(&m_FormatContext, nullptr, nullptr, audioFileName.toLocal8Bit().data());
	if (result < 0)
		return false;

	m_OutputFormat = m_FormatContext->oformat;

	// 创建音频流
	AVStream *audioStream = avformat_new_stream(m_FormatContext, m_AudioCodec);
	if (audioStream == nullptr)
	{
		avformat_free_context(m_FormatContext);
		return false;
	}
	
	// 设置流中的参数
	audioStream->id = m_FormatContext->nb_streams - 1;
	audioStream->time_base = { 1, 44100 };
	result = avcodec_parameters_from_context(audioStream->codecpar, m_AudioCodecContext);
	if (result < 0)
	{
		avformat_free_context(m_FormatContext);
		return false;
	}

	// 打印FormatContext信息
	av_dump_format(m_FormatContext, 0, audioFileName.toLocal8Bit().data(), 1);

	// 打开文件IO
	if (!(m_OutputFormat->flags & AVFMT_NOFILE))
	{
		result = avio_open(&m_FormatContext->pb, audioFileName.toLocal8Bit().data(), AVIO_FLAG_WRITE);
		if (result < 0)
		{
			avformat_free_context(m_FormatContext);
			return false;
		}
	}

	return true;
}
