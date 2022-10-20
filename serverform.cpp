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
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(clientConnect()));

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

void ServerForm::clientConnect()
{
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();

    connect(clientConnection, SIGNAL(readyRead()), SLOT(echoData()));
    connect(clientConnection, SIGNAL(disconnected()), SLOT(removeItem()));

    QString ip = clientConnection->peerAddress().toString();
    quint16 port = clientConnection->peerPort();

    QListWidgetItem* clientItem = new QListWidgetItem(ip + ":" +QString::number(port));
    ui->clientListWidget->addItem(clientItem);

    clientList.append(clientConnection);

}

void ServerForm::echoData()
{
    QTcpSocket *clientConnection = qobject_cast<QTcpSocket*>(sender());
    // 슬롯을 호출한 시그널의 객체를 가져옴
    QByteArray byteArray = clientConnection->read(BLOCK_SIZE);
    // 소켓으로부터 서버에서 전송한 데이터
    char type = byteArray.at(0);    // 받은 데이터의 첫 1바이트는 서버에서 수행할 동작을 의미
    QString data = byteArray.remove(0, 1);         // 동작 바이트를 제거 및 문자열로 저장

    QString ip = clientConnection->peerAddress().toString();
    quint16 port = clientConnection->peerPort();
    QString listName = ip + ":" + QString::number(port);

    switch(type){
    case Chat_Login:
    {
        QList<QListWidgetItem*> result = ui->clientListWidget->findItems(listName, Qt::MatchExactly);
        clientName.insert(listName, data);    // 이름을 QList에 저장

        if(result.isEmpty())
            clientConnection->write("Error");
        else {
            QListWidgetItem* listWidgetItem = result.first();
            listWidgetItem->setText(data);

//            clientConnection->write(&type);   // 불필요한 코드
            data = "ENROLL " + data;
        }
    } break;

    case Chat_In:
    {
        foreach(QTcpSocket* sock, clientList){
            if(clientConnection != sock)
                sock->write(type + clientName[listName].toUtf8() + " enter the Chat room");
        }

        QByteArray nameListByteArray;
        QList nameList = ui->clientListWidget->findItems("", Qt::MatchContains);
        foreach(QListWidgetItem* item, nameList)
            nameListByteArray += item->text().toUtf8();     // 채팅방 참여자 출력 기능 보류

        data = "ENTER " + data;
    } break;

    case Chat_Talk:
        foreach(QTcpSocket* sock, clientList){
            if(sock != clientConnection){
                sock->write(type + clientName[listName].toUtf8() + " : " + byteArray);
            }
        }
        data = "MESSAGE " + data;
        break;
    }

    QTreeWidgetItem* log = new QTreeWidgetItem(ui->logTreeWidget);
    log->setText(0, QDateTime::currentDateTime().toString());
    log->setText(1, ip);
    log->setText(2, QString::number(port));
    log->setText(3, clientName[listName]);
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

    char chatKickOut = Chat_KickOut;
    QByteArray bytearray = chatKickOut + name.toUtf8();

    foreach(QTcpSocket* socket, clientList){
        if(socket->peerAddress().toString() + ":" + QString::number(socket->peerPort()) == ipPort){
            socket->write(bytearray);

            QTreeWidgetItem* log = new QTreeWidgetItem(ui->logTreeWidget);
            log->setText(0, QDateTime::currentDateTime().toString());
            log->setText(1, socket->peerAddress().toString());
            log->setText(2, QString::number(socket->peerPort()));
            log->setText(3, name);
            log->setText(4, "KICK OUT " + name);
        }
    }
}

void ServerForm::inviteClient()
{
    QString name = ui->clientListWidget->currentItem()->text();
    QString ipPort = clientName.key(name.toUtf8());

    char chatInvite = Chat_Invite;
    QByteArray bytearray = chatInvite + name.toUtf8();

    foreach(QTcpSocket* socket, clientList){
        if(socket->peerAddress().toString() + ":" + QString::number(socket->peerPort()) == ipPort){
            socket->write(bytearray);

            QTreeWidgetItem* log = new QTreeWidgetItem(ui->logTreeWidget);
            log->setText(0, QDateTime::currentDateTime().toString());
            log->setText(1, socket->peerAddress().toString());
            log->setText(2, QString::number(socket->peerPort()));
            log->setText(3, name);
            log->setText(4, "INVITE " + name);
        }
    }
}

void ServerForm::acceptConnection()
{
    qDebug("Connected, preparing to receive files!");
    ui->textEdit->append(tr("Connected, preparing to receive files!"));

    receivedSocket = ftpServer->nextPendingConnection();

    connect(receivedSocket, SIGNAL(readyRead()), this, SLOT(readClient()));
}

void ServerForm::readClient()
{
    qDebug("Receiving file ...");
    ui->textEdit->append(tr("Receiving file ..."));

    if(byteReceived == 0) {     // 데이터를 받기 시작할 때 동작, 파일의 정보를 가져옴
        progressDialog->reset();
        progressDialog->show();

        QDataStream in(receivedSocket);
        in >> totalSize >> byteReceived >> filename;
        progressDialog->setMaximum(totalSize);

        QFileInfo info(filename);
        QString currentFileName = info.fileName();
        newFile = new QFile(currentFileName);
        newFile->open(QFile::WriteOnly);
    } else {
        inBlock = receivedSocket->readAll();

        byteReceived += inBlock.size();
        newFile->write(inBlock);
        newFile->flush();
    }

    progressDialog->setValue(byteReceived);

    if(byteReceived == totalSize){
        qDebug() << QString("%1 receive completed").arg(filename);
        ui->textEdit->append(tr("%1 receive completed").arg(filename));

        inBlock.clear();
        byteReceived = 0;
        totalSize = 0;
        progressDialog->reset();
        progressDialog->hide();
        newFile->close();

        delete newFile;
    }

}
