#include "searchingdialog.h"
#include "ui_searchingdialog.h"

SearchingDialog::SearchingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchingDialog)
{
    ui->setupUi(this);

    ui->clientComboBox->addItem("ID");
    ui->clientComboBox->addItem("Name");
    ui->clientComboBox->addItem("Phone Number");
    ui->clientComboBox->addItem("Address");

}

SearchingDialog::~SearchingDialog()
{
    delete ui;
}

void SearchingDialog::returnId(int id)
{
    QTableWidgetItem* searchId = new QTableWidgetItem;
    searchId->setText(QString::number(id));

    emit searchedClient(0, QString::number(id));
}

void SearchingDialog::openDialog()
{
    show();
}

void SearchingDialog::displayRow(QList<QString> result)
{
    ui->tableWidget->setRowCount(0);

    while ( !result.empty() ) {
        int tableRowCount = ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount(tableRowCount + 1);

        ui->tableWidget->setItem(tableRowCount, 0, new QTableWidgetItem(result.takeFirst()));
        ui->tableWidget->setItem(tableRowCount, 1, new QTableWidgetItem(result.takeFirst()));
        ui->tableWidget->setItem(tableRowCount, 2, new QTableWidgetItem(result.takeFirst()));
        ui->tableWidget->setItem(tableRowCount, 3, new QTableWidgetItem(result.takeFirst()));
    }

}


void SearchingDialog::on_clientPushButton_clicked()
{
    QString searchingContent = ui->clientComboBox->currentText();
    if( "ID" == searchingContent ){
        emit searchedClient(0, ui->lineEdit->text());
    } else if( "Name" == searchingContent ){
        emit searchedClient(1, ui->lineEdit->text());
    } else if( "Phone Number" == searchingContent ){
        emit searchedClient(2, ui->lineEdit->text());
    } else if( "Address" == searchingContent ){
        emit searchedClient(3, ui->lineEdit->text());
    } else {

    }
}

