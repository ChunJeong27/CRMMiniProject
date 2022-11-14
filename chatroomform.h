#ifndef CHATROOMFORM_H
#define CHATROOMFORM_H

#include <QWidget>

class QTcpSocket;
class QFile;
class QProgressDialog;
class QListWidgetItem;
class ChatLogThread;

namespace Ui {
class ChatRoomForm;
}

namespace Chat {
typedef enum {
    Connect = 0x30,     // 연결(서버 접속)   --> 초대를 위한 정보 저장
    Enter,              // 채팅방 입장
    Message,            // 채팅
    Leave,              // 채팅방 퇴장         --> 초대 가능
    Disconnect,         // 연결 종료(서버 단절) --> 초대 불가능
    Invite,             // 초대
    KickOut,            // 강제 퇴장
    ClientList,         // 고객 목록 갱신
    FileList,           // 파일 목록 갱신
    FileDownload        // 파일 다운로드 완료
} Header;
}

class ChatRoomForm : public QWidget
{
    Q_OBJECT

public:
    explicit ChatRoomForm(QWidget *parent = nullptr);
    ~ChatRoomForm();

private:
    Ui::ChatRoomForm *ui;

private:
    void closeEvent(QCloseEvent*) override; // 프로그램이 닫혔을 때 수행되는 함수
    void writeSocket(char, QByteArray); // 소켓 통신을 위한 함수

    const int BLOCK_SIZE = 1024;    // 소켓 통신 데이터 사이즈

    QTcpSocket* chatSocket; // 채팅용 소켓 변수
    QTcpSocket* uploadSocket;   // 파일 업로드용 소켓 변수
    QFile* file;    // 업로드 파일 변수
    qint64 loadSize;    // 보낼 바이트 크기
    qint64 byteToWrite; // 파일에 기록할 남은 데이터 크기
    qint64 totalSize;   // 전체 파일 크기
    QByteArray outBlock;    // 서버로 전송할 블록
    bool isSent = false;    // 이전에 데이터를 보냈는지 확인하는 변수
    QProgressDialog* progressDialog;    // 파일 진행 확인 다이얼로그

    QTcpSocket* downloadSocket; // 파일 다운로드용 소켓 변수
    qint64 downloadTotalSize;   // 다운로드 파일 전체 크기
    qint64 byteReceived = 0;    // 다운로드 받은 파일 크기
    QString filename;   // 파일 이름 변수
    QFile* newFile;     // 다운로드 된 새로운 파일 변수
    QByteArray inBlock; // 서버로부터 데이터를 받을 블록 변수

    ChatLogThread* chatLogThread;   // 채팅방 로그 기록 쓰레드

private slots:
    void disconnectServer();    // 서버와 연결이 끊어졌을 때 실행하는 슬롯 함수
    void connectPushButton();   // 연결 버튼을 눌렸을 때 실행하는 슬롯 함수
    void receiveData();     // 서버로부터 받은 데이터를 처리하는 슬롯 함수
    void sendMessage();     // 서버로 데이터를 보내는 슬롯 함수
    void sendFile();        // 파일 데이터를 보내는 슬롯 함수
    void goOnSend(qint64);  // 이전에 받은 데이터와 연결해서 데이터를 보내는 슬롯 함수
    void downloadFile();    // 서버로부터 파일 데이터를 받는 슬롯 함수
    void clickedFileList(QListWidgetItem*); // 리스트가 클릭되면 파일을 수신 신호를 보내는 슬롯 함수

};

#endif // CHATROOMFORM_H
