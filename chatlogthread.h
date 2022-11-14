#ifndef CHATLOGTHREAD_H
#define CHATLOGTHREAD_H

#include <QThread>
#include <QQueue>

class ChatLogThread : public QThread
{
    Q_OBJECT
public:
    explicit ChatLogThread(QObject *parent = nullptr);
    ~ChatLogThread();

private:
    void run() override;

    QQueue<QString> logList;    // 로그 목록을 저장하는 큐
    QString filename;       // 파일 이름 변수

public slots:
    void appendData(QString);   // 로그 목록을 추가하는 슬롯 함수
    void saveData();        // 로그 목록을 파일로 저장하는 슬롯 함수

};

#endif // CHATLOGTHREAD_H
