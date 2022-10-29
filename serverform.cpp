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
    delete ui;

    foreach(QTcpSocket* socket, clientSocketList){
        socket->disconnectFromHost();
    }

    logThread->terminate();
    tcpServer->close();
    uploadServer->close();
    transferServer->close();
}

void ServerForm::removeItem()
{
    QTcpSocket *clientConnection = qobject_cast<QTcpSocket *>(sender());
    clientSocketList.removeOne(clientConnection);
    clientConnection->deleteLater();
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
            action = "CONNECT";
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
        waitingClient.removeOne(clientSocket);
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

        foreach(QTcpSocket* sock, clientSocketList){
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
        action = "ENTER";
    } break;

    case Chat::Leave:
        waitingClient.append(clientSocket);
        action = "LEAVE";
        break;

    case Chat::Message:
    {
        QByteArray outByteArray(clientName[ipPort].toUtf8() + " : " + byteArray);

        foreach(QTcpSocket* sock, clientSocketList){
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
        waitingClient.removeOne(clientSocket);
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

        foreach(QTcpSocket* sock, clientSocketList){
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
        action = "INVITE";
    } break;

    case Chat::KickOut:
    {
        waitingClient.append(clientSocket);
        QByteArray msg;
        msg.append("The server kicked ");
        msg.append(clientName.value(ipPort).toUtf8());
        msg.append(" out of the chat room.");

//        writeSocket( clientSocket, Chat::Message, msg);

        foreach(QTcpSocket* sock, clientSocketList){
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
        action = "KICK OUT";
    } break;

    case Chat::Disconnect:
    {
        clientSocketList.removeOne(clientSocket);
        waitingClient.removeOne(clientSocket);

        QList<QListWidgetItem*> findList = ui->clientListWidget->findItems(body, Qt::MatchExactly);
        if(!findList.isEmpty()){
            ui->clientListWidget->takeItem(ui->clientListWidget->row(findList.first()));
        }
        QList<QListWidgetItem*> sendItemLsit = ui->clientListWidget->findItems("", Qt::MatchContains);
        QByteArray sendList;
        foreach(auto list, sendItemLsit){
            sendList.append(list->text().toUtf8() + "/");
        }

        foreach(QTcpSocket* sock, clientSocketList){
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
        action = "DISCONNECT";
    } break;

    case Chat::FileDownload:
        action = "DOWNLOAD";
        break;

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

void ServerForm::inviteClient()
{
    QString name(ui->clientListWidget->currentItem()->text());
    QString ipPort(clientName.key(name));

    foreach(QTcpSocket* socket, clientSocketList){
        if(socket->peerAddress().toString() + ":" + QString::number(socket->peerPort()) == ipPort){
            qDebug("invite");
            socket->write(Chat::Invite + name.toUtf8());
        }
    }
}

void ServerForm::kickOutClient()
{
    QString name(ui->clientListWidget->currentItem()->text());
    QString ipPort(clientName.key(name));

    foreach(QTcpSocket* socket, clientSocketList){
        if(socket->peerAddress().toString() + ":" + QString::number(socket->peerPort()) == ipPort){
            socket->write(Chat::KickOut + name.toUtf8());
        }
    }
}

void ServerForm::isClient(bool chk){
    idCheck = chk;
}

void ServerForm::acceptUploadConnection()
{
    qDebug("Connected, preparing to receive files!");
    ui->textEdit->append(tr("Connected, preparing to receive files!"));

    QTcpSocket* ftpSocket = uploadServer->nextPendingConnection();
    connect(ftpSocket, SIGNAL(readyRead()), this, SLOT(readClient()));
}

void ServerForm::acceptTransferConnection()
{
    qDebug("Connected, preparing to transfer files!(Download)");
    ui->textEdit->append(tr("Connected, preparing to transfer files!(Download)"));

    transferSocket = transferServer->nextPendingConnection();
    connect(transferSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(goOnSend(qint64)));
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
        in >> uploadTotalSize >> byteReceived >> filename;
        progressDialog->setMaximum(uploadTotalSize);

        QFileInfo info(filename);
        QString currentFileName = info.fileName();
        newFile = new QFile(currentFileName);
        newFile->open(QFile::WriteOnly);

        QTreeWidgetItem* log = new QTreeWidgetItem(ui->logTreeWidget);
        log->setText(0, QDateTime::currentDateTime().toString("yyMMdd hh:mm:ss"));
        log->setText(1, socket->peerAddress().toString());
        log->setText(2, QString::number(socket->peerPort()));
//        log->setText(3, ipToClientName.value(ip));
        log->setText(3, ipToClientName.value(ip));
        log->setText(4, "START UPLOADING");
        log->setText(5, currentFileName);
        log->setToolTip(5, currentFileName);
        logThread->appendData(log);
    } else {
        inBlock = socket->readAll();        // 소켓 변경

        byteReceived += inBlock.size();
        newFile->write(inBlock);
        newFile->flush();
    }

    progressDialog->setValue(byteReceived);

    if(byteReceived == uploadTotalSize){
        qDebug() << QString("%1 receive completed").arg(filename);
        ui->textEdit->append(tr("%1 receive completed").arg(filename));

        QFileInfo info(filename);
        QString currentFileName = info.fileName();

        QTreeWidgetItem* log = new QTreeWidgetItem(ui->logTreeWidget);
        log->setText(0, QDateTime::currentDateTime().toString("yyMMdd hh:mm:ss"));
        log->setText(1, socket->peerAddress().toString());
        log->setText(2, QString::number(socket->peerPort()));
//        log->setText(3, ipToClientName.value(ip));
        log->setText(3, ipToClientName.value(ip));
        log->setText(4, "UPLOAD FINSHED");
        log->setText(5, currentFileName);
        log->setToolTip(5, currentFileName);
        logThread->appendData(log);

        QListWidgetItem* fileItem = new QListWidgetItem(currentFileName, ui->fileListWidget);
        ui->fileListWidget->addItem(fileItem);

        QByteArray outByteArray;
        QList<QListWidgetItem*> fileList = ui->fileListWidget->findItems("", Qt::MatchContains);
        foreach(QListWidgetItem* item, fileList){
            outByteArray.append(item->text().toUtf8() + "/");
        }

        foreach(QTcpSocket* sock, clientSocketList){
            if(sock->peerAddress().toString() == ip)
                sock->write(Chat::FileList + outByteArray);
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

/* 서버에서 채팅방으로 파일을 전송하기 위한 코드 */
// 파일을 보내는 신호가 도착했을 때!!
void ServerForm::sendFile(QListWidgetItem* fileNameItem) // Open the file and get the file name (including path)
{
    loadSize = 0;
    byteToWrite = 0;
    transferTotalSize = 0;
    outBlock.clear();

    QString filename = QDir::currentPath() + "/" + fileNameItem->text();
    file = new QFile(filename);
    file->open(QFile::ReadOnly);

    qDebug() << QString("file %1 is opened").arg(filename);
    progressDialog->setValue(0); // Not sent for the first time

    // When sending for the first time, connectToHost initiates the connect signal to call send, and you need to call send after the second time

    byteToWrite = transferTotalSize = file->size(); // The size of the remaining data

    qDebug() << transferTotalSize;

    loadSize = 1024; // The size of data sent each time

    QDataStream out(&outBlock, QIODevice::WriteOnly);
    out << qint64(0) << qint64(0) << filename;

    transferTotalSize += outBlock.size(); // The total size is the file size plus the size of the file name and other information
    qDebug() << transferTotalSize;
    byteToWrite += outBlock.size();

    out.device()->seek(0); // Go back to the beginning of the byte stream to write a qint64 in front, which is the total size and file name and other information size
    out << transferTotalSize << qint64(outBlock.size());

    qDebug() << transferTotalSize;

    transferSocket->write(outBlock); // Send the read file to the socket

    progressDialog->setMaximum(transferTotalSize);
    progressDialog->setValue(transferTotalSize - byteToWrite);
    progressDialog->show();

    qDebug() << QString("Sending file %1").arg(filename);
}

void ServerForm::goOnSend(qint64 numBytes) // Start sending file content
{
    qDebug("goOnSend");
    byteToWrite -= numBytes; // Remaining data size
    outBlock = file->read(qMin(byteToWrite, numBytes));
    transferSocket->write(outBlock);

    progressDialog->setMaximum(transferTotalSize);
    progressDialog->setValue(transferTotalSize-byteToWrite);

    if (byteToWrite == 0) { // Send completed
        qDebug("File sending completed!");
        progressDialog->reset();
    }
}
