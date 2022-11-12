#include "chatlogthread.h"

#include <QFile>
#include <QDateTime>
#include <QTreeWidgetItem>

ChatLogThread::ChatLogThread(QObject *parent)
    : QThread{parent}
{
    filename = "chat_room_log_.txt";
    filename.insert(14, QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
}

ChatLogThread::~ChatLogThread()
{
    QFile file(filename);
    file.close();
}

void ChatLogThread::run()
{
    forever {
        saveData();
        sleep(60);
    }
}

void ChatLogThread::appendData(QString log)
{
    logList.append(log);
}

void ChatLogThread::saveData()
{
    if ( logList.count() > 0 ) {
        QFile file(filename);
        if ( !file.open(QIODevice::WriteOnly | QIODevice::Text ) )  return;

        QTextStream out(&file);
        foreach ( QString item, logList ) {
            out << item << '\n';
        }
    }
}
