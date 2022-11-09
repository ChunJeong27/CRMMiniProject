#include "orderform.h"
#include "ui_orderform.h"

#include <QFile>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QTableView>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <searchingdialog.h>

/* 생성자에서 ui에 대한 시그널과 슬롯을 연결 */
OrderForm::OrderForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrderForm)
{
    ui->setupUi(this);

    connect(ui->tableView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(displayLineEdit(QModelIndex)));  // 클릭한 셀의 행 데이터를 라인에디터에 보여주는 기능
    // 각각 ui 버튼의 기능을 슬롯 함수와 연결
    connect(ui->clearPushButton, SIGNAL(clicked()), this, SLOT(clearLineEdit()));
    connect(ui->addPushButton, SIGNAL(clicked()), this, SLOT(addTableRow()));
    connect(ui->modifyPushButton, SIGNAL(clicked()),
            this, SLOT(modifyTableRow()));
    connect(ui->removePushButton, SIGNAL(clicked()),
            this, SLOT(removeTableRow()));
    // 라인에디터에 따라 맞는 열을 검색하고 결과를 활성화시키는 기능
    connect(ui->clientNameLineEdit, SIGNAL(returnPressed()),
            this, SLOT(selectReturnPressedLineEdit()));
    connect(ui->clientPhoneNumLineEdit, SIGNAL(returnPressed()),
            this, SLOT(selectReturnPressedLineEdit()));
    connect(ui->clientAddressLineEdit, SIGNAL(returnPressed()),
            this, SLOT(selectReturnPressedLineEdit()));
    connect(ui->productNameLineEdit, SIGNAL(returnPressed()),
            this, SLOT(selectReturnPressedLineEdit()));
    connect(ui->productPriceLineEdit, SIGNAL(returnPressed()),
            this, SLOT(selectReturnPressedLineEdit()));
    connect(ui->amountLineEdit, SIGNAL(returnPressed()),
            this, SLOT(selectReturnPressedLineEdit()));

    connect(ui->orderIdLineEdit, SIGNAL(returnPressed()),
            this, SLOT(selectReturnPressedId())); // 아이디를 검색하고 행을 활성화하는 기능
    // 스핀박스 값이 바뀔 때 총액을 설정하는 기능을 람다함수로 구현
    connect(ui->quantitySpinBox, &QSpinBox::valueChanged, this, [=](int i){
        quint32 amount = ui->productPriceLineEdit->text().toUInt() * i;
        ui->amountLineEdit->setText(QString::number(amount));});

    connect(ui->selectPushButton, SIGNAL(clicked()), this, SIGNAL(clickedSearchButton()));


    orderQueryModel = new QSqlQueryModel;
    orderQueryModel->setQuery("SELECT s.order_id, s.client_id, s.product_id, c.client_name, c.phone_number, c.address, p.product_name, p.price, s.quantity, s.amount FROM SHOPPING s, CLIENT c, PRODUCT p WHERE s.client_id = c.client_id AND s.product_id = p.product_id;");

    ui->tableView->setModel(orderQueryModel);

    ui->tableView->hideColumn(1);
    ui->tableView->hideColumn(2);

}

/* 기록한 데이터를 소멸자에서 csv파일에 저장 */
OrderForm::~OrderForm()
{
    delete ui;  // 소멸자이므로 사용이 끝난 ui객체를 메모리 해제
}

/* 시그널에서 발생한 행의 값을 통해 테이블위젯의 행을 라인에디터에 표시하는 슬롯 함수 */
void OrderForm::displayLineEdit(const QModelIndex &index)
{
    QString orderId = orderQueryModel->data(orderQueryModel->index(index.row(), 0)).toString();
    QString orderClientId = orderQueryModel->data(orderQueryModel->index(index.row(), 1)).toString();
    QString orderProductId = orderQueryModel->data(orderQueryModel->index(index.row(), 2)).toString();
    QString clientName = orderQueryModel->data(orderQueryModel->index(index.row(), 3)).toString();
    QString phoneNum = orderQueryModel->data(orderQueryModel->index(index.row(), 4)).toString();
    QString address = orderQueryModel->data(orderQueryModel->index(index.row(), 5)).toString();
    QString productName = orderQueryModel->data(orderQueryModel->index(index.row(), 6)).toString();
    QString price = orderQueryModel->data(orderQueryModel->index(index.row(), 7)).toString();
    int quantity = orderQueryModel->data(orderQueryModel->index(index.row(), 8)).toInt();
    QString amount = orderQueryModel->data(orderQueryModel->index(index.row(), 9)).toString();
    // 테이블위젯에서 행에 맞는 값을 가져온 후 라인에디터에 저장
    clientId = orderClientId;
    productId = orderProductId;
    ui->orderIdLineEdit->setText(orderId);
    ui->clientNameLineEdit->setText(clientName);
    ui->clientPhoneNumLineEdit->setText(phoneNum);
    ui->clientAddressLineEdit->setText(address);
    ui->productNameLineEdit->setText(productName);
    ui->productPriceLineEdit->setText(price);
    ui->quantitySpinBox->setValue(quantity);
    ui->amountLineEdit->setText(amount);
}

