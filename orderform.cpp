#include "orderform.h"
#include "ui_orderform.h"

#include <QFile>
#include <QTableWidgetItem>
#include <searchingdialog.h>

OrderForm::OrderForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrderForm)
{
    ui->setupUi(this);

    connect(ui->tableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(displayItem(int,int)));

    connect(ui->clientNameLineEdit, SIGNAL(returnPressed()), this, SLOT(returnPressedSearching()));
    connect(ui->clientPhoneNumLineEdit, SIGNAL(returnPressed()), this, SLOT(returnPressedSearching()));
    connect(ui->clientAddressLineEdit, SIGNAL(returnPressed()), this, SLOT(returnPressedSearching()));
    connect(ui->productNameLineEdit, SIGNAL(returnPressed()), this, SLOT(returnPressedSearching()));
    connect(ui->productPriceLineEdit, SIGNAL(returnPressed()), this, SLOT(returnPressedSearching()));
    connect(ui->amountLineEdit, SIGNAL(returnPressed()), this, SLOT(returnPressedSearching()));
    connect(ui->orderIdLineEdit, SIGNAL(returnPressed()), this, SLOT(on_searchPushButton_clicked()));
    connect(ui->quantitySpinBox, &QSpinBox::valueChanged, this,
            [=](int i){quint32 amount = ui->productPriceLineEdit->text().toUInt() * i;
        ui->amountLineEdit->setText(QString::number(amount));});

    connect(ui->selectPushButton, SIGNAL(clicked()), this, SIGNAL(clickedSearchButton()));
}

OrderForm::~OrderForm()
{
    QFile file("orderForm.txt");

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    int tableRowCount = ui->tableWidget->rowCount();
    for (int i = 0; i < tableRowCount; i++) {
        out << ui->tableWidget->item(i, 0)->text() << ","
            << ui->tableWidget->item(i, 1)->text() << ","
            << ui->tableWidget->item(i, 2)->text() << ","
            << ui->tableWidget->item(i, 8)->text() << "\n";
    }
    file.close();

    delete ui;
}

void OrderForm::loadData()
{
    QFile file("orderForm.txt");

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList row = line.split(",");
        if (!row.empty()) {
            QTableWidgetItem* orderIdItem = new QTableWidgetItem(row.at(0));
            QTableWidgetItem* clientIdItem = new QTableWidgetItem(row.at(1));
            QTableWidgetItem* productIdItem = new QTableWidgetItem(row.at(2));
            QTableWidgetItem* orderQuantityItem = new QTableWidgetItem(row.at(3));

            emit searchedClient(0, row.at(1));
            emit searchedProduct(0, row.at(2));

            QTableWidgetItem* clientNameItem = new QTableWidgetItem(clientName);
            QTableWidgetItem* clientPhoneNumItem = new QTableWidgetItem(clientPhoneNum);
            QTableWidgetItem* clientAddressItem = new QTableWidgetItem(clientAddress);
            QTableWidgetItem* productNameItem = new QTableWidgetItem(productName);
            QTableWidgetItem* productPriceItem = new QTableWidgetItem(productPrice);

            quint32 amount = productPrice.toUInt() * row.at(3).toUInt();
            QTableWidgetItem* orderAmountItem = new QTableWidgetItem(QString::number(amount));

            int tableRowCount = ui->tableWidget->rowCount();
            ui->tableWidget->setRowCount( tableRowCount + 1);

            ui->tableWidget->setItem(tableRowCount, 0, orderIdItem);
            ui->tableWidget->setItem(tableRowCount, 1, clientIdItem);
            ui->tableWidget->setItem(tableRowCount, 2, productIdItem);
            ui->tableWidget->setItem(tableRowCount, 3, clientNameItem);
            ui->tableWidget->setItem(tableRowCount, 4, clientPhoneNumItem);
            ui->tableWidget->setItem(tableRowCount, 5, clientAddressItem);
            ui->tableWidget->setItem(tableRowCount, 6, productNameItem);
            ui->tableWidget->setItem(tableRowCount, 7, productPriceItem);
            ui->tableWidget->setItem(tableRowCount, 8, orderQuantityItem);
            ui->tableWidget->setItem(tableRowCount, 9, orderAmountItem);
        }
    }
    on_clearPushButton_clicked();
    file.close();

}

