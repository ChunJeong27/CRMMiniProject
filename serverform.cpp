#include "serverform.h"
#include "ui_serverform.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QApplication>
#include <QMessageBox>
#include <QDateTime>

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

//    ui->textEdit->append(tr("The server is running on port %1.").arg(tcpServer->serverPort()));
    qDebug()<<tr("The server is running on port %1.").arg(tcpServer->serverPort());
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

    QListWidgetItem* clientItem = new QListWidgetItem(ip + "::" +QString::number(port));
    ui->clientListWidget->addItem(clientItem);

//    ui->textEdit->append("new connection " + ip + " is established... Port : " + QString::number(port));
//    ui->textEdit->append(QDateTime::currentDateTime().toString());

    clientList.append(clientConnection);

}

void ServerForm::echoData()
{
    qDebug("Server echoData");
    QTcpSocket *clientConnection = qobject_cast<QTcpSocket*>(sender());

    QByteArray bytearray = clientConnection->read(BLOCK_SIZE);
    char type = bytearray.at(0);
    bytearray.remove(0, 1);

    QString ip = clientConnection->peerAddress().toString();
    quint16 port = clientConnection->peerPort();
    QString listName = ip + "::" + QString::number(port);

    switch(type){
    case Chat_Login:
    {
        QList<QListWidgetItem*> result = ui->clientListWidget->findItems(listName, Qt::MatchExactly);
        clientName.insert(ip, bytearray);

        if(result.isEmpty())
            clientConnection->write("Error");
        else {
            QListWidgetItem* listWidgetItem = result.first();
            listWidgetItem->setText(bytearray.toStdString().data());

            clientConnection->write(&type);
            bytearray = "ENROLL " + bytearray;

        }

    }
        break;

    case Chat_In:
    {
        foreach(QTcpSocket* sock, clientList){
            sock->write(bytearray + " enter the Chat Room");
        }
        bytearray = "ENTER " + bytearray;

//        ui->textEdit->append(QString(bytearray) + " enter the Chat Room");
    }
        break;

    case Chat_Talk:
        foreach(QTcpSocket* sock, clientList){
            if(sock != clientConnection){
                sock->write(clientName[ip] + " : " + bytearray);
            }
        }
        bytearray = "MESSAGE " + bytearray;
//        ui->textEdit->append(QString(bytearray));
        break;
    }

    QTreeWidgetItem* log = new QTreeWidgetItem(ui->logTreeWidget);
    log->setText(0, QDateTime::currentDateTime().toString());
    log->setText(1, ip);
    log->setText(2, QString::number(port));
    log->setText(3, clientName[ip]);
    log->setText(4, bytearray);

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
    QString ip = clientName.key(name.toUtf8());

    char chatKickOut = Chat_KickOut;
    QByteArray bytearray = chatKickOut + name.toUtf8();

    foreach(QTcpSocket* socket, clientList){
        if(socket->peerAddress().toString() == ip){
            socket->write(bytearray);

            QTreeWidgetItem* log = new QTreeWidgetItem(ui->logTreeWidget);
            log->setText(0, QDateTime::currentDateTime().toString());
            log->setText(1, ip);
            log->setText(2, QString::number(socket->peerPort()));
            log->setText(3, name);
            log->setText(4, "KICK OUT " + name);
        }
    }
}

void ServerForm::inviteClient()
{
    QString name = ui->clientListWidget->currentItem()->text();
    QString ip = clientName.key(name.toUtf8());

    char chatInvite = Chat_Invite;
    QByteArray bytearray = chatInvite + name.toUtf8();

    foreach(QTcpSocket* socket, clientList){
        if(socket->peerAddress().toString() == ip){
            socket->write(bytearray);

            QTreeWidgetItem* log = new QTreeWidgetItem(ui->logTreeWidget);
            log->setText(0, QDateTime::currentDateTime().toString());
            log->setText(1, ip);
            log->setText(2, QString::number(socket->peerPort()));
            log->setText(3, name);
            log->setText(4, "INVITE " + name);
        }
    }
}
