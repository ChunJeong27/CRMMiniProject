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

private:
    Ui::SearchingDialog *ui;

protected:
//    void closeEvent(QCloseEvent*) override;

signals:
    void searchedClient(int, QString);
    void searchedProduct(int, QString);
    void returnClient(QList<QString>);
    void returnProduct(QList<QString>);

public slots:
    void displayRow(QList<QString>);
    void returnSearching(int, int);

};

#endif // SEARCHINGDIALOG_H
