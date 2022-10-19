#ifndef FILESERVERFORM_H
#define FILESERVERFORM_H

#include <QWidget>

class QFile;
class QProgressDialog;
class QTcpServer;
class QTcpSocket;

namespace Ui {
class FileServerForm;
}

class FileServerForm : public QWidget
{
    Q_OBJECT

public:
    explicit FileServerForm(QWidget *parent = nullptr);
    ~FileServerForm();

    QTcpServer* server;
    QTcpSocket* receivedSocket;
    QFile* newFile;
    QProgressDialog* progressDialog;

    QByteArray inBlock;
    QString filename;
    qint64 totalSize;
    qint64 byteReceived;

private slots:
    void clickButton();
    void acceptConnection();
    void readClient();


private:
    Ui::FileServerForm *ui;
};

#endif // FILESERVERFORM_H
