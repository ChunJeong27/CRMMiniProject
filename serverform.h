#ifndef SERVERFORM_H
#define SERVERFORM_H

#include <QWidget>

class QTcpServer;
class QTcpSocket;

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

private slots:
    void clientConnect();
    void echoData();

private:
    QTcpServer *tcpServer;

    QList<QTcpSocket*> clientList;
};

#endif // SERVERFORM_H
