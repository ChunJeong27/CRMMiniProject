#include "serverform.h"
#include "ui_serverform.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QApplication>
#include <QMessageBox>
#include <QDateTime>
#include <QFileInfo>
#include <QProgressDialog>
#include <QFileDialog>
#include <QDir>

#include "chatroomform.h"
#include "logthread.h"

ServerForm::ServerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServerForm)
{
    ui->setupUi(this);

    connect(ui->banishPushButton, SIGNAL(clicked()),
            this, SLOT(kickOutClient()));
    connect(ui->invitePushButton, SIGNAL(clicked()),
            this, SLOT(inviteClient()));

    logThread = new LogThread(this);
    logThread->start();

    connect(ui->logSavePushButton, SIGNAL(clicked()),
            logThread, SLOT(saveData()));

    tcpServer = new QTcpServer(this);
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(connectClient()));

    if(!tcpServer->listen(QHostAddress::Any, 19000)){
        QMessageBox::critical(this, tr("Chatting Server"),
                              tr("Unable to start the server: %1.")
                              .arg(tcpServer->errorString()));
        return;
    }

    ui->portNumLineEdit->setText("Port Number : "
                                 + QString::number(tcpServer->serverPort()));
    qDebug()<<tr("The server is running on port %1.")
              .arg(tcpServer->serverPort());

    ui->textEdit->setText("File Upload Server Start!!!");

    uploadTotalSize = 0;
    byteReceived = 0;

    uploadServer = new QTcpServer(this);
    connect(uploadServer, SIGNAL(newConnection()),
            this, SLOT(acceptUploadConnection()));

    if(!uploadServer->listen(QHostAddress(QHostAddress::Any), 19100)){
        QMessageBox::critical(this, tr("File Upload Server"),
                              tr("Unable to start the server: %1.")
                              .arg(uploadServer->errorString( )));
        return;
    }

    ui->textEdit->append(tr("Start Upload listenint ..."));
    qDebug("Start Upload listening ...");

    ui->textEdit->setText("File Download Server Start!!!");

    transferServer = new QTcpServer(this);
    connect(transferServer, SIGNAL(newConnection()),
            this, SLOT(acceptTransferConnection()));

    if(!transferServer->listen(QHostAddress(QHostAddress::Any), 19200)){
        QMessageBox::critical(this, tr("File Download Server"),
                              tr("Unable to start the server: %1.")
                              .arg(uploadServer->errorString( )));
        return;
    }

    ui->textEdit->append(tr("Start Download listenint ..."));
    qDebug("Start Download listening ...");

    progressDialog = new QProgressDialog(0);
    progressDialog->setAutoClose(true);
    progressDialog->reset();

}

ServerForm::~ServerForm()
{
    qDebug() << "close";
    foreach(QTcpSocket* socket, clientSocketList){
        qDebug() << socket->peerPort();
        socket->disconnectFromHost();
    }

    logThread->terminate();
    tcpServer->close();
    uploadServer->close();
    transferServer->close();

    delete ui;
}

void ServerForm::writeSocket(QTcpSocket* socket, char type, QByteArray message)
{
    QByteArray outByteArray;
    outByteArray.append(type);
    outByteArray.append(message);

    socket->write(outByteArray);
    socket->flush();
    while(socket->waitForBytesWritten());
}

void ServerForm::sendActivatedChatRoom(char type, QByteArray byteArray)
{
    foreach(QTcpSocket* sock, clientSocketList){
        // 리스트의 접속된 클라이언트 소켓에 보내기 위해 반복
        if(!waitingClient.isEmpty()){   // 대기방 고객 리스트가 비어있지 않다면
            foreach(QTcpSocket* waiting, waitingClient){
                // 대기방의 고객에는 전송하지 않기 위해 반복
                if(sock != waiting){    // 대기방에 없다면
                    writeSocket(sock, type, byteArray);
                    // 입장 프로토콜로 저장한 데이터를 전송
                }
            }
        } else {
            writeSocket(sock, type, byteArray);
            // 입장 프로토콜로 저장한 데이터를 전송
        }
    }
}

