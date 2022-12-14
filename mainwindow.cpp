#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTableView>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

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
    ui->toolBar->setHidden(true);

    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setHostName("localhost");
    db.setDatabaseName("Oracle11gx64");
    db.setUserName("EJH");
    db.setPassword("1203");
    if (!db.open()) {
        qDebug() << db.lastError().text();
    } else {
        qDebug("DB Connection Successful");
    }

    clientForm = new ClientForm;
    ui->tabWidget->addTab(clientForm, "Client Manager");

    productForm = new ProductForm;
    ui->tabWidget->addTab(productForm, "Product Manager");

    orderForm = new OrderForm;
    connect(orderForm, SIGNAL(clickedSelectButton()), this, SLOT(createSeachingDialog()));
    ui->tabWidget->addTab(orderForm, "Order Manager");

    serverForm = new ServerForm;
    ui->tabWidget->addTab(serverForm, "Chat Server");

    connect(ui->action_Add, &QAction::triggered, this, [=](){
        switch(ui->tabWidget->currentIndex()){
        case 0:
            clientForm->addTableRow();
            break;
        case 1:
            productForm->addTableRow();
            break;
        case 2:
            orderForm->addTableRow();
            break;
        default:
            break;
        }});
    connect(ui->action_Search, &QAction::triggered, this, [=](){
        switch(ui->tabWidget->currentIndex()){
        case 0:
            clientForm->selectReturnPressedId();
            break;
        case 1:
            productForm->selectReturnPressedId();
            break;
        case 2:
            orderForm->selectReturnPressedId();
            break;
        default:
            break;
        }});
    connect(ui->action_Modify, &QAction::triggered, this, [=](){
        switch(ui->tabWidget->currentIndex()){
        case 0:
            clientForm->modifyTableRow();
            break;
        case 1:
            productForm->modifyTableRow();
            break;
        case 2:
            orderForm->modifyTableRow();
            break;
        default:
            break;
        }});
    connect(ui->action_Remove, &QAction::triggered, this, [=](){
        switch(ui->tabWidget->currentIndex()){
        case 0:
            clientForm->removeTableRow();
            break;
        case 1:
            productForm->removeTableRow();
            break;
        case 2:
            orderForm->removeTableRow();
            break;
        default:
            break;
        }});
    connect(ui->action_Client_Manager, &QAction::triggered, this, [=](){
        emit triggeredClientAction(qobject_cast<QWidget*>(clientForm));});
    connect(this, SIGNAL(triggeredClientAction(QWidget*)), ui->tabWidget, SLOT(setCurrentWidget(QWidget*)));
    connect(ui->action_Product_Manager, &QAction::triggered, this, [=](){
        emit triggeredProductAction(qobject_cast<QWidget*>(productForm));});
    connect(this, SIGNAL(triggeredProductAction(QWidget*)), ui->tabWidget, SLOT(setCurrentWidget(QWidget*)));
    connect(ui->action_Order_Manager, &QAction::triggered, this, [=](){
        emit triggeredOrderAction(qobject_cast<QWidget*>(orderForm));});
    connect(this, SIGNAL(triggeredOrderAction(QWidget*)), ui->tabWidget, SLOT(setCurrentWidget(QWidget*)));
    connect(ui->action_New_Chat_Room, SIGNAL(triggered(bool)), this, SLOT(createChatRoom()));

    connect(serverForm, SIGNAL(checkClientId(QString,QString)), clientForm, SLOT(checkClient(QString,QString)));
    connect(clientForm, SIGNAL(checkedIdName(bool)), serverForm, SLOT(isClient(bool)));


}

MainWindow::~MainWindow()
{

    delete ui;
}

void MainWindow::closeEvent(QCloseEvent*)
{
    delete serverForm;
}

void MainWindow::createSeachingDialog()
{
    SearchingDialog* searchingDialog = new SearchingDialog(orderForm);

    connect(searchingDialog, SIGNAL(returnClient(QList<QString>)), orderForm, SLOT(receiveClientInfo(QList<QString>)));
    connect(searchingDialog, SIGNAL(returnProduct(QList<QString>)), orderForm, SLOT(receiveProductInfo(QList<QString>)));
    connect(searchingDialog, SIGNAL(searchedClient(QString,QString)), clientForm, SLOT(searching(QString,QString)));
    connect(clientForm, SIGNAL(returnSearching(QList<QString>)), searchingDialog, SLOT(displayTableRow(QList<QString>)));
    connect(searchingDialog, SIGNAL(searchedProduct(QString,QString)), productForm, SLOT(searching(QString,QString)));
    connect(productForm, SIGNAL(returnSearching(QList<QString>)), searchingDialog, SLOT(displayTableRow(QList<QString>)));

    searchingDialog->open();
}

void MainWindow::createChatRoom()
{
    ChatRoomForm* chatRoomForm = new ChatRoomForm;
    chatRoomForm->show();
}
