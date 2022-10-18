#ifndef CHATTINGFORM_H
#define CHATTINGFORM_H

#include <QWidget>

class QTcpSocket;

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

private:
    QTcpSocket *clientSocket;
};

#endif // CHATTINGFORM_H
