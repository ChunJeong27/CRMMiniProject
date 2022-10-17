#include "clientform.h"
#include "ui_clientform.h"

#include <QFile>
#include <QMessageBox>

ClientForm::ClientForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientForm)
{
    ui->setupUi(this);

    connect(ui->nameLineEdit, SIGNAL(returnPressed()), this, SLOT(on_searchPushButton_clicked()));
    connect(ui->phoneNumLineEdit, SIGNAL(returnPressed()), this, SLOT(on_searchPushButton_clicked()));
    connect(ui->addressLineEdit, SIGNAL(returnPressed()), this, SLOT(on_searchPushButton_clicked()));
    connect(ui->tableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(displayItem(int,int)));

    loadData();
}

ClientForm::~ClientForm()
{
    QFile file("clientForm.txt");

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

void ClientForm::loadData()
{
    QFile file("clientForm.txt");

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList row = line.split(",");
        if (!row.empty()) {
            QTableWidgetItem* clientID = new QTableWidgetItem(row.at(0));
            QTableWidgetItem* clientName = new QTableWidgetItem(row.at(1));
            QTableWidgetItem* clientPhoneNum = new QTableWidgetItem(row.at(2));
            QTableWidgetItem* clientAddress = new QTableWidgetItem(row.at(3));

            int tableRowCount = ui->tableWidget->rowCount();
            ui->tableWidget->setRowCount( tableRowCount + 1);

            ui->tableWidget->setItem(tableRowCount, 0, clientID);
            ui->tableWidget->setItem(tableRowCount, 1, clientName);
            ui->tableWidget->setItem(tableRowCount, 2, clientPhoneNum);
            ui->tableWidget->setItem(tableRowCount, 3, clientAddress);
        }
    }
    file.close();

}

int ClientForm::makeId()
{
    int lastColumn = ui->tableWidget->rowCount();

    if( lastColumn <= 0 )
        return 1000;

    int creatId = ui->tableWidget->item(lastColumn - 1, 0)->text().toInt();
    return creatId + 1;
}

void ClientForm::displayItem(int row,int column)
{
    Q_UNUSED(column);

    ui->idLineEdit->setText(ui->tableWidget->item(row, 0)->text());
    ui->nameLineEdit->setText(ui->tableWidget->item(row, 1)->text());
    ui->phoneNumLineEdit->setText(ui->tableWidget->item(row, 2)->text());
    ui->addressLineEdit->setText(ui->tableWidget->item(row, 3)->text());
}

void ClientForm::on_clearPushButton_clicked()
{
    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->phoneNumLineEdit->clear();
    ui->addressLineEdit->clear();
}

void ClientForm::on_addPushButton_clicked()
{
    QString name = ui->nameLineEdit->text();
    QString phoneNum = ui->phoneNumLineEdit->text();
    QString address = ui->addressLineEdit->text();

    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->phoneNumLineEdit->clear();
    ui->addressLineEdit->clear();

    if ( name.length() <= 0 || phoneNum.length() <= 2 || address.length() <= 0 ) {
        QMessageBox::warning(this, "Client Manager", "Check your input again.", QMessageBox::Ok);
        return;
    }
    else {
        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(makeId()));
        QTableWidgetItem* nameItem = new QTableWidgetItem(name);
        QTableWidgetItem* phoneNumItem = new QTableWidgetItem(phoneNum);
        QTableWidgetItem* addressItem = new QTableWidgetItem(address);

        int tableRowCount = ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount( tableRowCount + 1);

        ui->tableWidget->setItem(tableRowCount, 0, idItem);
        ui->tableWidget->setItem(tableRowCount, 1, nameItem);
        ui->tableWidget->setItem(tableRowCount, 2, phoneNumItem);
        ui->tableWidget->setItem(tableRowCount, 3, addressItem);
    }
}

void ClientForm::on_searchPushButton_clicked()
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(sender());
    if(lineEdit == nullptr)     return;

    int lineEditType(0);
    if( lineEdit->objectName() == "nameLineEdit" ){
        lineEditType = 1;
    } else if( lineEdit->objectName() == "phoneNumLineEdit" ){
        lineEditType = 2;
    } else if( lineEdit->objectName() == "addressLineEdit" ){
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

void ClientForm::on_modifyPushButton_clicked()
{
    int tableCurrentRow = ui->tableWidget->currentRow();

    QTableWidgetItem* clientName = new QTableWidgetItem(ui->nameLineEdit->text());
    QTableWidgetItem* clientPhoneNum = new QTableWidgetItem(ui->phoneNumLineEdit->text());
    QTableWidgetItem* clientAddress = new QTableWidgetItem(ui->addressLineEdit->text());

    ui->tableWidget->setItem(tableCurrentRow, 1, clientName);
    ui->tableWidget->setItem(tableCurrentRow, 2, clientPhoneNum);
    ui->tableWidget->setItem(tableCurrentRow, 3, clientAddress);
}

void ClientForm::on_removePushButton_clicked()
{
    if(QMessageBox::warning(this, "Client Manager", "Are you sure you want to delete it?",
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        ui->tableWidget->removeRow(ui->tableWidget->currentRow());
}

void ClientForm::on_idLineEdit_returnPressed()
{
    QString searchingText = ui->idLineEdit->text();
    QList<QTableWidgetItem*> searchingResult;
    searchingResult = ui->tableWidget->findItems(searchingText, Qt::MatchFixedString);

    if(searchingResult.empty())     return;

    int searchingRow = searchingResult.first()->row();
    ui->tableWidget->selectRow(searchingRow);
    displayItem(searchingRow, 0);
}

void ClientForm::searching(int type, QString content)
{
    QList<QTableWidgetItem*> searchingResult;
    searchingResult = ui->tableWidget->findItems(content, Qt::MatchFixedString);

    if(searchingResult.empty())     return;

    QList<QString> returnResult;
    foreach( QTableWidgetItem* item, searchingResult ){
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
