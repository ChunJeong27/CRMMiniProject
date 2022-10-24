#ifndef PRODUCTFORM_H
#define PRODUCTFORM_H

#include <QWidget>

class QTableWidgetItem;

namespace Ui {
class ProductForm;
}

class ProductForm : public QWidget
{
    Q_OBJECT

public:
    explicit ProductForm(QWidget *parent = nullptr);
    ~ProductForm();

private:
    Ui::ProductForm *ui;

public:
    void loadData();

private:
    QList<QTableWidgetItem*> searchingList;

signals:
    void returnSearching(QList<QString>);

public slots:
    int makeId();
    void displayItem(int,int);
    void searching(int, QString);

private slots:
    void on_clearPushButton_clicked();

    void on_addPushButton_clicked();

    void on_searchPushButton_clicked();

    void on_modifyPushButton_clicked();

    void on_removePushButton_clicked();

    void on_idLineEdit_returnPressed();

};

#endif // PRODUCTFORM_H
