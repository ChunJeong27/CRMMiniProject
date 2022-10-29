#ifndef ORDERFORM_H
#define ORDERFORM_H

#include <QWidget>

class QTableWidgetItem;

namespace Ui {
class OrderForm;
}

class OrderForm : public QWidget
{
    Q_OBJECT

public:
    // 시그널 슬롯을 연결하고 저장된 csv파일에서 데이터를 불러오는 생성자
    explicit OrderForm(QWidget *parent = nullptr);
    // 테이블위젯의 데이터를 저장하고 할당된 메모리를 해제하는 소멸자
    ~OrderForm();

private:
    Ui::OrderForm *ui;  // Qt 디자이너를 통해 구현한 ui들이 모여있는 객체

public:
    void loadData();    // csv파일에서 데이터를 불러와 테이블위젯에 저장하는 함수

private:
    // 고객 관리 및 상품 관리 탭에서 가져온 데이터를 임시로 저장하는 멤버 변수
    QString clientId;
    QString clientName;
    QString clientAddress;
    QString clientPhoneNum;
    QString productId;
    QString productName;
    QString productPrice;
    QString productStock;

    QList<QTableWidgetItem*> searchingList; // 검색 결과를 임시로 저장하는 변수

signals:
    void searchedClient(int, QString);
    void searchedProduct(int, QString);
    void clickedSearchButton();

public slots:
    int makeId();   // ID가 중복되지 않도록 생성하는 함수
    void displayLineEdit(int,int);  // 테이블에서 선택한 열을 라인에디터에 표시하는 함수
    void clearLineEdit();   // 모든 라인에디터를 초기화하는 함수
    void receiveClientInfo(QList<QString>); // 시그널과 함께 전달된 고객 데이터를 저장하는 함수
    void receiveProductInfo(QList<QString>);    // 시그널과 함께 전달된 상품 데이터를 저장하는 함수
    void addTableRow(); // 테이블의 행 데이터를 추가하는 함수
    void selectReturnPressedId();   // ID에 맞는 행을 선택하는 함수
    void selectReturnPressedLineEdit(); // 라인에디터 값에 맞는 행을 선택하는 함수
    void modifyTableRow();  // 선택된 행을 수정하는 함수
    void removeTableRow();  // 선택된 행을 제거하는 함수

};

#endif // ORDERFORM_H
