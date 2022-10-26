#include "logthread.h"

#include <QDateTime>
#include <QFile>
#include <QTreeWidgetItem>

LogThread::LogThread(QObject *parent)
    : QThread{parent}
{
    filename = "log_.txt";
    filename.insert(4, QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
}

void LogThread::run()
{
    forever {
        saveData();
        sleep(60);
    }
}

void LogThread::appendData(QTreeWidgetItem* item)
{
    itemList.append(item);
}

void LogThread::saveData()
{
    if(itemList.count() > 0) {
        QFile file(filename);
        if( !file.open(QIODevice::WriteOnly | QIODevice::Text) )    return;

        QTextStream out(&file);
        foreach(QTreeWidgetItem* item, itemList) {
            out << item->text(0) << ','
                << item->text(1) << ','
                << item->text(2) << ','
                << item->text(3) << ','
                << item->text(4) << ','
                << item->text(5) << '\n';

        }
    }
}
