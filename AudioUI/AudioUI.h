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

	// ��Ƶ¼��
	QAudioInput *m_AudioInput = nullptr;
	// ��Ƶ¼������IO
	QIODevice *m_AudioRecord = nullptr;
	// ��ʼ�� Record Device
	void initAudioRecordDevice(void);
	// ��ȡ¼������
	void readRecordDevice(void);

	QByteArray m_AudioRecordByte;

	QTimer *m_Timer = nullptr;
	QTime m_CurrentTime;

	// ��Ƶ�ļ��������
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
