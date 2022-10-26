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
            this, SLOT(banishClient()));
    connect(ui->invitePushButton, SIGNAL(clicked()),
            this, SLOT(inviteClient()));

    logThread = new LogThread(this);
    logThread->start();

    connect(ui->logSavePushButton, SIGNAL(clicked()),
            logThread, SLOT(saveData()));

    tcpServer = new QTcpServer(this);
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(ConnectClient()));

    if(!tcpServer->listen(QHostAddress::Any, 19000)){
        QMessageBox::critical(this, tr("Chatting Server"),
                              tr("Unable to start the server: %1.")
                              .arg(tcpServer->errorString()));
//        close();
        return;
    }

    ui->portNumLineEdit->setText("Port Number : "
                                 + QString::number(tcpServer->serverPort()));
    qDebug()<<tr("The server is running on port %1.")
              .arg(tcpServer->serverPort());

    ui->textEdit->setText("File Upload Server Start!!!");

    uploadTotalSize = 0;
    uploadByteReceived = 0;

    ftpUploadServer = new QTcpServer(this);
    connect(ftpUploadServer, SIGNAL(newConnection()),
            this, SLOT(acceptUploadConnection()));

    if(!ftpUploadServer->listen(QHostAddress(QHostAddress::Any), 19100)){
        QMessageBox::critical(this, tr("File Upload Server"),
                              tr("Unable to start the server: %1.")
                              .arg(ftpUploadServer->errorString( )));
//        close( );
        return;
    }

    ui->textEdit->append(tr("Start Upload listenint ..."));
    qDebug("Start Upload listening ...");

    ui->textEdit->setText("File Download Server Start!!!");

    ftpTransferServer = new QTcpServer(this);
    connect(ftpTransferServer, SIGNAL(newConnection()),
            this, SLOT(acceptTransferConnection()));

    if(!ftpTransferServer->listen(QHostAddress(QHostAddress::Any), 19200)){
        QMessageBox::critical(this, tr("File Download Server"),
                              tr("Unable to start the server: %1.")
                              .arg(ftpUploadServer->errorString( )));
//        close( );
        return;
    }

    ui->textEdit->append(tr("Start Download listenint ..."));
    qDebug("Start Download listening ...");

    progressDialog = new QProgressDialog(0);
    progressDialog->setAutoClose(true);
    progressDialog->reset();

//    QString filename = QFileDialog::getOpenFileName(this);
//    qDebug() << filename;

    transferProgressDialog = new QProgressDialog(this);
    transferProgressDialog->setAutoClose(true);
    transferProgressDialog->reset();

}

