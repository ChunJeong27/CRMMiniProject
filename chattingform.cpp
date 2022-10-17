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

    connect(ui->statusPushButton, SIGNAL(clicked()), this, SLOT(connectPushButton()));
    connect(ui->nameLineEdit, SIGNAL(returnPressed()), this, SLOT(connectPushButton()));
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
        QString msg("Enter the Chat Room");
        bytearray = type + msg.toUtf8();
        clientSocket->write(bytearray);

        ui->statusPushButton->setDisabled(true);
    }

}

void ChattingForm::echoData()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());

    if(clientSocket->bytesAvailable() > BLOCK_SIZE)     return;

    QByteArray bytearray = clientSocket->read(BLOCK_SIZE);
    char type = bytearray.at(0);
qDebug() << type;

    if(type != Chat_Login)
        ui->messageTextEdit->append(QString(bytearray));
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
    }
    ui->messageTextEdit->append("Me : " + str);
}

void ChattingForm::enterRoom()
{
//    QString str = ui->nameLineEdit->text();
//    char type = Chat_In;

//    if(str.length()){
//        QByteArray bytearray = type + str.toUtf8();
//        clientSocket->write(bytearray);
//    }
}


