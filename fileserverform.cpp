#include "fileserverform.h"
#include "ui_fileserverform.h"

#include <QFile>
#include <QProgressDialog>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFileInfo>

FileServerForm::FileServerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileServerForm)
{
    ui->setupUi(this);

    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(clickButton()));

    ui->textEdit->append("File Server Start!!!");

    progressDialog = new QProgressDialog(0);
    progressDialog->setAutoClose(true);
    progressDialog->reset();
}

FileServerForm::~FileServerForm()
{
    delete ui;
}

void FileServerForm::clickButton()
{
    totalSize = 0;
    byteReceived = 0;

    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
    server->listen(QHostAddress(QHostAddress::Any), 20000);

    ui->textEdit->append(tr("Start listenint ..."));

}

void FileServerForm::acceptConnection()
{
    ui->textEdit->append(tr("Connected, preparing to receive files!"));

    receivedSocket = server->nextPendingConnection();

    connect(receivedSocket, SIGNAL(readyRead()), this, SLOT(readClient()));
}

void FileServerForm::readClient()
{
    ui->textEdit->append(tr("Receiving file ..."));

    if(byteReceived == 0) {     // 데이터를 받기 시작할 때 동작, 파일의 정보를 가져옴
        progressDialog->reset();
        progressDialog->show();

        QDataStream in(receivedSocket);
        in >> totalSize >> byteReceived >> filename;
        progressDialog->setMaximum(totalSize);

        QFileInfo info(filename);
        QString currentFileName = info.fileName();
        newFile = new QFile(currentFileName);
        newFile->open(QFile::WriteOnly);
    } else {
        inBlock = receivedSocket->readAll();

        byteReceived += inBlock.size();
        newFile->write(inBlock);
        newFile->flush();
    }

    progressDialog->setValue(byteReceived);

    if(byteReceived == totalSize){
        ui->textEdit->setText(tr("%1 receive completed").arg(filename));

        inBlock.clear();
        byteReceived = 0;
        totalSize = 0;
        progressDialog->reset();
        progressDialog->hide();
        newFile->close();

        delete newFile;
    }

}
