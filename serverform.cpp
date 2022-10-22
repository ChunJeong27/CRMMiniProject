#include "serverform.h"
#include "ui_serverform.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QApplication>
#include <QMessageBox>
#include <QDateTime>
#include <QFileInfo>
#include <QProgressDialog>

#include "chattingform.h"

#define BLOCK_SIZE  1024

ServerForm::ServerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServerForm)
{
    ui->setupUi(this);

    connect(ui->quitPushButton, SIGNAL(clicked()), qApp, SLOT(quit()));
    connect(ui->banishPushButton, SIGNAL(clicked()),
            this, SLOT(banishClient()));
    connect(ui->invitePushButton, SIGNAL(clicked()),
            this, SLOT(inviteClient()));

    tcpServer = new QTcpServer(this);
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(ConnectClient()));

    if(!tcpServer->listen(QHostAddress::Any, 19000)){
        QMessageBox::critical(this, tr("Chatting Server"),
                              tr("Unable to start the server: %1.")
                              .arg(tcpServer->errorString()));
        close();
        return;
    }

    ui->portNumLineEdit->setText("Port Number : "
                                 + QString::number(tcpServer->serverPort()));
    qDebug()<<tr("The server is running on port %1.")
              .arg(tcpServer->serverPort());

    ui->textEdit->setText("File Server Start!!!");

    totalSize = 0;
    byteReceived = 0;

    ftpServer = new QTcpServer(this);
    connect(ftpServer, SIGNAL(newConnection()),
            this, SLOT(acceptConnection()));

    if(!ftpServer->listen(QHostAddress(QHostAddress::Any), 19100)){
        QMessageBox::critical(this, tr("File Server"),
                              tr("Unable to start the server: %1.")
                              .arg(ftpServer->errorString( )));
        close( );
        return;
    }

    ui->textEdit->append(tr("Start listenint ..."));
    qDebug("Start listening ...");

    progressDialog = new QProgressDialog(0);
    progressDialog->setAutoClose(true);
    progressDialog->reset();

}

ServerForm::~ServerForm()
{
    delete ui;
}

void ServerForm::ConnectClient()
{
    QTcpSocket *clientSocket(tcpServer->nextPendingConnection());
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(recieveData()));
    connect(clientSocket, SIGNAL(disconnected()), this, SLOT(removeItem()));

    QString ip(clientSocket->peerAddress().toString());
    quint16 port(clientSocket->peerPort());

    QListWidgetItem* clientItem = new QListWidgetItem(ip + ":"
                                                      +QString::number(port));
    ui->clientListWidget->addItem(clientItem);

    clientList.append(clientSocket);
    waitingClient.append(clientSocket);

    clientSocket->write(Chat::Connect + ip.toUtf8());
}

