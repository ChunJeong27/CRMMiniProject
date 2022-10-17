#ifndef SERVERFORM_H
#define SERVERFORM_H

#include <QWidget>
#include <QList>

class QTcpServer;
class QTcpSocket;

namespace Ui {
class ServerForm;
}

typedef enum {
    Chat_Login = 0x30,             // 로그인(서버 접속)   --> 초대를 위한 정보 저장
    Chat_In,                // 채팅방 입장
    Chat_Talk,              // 채팅
    Chat_Close,             // 채팅방 퇴장         --> 초대 가능
    Chat_LogOut,            // 로그 아웃(서버 단절) --> 초대 불가능
    Chat_Invite,            // 초대
    Chat_KickOut,           // 강퇴
    Chat_FileTrans_Start,   // 파일 전송 시작(파일명) --> 파일 오픈
    Chat_FileTransfer,      // 파일 데이터 전송      --> 데이터를 파일에 저장
    Chat_FileTrans_End,     // 파일 전송 완료        --> 파일 닫기
} Chat_Status;

class ServerForm : public QWidget
{
    Q_OBJECT

public:
    explicit ServerForm(QWidget *parent = nullptr);
    ~ServerForm();

private:
    Ui::ServerForm *ui;

private slots:
    void clientConnect();
    void echoData();
    void removeItem();
    void banishClient();

private:
    QTcpServer *tcpServer;

    QList<QTcpSocket*> clientList;
};

#endif // SERVERFORM_H
