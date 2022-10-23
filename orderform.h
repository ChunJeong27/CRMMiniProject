#ifndef ORDERFORM_H
#define ORDERFORM_H

#include <QWidget>

class QTableWidgetItem;

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
    QList<QTableWidgetItem*> searchingList;

signals:
    void searchedClient(int, QString);
    void searchedProduct(int, QString);
    void clientSearching(QString);

    void returnDialog(int);
    void clickedSearchButton();

public slots:
    int makeId();
    void displayItem(int,int);

    void addClientResult(QList<QString>);
    void addProductResult(QList<QString>);
    void returnPressedSearching();


private slots:
    void on_addPushButton_clicked();

    void on_modifyPushButton_clicked();

    void on_removePushButton_clicked();

    void on_clearPushButton_clicked();

    void on_searchPushButton_clicked();

private:
    Ui::OrderForm *ui;
};

#endif // ORDERFORM_H
