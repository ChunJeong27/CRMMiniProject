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

    tcpServer = new QTcpServer(this);
    connect(tcpServer, SIGNAL(newConnection()), SLOT(clientConnect()));
    if(!tcpServer->listen(QHostAddress::Any, 19000)){
        QMessageBox::critical(this, tr("Echo Server"),
                              tr("Unable to start the server: %1.")
                              .arg(tcpServer->errorString()));
        close();
        return;
    }

    ui->textEdit->append(tr("The server is running on port %1.").arg(tcpServer->serverPort()));
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

    QListWidgetItem* clientItem = new QListWidgetItem(ip);
    ui->clientListWidget->addItem(clientItem);

    ui->textEdit->append("new connection " + ip + " is established... Port : " + QString::number(port));
    ui->textEdit->append(QDateTime::currentDateTime().toString());

    clientList.append(clientConnection);
}

void ServerForm::echoData()
{
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket*>(sender());

    QByteArray bytearray = clientConnection->read(BLOCK_SIZE);
    QString ip;
    foreach(QTcpSocket* sock, clientList){
        if(sock != clientConnection){
//            ip = sock->peerAddress().toString().toUtf8();
            sock->write(bytearray);
        }
    }
    ui->textEdit->append(QString(bytearray));
}

void ServerForm::removeItem()
{
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender());
    clientList.removeOne(clientConnection);
    clientConnection->deleteLater();
}
