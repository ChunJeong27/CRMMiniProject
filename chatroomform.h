#ifndef CHATROOMFORM_H
#define CHATROOMFORM_H

#include <QWidget>

class QTcpSocket;
class QFile;
class QProgressDialog;
class QListWidgetItem;

namespace Ui {
class ChatRoomForm;
}

namespace Chat {
typedef enum {
    Connect = 0x30,       // 로그인(서버 접속)   --> 초대를 위한 정보 저장
    Enter,                // 채팅방 입장
    Message,              // 채팅
    Leave,             // 채팅방 퇴장         --> 초대 가능
    Disconnect,            // 로그 아웃(서버 단절) --> 초대 불가능
    Invite,            // 초대
    Banish,           // 강퇴
    FileTrans_Start,   // 파일 전송 시작(파일명) --> 파일 오픈
    FileTransfer,      // 파일 데이터 전송      --> 데이터를 파일에 저장
    FileTrans_End,     // 파일 전송 완료        --> 파일 닫기
    ClientList,
    FileList,
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

signals:
    void clickedFileList(QListWidgetItem*);

private slots:
    void receiveData();
    void sendData();
    void disconnect();
    void connectPushButton();
    void goOnSend(qint64);
    void sendFile();

    void downloadFile();

private:
    void closeEvent(QCloseEvent*) override;

    QTcpSocket *clientSocket;
    QTcpSocket *fileClient;
    QProgressDialog* progressDialog;    // 파일 진행 확인
    QFile* file;
    qint64 loadSize;
    qint64 byteToWrite;
    qint64 totalSize;
    QByteArray outBlock;
    bool isSent = false;

    QTcpSocket* downloadFileClient;
    QProgressDialog* downloadProgressDialog;
    qint64 downloadTotalSize;
    qint64 downloadByteReceived = 0;
    QString downloadFilename;
    QFile* downloadNewFile;
    QByteArray downloadInBlock;

};

#endif // CHATROOMFORM_H