int OrderForm::makeId()
{
    int lastColumn = ui->tableWidget->rowCount();

    if( lastColumn <= 0 )
        return 3000;

    int creatId = ui->tableWidget->item(lastColumn - 1, 0)->text().toInt();
    return creatId + 1;
}

void OrderForm::displayItem(int row,int column)
{
    Q_UNUSED(column);

    ui->orderIdLineEdit->setText(ui->tableWidget->item(row, 0)->text());
    ui->clientNameLineEdit->setText(ui->tableWidget->item(row, 3)->text());
    ui->clientPhoneNumLineEdit->setText(ui->tableWidget->item(row, 4)->text());
    ui->clientAddressLineEdit->setText(ui->tableWidget->item(row, 5)->text());
    ui->productNameLineEdit->setText(ui->tableWidget->item(row, 6)->text());
    ui->productPriceLineEdit->setText(ui->tableWidget->item(row, 7)->text());
    ui->quantitySpinBox->setValue(ui->tableWidget->item(row, 8)->text().toInt());
    ui->amountLineEdit->setText(ui->tableWidget->item(row, 9)->text());
}

void OrderForm::on_addPushButton_clicked()
{
    QTableWidgetItem* orderIdItem = new QTableWidgetItem(QString::number(makeId()));
    QTableWidgetItem* clientIdItem = new QTableWidgetItem(clientId);
    QTableWidgetItem* productIdItem = new QTableWidgetItem(productId);

    QTableWidgetItem* clientNameItem = new QTableWidgetItem(clientName);
    QTableWidgetItem* clientPhoneNumItem = new QTableWidgetItem(clientPhoneNum);
    QTableWidgetItem* clientAddressItem = new QTableWidgetItem(clientAddress);

    QTableWidgetItem* productNameItem = new QTableWidgetItem(productName);
    QTableWidgetItem* productPriceItem = new QTableWidgetItem(productPrice);

    QTableWidgetItem* orderQuantityItem = new QTableWidgetItem(ui->quantitySpinBox->text());
    quint32 amount = productPrice.toUInt() * ui->quantitySpinBox->text().toUInt();
    QTableWidgetItem* orderAmountItem = new QTableWidgetItem(QString::number(amount));

    int tableRowCount = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount( tableRowCount + 1);

    ui->tableWidget->setItem(tableRowCount, 0, orderIdItem);
    ui->tableWidget->setItem(tableRowCount, 1, clientIdItem);
    ui->tableWidget->setItem(tableRowCount, 2, productIdItem);
    ui->tableWidget->setItem(tableRowCount, 3, clientNameItem);
    ui->tableWidget->setItem(tableRowCount, 4, clientPhoneNumItem);
    ui->tableWidget->setItem(tableRowCount, 5, clientAddressItem);
    ui->tableWidget->setItem(tableRowCount, 6, productNameItem);
    ui->tableWidget->setItem(tableRowCount, 7, productPriceItem);
    ui->tableWidget->setItem(tableRowCount, 8, orderQuantityItem);
    ui->tableWidget->setItem(tableRowCount, 9, orderAmountItem);

}


void OrderForm::on_modifyPushButton_clicked()
{
    int tableCurrentRow = ui->tableWidget->currentRow();

    QTableWidgetItem* clientIdItem = new QTableWidgetItem(clientId);
    QTableWidgetItem* productIdItem = new QTableWidgetItem(productId);

    QTableWidgetItem* clientNameItem = new QTableWidgetItem(ui->clientNameLineEdit->text());
    QTableWidgetItem* clientPhoneNumItem = new QTableWidgetItem(ui->clientPhoneNumLineEdit->text());
    QTableWidgetItem* clientAddressItem = new QTableWidgetItem(ui->clientAddressLineEdit->text());

    QTableWidgetItem* productNameItem = new QTableWidgetItem(ui->productNameLineEdit->text());
    QTableWidgetItem* productPriceItem = new QTableWidgetItem(ui->productPriceLineEdit->text());

    QTableWidgetItem* orderQuantityItem = new QTableWidgetItem(ui->quantitySpinBox->text());
    QTableWidgetItem* orderAmountItem = new QTableWidgetItem(ui->amountLineEdit->text());

    ui->tableWidget->setItem(tableCurrentRow, 1, clientIdItem);
    ui->tableWidget->setItem(tableCurrentRow, 2, productIdItem);
    ui->tableWidget->setItem(tableCurrentRow, 3, clientNameItem);
    ui->tableWidget->setItem(tableCurrentRow, 4, clientPhoneNumItem);
    ui->tableWidget->setItem(tableCurrentRow, 5, clientAddressItem);
    ui->tableWidget->setItem(tableCurrentRow, 6, productNameItem);
    ui->tableWidget->setItem(tableCurrentRow, 7, productPriceItem);
    ui->tableWidget->setItem(tableCurrentRow, 8, orderQuantityItem);
    ui->tableWidget->setItem(tableCurrentRow, 9, orderAmountItem);

}

