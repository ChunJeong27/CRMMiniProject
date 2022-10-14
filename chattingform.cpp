#include "chattingform.h"
#include "ui_chattingform.h"
#include <QTcpSocket>

#define BLOCK_SIZE      1024

ChattingForm::ChattingForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChattingForm)
{
    ui->setupUi(this);

    connect(ui->sentPushButton, SIGNAL(clicked()), SLOT(sendData()));
    connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(sendData()));

    clientSocket = new QTcpSocket(this);
    connect(clientSocket, &QAbstractSocket::errorOccurred,
            [=]{qDebug()<<clientSocket->errorString();});
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
    QString ip = clientSocket->peerAddress().toString();
    if(clientSocket->bytesAvailable() > BLOCK_SIZE)     return;
    QByteArray bytearray = clientSocket->read(BLOCK_SIZE);
    ui->messageTextEdit->append(ip + " : " + QString(bytearray));
}

void ChattingForm::sendData()
{
    QString str = ui->lineEdit->text();
    ui->lineEdit->clear();
    if(str.length()){
        QByteArray bytearray;
        bytearray = str.toUtf8();
        clientSocket->write(bytearray);
    }
    ui->messageTextEdit->append("Me : " + str);
}