void ServerForm::sendMessage(QTcpSocket* clientSocket, QByteArray byteArray)
{
    foreach(QTcpSocket* sock, clientSocketList){
        // 리스트에 접속된 클라이언트 소켓으로 보내기 위해 반복
        if(!waitingClient.isEmpty()){   // 대기방 고객 리스트가 비어있지 않다면
            foreach(QTcpSocket* waiting, waitingClient){
                if(sock != waiting && sock != clientSocket){
                    writeSocket(sock, Chat::Message, byteArray);
                }
            }
        } else {
            if(sock != clientSocket){
                writeSocket(sock, Chat::Message, byteArray);
            }
        }
    }
}

void ServerForm::connectClient()
{
    QTcpSocket *clientSocket(tcpServer->nextPendingConnection());
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(recieveData()));
    connect(clientSocket, SIGNAL(disconnected()), this, SLOT(removeItem()));

    QString ip(clientSocket->peerAddress().toString());
    quint16 port(clientSocket->peerPort());

    QListWidgetItem* clientItem = new QListWidgetItem(ip + ":"
                                                      +QString::number(port));
    ui->clientListWidget->addItem(clientItem);

    clientSocketList.append(clientSocket);
    waitingClient.append(clientSocket);

    clientSocket->write(Chat::Connect + ip.toUtf8());
}

/* 연결된 소켓의 접속이 끊어졌을 때 리스트에서 소켓을 삭제하는 슬롯 함수 */
void ServerForm::removeItem()
{
    QTcpSocket *clientConnection = qobject_cast<QTcpSocket *>(sender());
    // 신호를 방출한 객체를 sender로 받고 소켓으로 선언
    clientSocketList.removeOne(clientConnection);   // 신호가 끊어진 객체를 리스트에서 삭제
    waitingClient.removeOne(clientConnection);  // 신호가 끊어진 객체를 대기방 리스트에서 삭제
    clientConnection->deleteLater();    // 신호가 끊어진 객체를 삭제
}

