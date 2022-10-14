#include "serverform.h"
#include "ui_serverform.h"
#include <QMessageBox>
#include <QTcpServer>
#include <QTcpSocket>

#define BLOCK_SIZE  1024

ServerForm::ServerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServerForm)
{
    ui->setupUi(this);

    connect(ui->quitPushButton, SIGNAL(clicked()), qApp, SLOT(quit()));

    tcpServer = new QTcpServer(this);
    connect(tcpServer, SIGNAL(newConnection()), SLOT(clientConnect()));
    if(!tcpServer->listen(QHostAddress::Any, 9000)){
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
    connect(clientConnection, SIGNAL(disconnected()), clientConnection, SLOT(deleteLater()));
    connect(clientConnection, SIGNAL(readyRead()), SLOT(echoData()));
    ui->textEdit->append("new connection is established...");

    clientList.append(clientConnection);
}

void ServerForm::echoData()
{
    QTcpSocket *clientConnection = qobject_cast<QTcpSocket*>(sender());

    QByteArray bytearray = clientConnection->read(BLOCK_SIZE);
    Q_FOREACH(QTcpSocket* sock, clientList){
        if(sock != clientConnection)
            sock->write(bytearray);
    }
    ui->textEdit->append(QString(bytearray));
}
