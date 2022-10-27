#ifndef PRODUCTFORM_H
#define PRODUCTFORM_H

#include <QWidget>

class QTableWidgetItem;

namespace Ui {
class ProductForm;
}

class ProductForm : public QWidget
{
    Q_OBJECT

public:
    // 시그널 슬롯을 연결하고 저장된 csv파일에서 데이터를 불러오는 생성자
    explicit ProductForm(QWidget *parent = nullptr);
    // 테이블위젯의 데이터를 저장하고 ui 객체의 메모리를 해제하는 소멸자
    ~ProductForm();

private:
    Ui::ProductForm *ui;    // Qt 디자이너를 통해 구현한 ui들이 모여있는 객체

public:
    void loadData();    // csv파일에서 데이터를 불러오는 함수

private:
    QList<QTableWidgetItem*> searchingList; // 검색결과를 임시로 저장하는 리스트

signals:
    void returnSearching(QList<QString>);   // 검색결과를 반환하는 시그널

public slots:
    int makeId();   // ID가 중복되지 않도록 생성하는 함수
    void displayLineEdit(int,int);  // 테이블위젯에서 선택한 열을 라인에디터에 표시하는 함수
    void clearLineEdit();   // 모든 라인에디터를 초기화하는 함수
    void addTableRow();     // 라인에디터의 데이터를 통해 테이블위젯에 추가하는 함수
    void selectReturnPressedId();   // ID를 통해 테이블위젯의 데이터를 검색하는 함수
    void selectReturnPressedLineEdit();
    // 라인에디터에서 returnPressed가 발생했을 때 검색기능을 수행하는 함수
    void modifyTableRow();  // 선택된 테이블위젯의 데이터를 변경하는 함수
    void removeTableRow();  // 선택된 테이블위젯을 제거하는 함수
    void searching(int, QString);   // 열에 따라 데이터를 검색하는 함수

};

#endif // PRODUCTFORM_H