ServerForm::~ServerForm()
{
    delete ui;

    logThread->terminate();
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
        if(idCheck){
            QList<QListWidgetItem*> result =
                    ui->clientListWidget->findItems(ipPort, Qt::MatchExactly);
            clientName.insert(ipPort, idName.at(0)
                              + "(ID:" + idName.at(1) + ")");    // 이름을 QList에 저장
            ipToClientName.insert(ip, idName.at(0)
                                  + "(ID:" + idName.at(1) + ")");

            if(result.isEmpty()){

            } else {
                if(idCheck){
                QListWidgetItem* listWidgetItem(result.first());
                listWidgetItem->setText(idName.at(0)
                                        + "(ID:" + idName.at(1) + ")");
                }
            }
            action = "Connect";
        } else {
            disconnect(clientSocket);
            QList<QListWidgetItem*> findList = ui->clientListWidget->findItems("::ffff:", Qt::MatchStartsWith);
            if(!findList.isEmpty()){
                ui->clientListWidget->takeItem(ui->clientListWidget->row(findList.first()));
            }
            writeSocket(clientSocket, Chat::Disconnect, ipPort.toUtf8());
            clientSocket->disconnectFromHost();
        }
    } break;

    case Chat::Enter:
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

        if( "Invite" != action )
            action = "Enter";
    } break;

    case Chat::Leave:
        waitingClient.append(clientSocket);
        action = "Leave";
        break;

    case Chat::Message:
    {
        QByteArray outByteArray(clientName[ipPort].toUtf8() + " : " + byteArray);

        foreach(QTcpSocket* sock, clientList){
            if(!waitingClient.isEmpty()){
                foreach(QTcpSocket* waiting, waitingClient){
                    if(sock != waiting && sock != clientSocket){
                        writeSocket(sock, Chat::Message, outByteArray);
                    }
                }
            } else {
                if(sock != clientSocket){
                    writeSocket(sock, Chat::Message, outByteArray);
                }
            }
        }
        action = "MESSAGE ";
    } break;

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
        outByteArray.append(clientName[ipPort].toUtf8() + " invited by server./");

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

        action = "Invite";
    } break;

    case Chat::Banish:
    {
        waitingClient.append(clientSocket);
        QByteArray msg;
        msg.append("The server kicked ");
        msg.append(clientName.value(ipPort).toUtf8());
        msg.append(" out of the chat room.");

//        writeSocket( clientSocket, Chat::Message, msg);

        foreach(QTcpSocket* sock, clientList){
            if(!waitingClient.isEmpty()){
                foreach(QTcpSocket* waiting, waitingClient){
                    if(sock != waiting && sock != clientSocket)
                        writeSocket(sock, Chat::Message, msg);
                }
            } else {
                if(sock != clientSocket)
                    writeSocket(sock, Chat::Message, msg);
            }
        }

        action = "Banish";
    } break;

    case Chat::Disconnect:
    {
        disconnect(clientSocket);

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
    } break;

    default:
        break;
    }

    QTreeWidgetItem* log = new QTreeWidgetItem(ui->logTreeWidget);
    log->setText(0, QDateTime::currentDateTime().toString("yyMMdd hh:mm:ss"));
    log->setText(1, ip);
    log->setText(2, QString::number(port));
    log->setText(3, clientName[ipPort]);
    log->setText(4, action);
    log->setText(5, body);

    logThread->appendData(log);

}

void ServerForm::removeItem()
{
    QTcpSocket *clientConnection = qobject_cast<QTcpSocket *>(sender());
    clientList.removeOne(clientConnection);
    clientConnection->deleteLater();
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

void ServerForm::banishClient()
{
    QString name(ui->clientListWidget->currentItem()->text());
    QString ipPort(clientName.key(name));

    foreach(QTcpSocket* socket, clientList){
        if(socket->peerAddress().toString() + ":" + QString::number(socket->peerPort()) == ipPort){
            socket->write(Chat::Banish + name.toUtf8());
        }
    }
}

void ServerForm::acceptUploadConnection()
{
    qDebug("Connected, preparing to receive files!");
    ui->textEdit->append(tr("Connected, preparing to receive files!"));

    QTcpSocket* ftpSocket = ftpUploadServer->nextPendingConnection();

    connect(ftpSocket, SIGNAL(readyRead()), this, SLOT(readClient()));
}

void ServerForm::acceptTransferConnection()
{
    qDebug("Connected, preparing to receive files!(Download)");
    ui->textEdit->append(tr("Connected, preparing to receive files!(Download)"));

    transferFileClient = ftpTransferServer->nextPendingConnection();

    connect(transferFileClient, SIGNAL(bytesWritten(qint64)), this, SLOT(goOnSend(qint64)));
}

void ServerForm::readClient()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    QString ip = socket->peerAddress().toString();

    qDebug("Receiving file ...");
    ui->textEdit->append(tr("Receiving file ..."));

    if(uploadByteReceived == 0) {     // 데이터를 받기 시작할 때 동작, 파일의 정보를 가져옴
        progressDialog->reset();
        progressDialog->show();

        QDataStream in(socket);     // 소켓 변경
        in >> uploadTotalSize >> uploadByteReceived >> filename;
        progressDialog->setMaximum(uploadTotalSize);

        QTreeWidgetItem* log = new QTreeWidgetItem(ui->logTreeWidget);
        log->setText(0, QDateTime::currentDateTime().toString());
        log->setText(1, socket->peerAddress().toString());
        log->setText(2, QString::number(socket->peerPort()));
        log->setText(3, ipToClientName.value(ip));
        log->setText(4, "File Start " + filename);
        log->setToolTip(4, "File Start " + filename);

        logThread->appendData(log);

        QFileInfo info(filename);
        QString currentFileName = info.fileName();
        newFile = new QFile(currentFileName);
        newFile->open(QFile::WriteOnly);

    } else {
        inBlock = socket->readAll();        // 소켓 변경

        uploadByteReceived += inBlock.size();
        newFile->write(inBlock);
        newFile->flush();
    }

    progressDialog->setValue(uploadByteReceived);

    if(uploadByteReceived == uploadTotalSize){
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
        uploadByteReceived = 0;
        uploadTotalSize = 0;
        progressDialog->reset();
        progressDialog->hide();
        newFile->close();

        delete newFile;
    }

}

