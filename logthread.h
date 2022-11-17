#ifndef LOGTHREAD_H
#define LOGTHREAD_H

#include <QThread>
#include <QQueue>

class QTreeWidgetItem;

class LogThread : public QThread
{
    Q_OBJECT
public:
    explicit LogThread(QObject *parent = nullptr);
    ~LogThread();

private:
    void run() override;

    QQueue<QTreeWidgetItem*> itemList;  // 로그 작업을 저장하는 큐
    QString filename;   // 파일 이름 문자열 변수

public slots:
    void appendData(QTreeWidgetItem*);  // 로그 작업을 추가하는 슬롯 함수
    void saveData();    // 로그 리스트를 파일로 저장하는 슬롯 함수

};

#endif // LOGTHREAD_H
