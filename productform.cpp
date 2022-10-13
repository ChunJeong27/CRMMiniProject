#include "productform.h"
#include "ui_productform.h"

#include <QFile>

ProductForm::ProductForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProductForm)
{
    ui->setupUi(this);

    connect(ui->tableWidget, SIGNAL(cellClicked(int,int)),
            SLOT(displayItem(int,int)));
    connect(ui->nameLineEdit, SIGNAL(returnPressed()), SLOT(on_searchPushButton_clicked()));
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

void ProductForm::on_addPushButton_clicked()
{
    QTableWidgetItem* productID = new QTableWidgetItem(QString::number(makeId()));
    QTableWidgetItem* productName = new QTableWidgetItem(ui->nameLineEdit->text());
    QTableWidgetItem* productPrice = new QTableWidgetItem(ui->priceLineEdit->text());
    QTableWidgetItem* productStock = new QTableWidgetItem(ui->stockLineEdit->text());

    int tableRowCount = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount( tableRowCount + 1);
    ui->tableWidget->setItem(tableRowCount, 0, productID);
    ui->tableWidget->setItem(tableRowCount, 1, productName);
    ui->tableWidget->setItem(tableRowCount, 2, productPrice);
    ui->tableWidget->setItem(tableRowCount, 3, productStock);
}


void ProductForm::on_searchPushButton_clicked()
{
    QString searchingName = ui->nameLineEdit->text();
    QList<QTableWidgetItem*> searchingResult;
    searchingResult = ui->tableWidget->findItems(searchingName, Qt::MatchFixedString);


    if(searchingResult.empty())     return;

    int searchingRow = searchingResult.first()->row();
    ui->tableWidget->selectRow(searchingRow);
    displayItem(searchingRow, 0);
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
    ui->tableWidget->removeRow(ui->tableWidget->currentRow());
}

void ProductForm::searchProductName(QString name)
{
    QList<QTableWidgetItem*> searchingResult;
    searchingResult = ui->tableWidget->findItems(name, Qt::MatchFixedString);

    if(searchingResult.empty())     return;

    QList<QString> returnResult;

    int searchingRow = searchingResult.first()->row();
    returnResult << ui->tableWidget->item(searchingRow, 0)->text()
                 << ui->tableWidget->item(searchingRow, 1)->text()
                 << ui->tableWidget->item(searchingRow, 2)->text()
                 <<ui->tableWidget->item(searchingRow, 3)->text();

    emit productSearchingResult(returnResult);
}

void ProductForm::searchProductId(QString id)
{
    QList<QTableWidgetItem*> searchingResult;
    searchingResult = ui->tableWidget->findItems(id, Qt::MatchFixedString);

    if(searchingResult.empty())     return;

    QList<QString> returnResult;

    int searchingRow = searchingResult.first()->row();
    returnResult << ui->tableWidget->item(searchingRow, 0)->text()
                 << ui->tableWidget->item(searchingRow, 1)->text()
                 << ui->tableWidget->item(searchingRow, 2)->text()
                 <<ui->tableWidget->item(searchingRow, 3)->text();

    emit productSearchingResult(returnResult);
}
