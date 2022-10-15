#ifndef SEARCHINGDIALOG_H
#define SEARCHINGDIALOG_H

#include <QDialog>

namespace Ui {
class SearchingDialog;
}

class SearchingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchingDialog(QWidget *parent = nullptr);
    ~SearchingDialog();

protected:
//    void closeEvent(QCloseEvent*) override;

signals:
    void searchedClient(int, QString);
    void searchedProduct(int, QString);
    void returnOrderForm(QList<QString>);

public slots:
    void displayRow(QList<QString>);
    void returnSearching(int, int);

private slots:

    void on_clientPushButton_clicked();

    void on_productPushButton_clicked();

private:
    Ui::SearchingDialog *ui;
};

#endif // SEARCHINGDIALOG_H
