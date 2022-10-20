#include "chattingform.h"
#include "ui_chattingform.h"
#include <QTcpSocket>
#include <QFile>
#include <QProgressDialog>
#include <QFileDialog>
#include <QSettings>

#include "serverform.h"

#define BLOCK_SIZE      1024

ChattingForm::ChattingForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChattingForm)
{
    ui->setupUi(this);

    setWindowTitle(tr("Client Chatting Application"));

    connect(ui->statusPushButton, SIGNAL(clicked()), this, SLOT(connectPushButton()));
    connect(ui->sentPushButton, SIGNAL(clicked()), SLOT(sendData()));
    connect(ui->messageLineEdit, SIGNAL(returnPressed()), this, SLOT(sendData()));

    ui->ipLineEdit->setText("127.0.0.1");
    ui->portLineEdit->setText("19000");

    ui->messageLineEdit->setEnabled(false);
    ui->sentPushButton->setEnabled(false);

    clientSocket = new QTcpSocket(this);
    connect(clientSocket, &QAbstractSocket::errorOccurred, this,
            [=]{qDebug() << clientSocket->errorString(); });
    connect(clientSocket, SIGNAL(readyRead()), SLOT(echoData()));



    connect(clientSocket, SIGNAL(disconnected( )), SLOT(disconnect( )));

    QSettings settings("ChatClient", "Chat Client");
    ui->nameLineEdit->setText(settings.value("ChatClient/ID").toString());

    fileClient = new QTcpSocket(this);
    connect(fileClient, SIGNAL(bytesWritten(qint64)), SLOT(goOnSend(qint64)));

    progressDialog = new QProgressDialog(0);
    progressDialog->setAutoClose(true);
    progressDialog->reset();

    connect(ui->uploadPushButton, SIGNAL(clicked( )), SLOT(sendFile( )));
//    ui->uploadPushButton->setDisabled(true);

}

ChattingForm::~ChattingForm()
{
    clientSocket->close();

    delete ui;
}

//void ChattingForm::closeEvent(QCloseEvent*)
//{
//    sendProtocol(Chat_LogOut, name->text().toStdString().data());
//    clientSocket->disconnectFromHost();
//    if(clientSocket->state() != QAbstractSocket::UnconnectedState)
//        clientSocket->waitForDisconnected();
//}

void ChattingForm::connectPushButton()
{
    QString name = ui->nameLineEdit->text();
    QString buttonText = ui->statusPushButton->text();
    QByteArray bytearray;

    if( "Log In" == buttonText ) {
        clientSocket->connectToHost(ui->ipLineEdit->text(), ui->portLineEdit->text().toInt());

        if(name.length()){
//            clientSocket->write(Chat_Login + name.toUtf8());
            ui->statusPushButton->setText("Chat In");
        }
    } else if( "Chat In" == buttonText ) {

//        ui->statusPushButton->setDisabled(true);
        ui->messageLineEdit->setEnabled(true);
        ui->sentPushButton->setEnabled(true);
        ui->chattingTextEdit->append("Enter the chat room");

        clientSocket->write(Chat_In + name.toUtf8());
        ui->statusPushButton->setText("Chat Out");
    } else if( "Chat Out" == buttonText){
        ui->statusPushButton->setDisabled(false);
        ui->messageLineEdit->setEnabled(false);
        ui->sentPushButton->setEnabled(false);
        ui->chattingTextEdit->append("Chat Room Ended.");

        clientSocket->write(Chat_Close + name.toUtf8());
        ui->statusPushButton->setText("Chat In");
    }

}

void ChattingForm::echoData()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());

    if(clientSocket->bytesAvailable() > BLOCK_SIZE)     return;

    QByteArray bytearray = clientSocket->read(BLOCK_SIZE);
    QString name = ui->nameLineEdit->text();
    char type = bytearray.at(0);
    QString data = bytearray.remove(0, 1);

    switch(type){
    case Chat_Login:
        clientSocket->write(Chat_Login + ui->nameLineEdit->text().toUtf8());

        break;
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

        clientSocket->write(Chat_KickOut + name.toUtf8());
        break;

    case Chat_Invite:
        ui->statusPushButton->setDisabled(true);
        ui->messageLineEdit->setEnabled(true);
        ui->sentPushButton->setEnabled(true);
        ui->chattingTextEdit->append("Invited to chat room by server.");

        clientSocket->write(Chat_Invite + name.toUtf8());
        break;
    }
}

void ChattingForm::sendData()
{
    QString message = ui->messageLineEdit->text();

    ui->messageLineEdit->clear();
    if(message.length()){
        QByteArray bytearray = Chat_Talk + message.toUtf8();
        clientSocket->write(bytearray);

        ui->chattingTextEdit->append("Me : " + message);
    }

}

void ChattingForm::goOnSend(qint64 numBytes) // Start sending file content
{
    byteToWrite -= numBytes; // Remaining data size
    outBlock = file->read(qMin(byteToWrite, numBytes));
    fileClient->write(outBlock);

    progressDialog->setMaximum(totalSize);
    progressDialog->setValue(totalSize-byteToWrite);

    if (byteToWrite == 0) { // Send completed
        qDebug("File sending completed!");
        progressDialog->reset();
    }
}

void ChattingForm::sendFile() // Open the file and get the file name (including path)
{
    loadSize = 0;
    byteToWrite = 0;
    totalSize = 0;
    outBlock.clear();

    QString filename = QFileDialog::getOpenFileName(this);
    file = new QFile(filename);
    file->open(QFile::ReadOnly);

    qDebug() << QString("file %1 is opened").arg(filename);
    progressDialog->setValue(0); // Not sent for the first time

    if (!isSent) { // Only the first time it is sent, it happens when the connection generates the signal connect
        fileClient->connectToHost(ui->ipLineEdit->text( ),
                                  ui->portLineEdit->text( ).toInt( ) + 100);
        isSent = true;
    }

    // When sending for the first time, connectToHost initiates the connect signal to call send, and you need to call send after the second time

    byteToWrite = totalSize = file->size(); // The size of the remaining data
    loadSize = 1024; // The size of data sent each time

    QDataStream out(&outBlock, QIODevice::WriteOnly);
    out << qint64(0) << qint64(0) << filename;

    totalSize += outBlock.size(); // The total size is the file size plus the size of the file name and other information
    byteToWrite += outBlock.size();

    out.device()->seek(0); // Go back to the beginning of the byte stream to write a qint64 in front, which is the total size and file name and other information size
    out << totalSize << qint64(outBlock.size());

    fileClient->write(outBlock); // Send the read file to the socket

    progressDialog->setMaximum(totalSize);
    progressDialog->setValue(totalSize-byteToWrite);
    progressDialog->show();

    qDebug() << QString("Sending file %1").arg(filename);
}
