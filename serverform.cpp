#include "serverform.h"
#include "ui_serverform.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QApplication>
#include <QMessageBox>
#include <QDateTime>
#include <QFileInfo>
#include <QProgressDialog>

#define BLOCK_SIZE  1024

ServerForm::ServerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServerForm)
{
    ui->setupUi(this);

    connect(ui->quitPushButton, SIGNAL(clicked()), qApp, SLOT(quit()));
    connect(ui->banishPushButton, SIGNAL(clicked()), this, SLOT(banishClient()));
    connect(ui->invitePushButton, SIGNAL(clicked()), this, SLOT(inviteClient()));

    tcpServer = new QTcpServer(this);
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(ConnectClient()));

    if(!tcpServer->listen(QHostAddress::Any, 19000)){
        QMessageBox::critical(this, tr("Echo Server"),
                              tr("Unable to start the server: %1.")
                              .arg(tcpServer->errorString()));
        close();
        return;
    }

    ui->portNumLineEdit->setText("Port : " + QString::number(tcpServer->serverPort()));
    qDebug()<<tr("The server is running on port %1.").arg(tcpServer->serverPort());

//    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(clickButton()));

    ui->textEdit->setText("File Server Start!!!");

    totalSize = 0;
    byteReceived = 0;

    ftpServer = new QTcpServer(this);
    connect(ftpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
    if(!ftpServer->listen(QHostAddress(QHostAddress::Any), 19100)){
        QMessageBox::critical(this, tr("Chatting Server"), \
                              tr("Unable to start the server: %1.") \
                              .arg(ftpServer->errorString( )));
        close( );
        return;
    }

    qDebug("Start listening ...");
    ui->textEdit->append(tr("Start listenint ..."));

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
    QTcpSocket *clientSocket = tcpServer->nextPendingConnection();

    connect(clientSocket, SIGNAL(readyRead()), SLOT(recieveData()));
    connect(clientSocket, SIGNAL(disconnected()), SLOT(removeItem()));

    QString ip = clientSocket->peerAddress().toString();
    quint16 port = clientSocket->peerPort();

    QListWidgetItem* clientItem = new QListWidgetItem(ip + ":" +QString::number(port));
    ui->clientListWidget->addItem(clientItem);

    clientList.append(clientSocket);
    waitingClient.append(clientSocket);
    foreach(auto t, waitingClient){
        qDebug() << t->peerPort();
    }

    clientSocket->write(Chat_Login + ip.toUtf8());
}

void ServerForm::recieveData()
{
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    // 슬롯을 호출한 시그널의 객체를 가져옴
    if(clientSocket == nullptr)     return;
    QByteArray byteArray = clientSocket->read(BLOCK_SIZE);
    // 소켓으로부터 서버에서 전송한 데이터
    QString ip = clientSocket->peerAddress().toString();
    quint16 port = clientSocket->peerPort();
    QString ipPort = ip + ":" + QString::number(port);

    char type = byteArray.at(0);                // 받은 데이터의 첫 1바이트는 서버에서 수행할 동작을 의미
    QString data = byteArray.remove(0, 1);      // 동작 바이트를 제거 및 문자열로 저장

    switch(type){
    case Chat_Login:
    {
        QList<QListWidgetItem*> result = ui->clientListWidget->findItems(ipPort, Qt::MatchExactly);
        clientName.insert(ipPort, data);    // 이름을 QList에 저장
        ipToClientName.insert(ip, data);

        if(result.isEmpty())
//            clientSocket->write(Chat_Talk + "Login Error");
            writeSocket(clientSocket, Chat_Talk, "Login Error");
        else {
            QListWidgetItem* listWidgetItem = result.first();
            listWidgetItem->setText(data);
        }

        data = "ENROLL " + data;
    } break;

    case Chat_In:
    case Chat_Invite:
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





        QList nameList = ui->clientListWidget->findItems("", Qt::MatchContains);
        outByteArray.append(QString::number(nameList.size()).toUtf8() + "/");
        foreach(QListWidgetItem* item, nameList){
            outByteArray.append(item->text().toUtf8() + "/");     // 채팅방 참여자 출력 기능 보류
        }
        qDebug("write");

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
                        writeSocket(sock, Chat_In, outByteArray);
                    }
                }
            } else {
                writeSocket(sock, Chat_In, outByteArray);
            }
        }

        data = "ENTER " + data;
    } break;

    case Chat_Talk:
    {
        QByteArray outByteArray = clientName[ipPort].toUtf8() + " : " + byteArray;

        foreach(QTcpSocket* sock, clientList){
            if(!waitingClient.isEmpty()){
                foreach(QTcpSocket* waiting, waitingClient){
                    if(sock != waiting && sock != clientSocket){
//                        sock->write(type + clientName[ipPort].toUtf8() + " : " + byteArray);
                        writeSocket(sock, Chat_Talk, outByteArray);
                    }
                }
            } else {
                if(sock != clientSocket){
//                    sock->write(type + clientName[ipPort].toUtf8() + " : " + byteArray);
                    writeSocket(sock, Chat_Talk, outByteArray);
                }
            }
        }
        data = "MESSAGE " + data;
    } break;

    case Chat_KickOut:
    case Chat_Close:
        waitingClient.append(clientSocket);
        data = "KICK OUT " + data;

        break;
    }

    QTreeWidgetItem* log = new QTreeWidgetItem(ui->logTreeWidget);
    log->setText(0, QDateTime::currentDateTime().toString());
    log->setText(1, ip);
    log->setText(2, QString::number(port));
    log->setText(3, clientName[ipPort]);
    log->setText(4, data);

}

void ServerForm::removeItem()
{
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender());
    clientList.removeOne(clientConnection);
    clientConnection->deleteLater();
}

void ServerForm::banishClient()
{
    QString name = ui->clientListWidget->currentItem()->text();
    QString ipPort = clientName.key(name.toUtf8());

    foreach(QTcpSocket* socket, clientList){
        if(socket->peerAddress().toString() + ":" + QString::number(socket->peerPort()) == ipPort){
            socket->write(Chat_KickOut + name.toUtf8());

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
    QString name = ui->clientListWidget->currentItem()->text();
    QString ipPort = clientName.key(name.toUtf8());

    foreach(QTcpSocket* socket, clientList){
        if(socket->peerAddress().toString() + ":" + QString::number(socket->peerPort()) == ipPort){
            qDebug("invite");
            socket->write(Chat_Invite + name.toUtf8());

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
                sock->write(Chat_FileList + currentFileName.toUtf8());
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
