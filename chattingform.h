#ifndef CHATTINGFORM_H
#define CHATTINGFORM_H

#include <QWidget>

class QTcpSocket;
class QFile;
class QProgressDialog;

namespace Ui {
class ChattingForm;
}

class ChattingForm : public QWidget
{
    Q_OBJECT

public:
    explicit ChattingForm(QWidget *parent = nullptr);
    ~ChattingForm();

private:
    Ui::ChattingForm *ui;

private slots:
    void echoData();
    void sendData();
    void connectPushButton();
    void goOnSend(qint64);
    void sendFile();

private:
    QTcpSocket *clientSocket;
    QTcpSocket *fileClient;
    QProgressDialog* progressDialog;    // 파일 진행 확인
    QFile* file;
    qint64 loadSize;
    qint64 byteToWrite;
    qint64 totalSize;
    QByteArray outBlock;
    bool isSent = false;

};

#endif // CHATTINGFORM_H
