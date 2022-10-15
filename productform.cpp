#include "productform.h"
#include "ui_productform.h"

#include <QFile>
#include <QMessageBox>

ProductForm::ProductForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProductForm)
{
    ui->setupUi(this);

    connect(ui->nameLineEdit, SIGNAL(returnPressed()), this, SLOT(on_searchPushButton_clicked()));
    connect(ui->priceLineEdit, SIGNAL(returnPressed()), this, SLOT(on_searchPushButton_clicked()));
    connect(ui->stockLineEdit, SIGNAL(returnPressed()), this, SLOT(on_searchPushButton_clicked()));
    connect(ui->tableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(displayItem(int,int)));

    loadData();
}

ProductForm::~ProductForm()
{
    QFile file("productForm.txt");

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    int tableRowCount = ui->tableWidget->rowCount();
    for (int i = 0; i < tableRowCount; i++) {
        out << ui->tableWidget->item(i, 0)->text() << ","
            << ui->tableWidget->item(i, 1)->text() << ","
            << ui->tableWidget->item(i, 2)->text() << ","
            << ui->tableWidget->item(i, 3)->text() << "\n";
    }
    file.close();

    delete ui;
}

void ProductForm::loadData()
{
    QFile file("productForm.txt");

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList row = line.split(",");
        if (!row.empty()) {
            QTableWidgetItem* productID = new QTableWidgetItem(row.at(0));
            QTableWidgetItem* productName = new QTableWidgetItem(row.at(1));
            QTableWidgetItem* productPrice = new QTableWidgetItem(row.at(2));
            QTableWidgetItem* productStock = new QTableWidgetItem(row.at(3));

            int tableRowCount = ui->tableWidget->rowCount();
            ui->tableWidget->setRowCount( tableRowCount + 1);

            ui->tableWidget->setItem(tableRowCount, 0, productID);
            ui->tableWidget->setItem(tableRowCount, 1, productName);
            ui->tableWidget->setItem(tableRowCount, 2, productPrice);
            ui->tableWidget->setItem(tableRowCount, 3, productStock);
        }
    }
    file.close();

}

int ProductForm::makeId()
{
    int lastColumn = ui->tableWidget->rowCount();

    if( lastColumn <= 0 )
        return 2000;

    int creatId = ui->tableWidget->item(lastColumn - 1, 0)->text().toInt();
    return creatId + 1;
}

void ProductForm::displayItem(int row,int column)
{
    Q_UNUSED(column);

    ui->idLineEdit->setText(ui->tableWidget->item(row, 0)->text());
    ui->nameLineEdit->setText(ui->tableWidget->item(row, 1)->text());
    ui->priceLineEdit->setText(ui->tableWidget->item(row, 2)->text());
    ui->stockLineEdit->setText(ui->tableWidget->item(row, 3)->text());
}

void ProductForm::on_clearPushButton_clicked()
{
    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->priceLineEdit->clear();
    ui->stockLineEdit->clear();
}

void ProductForm::on_addPushButton_clicked()
{
    QString name = ui->nameLineEdit->text();
    QString price = ui->priceLineEdit->text();
    QString stock = ui->stockLineEdit->text();

    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->priceLineEdit->clear();
    ui->stockLineEdit->clear();

    if ( name.length() <= 0 || price.length() <=0 || stock.length() <= 0 ) {
        QMessageBox::warning(this, "Product Manager", "Check your input again.", QMessageBox::Ok);
        return;
    }
    else {
        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(makeId()));
        QTableWidgetItem* nameItem = new QTableWidgetItem(name);
        QTableWidgetItem* priceItem = new QTableWidgetItem(price);
        QTableWidgetItem* stockItem = new QTableWidgetItem(stock);

        int tableRowCount = ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount( tableRowCount + 1);

        ui->tableWidget->setItem(tableRowCount, 0, idItem);
        ui->tableWidget->setItem(tableRowCount, 1, nameItem);
        ui->tableWidget->setItem(tableRowCount, 2, priceItem);
        ui->tableWidget->setItem(tableRowCount, 3, stockItem);
    }
}


void ProductForm::on_searchPushButton_clicked()
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(sender());
    if(lineEdit == nullptr)     return;

    int lineEditType(0);
    if( lineEdit->objectName() == "nameLineEdit" ){
        lineEditType = 1;
    } else if( lineEdit->objectName() == "priceLineEdit" ){
        lineEditType = 2;
    } else if( lineEdit->objectName() == "stockLineEdit" ){
        lineEditType = 3;
    } else {
        return;
    }

    QString searchingText = lineEdit->text();
    if ( searchingList.empty()
         || searchingList.first()->text().compare(searchingText, Qt::CaseInsensitive)) {
        searchingList = ui->tableWidget->findItems(searchingText, Qt::MatchFixedString);
        if(searchingList.empty())     return;
    }

    QTableWidgetItem* searchingResult = searchingList.takeFirst();
    while ( searchingResult->column() != lineEditType ) {
        if(searchingList.empty())     return;
        searchingResult = searchingList.takeFirst();
    }

    int searchingRow = searchingResult->row();
    int searchingCoulmn = searchingResult->column();
    if ( searchingCoulmn == lineEditType )
    {
        ui->tableWidget->selectRow(searchingRow);
        displayItem(searchingRow, 0);
    }
}


void ProductForm::on_modifyPushButton_clicked()
{
    int tableCurrentRow = ui->tableWidget->currentRow();

    QTableWidgetItem* productName = new QTableWidgetItem(ui->nameLineEdit->text());
    QTableWidgetItem* productPrice = new QTableWidgetItem(ui->priceLineEdit->text());
    QTableWidgetItem* productStock = new QTableWidgetItem(ui->stockLineEdit->text());

    ui->tableWidget->setItem(tableCurrentRow, 1, productName);
    ui->tableWidget->setItem(tableCurrentRow, 2, productPrice);
    ui->tableWidget->setItem(tableCurrentRow, 3, productStock);
}


void ProductForm::on_removePushButton_clicked()
{
    if(QMessageBox::warning(this, "Product Manager", "Are you sure you want to delete it?",
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    ui->tableWidget->removeRow(ui->tableWidget->currentRow());
}

void ProductForm::on_idLineEdit_returnPressed()
{
    QString searchingText = ui->idLineEdit->text();
    QList<QTableWidgetItem*> searchingResult;
    searchingResult = ui->tableWidget->findItems(searchingText, Qt::MatchFixedString);

    if(searchingResult.empty())     return;

    int searchingRow = searchingResult.first()->row();
    ui->tableWidget->selectRow(searchingRow);
    displayItem(searchingRow, 0);
}

void ProductForm::searching(int type, QString content)
{
    QList<QTableWidgetItem*> searchingResult;
    searchingResult = ui->tableWidget->findItems(content, Qt::MatchFixedString);

    if(searchingResult.empty())     return;

    QList<QString> returnResult;
    Q_FOREACH( QTableWidgetItem* item, searchingResult ){
        if( item->column() == type ){
            int searchingRow = item->row();
            returnResult << ui->tableWidget->item(searchingRow, 0)->text()
                         << ui->tableWidget->item(searchingRow, 1)->text()
                         << ui->tableWidget->item(searchingRow, 2)->text()
                         << ui->tableWidget->item(searchingRow, 3)->text();
        }
    }

    emit returnSearching(returnResult);
}
