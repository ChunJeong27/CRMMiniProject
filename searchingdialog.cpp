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

    ui->productComboBox->addItem("ID");
    ui->productComboBox->addItem("Name");
    ui->productComboBox->addItem("Price");
    ui->productComboBox->addItem("Stock");

    connect(ui->clientTableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(returnSearching(int,int)));
    connect(ui->productTableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(returnSearching(int,int)));

}

SearchingDialog::~SearchingDialog()
{
    delete ui;
}

void SearchingDialog::on_clientPushButton_clicked()
{
    QString comboBoxText = ui->clientComboBox->currentText();

    if( "ID" == comboBoxText ){
        emit searchedClient(0, ui->clientLineEdit->text());
    } else if( "Name" == comboBoxText ){
        emit searchedClient(1, ui->clientLineEdit->text());
    } else if( "Phone Number" == comboBoxText ){
        emit searchedClient(2, ui->clientLineEdit->text());
    } else if( "Address" == comboBoxText ){
        emit searchedClient(3, ui->clientLineEdit->text());
    } else {
        return;
    }
}

void SearchingDialog::on_productPushButton_clicked()
{
    QString comboBoxText = ui->productComboBox->currentText();

    if( "ID" == comboBoxText ){
        emit searchedProduct(0, ui->productLineEdit->text());
    } else if( "Name" == comboBoxText ){
        emit searchedProduct(1, ui->productLineEdit->text());
    } else if( "Price" == comboBoxText ){
        emit searchedProduct(2, ui->productLineEdit->text());
    } else if( "Stock" == comboBoxText ){
        emit searchedProduct(3, ui->productLineEdit->text());
    } else {
        return;
    }
}

void SearchingDialog::displayRow(QList<QString> result)
{
    QWidget* form = qobject_cast<QWidget*>(sender());

    if( form->objectName() == "ClientForm" ){
        ui->clientTableWidget->setRowCount(0);

        while ( !result.empty() ) {
            int tableRowCount = ui->clientTableWidget->rowCount();
            ui->clientTableWidget->setRowCount(tableRowCount + 1);

            ui->clientTableWidget->setItem(tableRowCount, 0, new QTableWidgetItem(result.takeFirst()));
            ui->clientTableWidget->setItem(tableRowCount, 1, new QTableWidgetItem(result.takeFirst()));
            ui->clientTableWidget->setItem(tableRowCount, 2, new QTableWidgetItem(result.takeFirst()));
            ui->clientTableWidget->setItem(tableRowCount, 3, new QTableWidgetItem(result.takeFirst()));
        }
    } else if( form->objectName() == "ProductForm" ){
        ui->productTableWidget->setRowCount(0);

        while ( !result.empty() ) {
            int tableRowCount = ui->productTableWidget->rowCount();
            ui->productTableWidget->setRowCount(tableRowCount + 1);

            ui->productTableWidget->setItem(tableRowCount, 0, new QTableWidgetItem(result.takeFirst()));
            ui->productTableWidget->setItem(tableRowCount, 1, new QTableWidgetItem(result.takeFirst()));
            ui->productTableWidget->setItem(tableRowCount, 2, new QTableWidgetItem(result.takeFirst()));
            ui->productTableWidget->setItem(tableRowCount, 3, new QTableWidgetItem(result.takeFirst()));
        }
    }
}

void SearchingDialog::returnSearching(int row, int column)
{
    Q_UNUSED(column);

    QTableWidget* tableWidget = qobject_cast<QTableWidget*>(sender());

    QList<QString> result;
    result << tableWidget->item(row, 0)->text()
           << tableWidget->item(row, 1)->text()
           << tableWidget->item(row, 2)->text()
           << tableWidget->item(row, 3)->text();

    QString objectName = tableWidget->objectName();

    if("clientTableWidget" == objectName)
        emit returnClient(result);
    else if("productTableWidget" == objectName){
        emit returnProduct(result);
    }
}

