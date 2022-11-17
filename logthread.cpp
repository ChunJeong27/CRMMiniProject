#include "logthread.h"

#include <QDateTime>
#include <QFile>
#include <QTreeWidgetItem>

LogThread::LogThread(QObject *parent)
    : QThread{parent}
{
    // 파일 이름 생성
    filename = "server_log_.txt";   // 기본 파일 이름 틀 설정
    filename.insert(11, QDateTime::currentDateTime()
                    .toString("yyyyMMdd_hhmmss"));  // 마지막 위치에 날짜시간 추가
}

/* 쓰레드가 소멸할 때 파일을 닫는 소멸자 */
LogThread::~LogThread()
{
    QFile file(filename);   // 파일 변수 선언
    file.close();   // 파일 닫기
}

/* 쓰레드 메인 함수 */
void LogThread::run()
{
    forever {   // 무한 반복문
        saveData(); // 로그 데이터를 저장
        sleep(60);  // 1분 간격으로 저장
    }
}

/* 로그를 담는 큐에 로그를 추가하는 함수 */
void LogThread::appendData(QTreeWidgetItem* item)
{
    itemList.enqueue(item); // 트리위젯아이템을 리스트에 저장
}

/* 로그를 파일에 기록하는 함수 */
void LogThread::saveData()
{
    if( !itemList.isEmpty() ) {  // 리스트가 존재한다면
        QFile file(filename);   // 파일 변수 선언
        if( !file.open(QIODevice::WriteOnly | QIODevice::Append
                       | QIODevice::Text) )    return;
        // 쓰기 전용, 추가, 텍스트 옵션으로 파일 열기

        QTextStream out(&file); // 파일을 텍스트 스트림으로 선언
        foreach(QTreeWidgetItem* item, itemList) {  // 아이템을 로그로 파일에 저장
            out << item->text(0) << ','
                << item->text(1) << ','
                << item->text(2) << ','
                << item->text(3) << ','
                << item->text(4) << ','
                << item->text(5) << '\n';
        }
        itemList.clear();   // 기록을 마친 로그 리스트 초기화 (Dequeue)
    }
}