/* 클라이언트로부터 데이터가 도착했을 때 프로토콜에 맞춰 처리하는 슬롯 함수 */
void ServerForm::recieveData()
{
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    // 슬롯 함수를 호출한 시그널의 객체를 소켓으로 선언
    if(clientSocket == nullptr)     return; // 객체를 가져오지 못할 경우 함수 종료
    QByteArray byteArray = clientSocket->read(BLOCK_SIZE);
    // 클라이언트에서 전송한 데이터 바이트 어레이 형태로 선언
    QString ip = clientSocket->peerAddress().toString();
    // 받아온 소켓에서 메소드를 통해 ip를 문자열로 선언
    quint16 port = clientSocket->peerPort();
    // 소켓에서 포트를 정수형으로 선언
    QString ipPort = ip + ":" + QString::number(port);  // ip:port 형식의 문자열로 선언

    char header(byteArray.at(0));
    // 받은 데이터의 첫 1바이트는 서버에서 수행할 동작을 의미하며 별로의 변수로 선언
    QString body = byteArray.remove(0, 1);  // 동작 바이트를 제거 및 데이터를 문자열로 저장
    QString action; // 서버에서 동작한 로그를 문자열로 저장하기 위한 변수 선언

    switch(header){
    case Chat::Connect: // 연결 프로토콜
    {
        QList<QString> idName = body.split("@");
        // id@이름 형식으로 된 데이터를 문자열 리스트로 저장하기 위한 메소드

        idCheck = false;
        emit checkClientId(idName.at(1), idName.at(0));

        // 고객 정보 폼에서 데이터를 확인하기 위한 시그널 방출
        if( idCheck ){    // 고객 정보가 존재한다면 bool 변수가 슬롯 함수에서 true로 변경됨
            QList<QListWidgetItem*> result =
                    ui->clientListWidget->findItems(ipPort, Qt::MatchExactly);
            // ip:port 형식으로 리스트에 저장함으로써 연결됨을 표시
            clientName.insert(ipPort, idName.at(0)
                              + "(ID:" + idName.at(1) + ")");
            // ip:port를 key로 클라이언트에서 받아온 id와 이름을 이름(ID:id) 형식으로 해쉬에 저장
            ipToClientName.insert(ip, idName.at(0)
                                  + "(ID:" + idName.at(1) + ")");
            // ip를 key로 클라이언트를 구분하고 이름을 저장하는 해쉬에 저장
            if(!result.isEmpty()){  // 리스트위젯에 ip, 포트가 일치하는 클라이언트가 존재하면
                QListWidgetItem* listWidgetItem = result.first();
                // 데이터를 변경하기 위해 ip:port 형태로 된 리스트아이템을 선언
                listWidgetItem->setText(idName.at(0)
                                        + "(ID:" + idName.at(1) + ")");
                // 이름(ID:id) 형식으로 리스트 아이템 텍스트를 변경
            }
            action = "CONNECT"; // 로그에 기록할 동작을 문자열로 저장
        } else {    // 고객 정보에 일치하는 고객이 존재하지 않을 경우
            clientSocketList.removeOne(clientSocket);   // 고객 소켓 리스트에서 소켓을 삭제
            waitingClient.removeOne(clientSocket);  // 대기방 소켓 리스트에서 소켓을 삭제
            QList<QListWidgetItem*> findList =
                    ui->clientListWidget->findItems("::ffff:", Qt::MatchStartsWith);
            // 리스트위젯에서 존재하지 않는 ip주소를 삭제하기 위해 이름이 설정되지 않은 리스트 아이템을 검색
            if(!findList.isEmpty()){    // 검색 결과가 존재하다면
                ui->clientListWidget->takeItem(
                            ui->clientListWidget->row(findList.first()));
                // 검색 결과 아이템의 행 번호를 구해서 리스트위젯에서 아이템을 삭제
            }
//            writeSocket(clientSocket, Chat::Disconnect, ipPort.toUtf8());
            clientSocket->disconnectFromHost(); // 소켓을 서버와 끊기
            return;
        }
    } break;

    case Chat::Enter:   // 입장 프로토콜
    {
        waitingClient.removeOne(clientSocket);  // 채팅방에 입장하기 위해 대기방에서 소켓을 삭제

        QByteArray outByteArray;    // 클라이언트에 보낼 데이터를 저장할 바이트어레이 변수 선언
        outByteArray.append( byteArray + " enter the Chat room./");
        // 전송할 바이트어레이 변수에 입장 메시지를 저장

        QList<QListWidgetItem*> nameList =
                ui->clientListWidget->findItems("", Qt::MatchContains);
        // 채팅방의 접속중인 고객 리스트를 갱신하기 위해 서버의 리스트를 변수로 선언
        outByteArray.append(QString::number(nameList.size()).toUtf8() + "/");
        // 접속중인 리스트의 수를 바이트어레이에 저장
        foreach(QListWidgetItem* item, nameList){   // 리스트의 개수만큼 반복
            outByteArray.append(item->text().toUtf8() + "/");
            // 리스트의 모든 내용을 /문자로 구분하며 저장
        }

        QList fileList = ui->fileListWidget->findItems("", Qt::MatchContains);
        // 서버의 파일 리스트를 리스트 변수로 선언
        outByteArray.append(QString::number(fileList.size()).toUtf8() + "/");
        // 리스트의 개수를 전송할 바이트 어레이에 저장
        foreach(QListWidgetItem* item, fileList){   // 리스트 개수만큼 반복
            outByteArray.append(item->text().toUtf8() + "/");
            // 모든 파일 이름 리스트를 /문자로 구분하며 저장
        }

        foreach(QTcpSocket* sock, clientSocketList){
            // 리스트의 접속된 클라이언트 소켓에 보내기 위해 반복
            if(!waitingClient.isEmpty()){   // 대기방 고객 리스트가 비어있지 않다면
                foreach(QTcpSocket* waiting, waitingClient){
                    // 대기방의 고객에는 전송하지 않기 위해 반복
                    if(sock != waiting){    // 대기방에 없다면
                        writeSocket(sock, Chat::Enter, outByteArray);
                        // 입장 프로토콜로 저장한 데이터를 전송
                    }
                }
            } else {
                writeSocket(sock, Chat::Enter, outByteArray);
                // 입장 프로토콜로 저장한 데이터를 전송
            }
        }
        action = "ENTER";   // 서버에서 수행한 동작을 문자열로 저장
    } break;

    case Chat::Message: // 메시지 프로토콜
        sendMessage(clientSocket, byteArray);
        // 데이터를 보낸 소켓을 제외한 소켓에 데이터를 전송
        action = "MESSAGE ";
    break;

    case Chat::Leave:   // 퇴장 프로토콜
    {
        waitingClient.append(clientSocket);
        // 다시 채팅방에 들어올 수 있고 메시지는 받지 않도록 대기방에 추가
        QByteArray msg; // 고객에게 보낼 데이터를 저장할 바이트 어레이 변수를 선언
        // 클라이언트에서 받아온 이름과 보낼 메시지를 바이트어레이 변수에 저장
        msg.append( byteArray );
        msg.append(" left the chat room.");

        sendMessage(clientSocket, msg); // 데이터를 보낸 고객을 제외하고 채팅방에 메시지를 전송
        action = "LEAVE";   // 서버에서 수행한 동작을 문자열로 저장
    } break;

    case Chat::Invite:  // 초대 프로토콜
    {
        waitingClient.removeOne(clientSocket);  // 채팅방을 입장하면 대기방에서 삭제

        QByteArray outByteArray;    // 클라이언트에 보낼 데이터를 저장할 바이트어레이 변수 선언
        outByteArray.append( byteArray + " invited by server./");
        // 전송할 바이트어레이 변수에 초대 메시지를 저장

        QList<QListWidgetItem*> nameList =
                ui->clientListWidget->findItems("", Qt::MatchContains);
        // 채팅방의 접속중인 고객 리스트를 갱신하기 위해 서버의 리스트를 변수로 선언
        outByteArray.append(QString::number(nameList.size()).toUtf8() + "/");
        // 접속중인 리스트의 수를 바이트어레이에 저장
        foreach(QListWidgetItem* item, nameList){   // 리스트의 개수만큼 반복
            outByteArray.append(item->text().toUtf8() + "/");
            // 리스트의 모든 내용을 /문자로 구분하며 저장
        }

        QList fileList = ui->fileListWidget->findItems("", Qt::MatchContains);
        // 서버의 파일 리스트를 리스트 변수로 선언
        outByteArray.append(QString::number(fileList.size()).toUtf8() + "/");
        // 리스트의 개수를 전송할 바이트 어레이에 저장
        foreach(QListWidgetItem* item, fileList){   // 리스트 개수만큼 반복
            outByteArray.append(item->text().toUtf8() + "/");
            // 모든 파일 이름 리스트를 /문자로 구분하며 저장
        }

        sendActivatedChatRoom(Chat::Enter, outByteArray);
        // 활성화된 고객에게만 데이터를 전송하는 함수
        action = "INVITE";  // 로그에 출력할 문자열을 저장
    } break;

    case Chat::KickOut: // 강퇴 프로토콜
    {
        waitingClient.append(clientSocket); // 강퇴 당한 고객을 대기방에 추가
        QByteArray msg; // 고객에게 보낼 데이터를 저장할 바이트 어레이 변수를 선언
        // 클라이언트에서 받아온 이름과 보낼 메시지를 바이트어레이 변수에 저장
        msg.append("The server kicked ");
        msg.append( byteArray );
        msg.append(" out of the chat room.");

        sendMessage(clientSocket, msg);
        // 대기방에 있는 고객을 제외하고 고객 소켓에 데이터를 전송
        action = "KICK OUT";    // 로그에 출력할 문자열을 저장
    } break;

    case Chat::Disconnect:  // 연결 끊겼을 때 동작하는 프로토콜
    {
        clientSocketList.removeOne(clientSocket);   // 연결이 끊어진 고객을 리스트에서 삭제
        waitingClient.removeOne(clientSocket);  // 연결이 끊어진 고객을 대기방에서 삭제

        QList<QListWidgetItem*> findList = ui->clientListWidget
                ->findItems(body, Qt::MatchExactly);
        // 리스트위젯에서 리스트아이템을 삭제하기 위해 아이템을 검색하고 리스트로 저장
        if(!findList.isEmpty()){    // 아이템이 존재한다면
            ui->clientListWidget->takeItem(ui->clientListWidget
                                           ->row(findList.first()));
            // 검색한 리스트 아이템을 가져와 리스트위젯에서 제거
        }
        QList<QListWidgetItem*> sendItemLsit = ui->clientListWidget
                ->findItems("", Qt::MatchContains);
        // 삭제한 리스트위젯을 보내기 위해 모든 아이템을 리스트로 저장
        QByteArray sendList;    // 보낼 리스트를 저장할 바이트 어레이 변수 선언
        foreach(auto list, sendItemLsit){   // 보낼 리스트 수 만큼 반복
            sendList.append(list->text().toUtf8() + "/");
            // 리스트를 /로 구분하고 바이트 어레이 변수에 저장
        }

        sendActivatedChatRoom(Chat::ClientList, sendList);
        // 활성화된 채팅방에만 데이터를 보내는 함수
        action = "DISCONNECT";  // 로그에 표시할 문자열을 저장
    } break;

    case Chat::FileDownload:    // 파일 다운로드 프로토콜
        action = "DOWNLOAD";    // 로그에 기록할 문자열을 저장
        break;

    default:
        break;
    }
    QTreeWidgetItem* log = new QTreeWidgetItem(ui->logTreeWidget);
    // 로그를 기록하기 위한 트리위젯아이템 선언
    log->setText(0, QDateTime::currentDateTime().toString("yyMMdd hh:mm:ss"));
    // 첫 번째 열에 현재 시각을 원하는 형식의 문자열로 저장
    log->setText(1, ip);    // 두 번째 열에 ip문자열을 저장
    log->setText(2, QString::number(port)); // 세 번째 문자열에 포트번호 저장
    log->setText(3, clientName[ipPort]);    // 네 번째 문자열로 이름과 아이디를 저장
    log->setText(4, action);    // 서버에 동작한 프로토콜을 문자열로 저장
    log->setText(5, body);  // 여섯 번째 문자열에 전송받은 데이터를 저장
    logThread->appendData(log); // 로그 기록 쓰레드에 아이템을 전달하여 저장

}

