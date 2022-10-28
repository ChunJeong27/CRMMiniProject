#ifndef FILEPROTOCOL_H
#define FILEPROTOCOL_H

#include <QObject>

class QTcpSocket;
class QFile;
class QProgressDialog;

class FileProtocol : public QObject
{
    Q_OBJECT
public:
    explicit FileProtocol(QString address = "127.0.0.1", quint16 port = 20000,
                          QObject *parent = nullptr);

signals:

public slots:
    void goOnSend(qint64);
    void sendFile();
    void receiveFile();

private:
    QString serverAddress;
    quint16 serverPort;

    QTcpSocket *uploadClient;
    QProgressDialog* uploadProgressDialog;    // 파일 진행 확인
    QFile* uploadFile;                    // 서버로 보내는 파일
    qint64 loadSize;                // 파일의 크기
    qint64 uploadByteToWrite;             // 보내는 파일의 크기
    qint64 uploadTotalSize;               // 전체 파일의 크기
    QByteArray uploadOutBlock;            // 전송을 위한 데이터
    bool uploadIsSent = 0;                    // 파일 서버에 접속되었는지 확인

    QTcpSocket* downloadFileClient;
    QProgressDialog* downloadProgressDialog;
    qint64 downloadTotalSize;
    qint64 downloadByteReceived = 0;
    QFile* downloadNewFile;
    QByteArray downloadInBlock;

};

#endif // FILEPROTOCOL_H
