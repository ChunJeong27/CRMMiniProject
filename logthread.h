#ifndef LOGTHREAD_H
#define LOGTHREAD_H

#include <QThread>

class QTreeWidgetItem;

class LogThread : public QThread
{
    Q_OBJECT
public:
    explicit LogThread(QObject *parent = nullptr);

private:
    void run() override;

    QList<QTreeWidgetItem*> itemList;
    QString filename;

signals:
    void send(int data);

public slots:
    void appendData(QTreeWidgetItem*);
    void saveData();

};

#endif // LOGTHREAD_H