/* 초대 버튼 신호가 발생했을 때 동작하는 슬롯 함수 */
void ServerForm::inviteClient()
{
    QString name = ui->clientListWidget->currentItem()->text();
    // 이름을 저장하고 위한 변수 선언
    QString ipPort = clientName.key(name);   // 이름으로 ip와 포트번호를 가져와서 선언

    foreach(QTcpSocket* socket, clientSocketList){  // 고객 리스트 수만큼 반복
        if(socket->peerAddress().toString() + ":"
                + QString::number(socket->peerPort()) == ipPort){
            // 이름과 id로 가져온 ip, 포트번호와 일치하는 소켓에 프로토콜을 전송
            writeSocket(socket, Chat::Invite, name.toUtf8());
            // 찾은 소켓에 프로토콜과 이름을 전송
        }
    }
}

/* 강퇴 버튼이 눌렸을 때 동작하는 슬롯 함수 */
void ServerForm::kickOutClient()
{
    QString name = ui->clientListWidget->currentItem()->text();
    // 이름 변수를 선언하고 문자열을 리스트위젯에서 가져와 저장
    QString ipPort = clientName.key(name);  // 이름으로 ip와 포트번호를 가져와서 선언

    foreach(QTcpSocket* socket, clientSocketList){  // 고객 리스트 수만큼 반복
        if(socket->peerAddress().toString() + ":"
                + QString::number(socket->peerPort()) == ipPort){
            // 이름과 id로 가져온 ip, 포트번호와 일치하는 소켓에 프로토콜과 이름 전송
            socket->write(Chat::KickOut + name.toUtf8());
        }
    }
}

