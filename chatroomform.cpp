#include "chatroomform.h"
#include "ui_chatroomform.h"
#include <QTcpSocket>
#include <QFile>
#include <QProgressDialog>
#include <QFileDialog>
#include <QSettings>

#define BLOCK_SIZE      1024

ChatRoomForm::ChatRoomForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatRoomForm)
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
    connect(clientSocket, SIGNAL(readyRead()), SLOT(receiveData()));



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

ChatRoomForm::~ChatRoomForm()
{
    clientSocket->close();

    delete ui;
}

void ChatRoomForm::closeEvent(QCloseEvent*)
{
//    sendProtocol(Chat_LogOut, name->text().toStdString().data());
    clientSocket->write(Chat::Connect + (ui->nameLineEdit->text() + "@"
                                         + ui->idLineEdit->text()).toUtf8());
    clientSocket->disconnectFromHost();
    if(clientSocket->state() != QAbstractSocket::UnconnectedState)
        clientSocket->waitForDisconnected();
}

void ChatRoomForm::connectPushButton()
{
    QString name = ui->nameLineEdit->text();
    QString buttonText = ui->statusPushButton->text();

    if( "Connect" == buttonText ) {
        if(name.length()){
            clientSocket->connectToHost(ui->ipLineEdit->text(), ui->portLineEdit->text().toInt());
            ui->statusPushButton->setText("Enter");
        }
    } else if( "Enter" == buttonText ) {
        ui->messageLineEdit->setEnabled(true);
        ui->sentPushButton->setEnabled(true);
        ui->chattingTextEdit->append("Enter the chat room");

        clientSocket->write(Chat::Enter + name.toUtf8());
        ui->statusPushButton->setText("Leave");
    } else if( "Leave" == buttonText){
        ui->statusPushButton->setDisabled(false);
        ui->messageLineEdit->setEnabled(false);
        ui->sentPushButton->setEnabled(false);
        ui->chattingTextEdit->append("Chat Room Ended.");

        clientSocket->write(Chat::Leave + name.toUtf8());
        ui->statusPushButton->setText("Enter");
    }

}

void ChatRoomForm::receiveData()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());

    if(clientSocket->bytesAvailable() > BLOCK_SIZE)     return;

    QByteArray byteArray = clientSocket->read(BLOCK_SIZE);

    QString id = ui->idLineEdit->text();
    QString name = ui->nameLineEdit->text();

    char header(byteArray.at(0));
    QString body(byteArray.remove(0, 1));

    switch(header){
    case Chat::Connect:
        clientSocket->write(Chat::Connect + name.toUtf8() + "@" + id.toUtf8());
        break;

    case Chat::Enter:
    {
        QList<QString> dataList = body.split("/");
        qDebug() << dataList;

        ui->chattingTextEdit->append(dataList.takeFirst());
        ui->clientListWidget->clear();

        int cnt(dataList.takeFirst().toInt());
        for( int i = 0; i < cnt; i++){
            QListWidgetItem* nameItem = new QListWidgetItem(dataList.takeFirst());
            ui->clientListWidget->addItem(nameItem);
        }
        cnt = dataList.takeFirst().toInt();
        for( int i = 0; i < cnt; i++){
            QListWidgetItem* fileItem = new QListWidgetItem(dataList.takeFirst());
            ui->listWidget->addItem(fileItem);
        }
    } break;

    case Chat::Message:
        ui->chattingTextEdit->append(body);
        break;

    case Chat::Banish:
        ui->statusPushButton->setDisabled(false);
        ui->messageLineEdit->setEnabled(false);
        ui->sentPushButton->setEnabled(false);
        ui->chattingTextEdit->append("Terminated in chat rooms from the server.");

        clientSocket->write(Chat::Banish + name.toUtf8());
        break;

    case Chat::Invite:
        ui->statusPushButton->setDisabled(true);
        ui->messageLineEdit->setEnabled(true);
        ui->sentPushButton->setEnabled(true);
        ui->chattingTextEdit->append("Invited to chat room by server.");

        clientSocket->write(Chat::Invite + name.toUtf8());
        break;

    case Chat::Disconnect:
        ui->statusPushButton->setText("Connect");
        break;

    case Chat::ClientList:
    {
        QList<QString> dataList = body.split("/");
        qDebug() << dataList;

        ui->clientListWidget->clear();
        foreach(QString list, dataList){
            QListWidgetItem* nameItem = new QListWidgetItem(list);
            ui->clientListWidget->addItem(nameItem);
        }
    } break;

    case Chat::FileList:
        QListWidgetItem* fileItem = new QListWidgetItem(body);
        ui->listWidget->clear();
        ui->listWidget->addItem(fileItem);
        break;
    }
}

void ChatRoomForm::sendData()
{
    QString message = ui->messageLineEdit->text();
    ui->messageLineEdit->clear();

    if(message.length()){
        clientSocket->write(Chat::Message + message.toUtf8());
        ui->chattingTextEdit->append("Me : " + message);
    }

}

void ChatRoomForm::disconnect( )
{
//    QMessageBox::critical(this, tr("Chatting Client"), \
//                          tr("Disconnect from Server"));
    ui->messageLineEdit->setEnabled(false);
    ui->nameLineEdit->setReadOnly(false);
    ui->sentPushButton->setEnabled(false);
    ui->statusPushButton->setText(tr("Connect"));
}

void ChatRoomForm::goOnSend(qint64 numBytes) // Start sending file content
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

void ChatRoomForm::sendFile() // Open the file and get the file name (including path)
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
