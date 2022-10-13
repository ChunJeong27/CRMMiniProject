#ifndef CLIENTFORM_H
#define CLIENTFORM_H

#include <QWidget>
#include <QTableWidgetItem>

namespace Ui {
class ClientForm;
}

class ClientForm : public QWidget
{
    Q_OBJECT

public:
    explicit ClientForm(QWidget *parent = nullptr);
    ~ClientForm();
    void loadData();

private:
    QList<QTableWidgetItem*> searchingList;

signals:
    void clientIdSearchingResult(QList<QString>);
    void clientId(int);

public slots:
    int makeId();
    void displayItem(int,int);
    void searchClientId(QString);
    void searchClientName(QString);
    void searchClientReturnId(QString);

private slots:
    void on_addPushButton_clicked();
    void on_modifyPushButton_clicked();
    void on_searchPushButton_clicked();
    void on_removePushButton_clicked();

    void on_nameLineEdit_returnPressed();

    void on_phoneNumLineEdit_returnPressed();

    void on_addressLineEdit_returnPressed();

    void on_idLineEdit_returnPressed();

    void on_clearPushButton_clicked();

private:
    Ui::ClientForm *ui;
};

#endif // CLIENTFORM_H
