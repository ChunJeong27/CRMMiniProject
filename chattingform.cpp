#include "chattingform.h"
#include "ui_chattingform.h"
#include <QTcpSocket>

#define BLOCK_SIZE      1024

ChattingForm::ChattingForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChattingForm)
{
    ui->setupUi(this);

    connect(ui->enterPushButton, SIGNAL(clicked()), this, SLOT(enterRoom()));
    connect(ui->nameLineEdit, SIGNAL(returnPressed()), this, SLOT(enterRoom()));
    connect(ui->sentPushButton, SIGNAL(clicked()), SLOT(sendData()));
    connect(ui->messageLineEdit, SIGNAL(returnPressed()), this, SLOT(sendData()));

    clientSocket = new QTcpSocket(this);
    connect(clientSocket, &QAbstractSocket::errorOccurred,
            [=]{qDebug() << clientSocket->errorString(); });
    connect(clientSocket, SIGNAL(readyRead()), SLOT(echoData()));
//    setWindowTitle(tr("Echo Client"));

    clientSocket->connectToHost("127.0.0.1", 19000);
}

ChattingForm::~ChattingForm()
{
    clientSocket->close();

    delete ui;
}

void ChattingForm::echoData()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());

    if(clientSocket->bytesAvailable() > BLOCK_SIZE)     return;
    QByteArray bytearray = clientSocket->read(BLOCK_SIZE);
    ui->messageTextEdit->append(QString(bytearray));
}

void ChattingForm::sendData()
{
    QString str = ui->messageLineEdit->text();
    QString sendMsg = "Message : " + str;

    ui->messageLineEdit->clear();
    if(str.length()){
        QByteArray bytearray;
        bytearray = sendMsg.toUtf8();
        clientSocket->write(bytearray);
    }
    ui->messageTextEdit->append("Me : " + str);
}

void ChattingForm::enterRoom()
{
    QString str = "Client name : " + ui->nameLineEdit->text();

    if(str.length()){
        QByteArray bytearray = str.toUtf8();
        clientSocket->write(bytearray);
    }
}
