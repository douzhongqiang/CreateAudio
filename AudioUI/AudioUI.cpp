#include "AudioUI.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QListView>
#include <QFileDialog>
#include "UIBase/UIGlobalTool.h"
#include "UIBase/UIMessageBox.h"

AudioUI::AudioUI(QWidget *parent)
	:UIBaseWindow(parent)
{
	this->setCustomerWindowTitle("Audio Record Tool");
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addStretch(30);

	// Row1
	QHBoxLayout *row1Layout = new QHBoxLayout;
	m_SelectedAudioType = new QComboBox;
	m_SelectedAudioType->setView(new QListView(m_SelectedAudioType));
	m_SelectedAudioType->addItem("wav");
	m_SelectedAudioType->addItem("mp3");
	g_GlobalTool->addShadowEffect(m_SelectedAudioType);

	QLabel *slectedAudioTypeTag = new QLabel("Audio Format: ");
	row1Layout->addWidget(slectedAudioTypeTag);
	row1Layout->addWidget(m_SelectedAudioType);

	// Row2
	QHBoxLayout *row2Layout = new QHBoxLayout;
	QLabel *fileNameTag = new QLabel("File Name: ");
	m_AudioFileNameEdit = new QLineEdit;
	m_AudioFileNameEdit->setText("Test");
	row2Layout->addWidget(fileNameTag);
	row2Layout->addWidget(m_AudioFileNameEdit);

	// Row3
	QHBoxLayout *row3Layout = new QHBoxLayout;
	QLabel *selectedAudioFilePath = new QLabel("File Path: ");
	m_AudioFilePathEdit = new QLineEdit;
	m_AudioFilePathEdit->setText("./");
	QPushButton *browseButton = new QPushButton("Browse");
	QObject::connect(browseButton, SIGNAL(clicked()), this, SLOT(onClickBrowse()));
	g_GlobalTool->addShadowEffect(browseButton);
	row3Layout->addWidget(selectedAudioFilePath);
	row3Layout->addWidget(m_AudioFilePathEdit);
	row3Layout->addWidget(browseButton);

	// Row3
	QHBoxLayout *row4Layout = new QHBoxLayout;
	QPushButton *startButton = new QPushButton("Start");
	QObject::connect(startButton, SIGNAL(clicked()), this, SLOT(onClickedStart()));
	QPushButton *pauseButton = new QPushButton("Pause");
	QObject::connect(pauseButton, SIGNAL(clicked()), this, SLOT(onClickedPause()));
	QPushButton *stopButton = new QPushButton("Stop");
	QObject::connect(stopButton, SIGNAL(clicked()), this, SLOT(onClickedStop()));
	m_LCDNumber = new QLCDNumber;
	m_LCDNumber->display("00:00");
	row4Layout->addWidget(m_LCDNumber);
	row4Layout->addStretch();
	row4Layout->addWidget(startButton);
	g_GlobalTool->addShadowEffect(startButton);
	row4Layout->addWidget(pauseButton);
	g_GlobalTool->addShadowEffect(pauseButton);
	row4Layout->addWidget(stopButton);
	g_GlobalTool->addShadowEffect(stopButton);

	layout->addLayout(row1Layout);
	layout->addLayout(row2Layout);
	layout->addLayout(row3Layout);
	layout->addLayout(row4Layout);
	layout->addStretch();

	initAudioRecordDevice();
	// Init Timer
	m_Timer = new QTimer(this);
	m_Timer->setInterval(100);
	QObject::connect(m_Timer, SIGNAL(timeout()), this, SLOT(onTimeout()));

	m_AudioGenerater = new AudioGenerater(this);
}

AudioUI::~AudioUI()
{

}

void AudioUI::onClickBrowse(void)
{
	QString path = QFileDialog::getExistingDirectory(this, tr("Select Dir Path"), "./");
	if (path.isEmpty())
		return;

	m_AudioFilePathEdit->setText(path);
}

void AudioUI::initAudioRecordDevice(void)
{
	QAudioFormat format;
	format.setSampleRate(44100);
	format.setChannelCount(2);
	format.setSampleSize(16);
	format.setCodec("audio/pcm");
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleType(QAudioFormat::UnSignedInt);

	m_AudioInput = new QAudioInput(format, this);
}

void AudioUI::onClickedStart(void)
{
	m_AudioRecordByte.clear();
	m_Timer->start();
	m_AudioRecord = m_AudioInput->start();
	m_LCDNumber->display("00:00");
	m_CurrentTime = QTime::currentTime();
}

void AudioUI::onTimeout(void)
{
	// 计算并设置计时
	QTime currentTime = QTime::currentTime();
	int secCount = m_CurrentTime.secsTo(currentTime);
	int min = secCount / 60;
	int sec = secCount % 60;

	QString lcdNum = "%1%2:%3%4";
	lcdNum = lcdNum.arg(min / 10).arg(min % 10).arg(sec / 10).arg(sec % 10);
	m_LCDNumber->display(lcdNum);

	// 读取录制的音频信息到内存中
	readRecordDevice();
}

void AudioUI::readRecordDevice(void)
{
	int byteSize = m_AudioInput->bytesReady();
	int len = byteSize;
	while (len)
	{
		int readSize = qMax(1024, byteSize);
		char *pData = new char[readSize];
		len = m_AudioRecord->read(pData, readSize);

		// 保存数据
		m_AudioRecordByte.append(pData, len);

		delete[] pData;
	}
}

void AudioUI::onClickedPause(void)
{
	if (m_Timer->isActive())
	{
		m_Timer->stop();
		m_AudioInput->stop();
	}
	else
	{
		m_Timer->start();
		m_AudioInput->start();
	}
}

void AudioUI::onClickedStop(void)
{
	m_Timer->stop();
	m_AudioInput->stop();

	generateAudioFile();
}

void AudioUI::generateAudioFile(void)
{
	QString fileName = m_AudioFilePathEdit->text() + "/" + m_AudioFileNameEdit->text();
	AudioGenerater::AudioFormatType formatType = AudioGenerater::AudioFormat_WAV;
	if (m_SelectedAudioType->currentIndex() == 1)
	{
		formatType = AudioGenerater::AudioFormat_MP3;
		fileName += ".mp3";
	}
	else
		fileName += ".wav";

	m_AudioGenerater->generateAudioFile(formatType, fileName, m_AudioRecordByte);

	UIMessageBox messageBox;
	messageBox.abort(tr("Audio Created Successed"), tr("Audio Created Successed!"));
}
