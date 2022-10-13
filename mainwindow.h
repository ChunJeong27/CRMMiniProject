#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class ClientForm;
class ProductForm;
class OrderForm;

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
    ClientForm* clientForm;
    ProductForm* productForm;
    OrderForm* orderForm;

signals:
    void triggeredClientAction(QWidget*);
    void triggeredProductAction(QWidget*);
    void triggeredOrderAction(QWidget*);

public slots:
    void clientTabAction();
    void productTabAction();
    void orderTabAction();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