/* 고객 정보 폼에서 이름과 id의 유무를 멤버 변수에 저장하는 슬롯 함수 */
void ServerForm::isClient(bool chk){
    idCheck = chk;  // 고객 정보 폼에서 받아온 체크 결과를 멤버 변수에 저장
}

/* 업로드 서버에 새로운 연결이 발생했을 때 소켓을 저장하고 슬롯 함수를 연결하는 슬롯 함수 */
void ServerForm::acceptUploadConnection()
{
    qDebug("Connected, preparing to receive files!");
    ui->textEdit->append(tr("Connected, preparing to receive files!"));
    // 연결되었다는 메시지를 서버 폼에 출력

    QTcpSocket* ftpSocket = uploadServer->nextPendingConnection();
    // 통신에 사용할 소켓을 시그널 슬롯 연결을 위해 선언
    connect(ftpSocket, SIGNAL(readyRead()), this, SLOT(readClient()));
    // 서버에서 데이터를 전송하면 파일로 저장할 슬롯 함수를 연결
}

/* 다운로드 서버에 새로운 연결이 발생했을 때 소켓을 멤버 변수로 저장하고 슬롯 함수를 연결하는 슬롯 함수 */
void ServerForm::acceptTransferConnection()
{
    qDebug("Connected, preparing to transfer files!(Download)");
    ui->textEdit->append(tr("Connected, preparing to transfer files!(Download)"));

    transferSocket = transferServer->nextPendingConnection();
    connect(transferSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(goOnSend(qint64)));
}

