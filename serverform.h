#ifndef SERVERFORM_H
#define SERVERFORM_H

#include <QWidget>
#include <QList>

class QTcpServer;
class QTcpSocket;
class QFile;
class QProgressDialog;
class QListWidgetItem;
class LogThread;
class QSqlQueryModel;

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

private:
    void writeSocket(QTcpSocket*, char, QByteArray);
    void sendActivatedChatRoom(char, QByteArray);
    void sendMessage(QTcpSocket*, QByteArray);

    const int BLOCK_SIZE = 1024;
    QTcpServer* tcpServer;
    QList<QTcpSocket*> clientSocketList;
    QHash<QString, QString> clientName;     // ip:port, Name
    QList<QTcpSocket*> waitingClient;  // Socket

    QTcpServer* uploadServer;
    QFile* newFile;
    QProgressDialog* progressDialog;
    QHash<QString, QString> ipToClientName;
    QByteArray inBlock;
    QString filename;
    qint64 uploadTotalSize;
    qint64 byteReceived;
    bool idCheck;

    QTcpServer* transferServer;
    QTcpSocket* transferSocket;
    qint64 loadSize;
    qint64 byteToWrite;
    qint64 transferTotalSize;
    QByteArray outBlock;
    QFile* file;
    QList<QString> fileNameList;

    LogThread* logThread;

    QSqlQueryModel* clientQueryModel;

signals:
    void checkClientId(QString, QString);

public slots:
    void isClient(bool);

private slots:
    void removeItem();
    void connectClient();
    void recieveData();
    void inviteClient();
    void kickOutClient();

    void acceptUploadConnection();
    void readClient();
    void acceptTransferConnection();
    void sendFile(QListWidgetItem*);
    void goOnSend(qint64);

};

#endif // SERVERFORM_H
