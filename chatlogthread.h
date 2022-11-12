#ifndef CHATLOGTHREAD_H
#define CHATLOGTHREAD_H

#include <QThread>

class QTreeWidgetItem;

class ChatLogThread : public QThread
{
    Q_OBJECT
public:
    explicit ChatLogThread(QObject *parent = nullptr);
    ~ChatLogThread();

private:
    void run() override;

    QList<QString> logList;
    QString filename;

signals:
    void send(int data);

public slots:
    void appendData(QString);
    void saveData();

};

#endif // CHATLOGTHREAD_H
