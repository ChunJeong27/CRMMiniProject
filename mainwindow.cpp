#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "clientform.h"
#include "productform.h"
#include "orderform.h"
#include "searchingdialog.h"
#include "serverform.h"
#include "chatroomform.h"

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
    connect(orderForm, SIGNAL(clickedSearchButton()), this, SLOT(createSeachingDialog()));
    ui->tabWidget->addTab(orderForm, "Order Manager");

    serverForm = new ServerForm;
    ui->tabWidget->addTab(serverForm, "Chat Server");

    connect(orderForm, SIGNAL(searchedClient(int,QString)), clientForm, SLOT(searching(int,QString)));
    connect(clientForm, SIGNAL(returnSearching(QList<QString>)), orderForm, SLOT(addClientResult(QList<QString>)));
    connect(orderForm, SIGNAL(searchedProduct(int,QString)), productForm, SLOT(searching(int,QString)));
    connect(productForm, SIGNAL(returnSearching(QList<QString>)), orderForm, SLOT(addProductResult(QList<QString>)));

    connect(serverForm, SIGNAL(checkClientId(QString, QString)), clientForm, SLOT(checkIdName(QString,QString)));
    connect(clientForm, SIGNAL(checkedIdName(bool)), serverForm, SLOT(isClient(bool)));

    connect(ui->action_Client_Manager, SIGNAL(triggered()), this, SLOT(clientTabAction()));
    connect(this, SIGNAL(triggeredClientAction(QWidget*)), ui->tabWidget, SLOT(setCurrentWidget(QWidget*)));
    connect(ui->action_Product_Manager, SIGNAL(triggered()), this, SLOT(productTabAction()));
    connect(this, SIGNAL(triggeredProductAction(QWidget*)), ui->tabWidget, SLOT(setCurrentWidget(QWidget*)));
    connect(ui->action_Order_Manager, SIGNAL(triggered()), this, SLOT(orderTabAction()));
    connect(this, SIGNAL(triggeredOrderAction(QWidget*)), ui->tabWidget, SLOT(setCurrentWidget(QWidget*)));

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

void MainWindow::createSeachingDialog()
{
    SearchingDialog* searchingDialog = new SearchingDialog(orderForm);

    connect(searchingDialog, SIGNAL(searchedClient(int,QString)), clientForm, SLOT(searching(int,QString)));
    connect(clientForm, SIGNAL(returnSearching(QList<QString>)), searchingDialog, SLOT(displayRow(QList<QString>)));
    connect(searchingDialog, SIGNAL(searchedProduct(int,QString)), productForm, SLOT(searching(int,QString)));
    connect(productForm, SIGNAL(returnSearching(QList<QString>)), searchingDialog, SLOT(displayRow(QList<QString>)));

    connect(searchingDialog, SIGNAL(returnClient(QList<QString>)), orderForm, SLOT(addClientResult(QList<QString>)));
    connect(searchingDialog, SIGNAL(returnProduct(QList<QString>)), orderForm, SLOT(addProductResult(QList<QString>)));

    searchingDialog->open();
}

void MainWindow::on_action_Chatting_triggered()
{
    ChatRoomForm* chatRoomForm = new ChatRoomForm;
    connect(chatRoomForm, SIGNAL(clickedFileList(QListWidgetItem*)), serverForm, SLOT(sendFile(QListWidgetItem*)));
    chatRoomForm->show();
}

