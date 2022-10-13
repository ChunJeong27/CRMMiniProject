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
    void searchClientName(QString);

public slots:
    void returnId(int);

private slots:
    void on_lineEdit_returnPressed();

    void on_pushButton_clicked();

private:
    Ui::SearchingDialog *ui;
};

#endif // SEARCHINGDIALOG_H
