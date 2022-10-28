#include "fileprotocol.h"

#include <QTcpSocket>
#include <QFile>
#include <QProgressDialog>
#include <QFileDialog>

FileProtocol::FileProtocol(QString address, quint16 port, QObject *parent)
    : QObject{parent}, serverAddress(address), serverPort(port)
{
    /* 파일 전송을 위한 소켓 */
    uploadClient = new QTcpSocket(this);
    connect(uploadClient, SIGNAL(bytesWritten(qint64)), SLOT(goOnSend(qint64)));
//    connect(uploadClient, SIGNAL(disconnected( )), uploadClient, SLOT(deletelater( )));

    uploadProgressDialog = new QProgressDialog(0);
    uploadProgressDialog->setAutoClose(true);
    uploadProgressDialog->reset();

}

void FileProtocol::goOnSend(qint64 numBytes) // Start sending file content
{
    uploadByteToWrite -= numBytes; // Remaining data size
    uploadOutBlock = uploadFile->read(qMin(uploadByteToWrite, numBytes));
    uploadClient->write(uploadOutBlock);

    uploadProgressDialog->setMaximum(uploadTotalSize);
    uploadProgressDialog->setValue(uploadTotalSize - uploadByteToWrite);

    if (uploadByteToWrite == 0) { // Send completed
        qDebug("File sending completed!");
        uploadProgressDialog->reset();
    }
}

void FileProtocol::sendFile() // Open the file and get the file name (including path)
{
    loadSize = 0;
    uploadByteToWrite = 0;
    uploadTotalSize = 0;
    uploadOutBlock.clear();

    QString filename = QFileDialog::getOpenFileName();
    if(filename.length()) {
        uploadFile = new QFile(filename);
        uploadFile->open(QFile::ReadOnly);

        qDebug() << QString("file %1 is opened").arg(filename);
        uploadProgressDialog->setValue(0); // Not sent for the first time

        if (!uploadIsSent) { // Only the first time it is sent, it happens when the connection generates the signal connect
            uploadClient->connectToHost(serverAddress, serverPort);
            uploadIsSent = true;
        }

        // When sending for the first time, connectToHost initiates the connect signal to call send, and you need to call send after the second time

        uploadByteToWrite = uploadTotalSize = uploadFile->size(); // The size of the remaining data
        loadSize = 1024; // The size of data sent each time

        QDataStream out(&uploadOutBlock, QIODevice::WriteOnly);
        out << qint64(0) << qint64(0) << filename;

        uploadTotalSize += uploadOutBlock.size(); // The total size is the file size plus the size of the file name and other information
        uploadByteToWrite += uploadOutBlock.size();

        out.device()->seek(0); // Go back to the beginning of the byte stream to write a qint64 in front, which is the total size and file name and other information size
        out << uploadTotalSize << qint64(uploadOutBlock.size());

        uploadClient->write(uploadOutBlock); // Send the read file to the socket

        uploadProgressDialog->setMaximum(uploadTotalSize);
        uploadProgressDialog->setValue(uploadTotalSize- uploadByteToWrite);
        uploadProgressDialog->show();
    }
    qDebug() << QString("Sending file %1").arg(filename);
}

/* 서버에서 파일을 다운로드하기 위한 함수 */
void FileProtocol::receiveFile()   // readClient
{
    qDebug("Downloading file ...");
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
//    QString ip = socket->peerAddress().toString();
    QString fileName;

    if(downloadByteReceived == 0) {     // 데이터를 받기 시작할 때 동작, 파일의 정보를 가져옴
        downloadProgressDialog->reset();
        downloadProgressDialog->show();

        QDataStream in(socket);     // 소켓 변경
        in >> downloadTotalSize >> downloadByteReceived >> fileName;
        downloadProgressDialog->setMaximum(downloadTotalSize);

        QFileInfo info(fileName);
        QString currentFileName = info.fileName();
        downloadNewFile = new QFile(/*"client/" + */currentFileName);
        downloadNewFile->open(QFile::WriteOnly);

    } else {
        downloadInBlock = socket->readAll();        // 소켓 변경

        downloadByteReceived += downloadInBlock.size();
        downloadNewFile->write(downloadInBlock);
        downloadNewFile->flush();
    }

    downloadProgressDialog->setValue(downloadByteReceived);

    if(downloadByteReceived == downloadTotalSize){
        qDebug() << QString("%1 receive completed").arg(fileName);


        downloadInBlock.clear();
        downloadByteReceived = 0;
        downloadTotalSize = 0;
        downloadProgressDialog->reset();
        downloadProgressDialog->hide();

        downloadNewFile->close();
        delete downloadNewFile;
    }

}