/* 클라이언트에서 업로드를 받기 위한 슬롯 함수 */
void ServerForm::readClient()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    // 데이터를 보낸 소켓을 변수로 선언
    QString ip = socket->peerAddress().toString();  // ip를 문자열로 저장

    qDebug("Receiving file ...");
    ui->textEdit->append(tr("Receiving file ..."));

    if(byteReceived == 0) { // 데이터를 처음 받을 때 총 파일의 크기, 받은 파일의 크기, 파일의 정보를 가져옴
        progressDialog->reset();    // 화면에 표시할 다이얼로그를 초기화
        progressDialog->show(); // 다이얼로그 출력

        QDataStream in(socket); // 소켓을 데이터 스트림으로 사용
        in >> uploadTotalSize >> byteReceived >> filename;
        // 3개의 변수에 차례대로 데이터를 저장
        progressDialog->setMaximum(uploadTotalSize);
        // 파일의 총 크기를 다이얼로그 최대값으로 설정
        QFileInfo info(filename);
        // 경로를 포함한 파일이름으로 파일 정보를 가져옴
        QString currentFileName = info.fileName();  // 파일 정보에서 이름만 문자열로 저장
        newFile = new QFile(currentFileName);
        // client 폴더에 파일을 저장하기 위해 QFile 객체로 동적 할당
        newFile->open(QFile::WriteOnly);    // 다운로드할 파일을 쓰기 전용으로 열기

        // 서버에 로그를 기록하기 위해 아이템에 저장하고 로그로 기록
        QTreeWidgetItem* log = new QTreeWidgetItem(ui->logTreeWidget);
        log->setText(0, QDateTime::currentDateTime().toString("yyMMdd hh:mm:ss"));
        log->setText(1, socket->peerAddress().toString());
        log->setText(2, QString::number(socket->peerPort()));
        log->setText(3, ipToClientName.value(ip));
        log->setText(4, "START UPLOADING");
        log->setText(5, currentFileName);
        log->setToolTip(5, currentFileName);
        logThread->appendData(log);
    } else {
        inBlock = socket->readAll();    // 소켓의 모든 데이터를 읽어옴

        byteReceived += inBlock.size();
        // 읽어온 데이터 크기만큼 받은 데이터를 추가
        newFile->write(inBlock);    // 읽어온 데이터를 파일에 기록
        newFile->flush();   // 파일에 남은 데이터를 해제
    }

    progressDialog->setValue(byteReceived);
    // 데이터를 받은 만큼 다이얼로그에 값으로 설정

    if(byteReceived == uploadTotalSize){
        qDebug() << QString("%1 receive completed").arg(filename);
        ui->textEdit->append(tr("%1 receive completed").arg(filename));

        QFileInfo info(filename);
        QString currentFileName = info.fileName();

        // 서버에 로그를 기록하기 위해 아이템에 저장하고 로그로 기록
        QTreeWidgetItem* log = new QTreeWidgetItem(ui->logTreeWidget);
        log->setText(0, QDateTime::currentDateTime().toString("yyMMdd hh:mm:ss"));
        log->setText(1, socket->peerAddress().toString());
        log->setText(2, QString::number(socket->peerPort()));
        log->setText(3, ipToClientName.value(ip));
        log->setText(4, "UPLOAD FINSHED");
        log->setText(5, currentFileName);
        log->setToolTip(5, currentFileName);
        logThread->appendData(log);

        QListWidgetItem* fileItem = new QListWidgetItem(currentFileName, ui->fileListWidget);
        ui->fileListWidget->addItem(fileItem);

        QByteArray fileListByteArray;
        QList<QListWidgetItem*> fileList = ui->fileListWidget->findItems("", Qt::MatchContains);
        foreach(QListWidgetItem* item, fileList){
            fileListByteArray.append(item->text().toUtf8() + "/");
        }

        foreach(QTcpSocket* sock, clientSocketList){
//            if(sock->peerAddress().toString() == ip)
                sock->write(Chat::FileList + fileListByteArray);
        }

        inBlock.clear();
        byteReceived = 0;
        uploadTotalSize = 0;
        progressDialog->reset();
        progressDialog->hide();
        newFile->close();

        delete newFile;
    }

}

