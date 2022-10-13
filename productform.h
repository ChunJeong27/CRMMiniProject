#ifndef PRODUCTFORM_H
#define PRODUCTFORM_H

#include <QWidget>

namespace Ui {
class ProductForm;
}

class ProductForm : public QWidget
{
    Q_OBJECT

public:
    explicit ProductForm(QWidget *parent = nullptr);
    ~ProductForm();
    void loadData();

signals:
    void productSearchingResult(QList<QString>);

public slots:
    int makeId();
    void displayItem(int,int);
    void searchProductId(QString);
    void searchProductName(QString);

private slots:
    void on_addPushButton_clicked();

    void on_searchPushButton_clicked();

    void on_modifyPushButton_clicked();

    void on_removePushButton_clicked();

private:
    Ui::ProductForm *ui;
};

#endif // PRODUCTFORM_H
