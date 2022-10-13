#ifndef ORDERFORM_H
#define ORDERFORM_H

#include <QWidget>

class SearchingDialog;

namespace Ui {
class OrderForm;
}

class OrderForm : public QWidget
{
    Q_OBJECT

public:
    explicit OrderForm(QWidget *parent = nullptr);
    ~OrderForm();

    void loadData();

private:
    QString clientId;
    QString productId;
    QString clientName;
    QString clientAddress;
    QString clientPhoneNum;
    QString productName;
    QString productPrice;
    QString productStock;
    SearchingDialog* dlg;

signals:
    void clientReturnPressed(QString);
    void productReturnPressed(QString);
    void searchClientId(QString);
    void searchProductId(QString);
    void clientSearching(QString);

    void returnDialog(int);
    void clickedSearchButton();

public slots:
    int makeId();
    void displayItem(int,int);

    void addClientResult(QList<QString>);
    void addProductResult(QList<QString>);

    void returnId(int);


private slots:
    void on_addPushButton_clicked();

    void on_modifyPushButton_clicked();

    void on_removePushButton_clicked();

    void searchClientName(QString);
    void searchProductName();

private:
    Ui::OrderForm *ui;
};

#endif // ORDERFORM_H