void ServerForm::recieveData()
{
    QTcpSocket* clientSocket(qobject_cast<QTcpSocket*>(sender()));
    // 슬롯을 호출한 시그널의 객체
    if(clientSocket == nullptr)     return; // 객체를 가져오지 못할 경우 함수 종료
    QByteArray byteArray(clientSocket->read(BLOCK_SIZE));
    // 클라이언트에서 전송한 데이터
    QString ip(clientSocket->peerAddress().toString());
    quint16 port(clientSocket->peerPort());
    QString ipPort(ip + ":" + QString::number(port));

    char header(byteArray.at(0));     // 받은 데이터의 첫 1바이트는 서버에서 수행할 동작을 의미
    QString body(byteArray.remove(0, 1));      // 동작 바이트를 제거 및 문자열로 저장
    QString action;

    switch(header){
    case Chat::Connect:
    {
        QList<QString> idName = body.split("@");
        idCheck = false;
        emit checkClientId(idName.at(0), idName.at(1));
        QList<QListWidgetItem*> result =
                ui->clientListWidget->findItems(ipPort, Qt::MatchExactly);
        clientName.insert(ipPort, idName.at(0));    // 이름을 QList에 저장
        ipToClientName.insert(ip, idName.at(0));

        if(result.isEmpty())
//            clientSocket->write(Chat_Talk + "Login Error");
            writeSocket(clientSocket, Chat::Message, "Login Error");
        else {
            if(idCheck){
            QListWidgetItem* listWidgetItem(result.first());
            listWidgetItem->setText(idName.at(0)
                                    + "(ID:" + idName.at(1) + ")");
            }
        }

        action = "Connect";
    } break;

    case Chat::Enter:
    case Chat::Invite:
    {
        if(!waitingClient.isEmpty()){
            QList<QTcpSocket*>::Iterator eraseSock;
            for(auto sock = waitingClient.begin(); waitingClient.end() != sock; sock++){
                if(*sock == clientSocket){
                    eraseSock = sock;
                }
            }
            waitingClient.erase(eraseSock);
        }
        foreach(auto t, waitingClient)
            qDebug() << t->peerPort();

        QByteArray outByteArray;
        outByteArray.append(clientName[ipPort].toUtf8() + " enter the Chat room./");

//        foreach(QTcpSocket* sock, clientList){
//            if(!waitingClient.isEmpty()){
//                foreach(QTcpSocket* waiting, waitingClient){
//                    if(sock != waiting && sock != clientSocket){
////                        sock->write(Chat_Talk + clientName[ipPort].toUtf8() + msg);
//                        writeSocket(sock, Chat_Talk, outByteArray);
////                        sock->flush();
////                        while(sock->waitForBytesWritten());
//                    }
//                }
//            } else {
//                if(sock != clientSocket){
////                    sock->write(Chat_Talk + clientName[ipPort].toUtf8() + msg);
////                    sock->flush();
////                    while(sock->waitForBytesWritten());
//                    writeSocket(sock, Chat_Talk, outByteArray);

//                }
//            }
//        }


        QList<QListWidgetItem*> nameList = ui->clientListWidget->findItems("", Qt::MatchContains);
        outByteArray.append(QString::number(nameList.size()).toUtf8() + "/");
        foreach(QListWidgetItem* item, nameList){
            outByteArray.append(item->text().toUtf8() + "/");     // 채팅방 참여자 출력 기능 보류
        }

        QList fileList = ui->fileListWidget->findItems("", Qt::MatchContains);
        outByteArray.append(QString::number(fileList.size()).toUtf8() + "/");
        foreach(QListWidgetItem* item, fileList){
            outByteArray.append(item->text().toUtf8() + "/");     // 채팅방 참여자 출력 기능 보류
        }
        qDebug() << outByteArray;

        foreach(QTcpSocket* sock, clientList){
            if(!waitingClient.isEmpty()){
                foreach(QTcpSocket* waiting, waitingClient){
                    if(sock != waiting){
                        writeSocket(sock, Chat::Enter, outByteArray);
                    }
                }
            } else {
                writeSocket(sock, Chat::Enter, outByteArray);
            }
        }

        action = "Enter";
    } break;

    case Chat::Message:
    {
        QByteArray outByteArray(clientName[ipPort].toUtf8() + " : " + byteArray);

        foreach(QTcpSocket* sock, clientList){
            if(!waitingClient.isEmpty()){
                foreach(QTcpSocket* waiting, waitingClient){
                    if(sock != waiting && sock != clientSocket){
//                        sock->write(type + clientName[ipPort].toUtf8() + " : " + byteArray);
                        writeSocket(sock, Chat::Message, outByteArray);
                    }
                }
            } else {
                if(sock != clientSocket){
//                    sock->write(type + clientName[ipPort].toUtf8() + " : " + byteArray);
                    writeSocket(sock, Chat::Message, outByteArray);
                }
            }
        }
        action = "MESSAGE ";
    } break;

    case Chat::Banish:
    case Chat::Leave:
        waitingClient.append(clientSocket);
        action = "Leave";

        break;

    case Chat::Disconnect:
        if(!clientList.isEmpty()){
            QList<QTcpSocket*>::Iterator eraseSock;
            for(auto sock = clientList.begin(); clientList.end() != sock; sock++){
                if(*sock == clientSocket){
                    eraseSock = sock;
                }
            }
            clientList.erase(eraseSock);
        }

        if(!waitingClient.isEmpty()){
            QList<QTcpSocket*>::Iterator eraseSock;
            for(auto sock = waitingClient.begin(); waitingClient.end() != sock; sock++){
                if(*sock == clientSocket){
                    eraseSock = sock;
                }
            }
            waitingClient.erase(eraseSock);
        }

        QList<QListWidgetItem*> findList = ui->clientListWidget->findItems(body, Qt::MatchExactly);
        if(!findList.isEmpty()){
            ui->clientListWidget->takeItem(ui->clientListWidget->row(findList.first()));
        }
        QList<QListWidgetItem*> sendItemLsit = ui->clientListWidget->findItems("", Qt::MatchContains);
        QByteArray sendList;
        foreach(auto list, sendItemLsit){
            sendList.append(list->text().toUtf8() + "/");
        }

        foreach(QTcpSocket* sock, clientList){
            if(!waitingClient.isEmpty()){
                foreach(QTcpSocket* waiting, waitingClient){
                    if(sock != waiting){
                        writeSocket(sock, Chat::ClientList, sendList);
                    }
                }
            } else {
                writeSocket(sock, Chat::ClientList, sendList);
            }
        }

//        clientSocket->write(Chat_List + sendList);

        break;
    }

    QTreeWidgetItem* log = new QTreeWidgetItem(ui->logTreeWidget);
    log->setText(0, QDateTime::currentDateTime().toString("yyMMdd hh:mm:ss"));
    log->setText(1, ip);
    log->setText(2, QString::number(port));
    log->setText(3, clientName[ipPort]);
    log->setText(4, action);
    log->setText(5, body);

}

