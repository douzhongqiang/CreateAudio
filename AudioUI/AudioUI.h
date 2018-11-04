#ifndef AUDIO_UI_H
#define AUDIO_UI_H

#include "UIBase/UIBaseWindow.h"
#include "AudioGenerater.h"
#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QLCDNumber>
#include <QAudioFormat>
#include <QAudioInput>
#include <QIODevice>
#include <QTimer>
#include <QTime>
#include <QByteArray>

class AudioUI : public UIBaseWindow
{
	Q_OBJECT

public:
	AudioUI(QWidget *parent = nullptr);
	~AudioUI();

private:
	QComboBox *m_SelectedAudioType = nullptr;
	QLCDNumber *m_LCDNumber = nullptr;
	QLineEdit *m_AudioFileNameEdit = nullptr;
	QLineEdit *m_AudioFilePathEdit = nullptr;

	// 音频录制
	QAudioInput *m_AudioInput = nullptr;
	// 音频录制设置IO
	QIODevice *m_AudioRecord = nullptr;
	// 初始化 Record Device
	void initAudioRecordDevice(void);
	// 读取录制数据
	void readRecordDevice(void);

	QByteArray m_AudioRecordByte;

	QTimer *m_Timer = nullptr;
	QTime m_CurrentTime;

	// 音频文件生成相关
	AudioGenerater *m_AudioGenerater = nullptr;
	void generateAudioFile(void);

private slots:
	void onClickBrowse(void);
	void onClickedStart(void);
	void onClickedPause(void);
	void onClickedStop(void);
	void onTimeout(void);
};
#endif
