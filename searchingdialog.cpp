#include "searchingdialog.h"
#include "ui_searchingdialog.h"

SearchingDialog::SearchingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchingDialog)
{
    ui->setupUi(this);

    ui->comboBox->addItem("ID");
    ui->comboBox->addItem("Name");
    ui->comboBox->addItem("Phone Number");
    ui->comboBox->addItem("Address");

}

SearchingDialog::~SearchingDialog()
{
    delete ui;
}

void SearchingDialog::returnId(int id)
{
    QTableWidgetItem* searchId = new QTableWidgetItem;
    searchId->setText(QString::number(id));

    emit searchedClientId(QString::number(id));
}


void SearchingDialog::on_pushButton_clicked()
{

    emit searchedClientId(ui->lineEdit->text());
}

void SearchingDialog::openDialog()
{
    show();
}

void SearchingDialog::displayRow(QList<QString> result)
{
    QTableWidgetItem* clientId = new QTableWidgetItem(result.at(0));
    QTableWidgetItem* clientName = new QTableWidgetItem(result.at(1));
    QTableWidgetItem* clientPhoneNum = new QTableWidgetItem(result.at(2));
    QTableWidgetItem* clientAddress = new QTableWidgetItem(result.at(3));

    int tableRowCount = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(tableRowCount + 1);

    ui->tableWidget->setItem(tableRowCount, 0, clientId);
    ui->tableWidget->setItem(tableRowCount, 1, clientName);
    ui->tableWidget->setItem(tableRowCount, 2, clientPhoneNum);
    ui->tableWidget->setItem(tableRowCount, 3, clientAddress);

}