/* 클리어 버튼이 눌렸을 때 모든 라인에디터를 초기화하는 슬롯 함수 */
void OrderForm::clearLineEdit()
{
    // 화면의 모든 라인에디터를 클리어
    ui->clientNameLineEdit->clear();
    ui->clientPhoneNumLineEdit->clear();
    ui->clientAddressLineEdit->clear();
    ui->productNameLineEdit->clear();
    ui->productPriceLineEdit->clear();
    ui->orderIdLineEdit->clear();
    ui->quantitySpinBox->clear();
    ui->amountLineEdit->clear();
}

void OrderForm::receiveClientInfo(QList<QString> result)
{
    QWidget* sentWidget(qobject_cast<QWidget*>(sender()));
    // 시그널을 보낸 sender를 확인하기 위해 QWidget 형태로 받아옴

    // 고객 관리 탭에서 받아온 고객 정보를 멤버 변수에 임시 저장
    clientId = result.at(0);
    clientName = result.at(1);
    clientPhoneNum = result.at(2);
    clientAddress = result.at(3);
    if ( "SearchingDialog" == sentWidget->objectName() ) {
        // 다이얼로그에서 받아온 고객 정보의 경우 라인에디터에 저장
        ui->clientNameLineEdit->setText(result.at(1));
        ui->clientPhoneNumLineEdit->setText(result.at(2));
        ui->clientAddressLineEdit->setText(result.at(3));
    }
}

void OrderForm::receiveProductInfo(QList<QString> result)
{
    QWidget* sentWidget(qobject_cast<QWidget*>(sender()));
    //시그널을 보낸 sender를 확인하기 위해 QWidget 변수로 저장

    // 시그널로부터 받아온 상품 정보를 멤버 변수에 임시 저장
    productId = result.at(0);
    productName = result.at(1);
    productPrice = result.at(2);
    productStock = result.at(3);
    if ( "SearchingDialog" == sentWidget->objectName() ) {
        // 다이얼로그로부터 가져온 상품 정보는 라인에디터에도 저장
        ui->productNameLineEdit->setText(result.at(1));
        ui->productPriceLineEdit->setText(result.at(2));
    }
}

/* 라인에디터의 값들을 데이블위젯에 마지막 행에 추가하는 슬롯 함수 */
void OrderForm::addTableRow()
{
    QString quantity = ui->quantitySpinBox->text();
    QString amount = ui->amountLineEdit->text();

    if( ui->quantitySpinBox->text().toUInt() > productStock.toUInt()){
        QMessageBox::critical(this, "Order Manager",
                              "Order quantity exceeded product inventory.");
        return;
    }

    orderQueryModel->setQuery(QString("CALL INSERT_SHOPPING(%1, %2, %3, %4)")
                               .arg(clientId).arg(productId).arg(quantity).arg(amount));
    orderQueryModel->setQuery("SELECT s.order_id, s.client_id, s.product_id, c.client_name, c.phone_number, c.address, p.product_name, p.price, s.quantity, s.amount FROM SHOPPING s, CLIENT c, PRODUCT p WHERE s.client_id = c.client_id AND s.product_id = p.product_id;");
    ui->tableView->setModel(orderQueryModel);

}

/* ID를 통해 테이블 위젯을 검색하고 행을 활성화하는 함수 */
void OrderForm::selectReturnPressedId()
{
    QString id = ui->orderIdLineEdit->text();
    orderQueryModel->setQuery(QString("SELECT s.order_id, s.client_id, s.product_id, c.client_name, c.phone_number, c.address, p.product_name, p.price, s.quantity, s.amount FROM SHOPPING s, CLIENT c, PRODUCT p WHERE s.client_id = c.client_id AND s.product_id = p.product_id AND s.order_id = %1;").arg(id));
    ui->tableView->setModel(orderQueryModel);
    ui->tableView->hideColumn(1);
    ui->tableView->hideColumn(2);
}

