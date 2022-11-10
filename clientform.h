#ifndef CLIENTFORM_H
#define CLIENTFORM_H

#include <QWidget>

class QSqlQueryModel;

namespace Ui {
class ClientForm;
}

class ClientForm : public QWidget
{
    Q_OBJECT

public:
    // 객체를 생성할 때 DB Model을 생성하고 시그널 슬롯을 연결하는 생성자
    explicit ClientForm(QWidget *parent = nullptr);
    // 객체가 소멸할 때 ui 객체의 메모리를 해제하는 소멸자
    ~ClientForm();

private:
    Ui::ClientForm *ui; // Qt 디자이너를 통해 구현한 ui들이 모여있는 객체

private:
    QSqlQueryModel* clientQueryModel;   // clientForm에서 사용하는 DB Model

signals:
    void checkedIdName(bool);   // ID와 이름의 유무를 확인하고 반환하는 시그널

    void returnSearching(QList<QString>);   // orderForm에서 요청한 검색 결과를 반환하는 시그널

public slots:
    void displayLineEdit(const QModelIndex &);  // 테이블뷰에서 선택한 열을 라인에디터에 표시하는 함수
    void clearLineEdit();   // 모든 라인에디터를 초기화하는 함수
    void addTableRow();     // 라인에디터의 데이터를 DB에 추가하는 함수
    void selectReturnPressedId();   // ID를 통해 DB의 데이터를 검색하는 함수
    // 라인에디터에서 returnPressed가 발생했을 때 검색기능을 수행하는 함수
    void selectReturnPressedLineEdit();
    void modifyTableRow();  // 선택된 테이블위젯의 데이터를 변경하는 함수
    void removeTableRow();  // 선택된 테이블위젯을 제거하는 함수
    void searching(QString, QString);   // orderForm에서 보낸 데이터를 검색하는 함수

};

#endif // CLIENTFORM_H