void ServerForm::removeItem()
{
    QTcpSocket *clientConnection = qobject_cast<QTcpSocket *>(sender());
    clientList.removeOne(clientConnection);
    clientConnection->deleteLater();
}

void ServerForm::banishClient()
{
    QString name(ui->clientListWidget->currentItem()->text());
    QString ipPort(clientName.key(name));

    foreach(QTcpSocket* socket, clientList){
        if(socket->peerAddress().toString() + ":" + QString::number(socket->peerPort()) == ipPort){
            socket->write(Chat::Banish + name.toUtf8());

//            QTreeWidgetItem* log = new QTreeWidgetItem(ui->logTreeWidget);
//            log->setText(0, QDateTime::currentDateTime().toString());
//            log->setText(1, socket->peerAddress().toString());
//            log->setText(2, QString::number(socket->peerPort()));
//            log->setText(3, name);
//            log->setText(4, "KICK OUT " + name);
        }
    }
}

void ServerForm::inviteClient()
{
    QString name(ui->clientListWidget->currentItem()->text());
    QString ipPort(clientName.key(name));

    foreach(QTcpSocket* socket, clientList){
        if(socket->peerAddress().toString() + ":" + QString::number(socket->peerPort()) == ipPort){
            qDebug("invite");
            socket->write(Chat::Invite + name.toUtf8());

//            QTreeWidgetItem* log = new QTreeWidgetItem(ui->logTreeWidget);
//            log->setText(0, QDateTime::currentDateTime().toString());
//            log->setText(1, socket->peerAddress().toString());
//            log->setText(2, QString::number(socket->peerPort()));
//            log->setText(3, name);
//            log->setText(4, "INVITE " + name);
        }
    }
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

void ServerForm::acceptConnection()
{
    qDebug("Connected, preparing to receive files!");
    ui->textEdit->append(tr("Connected, preparing to receive files!"));

//    receivedSocket = ftpServer->nextPendingConnection();
    QTcpSocket* ftpSocket = ftpServer->nextPendingConnection();

//    connect(receivedSocket, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(ftpSocket, SIGNAL(readyRead()), this, SLOT(readClient()));
    ftpSocketList.append(ftpSocket);

}

void ServerForm::readClient()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    QString ip = socket->peerAddress().toString();

    qDebug("Receiving file ...");
    ui->textEdit->append(tr("Receiving file ..."));

    if(byteReceived == 0) {     // 데이터를 받기 시작할 때 동작, 파일의 정보를 가져옴
        progressDialog->reset();
        progressDialog->show();

        QDataStream in(socket);     // 소켓 변경
        in >> totalSize >> byteReceived >> filename;
        progressDialog->setMaximum(totalSize);

        QTreeWidgetItem* log = new QTreeWidgetItem(ui->logTreeWidget);
        log->setText(0, QDateTime::currentDateTime().toString());
        log->setText(1, socket->peerAddress().toString());
        log->setText(2, QString::number(socket->peerPort()));
        log->setText(3, ipToClientName.value(ip));
        log->setText(4, "File Start " + filename);
        log->setToolTip(4, "File Start " + filename);

        QFileInfo info(filename);
        QString currentFileName = info.fileName();
        newFile = new QFile(currentFileName);
        newFile->open(QFile::WriteOnly);

    } else {
        inBlock = socket->readAll();        // 소켓 변경

        byteReceived += inBlock.size();
        newFile->write(inBlock);
        newFile->flush();
    }

    progressDialog->setValue(byteReceived);

    if(byteReceived == totalSize){
        qDebug() << QString("%1 receive completed").arg(filename);
        ui->textEdit->append(tr("%1 receive completed").arg(filename));

        QFileInfo info(filename);
        QString currentFileName = info.fileName();

        QListWidgetItem* fileItem = new QListWidgetItem(currentFileName, ui->fileListWidget);
        ui->fileListWidget->addItem(fileItem);

        foreach(QTcpSocket* sock, clientList){
            if(sock->peerAddress().toString() == ip)
                sock->write(Chat::FileList + currentFileName.toUtf8());
        }

        inBlock.clear();
        byteReceived = 0;
        totalSize = 0;
        progressDialog->reset();
        progressDialog->hide();
        newFile->close();

        delete newFile;
    }

}

void ServerForm::isClient(bool chk){
    idCheck = chk;
}