/* 라인에디터에서 returnPressed 시그널을 발생시켰을 때 검색기능을 수행하기 위한 슬롯 함수*/
void OrderForm::selectReturnPressedLineEdit()
{
    // 신호를 발생시킨 라인에디터를 변수에 저장
//    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(sender());
//    if(lineEdit == nullptr)     return;     // 라인에디터가 존재하지 않을 경우 함수 종료

//    int lineEditColumn(0);  // 라인에디터의 값과 대응되는 행을 저장하기 위한 변수
//    // 라인에디터의 값이 저장되는 행을 객체 이름을 통해 조건문으로 처리하여 번호를 저장
//    if( lineEdit->objectName() == "clientNameLineEdit" ){
//        lineEditColumn = 3;
//    } else if( lineEdit->objectName() == "clientPhoneNumLineEdit" ){
//        lineEditColumn = 4;
//    } else if( lineEdit->objectName() == "clientAddressLineEdit" ){
//        lineEditColumn = 5;
//    } else if( lineEdit->objectName() == "productNameLineEdit" ){
//        lineEditColumn = 6;
//    } else if( lineEdit->objectName() == "productPriceLineEdit" ){
//        lineEditColumn = 7;
//    } else if( lineEdit->objectName() == "amountLineEdit" ){
//        lineEditColumn = 9;
//    } else {
//        return;     // 해당하는 객체 이름이 존재하지 않을 경우 함수 종료
//    }

//    QString searchingText(lineEdit->text());    // 검색할 내용의 문자열을 라인에디터로부터 가져옴
//    // 이전 검색 리스트가 비어있거나 현재 검색 내용과 일치하지 않으면 다시 검색 리스트를 생성
//    if ( searchingList.isEmpty() || searchingList.first()->text().compare(
//             searchingText, Qt::CaseInsensitive)) {
//        // 새롭게 검색 결과 리스트를 갱신한 후 저장
//        searchingList = ui->tableWidget->findItems(searchingText,
//                                                   Qt::MatchFixedString);
//        if(searchingList.isEmpty())     return;     // 갱신한 검색 결과가 없을 경우 함수를 종료
//    }
//    // 검색 결과 리스트의 가장 위의 값을 가져와서 사용, 가져온 값은 리스트에서 제거됨
//    QTableWidgetItem* searchingResult(searchingList.takeFirst());
//    // 리스트에서 가져온 값의 행이 다르다면 다음 값을 가져옴
//    while ( searchingResult->column() != lineEditColumn ) {
//        if(searchingList.isEmpty())     return;     // 검색 결과 리스트가 비어 있다면 함수를 종료
//        searchingResult = searchingList.takeFirst();    // 다음 검색 결과를 저장
//    }

//    int searchingRow = searchingResult->row();  // 행을 활성화하기 위한 행의 값
//    // 라인에디터와 맞는 열을 가져왔는지 확인하기 위한 열의 값
//    int searchingColumn = searchingResult->column();
//    // 라인에디터의 열의 값과 검색 결과의 열의 값이 일치할 경우 조건문 수행
//    if ( searchingColumn == lineEditColumn )
//    {
//        ui->tableWidget->selectRow(searchingRow);   // 검색 결과의 행을 활성화
//        displayLineEdit(searchingRow, 0);       // 검색 결과의 값을 라인에디터에 저장
//    }
}

/* 테이블위젯의 값을 변경하는 함수 */
void OrderForm::modifyTableRow()
{
    QString orderId = ui->orderIdLineEdit->text();
    QString quantity = ui->quantitySpinBox->text();
    QString amount = ui->amountLineEdit->text();

    orderQueryModel->setQuery(QString("CALL UPDATE_SHOPPING(%1, %2, %3, %4, %5)")
                               .arg(orderId).arg(clientId).arg(productId).arg(quantity).arg(amount));
    orderQueryModel->setQuery("SELECT s.order_id, s.client_id, s.product_id, c.client_name, c.phone_number, c.address, p.product_name, p.price, s.quantity, s.amount FROM SHOPPING s, CLIENT c, PRODUCT p WHERE s.client_id = c.client_id AND s.product_id = p.product_id;");
    ui->tableView->setModel(orderQueryModel);
    ui->tableView->hideColumn(1);
    ui->tableView->hideColumn(2);
    clearLineEdit();
}

/* 테이블위젯의 선택된 행을 삭제하는 함수 */
void OrderForm::removeTableRow()
{
    // 삭제에 관한 내용을 다시 한번 확인하기 위해 메시지박스를 출력
    if(QMessageBox::warning(this, "Order Manager",
                            "Are you sure you want to delete it?",
                            QMessageBox::Yes | QMessageBox::No)
            == QMessageBox::Yes){
        // Yes를 클릭할 경우 행을 삭제
        QString id = ui->orderIdLineEdit->text();
        orderQueryModel->setQuery(QString("CALL DELETE_SHOPPING('%1')").arg(id));
        clearLineEdit();
    }
    orderQueryModel->setQuery("SELECT s.order_id, s.client_id, s.product_id, c.client_name, c.phone_number, c.address, p.product_name, p.price, s.quantity, s.amount FROM SHOPPING s, CLIENT c, PRODUCT p WHERE s.client_id = c.client_id AND s.product_id = p.product_id;");
    ui->tableView->setModel(orderQueryModel);
    ui->tableView->hideColumn(1);
    ui->tableView->hideColumn(2);

}
