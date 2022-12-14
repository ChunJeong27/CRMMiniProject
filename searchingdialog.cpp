#include "searchingdialog.h"
#include "ui_searchingdialog.h"

/* 각 콤보박스를 설정하고 시그널과 슬롯을 연결하는 생성자 */
SearchingDialog::SearchingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchingDialog)
{
    ui->setupUi(this);
    // 고객콤보박스에 고객정보 검색을 위한 항목들을 추가
    ui->clientComboBox->addItem("ID");
    ui->clientComboBox->addItem("Name");
    ui->clientComboBox->addItem("Phone Number");
    ui->clientComboBox->addItem("Address");
    // 상품콤보박스에 상품정보 검색을 위한 항목들을 추가
    ui->productComboBox->addItem("ID");
    ui->productComboBox->addItem("Name");
    ui->productComboBox->addItem("Price");
    ui->productComboBox->addItem("Stock");

    // 고객 검색 버튼이 눌렸을 때 고객검색 시그널을 방출시키는 람다 함수와 연결
    connect(ui->clientPushButton, &QPushButton::clicked, this, [=](){
        QString comboBoxText(ui->clientComboBox->currentText());
        // 콤보박스의 항목 값을 문자열로 선언
        QString searchContent = ui->clientLineEdit->text(); // 검색 내용을 문자열로 저장
        emit searchedClient(comboBoxText, searchContent);
        // 검색 시그널을 client Form으로 방출
    });

    // 상품 검색 버튼이 눌렸을 때 상품검색 시그널을 방출시키는 람다 함수와 연결
    connect(ui->productPushButton, &QPushButton::clicked, this, [=](){
        QString comboBoxText(ui->productComboBox->currentText());
        // 콤보박스의 항목 값을 문자열로 선언
        QString searchContent = ui->productLineEdit->text();
        // 검색 내용을 문자열로 저장
        emit searchedProduct(comboBoxText, searchContent);
        // 검색 시그널을 product Form으로 방출
    });
    // 다이얼로그 테이블위젯의 셀을 클릭하면 행의 값들을 orderForm으로 전달하는 기능
    connect(ui->clientTableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(returnSearching(int,int)));
    connect(ui->productTableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(returnSearching(int,int)));

}

/* 객체가 소멸할 때 ui 객체를 삭제하는 소멸자 */
SearchingDialog::~SearchingDialog()
{
    delete ui;  // 다이얼로그 객체가 소멸할 때 ui 객체들을 소멸
}

/* 다른 Form에서 보낸 검색 결과를 다이얼로그 테이블위젯에 출력하는 슬롯 함수 */
void SearchingDialog::displayTableRow(QList<QString> result)
{
    QWidget* form = qobject_cast<QWidget*>(sender());
    // 보낸 from에 따라 데이터를 처리하기 위해 sender를 동적 변환 후 변수로 선언
    if( "ClientForm" == form->objectName() ) {  // 고객 관리에서 보낸 데이터일 경우
        ui->clientTableWidget->setRowCount(0);  // 첫 행부터 데이터를 삽입하기 위해 행을 0으로 설정

        while ( !result.isEmpty() ) { // 데이터가 없을 경우 반복문을 수행하지 않음
            int tableRowCount = ui->clientTableWidget->rowCount();
            ui->clientTableWidget->setRowCount(tableRowCount + 1);
            // 현재 행 수로 1을 증가시킨 행을 설정
            // 가장 앞에 있는 값을 아이템으로 선언하고 테이블에 저장, 저장한 데이터는 리스트에서 삭제
            ui->clientTableWidget
                    ->setItem(tableRowCount, 0,
                              new QTableWidgetItem(result.takeFirst()));
            ui->clientTableWidget
                    ->setItem(tableRowCount, 1,
                              new QTableWidgetItem(result.takeFirst()));
            ui->clientTableWidget
                    ->setItem(tableRowCount, 2,
                              new QTableWidgetItem(result.takeFirst()));
            ui->clientTableWidget
                    ->setItem(tableRowCount, 3,
                              new QTableWidgetItem(result.takeFirst()));
        }
    } else if( "ProductForm" == form->objectName() ){   // 상품 관리에서 보낸 데이터일 경우
        ui->productTableWidget->setRowCount(0);
        // 첫 행부터 데이터를 저장하기 위해 행 수를 0으로 설정
        while ( !result.isEmpty() ) {   // 데이터가 없을 경우 반복문을 수행하지 않음
            int tableRowCount = ui->productTableWidget->rowCount();
            ui->productTableWidget->setRowCount(tableRowCount + 1);
            // 현재 행 수에서 1을 증가시킨 후 행 수를 설정
            // 리스트의 가장 앞의 값을 아이템으로 선언하고 테이블위젯에 저장
            ui->productTableWidget
                    ->setItem(tableRowCount, 0,
                              new QTableWidgetItem(result.takeFirst()));
            ui->productTableWidget
                    ->setItem(tableRowCount, 1,
                              new QTableWidgetItem(result.takeFirst()));
            ui->productTableWidget
                    ->setItem(tableRowCount, 2,
                              new QTableWidgetItem(result.takeFirst()));
            ui->productTableWidget
                    ->setItem(tableRowCount, 3,
                              new QTableWidgetItem(result.takeFirst()));
        }
    }
}

/* 테이블위젯에서 선택한 데이터를 orderForm으로 시그널을 발생시키는 슬롯 함수 */
void SearchingDialog::returnSearching(int row, int column)
{
    Q_UNUSED(column);   // 사용하지 않는 열 변수를 명시적으로 표시

    QTableWidget* tableWidget = qobject_cast<QTableWidget*>(sender());
    // 데이터를 구분하기 위해 시그널을 발생시킨 테이블위젯 객체를 변수에 저장

    QList<QString> result;  // 데이터를 저장하고 내보낼 리스트 변수 선언
    // 테이블의 행 데이터를 순서대로 리스트에 저장
    result << tableWidget->item(row, 0)->text()
           << tableWidget->item(row, 1)->text()
           << tableWidget->item(row, 2)->text()
           << tableWidget->item(row, 3)->text();

    QString objectName = tableWidget->objectName();
    // 시그널을 보낸 테이블위젯의 이름을 문자열로 저장
    // 테이블에 맞는 시그널을 방출
    if("clientTableWidget" == objectName){
        emit returnClient(result);  // 데이터를 저장한 리스트와 함께 시그널 방출
    } else if("productTableWidget" == objectName){
        emit returnProduct(result);
    }
}
