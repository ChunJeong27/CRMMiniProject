#ifndef ORDERFORM_H
#define ORDERFORM_H

#include <QWidget>

class QSqlQueryModel;

namespace Ui {
class OrderForm;
}

class OrderForm : public QWidget
{
    Q_OBJECT

public:
    // 시그널 슬롯을 연결하고 저장된 DB Model 생성
    explicit OrderForm(QWidget *parent = nullptr);
    // ui 객체를 메모리 해제하는 소멸자
    ~OrderForm();

private:
    Ui::OrderForm *ui;  // Qt 디자이너를 통해 구현한 ui들이 모여있는 객체

private:
    // 고객 관리 및 상품 관리 탭에서 가져온 데이터를 임시로 저장하는 멤버 변수
    QString clientId;
    QString productId;
    QString productStock;

    QSqlQueryModel* orderQueryModel;    // 주문 관리 DB 모델

signals:
    void clickedSelectButton(); // 고객 정보와 상품 정보를 선택하기 위해 다이얼로그를 실행하는 시그널

public slots:
    void displayLineEdit(const QModelIndex &);  // 테이블뷰에서 선택한 열을 라인에디터에 표시하는 함수
    void clearLineEdit();   // 모든 라인에디터를 초기화하는 함수
    void receiveClientInfo(QList<QString>); // 시그널과 함께 전달된 고객 데이터를 저장하는 함수
    void receiveProductInfo(QList<QString>);    // 시그널과 함께 전달된 상품 데이터를 저장하는 함수
    void addTableRow(); // DB에 행 데이터를 추가하는 함수
    void selectReturnPressedId();   // ID에 맞는 행을 출력하는 함수
    void selectReturnPressedLineEdit(); // 라인에디터 값에 맞는 행을 출력하는 함수
    void modifyTableRow();  // id에 맞는 DB를 수정하는 함수
    void removeTableRow();  // id에 맞는 DB를 제거하는 함수

};

#endif // ORDERFORM_H
