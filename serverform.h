#ifndef SERVERFORM_H
#define SERVERFORM_H

#include <QWidget>
#include <QList>

class QTcpServer;
class QTcpSocket;
class QFile;
class QProgressDialog;
class QListWidgetItem;

namespace Ui {
class ServerForm;
}

class ServerForm : public QWidget
{
    Q_OBJECT

public:
    explicit ServerForm(QWidget *parent = nullptr);
    ~ServerForm();

private:
    Ui::ServerForm *ui;

signals:
    void checkClientId(QString, QString);

public slots:
    void isClient(bool);

private slots:
    void ConnectClient();
    void recieveData();
    void removeItem();
    void inviteClient();
    void banishClient();

    void acceptUploadConnection();
    void readClient();

    void acceptTransferConnection();

    void sendFile(QListWidgetItem*);
//    void triggeredFileSend(QListWidgetItem*);
    void goOnSend(qint64);

private:
    void writeSocket(QTcpSocket*, char, QByteArray);
    void disconnectSocket(QTcpSocket*);

    QTcpServer *tcpServer;

    QList<QTcpSocket*> clientList;
    QHash<QString, QString> clientName;     // ip:port, Name
    QList<QTcpSocket*> waitingClient;  // Socket

    QTcpServer* ftpUploadServer;
    QFile* newFile;
    QProgressDialog* progressDialog;

    QHash<QString, QString> ipToClientName;

    QByteArray inBlock;
    QString filename;
    qint64 uploadTotalSize;
    qint64 uploadByteReceived;

    bool idCheck;

    QList<QString> fileNameList;

    QTcpServer* ftpTransferServer;

    qint64 transferLoadSize;
    qint64 transferByteToWrite;
    qint64 transferTotalSize;
    QByteArray transferOutBlock;

    QFile* transferFile;
    bool isTransferSent = false;
    QTcpSocket* transferFileClient;

    QProgressDialog* transferProgressDialog;



};

#endif // SERVERFORM_H