void ServerForm::isClient(bool chk){
    idCheck = chk;
}

void ServerForm::disconnectSocket(QTcpSocket* sockect){
    if(!clientList.isEmpty()){
        QList<QTcpSocket*>::Iterator eraseSock;
        for(auto sock = clientList.begin(); clientList.end() != sock; sock++){
            if(*sock == sockect){
                eraseSock = sock;
            }
        }
        clientList.erase(eraseSock);
    }

    if(!waitingClient.isEmpty()){
        QList<QTcpSocket*>::Iterator eraseSock;
        for(auto sock = waitingClient.begin(); waitingClient.end() != sock; sock++){
            if(*sock == sockect){
                eraseSock = sock;
            }
        }
        waitingClient.erase(eraseSock);
    }
}

/* 서버에서 채팅방으로 파일을 전송하기 위한 코드 */
// 파일을 보내는 신호가 도착했을 때!!
void ServerForm::sendFile(QListWidgetItem* fileNameItem) // Open the file and get the file name (including path)
{
    transferLoadSize = 0;
    transferByteToWrite = 0;
    transferTotalSize = 0;
    transferOutBlock.clear();

//    QString filename = QFileDialog::getOpenFileName(this);
    QString filename = QDir::currentPath() + "/" + fileNameItem->text();
//    qDebug() << filename;
//    return;
    transferFile = new QFile(filename);
    transferFile->open(QFile::ReadOnly);

    qDebug() << QString("file %1 is opened").arg(filename);
    transferProgressDialog->setValue(0); // Not sent for the first time

    // When sending for the first time, connectToHost initiates the connect signal to call send, and you need to call send after the second time

    transferByteToWrite = transferTotalSize = transferFile->size(); // The size of the remaining data

    qDebug() << transferTotalSize;

    transferLoadSize = 1024; // The size of data sent each time

    QDataStream out(&transferOutBlock, QIODevice::WriteOnly);
    out << qint64(0) << qint64(0) << filename;

    transferTotalSize += transferOutBlock.size(); // The total size is the file size plus the size of the file name and other information
    qDebug() << transferTotalSize;
    transferByteToWrite += transferOutBlock.size();

    out.device()->seek(0); // Go back to the beginning of the byte stream to write a qint64 in front, which is the total size and file name and other information size
    out << transferTotalSize << qint64(transferOutBlock.size());

    qDebug() << transferTotalSize;

    transferFileClient->write(transferOutBlock); // Send the read file to the socket

    transferProgressDialog->setMaximum(transferTotalSize);
    transferProgressDialog->setValue(transferTotalSize-transferByteToWrite);
    transferProgressDialog->show();

    qDebug() << QString("Sending file %1").arg(filename);
}

void ServerForm::goOnSend(qint64 numBytes) // Start sending file content
{
    qDebug("goOnSend");
    transferByteToWrite -= numBytes; // Remaining data size
    transferOutBlock = transferFile->read(qMin(transferByteToWrite, numBytes));
    transferFileClient->write(transferOutBlock);

    transferProgressDialog->setMaximum(transferTotalSize);
    transferProgressDialog->setValue(transferTotalSize-transferByteToWrite);

    if (transferByteToWrite == 0) { // Send completed
        qDebug("File sending completed!");
        transferProgressDialog->reset();
    }
}
