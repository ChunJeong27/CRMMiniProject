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
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(clientConnect()));
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
    qDebug("Server echoData");
    QTcpSocket *clientConnection = qobject_cast<QTcpSocket*>(sender());

    QByteArray bytearray = clientConnection->read(BLOCK_SIZE);
    char type = bytearray.at(0);
    bytearray.remove(0, 1);

    QListWidgetItem* listWidgetItem = ui->clientListWidget->item(0);
    QString ipText;

    switch(type){
    case Chat_Login:
        ipText = "(" + listWidgetItem->text() + ")";
        listWidgetItem->setText(bytearray.toStdString().data() + ipText);

        clientConnection->write(&type);
        break;

    case Chat_In:
        ui->textEdit->append(QString(bytearray));
        break;

    case Chat_Talk:
        foreach(QTcpSocket* sock, clientList){
            if(sock != clientConnection){
                sock->write(bytearray);
            }
        }
        ui->textEdit->append(QString(bytearray));
        break;
    }
}

void ServerForm::removeItem()
{
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender());
    clientList.removeOne(clientConnection);
    clientConnection->deleteLater();
}

void ServerForm::banishClient()
{

}
