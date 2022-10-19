#include "chattingform.h"
#include "ui_chattingform.h"
#include <QTcpSocket>

#include "serverform.h"

#define BLOCK_SIZE      1024

ChattingForm::ChattingForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChattingForm)
{
    ui->setupUi(this);

    ui->ipLineEdit->setText("127.0.0.1");
    ui->portLineEdit->setText("19000");

    ui->messageLineEdit->setEnabled(false);
    ui->sentPushButton->setEnabled(false);

    connect(ui->statusPushButton, SIGNAL(clicked()), this, SLOT(connectPushButton()));
    connect(ui->sentPushButton, SIGNAL(clicked()), SLOT(sendData()));
    connect(ui->messageLineEdit, SIGNAL(returnPressed()), this, SLOT(sendData()));

    clientSocket = new QTcpSocket(this);
    connect(clientSocket, &QAbstractSocket::errorOccurred, this,
            [=]{qDebug() << clientSocket->errorString(); });
    connect(clientSocket, SIGNAL(readyRead()), SLOT(echoData()));
//    setWindowTitle(tr("Echo Client"));

}

ChattingForm::~ChattingForm()
{
    clientSocket->close();

    delete ui;
}

void ChattingForm::connectPushButton()
{
    QString name = ui->nameLineEdit->text();
    QString buttonText = ui->statusPushButton->text();
    QByteArray bytearray;
    if( "Connect" == buttonText ) {
        clientSocket->connectToHost(ui->ipLineEdit->text(), ui->portLineEdit->text().toInt());
        ui->statusPushButton->setText("Enroll");
    } else if( "Enroll" == buttonText ) {
        QString name = ui->nameLineEdit->text();

        if(name.length()){
            char type = Chat_Login;
            bytearray = type + name.toUtf8();
            clientSocket->write(bytearray);
            ui->statusPushButton->setText("Chat In");
        }
    } else if( "Chat In" == buttonText ) {
        char type = Chat_In;
        QString msg(name);
        bytearray = type + msg.toUtf8();
        clientSocket->write(bytearray);

        ui->statusPushButton->setDisabled(true);
        ui->messageLineEdit->setEnabled(true);
        ui->sentPushButton->setEnabled(true);
        ui->chattingTextEdit->append("Enter the chat room");
    }

}

void ChattingForm::echoData()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());

    if(clientSocket->bytesAvailable() > BLOCK_SIZE)     return;

    QByteArray bytearray = clientSocket->read(BLOCK_SIZE);
    char type = bytearray.at(0);
    QString data = bytearray.remove(0, 1);

qDebug("echo Data");

    switch(type){
    case Chat_In:
        ui->chattingTextEdit->append(QString(bytearray));
        break;

    case Chat_Talk:
        ui->chattingTextEdit->append(QString(bytearray));
        break;

    case Chat_KickOut:
        ui->statusPushButton->setDisabled(false);
        ui->messageLineEdit->setEnabled(false);
        ui->sentPushButton->setEnabled(false);
        ui->chattingTextEdit->append("Terminated in chat rooms from the server.");
        break;

    case Chat_Invite:
        ui->statusPushButton->setDisabled(true);
        ui->messageLineEdit->setEnabled(true);
        ui->sentPushButton->setEnabled(true);
        ui->chattingTextEdit->append("Invited to chat room by server.");
        break;
    }
}

void ChattingForm::sendData()
{
    QString str = ui->messageLineEdit->text();
    char type = Chat_Talk;

    ui->messageLineEdit->clear();
    if(str.length()){
        QByteArray bytearray;
        bytearray = type + str.toUtf8();
        clientSocket->write(bytearray);

        ui->chattingTextEdit->append("Me : " + str);
    }

}
