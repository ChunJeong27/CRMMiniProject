#ifndef SERVERFORM_H
#define SERVERFORM_H

#include <QWidget>
#include <QList>

class QTcpServer;
class QTcpSocket;
class QFile;
class QProgressDialog;

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
    void banishClient();
    void inviteClient();

    void acceptConnection();
    void readClient();

private:
    QTcpServer *tcpServer;

    QList<QTcpSocket*> clientList;
    QHash<QString, QString> clientName;     // ip:port, Name
    QList<QTcpSocket*> waitingClient;  // Socket

    QTcpServer* ftpServer;
//    QTcpSocket* receivedSocket;
    QFile* newFile;
    QProgressDialog* progressDialog;

    QList<QTcpSocket*> ftpSocketList;
    QHash<QString, QString> ipToClientName;

    QByteArray inBlock;
    QString filename;
    qint64 totalSize;
    qint64 byteReceived;

    bool idCheck;

    void writeSocket(QTcpSocket*, char, QByteArray);
};

#endif // SERVERFORM_H
