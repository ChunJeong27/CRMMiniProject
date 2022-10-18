#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include "chattingform.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "CRMMiniProjectApp_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();

//    ChattingForm c;
//    c.show();

    return a.exec();
}
