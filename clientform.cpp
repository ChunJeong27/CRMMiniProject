#include "clientform.h"
#include "ui_clientform.h"

#include <QFile>
#include <QMessageBox>

ClientForm::ClientForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientForm)
{
    ui->setupUi(this);

    connect(ui->tableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(displayItem(int,int)));
//    connect(ui->nameLineEdit, SIGNAL(returnPressed()), this, SLOT(on_searchPushButton_clicked()));

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

void ClientForm::on_addPushButton_clicked()
{
    QString name = ui->nameLineEdit->text();
    QString phoneNum = ui->phoneNumLineEdit->text();
    QString address = ui->addressLineEdit->text();

    if ( name.length() <= 0 || phoneNum.length() <= 2 || address.length() <= 0 ) {
        QMessageBox::warning(this, "Client Manager", "Check your input again.", QMessageBox::Ok);
        return;
    }
    else {
        QTableWidgetItem* clientID = new QTableWidgetItem(QString::number(makeId()));
        QTableWidgetItem* clientName = new QTableWidgetItem(name);
        QTableWidgetItem* clientPhoneNum = new QTableWidgetItem(phoneNum);
        QTableWidgetItem* clientAddress = new QTableWidgetItem(address);

        int tableRowCount = ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount( tableRowCount + 1);

        ui->tableWidget->setItem(tableRowCount, 0, clientID);
        ui->tableWidget->setItem(tableRowCount, 1, clientName);
        ui->tableWidget->setItem(tableRowCount, 2, clientPhoneNum);
        ui->tableWidget->setItem(tableRowCount, 3, clientAddress);
    }

}

void ClientForm::on_searchPushButton_clicked()
{
    QString searchingName = ui->nameLineEdit->text();
    QList<QTableWidgetItem*> searchingResult;
    searchingResult = ui->tableWidget->findItems(searchingName, Qt::MatchFixedString);


    if(searchingResult.empty())     return;

    int searchingRow = searchingResult.first()->row();
    ui->tableWidget->selectRow(searchingRow);
    displayItem(searchingRow, 0);
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
    ui->tableWidget->removeRow(ui->tableWidget->currentRow());
}

void ClientForm::clientSearching(int type, QString id)
{
    QList<QTableWidgetItem*> searchingResult;
    searchingResult = ui->tableWidget->findItems(id, Qt::MatchFixedString);

    if(searchingResult.empty())     return;

    QList<QString> returnResult;

    Q_FOREACH( QTableWidgetItem* item, searchingResult ){
        if( item->column() == type ){
            int searchingRow = item->row();
            returnResult << ui->tableWidget->item(searchingRow, 0)->text()
                         << ui->tableWidget->item(searchingRow, 1)->text()
                         << ui->tableWidget->item(searchingRow, 2)->text()
                         <<ui->tableWidget->item(searchingRow, 3)->text();
        }
    }

    emit returnSearching(returnResult);
}

void ClientForm::on_nameLineEdit_returnPressed()
{
    QString searchingText = ui->nameLineEdit->text();
    if ( searchingList.empty()
         || searchingList.first()->text().compare(searchingText, Qt::CaseInsensitive)) {
        searchingList = ui->tableWidget->findItems(searchingText, Qt::MatchFixedString);
        if(searchingList.empty())     return;
    }

    QTableWidgetItem* searchingResult = searchingList.takeFirst();
    while ( searchingResult->column() != 1 ) {
        if(searchingList.empty())     return;
        searchingResult = searchingList.takeFirst();
    }

    int searchingRow = searchingResult->row();
    int searchingCoulmn = searchingResult->column();
    if ( searchingCoulmn == 1 )
    {
        ui->tableWidget->selectRow(searchingRow);
        displayItem(searchingRow, 0);
        qDebug("ok");
    }

}


void ClientForm::on_phoneNumLineEdit_returnPressed()
{
    QString searchingText = ui->phoneNumLineEdit->text();
    QList<QTableWidgetItem*> searchingResult;
    searchingResult = ui->tableWidget->findItems(searchingText, Qt::MatchFixedString);


    if(searchingResult.empty())     return;

    int searchingRow = searchingResult.first()->row();
    ui->tableWidget->selectRow(searchingRow);
    displayItem(searchingRow, 0);
}


void ClientForm::on_addressLineEdit_returnPressed()
{
    QString searchingText = ui->addressLineEdit->text();
    QList<QTableWidgetItem*> searchingResult;
    searchingResult = ui->tableWidget->findItems(searchingText, Qt::MatchFixedString);


    if(searchingResult.empty())     return;

    int searchingRow = searchingResult.first()->row();
    ui->tableWidget->selectRow(searchingRow);
    displayItem(searchingRow, 0);
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

void ClientForm::searchClientName(QString name)
{
    QList<QTableWidgetItem*> searchingResult;
    searchingResult = ui->tableWidget->findItems(name, Qt::MatchFixedString);

    if(searchingResult.empty())     return;

    QList<QString> returnResult;
//    qDebug()<<searchingResult.at(1)->text(); return;

    int searchingRow = searchingResult.first()->row();
    returnResult << ui->tableWidget->item(searchingRow, 0)->text()
                 << ui->tableWidget->item(searchingRow, 1)->text()
                 << ui->tableWidget->item(searchingRow, 2)->text()
                 <<ui->tableWidget->item(searchingRow, 3)->text();

    qDebug("-1");
    emit returnSearching(returnResult);
    qDebug("1");
//    emit clientSearchingResult(ui->tableWidget->item(searchingRow, 0)->text());
}

void ClientForm::searchClientReturnId(QString name)
{
    QList<QTableWidgetItem*> searchingResult;
    searchingResult = ui->tableWidget->findItems(name, Qt::MatchFixedString);

    if(searchingResult.empty())     return;

    int searchingRow = searchingResult.first()->row();
    emit clientId(ui->tableWidget->item(searchingRow, 0)->text().toInt());
}


void ClientForm::on_clearPushButton_clicked()
{
    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->phoneNumLineEdit->clear();
    ui->addressLineEdit->clear();
}

