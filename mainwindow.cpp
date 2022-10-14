#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "clientform.h"
#include "productform.h"
#include "orderform.h"
#include "searchingdialog.h"
#include "serverForm.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    clientForm = new ClientForm;
    ui->tabWidget->addTab(clientForm, "Client Manager");
    productForm = new ProductForm;
    ui->tabWidget->addTab(productForm, "Product Manager");
    orderForm = new OrderForm;
    ui->tabWidget->addTab(orderForm, "Order Manager");
    serverForm = new ServerForm;
    ui->tabWidget->addTab(serverForm, "Chatting Server");

    SearchingDialog* dlg = new SearchingDialog;
//    connect(dlg, SIGNAL(searchClientName(QString)), orderForm, SLOT(searchClientName(QString)));
//    connect(this, SIGNAL(returnDialog(int)), dlg, SLOT(returnId(int)));
    dlg->hide();
    connect(orderForm, SIGNAL(clickedSearchButton()), dlg, SLOT(openDialog()));
    connect(dlg, SIGNAL(searchedClientId(QString)), clientForm, SLOT(clientSearching(QString)));
    connect(clientForm, SIGNAL(returnSearching(QList<QString>)), dlg, SLOT(displayRow(QList<QString>)));

    connect(ui->action_Client_Manager, SIGNAL(triggered()), this, SLOT(clientTabAction()));
    connect(this, SIGNAL(triggeredClientAction(QWidget*)), ui->tabWidget, SLOT(setCurrentWidget(QWidget*)));
    connect(ui->action_Product_Manager, SIGNAL(triggered()), this, SLOT(productTabAction()));
    connect(this, SIGNAL(triggeredProductAction(QWidget*)), ui->tabWidget, SLOT(setCurrentWidget(QWidget*)));
    connect(ui->action_Order_Manager, SIGNAL(triggered()), this, SLOT(orderTabAction()));
    connect(this, SIGNAL(triggeredOrderAction(QWidget*)), ui->tabWidget, SLOT(setCurrentWidget(QWidget*)));

//    connect(orderForm, SIGNAL(clientReturnPressed(QString)), clientForm, SLOT(searchClientName(QString)));
    connect(orderForm, SIGNAL(clientReturnPressed(QString)), clientForm, SLOT(searchClientReturnId(QString)));
    connect(clientForm, SIGNAL(clientIdSearchingResult(QList<QString>)), orderForm, SLOT(addClientResult(QList<QString>)));
    connect(orderForm, SIGNAL(productReturnPressed(QString)), productForm, SLOT(searchProductName(QString)));
    connect(productForm, SIGNAL(productSearchingResult(QList<QString>)), orderForm, SLOT(addProductResult(QList<QString>)));

    connect(orderForm, SIGNAL(searchClientId(QString)), clientForm, SLOT(searchClientId(QString)));
    connect(clientForm, SIGNAL(clientIdSearchingResult(QList<QString>)), orderForm, SLOT(addClientResult(QList<QString>)));
    connect(orderForm, SIGNAL(searchProductId(QString)), productForm, SLOT(searchProductId(QString)));
    connect(productForm, SIGNAL(productSearchingResult(QList<QString>)), orderForm, SLOT(addProductResult(QList<QString>)));

    connect(clientForm, SIGNAL(clientId(int)), orderForm, SLOT(returnId(int)));

    orderForm->loadData();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::clientTabAction()
{
    emit triggeredClientAction(qobject_cast<QWidget*>(clientForm));
}

void MainWindow::productTabAction()
{
    emit triggeredProductAction(qobject_cast<QWidget*>(productForm));
}

void MainWindow::orderTabAction()
{
    emit triggeredOrderAction(qobject_cast<QWidget*>(orderForm));
}
