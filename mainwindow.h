#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class ClientForm;
class ProductForm;
class OrderForm;
class ServerForm;
class ChattingForm;

class QSqlQueryModel;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private:
    ClientForm* clientForm;
    ProductForm* productForm;
    OrderForm* orderForm;
    ServerForm* serverForm;

    void closeEvent(QCloseEvent*) override;

signals:
    void triggeredClientAction(QWidget*);
    void triggeredProductAction(QWidget*);
    void triggeredOrderAction(QWidget*);

public slots:
    void createSeachingDialog();
    void createChatRoom();

};
#endif // MAINWINDOW_H
