#ifndef SEARCHINGDIALOG_H
#define SEARCHINGDIALOG_H

#include <QDialog>

class QSqlQueryModel;

namespace Ui {
class SearchingDialog;
}

class SearchingDialog : public QDialog
{
    Q_OBJECT

public:
    // 콤보박스 항목을 설정하고 시그널과 슬롯 함수를 연결하는 생성자
    explicit SearchingDialog(QWidget *parent = nullptr);
    ~SearchingDialog(); // ui 객체 메모리를 해제하는 소멸자

private:
    Ui::SearchingDialog *ui;

protected:
//    void closeEvent(QCloseEvent*) override;

private:
    QSqlQueryModel* clientQueryModel;
    QSqlQueryModel* productQueryModel;

signals:
    // 다이얼로그에서 검색했음을 알려주는 시그널
    void searchedClient(int, QString);
    void searchedProduct(int, QString);
    // orderForm으로 검색 결과를 전달하기 위한 시그널
    void returnClient(QList<QString>);
    void returnProduct(QList<QString>);

public slots:
    void displayTableRow(QList<QString>);    // 다이얼로그 테이블에 데이터를 표시하기 위한 슬롯 함수
    void returnSearching(int, int); // 테이블위젯에서 orderForm으로 데이터를 보내기 위한 슬롯 함수
    void returnSearching(const QModelIndex &index);

};

#endif // SEARCHINGDIALOG_H
