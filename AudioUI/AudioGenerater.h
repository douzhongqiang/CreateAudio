#ifndef AUIDO_GENERATER_H
#define AUIDO_GENERATER_H

#include <QObject>
extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
}

class AudioGenerater : public QObject
{
	Q_OBJECT

public:
	enum AudioFormatType
	{
		AudioFormat_WAV,
		AudioFormat_MP3
	};

public:
	AudioGenerater(QObject *parent = nullptr);
	~AudioGenerater();

	// 生成音频文件
	void generateAudioFile(AudioFormatType formatType, QString fileName, QByteArray pcmData);

private:
	AVCodec *m_AudioCodec = nullptr;
	AVCodecContext *m_AudioCodecContext = nullptr;
	AVFormatContext *m_FormatContext = nullptr;
	AVOutputFormat *m_OutputFormat = nullptr;

	// init Format
	bool initFormat(QString audioFileName);
};
#endif
