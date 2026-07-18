// main.cpp
#include <QApplication>
#include "MainWindow.h"
#include "VCDUFileReader.h"

// QString EAstring ;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString appDir = QCoreApplication::applicationDirPath();
    {
        QFile qssFile(appDir + "/resources/styles/vscode_dark.qss");
        if(qssFile.open(QFile::ReadOnly))
        {
            QString styleSheet = QString::fromUtf8(qssFile.readAll());
            a.setStyleSheet(styleSheet);
            qssFile.close();
        }
        else
        {
            qWarning() << "无法打开QSS文件:" << qssFile.fileName();
        }
    } 

    MainWindow w;
    w.show();
    return a.exec();
}