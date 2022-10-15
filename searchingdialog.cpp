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

    connect(ui->clientTableWidget, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(returnSearching(int, int)));

}

SearchingDialog::~SearchingDialog()
{
    delete ui;
}

void SearchingDialog::openDialog()
{
    show();
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

void SearchingDialog::displayRow(QList<QString> result)
{
    ui->clientTableWidget->setRowCount(0);

    while ( !result.empty() ) {
        int tableRowCount = ui->clientTableWidget->rowCount();
        ui->clientTableWidget->setRowCount(tableRowCount + 1);

        ui->clientTableWidget->setItem(tableRowCount, 0, new QTableWidgetItem(result.takeFirst()));
        ui->clientTableWidget->setItem(tableRowCount, 1, new QTableWidgetItem(result.takeFirst()));
        ui->clientTableWidget->setItem(tableRowCount, 2, new QTableWidgetItem(result.takeFirst()));
        ui->clientTableWidget->setItem(tableRowCount, 3, new QTableWidgetItem(result.takeFirst()));
    }

}

void SearchingDialog::returnSearching(int row, int column)
{
    Q_UNUSED(column);

    QList<QString> result;
    result << ui->clientTableWidget->item(row, 0)->text()
           << ui->clientTableWidget->item(row, 1)->text()
           << ui->clientTableWidget->item(row, 2)->text()
           << ui->clientTableWidget->item(row, 3)->text();

    emit returnOrderForm(result);

}