/* 파일 데이터를 보내기 위한 초기 설정 슬롯 함수 */
void ServerForm::sendFile(QListWidgetItem* fileNameItem)
// 파일을 열고 경로를 포함한 파일 이름을 서버로부터 가져옴
{
    loadSize = 0;   // 매번 서버로 보낼 바이트의 크기
    byteToWrite = 0;    // 보낼 데이터의 크기
    transferTotalSize = 0;  // 총 파일의 크기
    outBlock.clear();   // 서버로 보낼 바이트 어레이를 초기화

    QString filename = QDir::currentPath() + "/" + fileNameItem->text();
    // 워킹 경로에 있는 파일을 가져옴
    file = new QFile(filename); // QFile로 선언
    file->open(QFile::ReadOnly);    // 파일을 읽기 전용으로 열기

    qDebug() << QString("file %1 is opened").arg(filename);
    progressDialog->setValue(0);    // 첫 전송은 데이터를 보내지 않음으로 0으로 초기화

    byteToWrite = transferTotalSize = file->size();// 파일의 크기를 총 크기, 기록할 파일 크기로 저장

    loadSize = 1024; // 서버로 전송할 바이트의 크기를 설정

    QDataStream out(&outBlock, QIODevice::WriteOnly);
    // 서버로 보낼 바이트 배열 변수를 데이터 스트림 변수로 열어줌
    out << qint64(0) << qint64(0) << filename;
    // qint64(0) 바이트 공간을 확보해놓기 위해 사용, 마지막 위치에 파일 이름을 저장

    transferTotalSize += outBlock.size();   // 파일 크기와 기록할 파일 크기에 첫 바이트 블록의 크기를 추가
    qDebug() << transferTotalSize;
    byteToWrite += outBlock.size();

    out.device()->seek(0);  // 총 크기와 기록할 크기를 저장하기 위해 데이터 스트림의 기록위치를 가장 처음으로 변경
    out << transferTotalSize << qint64(outBlock.size());    // 총 파일 크기와 전송할 데이터의 크기를 저장

    qDebug() << transferTotalSize;  // 총 파일 크기와 전송할 데이터의 크기를 저장

    transferSocket->write(outBlock); // 읽어온 파일을 서버로 전송

    progressDialog->setMaximum(transferTotalSize);
    // 총 파일 크기를 다이얼로그의 최대 크기로 설정
    progressDialog->setValue(transferTotalSize - byteToWrite);
    // 총 크기 - 기록한 바이트 수로 남은 진행 상황을 다이얼로그로 표시
    progressDialog->show(); // 다이얼로그를 화면에 표시

    qDebug() << QString("Sending file %1").arg(filename);
}

/* 파일 내용을 연속적으로 전송하는 슬롯 함수 */
void ServerForm::goOnSend(qint64 numBytes)
{
    byteToWrite -= numBytes;    // 기록할 데이터를 빼줌으로써 기록할 데이터를 갱신
    outBlock = file->read(qMin(byteToWrite, numBytes));
    // 기록할 남은 바이트와 이전에 기록한 바이트 수 중 더 적은 값으로 파일에서 전송할 데이터를 읽어옴
    transferSocket->write(outBlock);    // 파일에서 읽어온 데이터를 서버로 전송
    // 다이얼로그에 총 파일 크기와 전송한 파일 크기를 설정
    progressDialog->setMaximum(transferTotalSize);
    progressDialog->setValue(transferTotalSize-byteToWrite);

    if (byteToWrite == 0) { // 기록할 바이트가 0이면 파일 전송 완료
        qDebug("File sending completed!");
        progressDialog->reset();    // 전송을 완료했으므로 다이얼로그 초기화
    }
}
