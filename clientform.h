#ifndef CLIENTFORM_H
#define CLIENTFORM_H

#include <QWidget>

class QTableWidgetItem;

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
    void returnSearching(QList<QString>);
    void checkedIdName(bool);

public slots:
    int makeId();
    void displayItem(int,int);
    void searching(int, QString);
    void checkIdName(QString, QString);

private slots:
    void on_clearPushButton_clicked();

    void on_addPushButton_clicked();

    void on_searchPushButton_clicked();

    void on_modifyPushButton_clicked();

    void on_removePushButton_clicked();

    void on_idLineEdit_returnPressed();

private:
    Ui::ClientForm *ui;
};

#endif // CLIENTFORM_H
