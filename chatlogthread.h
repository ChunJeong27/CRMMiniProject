#ifndef CHATLOGTHREAD_H
#define CHATLOGTHREAD_H

#include <QThread>
#include <QQueue>

class QTreeWidgetItem;

class ChatLogThread : public QThread
{
    Q_OBJECT
public:
    explicit ChatLogThread(QObject *parent = nullptr);
    ~ChatLogThread();

private:
    void run() override;

    QQueue<QString> logList;
    QString filename;

signals:
    void send(int data);

public slots:
    void appendData(QString);
    void saveData();

};

#endif // CHATLOGTHREAD_H
