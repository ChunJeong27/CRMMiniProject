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
    KickOut,           // 강퇴
    ClientList,
    FileList,
    FileUpload,
    FileDownload,
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
    void closeEvent(QCloseEvent*) override;
    void writeSocket(char, QByteArray);

    const int BLOCK_SIZE = 1024;

    QTcpSocket* chatSocket;
    QTcpSocket* uploadSocket;
    QProgressDialog* progressDialog;    // 파일 진행 확인
    QFile* file;
    qint64 loadSize;
    qint64 byteToWrite;
    qint64 totalSize;
    QByteArray outBlock;
    bool isSent = false;

    QTcpSocket* downloadSocket;
    qint64 downloadTotalSize;
    qint64 byteReceived = 0;
    QString filename;
    QFile* newFile;
    QByteArray inBlock;

signals:
    void clickedFileList(QListWidgetItem*);

private slots:
    void disconnectServer();
    void connectPushButton();
    void receiveData();
    void sendMessage();
    void sendFile();
    void goOnSend(qint64);
    void downloadFile();

};

#endif // CHATROOMFORM_H
