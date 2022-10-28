#ifndef UPLOADPROTOCOL_H
#define UPLOADPROTOCOL_H

#include <QObject>

class QTcpSocket;
class QFile;
class QProgressDialog;

class UploadProtocol : public QObject
{
    Q_OBJECT
public:
    explicit UploadProtocol(bool isServer = false, QTcpSocket* socket = nullptr,
                            QString address = "127.0.0.1",
                            quint16 port = 20000, QObject *parent = nullptr);

signals:

public slots:
    void goOnSend(qint64);
    void sendFile();

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
    bool isServer;

};

#endif // UPLOADPROTOCOL_H
