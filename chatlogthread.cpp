#include "chatlogthread.h"

#include <QFile>
#include <QDateTime>

ChatLogThread::ChatLogThread(QObject *parent)
    : QThread{parent}
{
    // 생성자에서 파일 이름 저장
    filename = "chat_room_log_.txt";    // 기본 파일 이름 설정
    filename.insert(14, QDateTime::currentDateTime()
                    .toString("yyyyMMdd_hhmmss"));  // 날짜위치에 현재 날짜시간 추가
}

/* 쓰레드가 소멸할 때 파일을 닫는 소멸자 */
ChatLogThread::~ChatLogThread()
{
    QFile file(filename);   // 파일 변수 선언
    file.close();   // 파일 닫기
}

/* 쓰레드 메인 함수 */
void ChatLogThread::run()
{
    forever {   // 무한 반복문
        saveData(); // 로그 데이터를 저장
        sleep(60);  // 1분 간격으로 저장
    }
}

/* 로그 리스트에 로그 한 줄을 추가하는 함수 */
void ChatLogThread::appendData(QString log)
{
    logList.enqueue(log);   // 문자열을 로그 리스트에 추가
}

/* 로그를 파일에 저장하는 함수 */
void ChatLogThread::saveData()
{
    if ( !logList.isEmpty() ) { // 로그 리스트가 존재하면
        QFile file(filename);   // 파일 변수를 선언
        if ( !file.open(QIODevice::WriteOnly | QIODevice::Append
                        | QIODevice::Text ) )  return;
        // 쓰기 전용, 추가, 텍스트 옵션으로 파일 열기

        QTextStream out(&file); // 파일을 텍스트 스트림으로 선언
        foreach ( QString item, logList ) {
            out << item << '\n';    // 로그 리스트 내용을 파일에 저장
        }
        logList.clear();    // 기록을 마친 로그 리스트 초기화 (Dequeue)
    }
}
