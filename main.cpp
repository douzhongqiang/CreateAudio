#include <QtWidgets/QApplication>
#include <QFile>
#include "AudioUI/AudioUI.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	
	QFile nFile("./styles/DarkStyle.css");
	if (nFile.open(QFile::ReadOnly))
	{
		QString styleSheetString = nFile.readAll().data();
		a.setStyleSheet(styleSheetString);
	}

	AudioUI w;
	w.resize(600, 220);
	w.show();

	return a.exec();
}
