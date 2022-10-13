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

    ui->tableWidget->setRowCount(1);
    ui->tableWidget->setItem(0, 0, searchId);
}


void SearchingDialog::on_pushButton_clicked()
{

    emit searchClientName(ui->lineEdit->text());
}

void SearchingDialog::openDialog()
{
    show();
}

