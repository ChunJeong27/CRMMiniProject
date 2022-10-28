#include "uploadprotocol.h"

#include <QTcpSocket>
#include <QFile>
#include <QProgressDialog>
#include <QFileDialog>

UploadProtocol::UploadProtocol( bool server, QTcpSocket* socket, QString address, quint16 port, QObject *parent)
    : QObject{parent}, serverAddress(address), serverPort(port), uploadClient(socket), isServer(server)
{
    /* 파일 전송을 위한 소켓 */
    if(uploadClient == nullptr){
        uploadClient = new QTcpSocket(this);
        connect(uploadClient, SIGNAL(bytesWritten(qint64)), SLOT(goOnSend(qint64)));
        //    connect(uploadClient, SIGNAL(disconnected( )), uploadClient, SLOT(deletelater( )));
    }

    uploadProgressDialog = new QProgressDialog(0);
    uploadProgressDialog->setAutoClose(true);
    uploadProgressDialog->reset();

}

void UploadProtocol::goOnSend(qint64 numBytes) // Start sending file content
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

void UploadProtocol::sendFile() // Open the file and get the file name (including path)
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

        if ( !(uploadIsSent || isServer) ) { // Only the first time it is sent, it happens when the connection generates the signal connect
            uploadClient->connectToHost(serverAddress, serverPort + 100);
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