void OrderForm::on_removePushButton_clicked()
{
    ui->tableWidget->removeRow(ui->tableWidget->currentRow());

}

void OrderForm::addClientResult(QList<QString> result)
{
    clientId = result.at(0);
    clientName = result.at(1);
    clientPhoneNum = result.at(2);
    clientAddress = result.at(3);
    ui->clientNameLineEdit->setText(result.at(1));
    ui->clientPhoneNumLineEdit->setText(result.at(2));
    ui->clientAddressLineEdit->setText(result.at(3));

}

void OrderForm::addProductResult(QList<QString> result)
{
    productId = result.at(0);
    productName = result.at(1);
    productPrice = result.at(2);
    productStock = result.at(3);
    ui->productNameLineEdit->setText(result.at(1));
    ui->productPriceLineEdit->setText(result.at(2));

}

void OrderForm::on_clearPushButton_clicked()
{
    ui->clientNameLineEdit->clear();
    ui->clientPhoneNumLineEdit->clear();
    ui->clientAddressLineEdit->clear();
    ui->productNameLineEdit->clear();
    ui->productPriceLineEdit->clear();
    ui->orderIdLineEdit->clear();
    ui->quantitySpinBox->clear();
    ui->amountLineEdit->clear();
}


void OrderForm::returnPressedSearching()
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(sender());
    if(lineEdit == nullptr)     return;

    int lineEditColumn(0);
    if( lineEdit->objectName() == "clientNameLineEdit" ){
        lineEditColumn = 3;
    } else if( lineEdit->objectName() == "clientPhoneNumLineEdit" ){
        lineEditColumn = 4;
    } else if( lineEdit->objectName() == "clientAddressLineEdit" ){
        lineEditColumn = 5;
    } else if( lineEdit->objectName() == "productNameLineEdit" ){
        lineEditColumn = 6;
    } else if( lineEdit->objectName() == "productPriceLineEdit" ){
        lineEditColumn = 7;
    } else if( lineEdit->objectName() == "amountLineEdit" ){
        lineEditColumn = 9;
    } else {
        return;
    }

    QString searchingText = lineEdit->text();
    if ( searchingList.isEmpty()
         || searchingList.first()->text().compare(searchingText, Qt::CaseInsensitive)) {
        searchingList = ui->tableWidget->findItems(searchingText, Qt::MatchFixedString);
        if(searchingList.isEmpty())     return;
    }

    QTableWidgetItem* searchingResult = searchingList.takeFirst();
    while ( searchingResult->column() != lineEditColumn ) {
        if(searchingList.isEmpty())     return;
        searchingResult = searchingList.takeFirst();
    }

    int searchingRow = searchingResult->row();
    int searchingColumn = searchingResult->column();
    if ( searchingColumn == lineEditColumn )
    {
        ui->tableWidget->selectRow(searchingRow);
        displayItem(searchingRow, 0);
    }
}


void OrderForm::on_searchPushButton_clicked()
{
    QString searchingText = ui->orderIdLineEdit->text();
    QList<QTableWidgetItem*> searchingResult;
    searchingResult = ui->tableWidget->findItems(searchingText, Qt::MatchFixedString);

    if(searchingResult.empty())     return;

    int searchingRow = searchingResult.first()->row();
    int searchingColumn = searchingResult.first()->column();
    if ( 0 == searchingColumn ){
    ui->tableWidget->selectRow(searchingRow);
    displayItem(searchingRow, 0);
    }
}

