#include "orderform.h"
#include "ui_orderform.h"

#include <QMessageBox>
#include <QSqlQueryModel>

/* 생성자에서 ui에 대한 시그널과 슬롯을 연결하고 DB Model 생성 */
OrderForm::OrderForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrderForm)
{
    ui->setupUi(this);

    // 클릭한 셀의 행 데이터를 라인에디터에 보여주는 기능
    connect(ui->tableView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(displayLineEdit(QModelIndex)));
    // 각 버튼의 시그널과 슬롯 함수를 연결
    connect(ui->clearPushButton, SIGNAL(clicked()),
            this, SLOT(clearLineEdit()));
    connect(ui->addPushButton, SIGNAL(clicked()), this, SLOT(addTableRow()));
    connect(ui->searchPushButton, SIGNAL(clicked()),
            this, SLOT(selectReturnPressedId()));
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
    // select 버튼 클릭시 mainWindow에 dialog를 여는 시그널 방출
    connect(ui->selectPushButton, SIGNAL(clicked()), this,
            SIGNAL(clickedSelectButton()));

    selectStatement = "SELECT s.order_id, "
                      "s.client_id, "
                      "s.product_id, "
                      "c.client_name, "
                      "c.phone_number, "
                      "c.address, "
                      "p.product_name, "
                      "p.price, "
                      "s.quantity, "
                      "s.amount "
                      "FROM SHOPPING s, CLIENT c, PRODUCT p "
                      "WHERE s.client_id = c.client_id "
                      "AND s.product_id = p.product_id";

    // 주문 관리 DB Model 생성
    orderQueryModel = new QSqlQueryModel;
    // 전체 DB를 출력하는 초기화
    orderQueryModel->setQuery(selectStatement + ';');
    orderQueryModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
    orderQueryModel->setHeaderData(3, Qt::Horizontal, tr("Client Name"));
    orderQueryModel->setHeaderData(4, Qt::Horizontal, tr("Phone Number"));
    orderQueryModel->setHeaderData(5, Qt::Horizontal, tr("Address"));
    orderQueryModel->setHeaderData(6, Qt::Horizontal, tr("Product Name"));
    orderQueryModel->setHeaderData(7, Qt::Horizontal, tr("Price"));
    orderQueryModel->setHeaderData(8, Qt::Horizontal, tr("Quantity"));
    orderQueryModel->setHeaderData(9, Qt::Horizontal, tr("Amount"));

    ui->tableView->setModel(orderQueryModel);   // 모델을 뷰에 설정
    ui->tableView->hideColumn(1);   // 사용자에게 고객 ID, 상품 ID를 숨김
    ui->tableView->hideColumn(2);

}

/* ui 객체를 해제하는 소멸자 */
OrderForm::~OrderForm()
{
    delete ui;  // 소멸자이므로 사용이 끝난 ui객체를 메모리 해제
}

/* 시그널에서 발생한 행의 값을 통해 테이블 뷰에 행을 라인에디터에 표시하는 슬롯 함수 */
void OrderForm::displayLineEdit(const QModelIndex &index)
{
    // 테이블 뷰의 시그널에서 받은 인덱스로 데이터를 변수에 저장
    QString orderId = orderQueryModel
            ->data(orderQueryModel->index(index.row(), 0)).toString();
    QString orderClientId = orderQueryModel
            ->data(orderQueryModel->index(index.row(), 1)).toString();
    QString orderProductId = orderQueryModel
            ->data(orderQueryModel->index(index.row(), 2)).toString();
    QString clientName = orderQueryModel
            ->data(orderQueryModel->index(index.row(), 3)).toString();
    QString phoneNum = orderQueryModel
            ->data(orderQueryModel->index(index.row(), 4)).toString();
    QString address = orderQueryModel
            ->data(orderQueryModel->index(index.row(), 5)).toString();
    QString productName = orderQueryModel
            ->data(orderQueryModel->index(index.row(), 6)).toString();
    QString price = orderQueryModel
            ->data(orderQueryModel->index(index.row(), 7)).toString();
    int quantity = orderQueryModel
            ->data(orderQueryModel->index(index.row(), 8)).toInt();
    QString amount = orderQueryModel
            ->data(orderQueryModel->index(index.row(), 9)).toString();
    // 테이블 뷰에서 행에 맞는 값을 가져온 후 라인에디터에 저장
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
    // 전체 DB를 출력하도록 초기화
    orderQueryModel->setQuery(selectStatement + ';');
    ui->tableView->hideColumn(1);   // 사용자에게 고객 ID, 상품 ID를 숨김
    ui->tableView->hideColumn(2);
}

/* 고객 관리 검색 결과를 반환하는 시그널을 받는 슬롯 함수 */
void OrderForm::receiveClientInfo(QList<QString> result)
{
    // client form에서 받은 데이터를 멤버 변수와 라인에디터에 저장
    clientId = result.at(0);
    ui->clientNameLineEdit->setText(result.at(1));
    ui->clientPhoneNumLineEdit->setText(result.at(2));
    ui->clientAddressLineEdit->setText(result.at(3));
}

/* 상품 관리 검색 결과를 반환하는 시그널을 받는 슬롯 함수 */
void OrderForm::receiveProductInfo(QList<QString> result)
{
    // product form에서 받은 데이터를 멤버 변수와 라인에디터에 저장
    productId = result.at(0);
    productStock = result.at(3);
    ui->productNameLineEdit->setText(result.at(1));
    ui->productPriceLineEdit->setText(result.at(2));
}

/* 라인에디터의 값들을 DB에 마지막 행에 추가하는 슬롯 함수 */
void OrderForm::addTableRow()
{
    QString quantity = ui->quantitySpinBox->text(); // 수량을 문자열로 저장
    QString amount = ui->amountLineEdit->text();    // 총액을 문자열로 저장

    // 재고보다 주문 수량이 더 많으면 경고 메시지를 출력
    if( ui->quantitySpinBox->text().toUInt() > productStock.toUInt()){
        QMessageBox::critical(this, "Order Manager",
                              "Order quantity exceeded product inventory.");
        return;
    }
    // 고객 ID, 상품 ID와 수량, 총액을 DB에 입력하는 쿼리문을 문자열로 저장
    QString instruction = QString("CALL INSERT_SHOPPING(%1, %2, %3, %4)")
            .arg(clientId).arg(productId).arg(quantity).arg(amount);
    orderQueryModel->setQuery(instruction); // 쿼리문 실행
    clearLineEdit();    // 라인에디터, 테이블 뷰 초기화
}

/* ID를 통해 테이블 뷰를 검색하고 출력하는 함수 */
void OrderForm::selectReturnPressedId()
{
    QString id = ui->orderIdLineEdit->text();   // 검색을 위한 id를 문자열로 저장
    // id를 통해 DB를 검색하는 쿼리문을 문자열로 저장
    QString instruction =
            QString(selectStatement + " AND s.order_id = %1;").arg(id);
    orderQueryModel->setQuery(instruction); // 쿼리문 실행
}

/* 라인에디터에서 returnPressed 시그널을 발생시켰을 때 검색 결과를 출력하기 위한 슬롯 함수*/
void OrderForm::selectReturnPressedLineEdit()
{
    // 신호를 발생시킨 라인에디터를 변수로 저장
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(sender());
    if(lineEdit == nullptr)     return;     // 라인에디터가 존재하지 않을 경우 함수 종료

    QString instruction;    // DB 검색을 위한 쿼리문을 저장할 변수 선언
    // 검색할 내용이 저장된 라인에디터에 맞는 검색 쿼리문 저장
    if( lineEdit->objectName() == "clientNameLineEdit" ){
        QString clientName = ui->clientNameLineEdit->text();
        instruction = QString(selectStatement + " AND c.client_name = '%1';")
                .arg(clientName);
    } else if( lineEdit->objectName() == "clientPhoneNumLineEdit" ){
        QString phoneNum = ui->clientPhoneNumLineEdit->text();
        instruction = QString(selectStatement + " AND c.phone_number = '%1';")
                .arg(phoneNum);
    } else if( lineEdit->objectName() == "clientAddressLineEdit" ){
        QString address = ui->clientAddressLineEdit->text();
        instruction = QString(selectStatement + " AND c.address = '%1';")
                .arg(address);
    } else if( lineEdit->objectName() == "productNameLineEdit" ){
        QString productName = ui->productNameLineEdit->text();
        instruction = QString(selectStatement + " AND p.product_name = '%1';")
                .arg(productName);
    } else if( lineEdit->objectName() == "productPriceLineEdit" ){
        QString price = ui->productPriceLineEdit->text();
        instruction = QString(selectStatement + " AND p.price = '%1';")
                .arg(price);
    } else if( lineEdit->objectName() == "amountLineEdit" ){
        QString amount = ui->amountLineEdit->text();
        instruction = QString(selectStatement + " AND s.amount = '%1';")
                .arg(amount);
    } else {
        return;     // 해당하는 객체 이름이 존재하지 않을 경우 함수 종료
    }
    orderQueryModel->setQuery(instruction); // 쿼리문 실행
}

/* DB의 값을 변경하는 함수 */
void OrderForm::modifyTableRow()
{
    // 변경할 DB를 선택할 id와 변경 내용을 변수로 저장
    QString orderId = ui->orderIdLineEdit->text();
    QString quantity = ui->quantitySpinBox->text();
    QString amount = ui->amountLineEdit->text();
    // id, 변경 내용들로 DB를 변경하는 쿼리문을 문자열로 저장
    QString instruction = QString("CALL UPDATE_SHOPPING(%1, %2, %3, %4, %5)")
            .arg(orderId).arg(clientId).arg(productId).arg(quantity)
            .arg(amount);
    orderQueryModel->setQuery(instruction); // 쿼리문 실행
    clearLineEdit();
}

/* id에 맞는 DB를 삭제하는 함수 */
void OrderForm::removeTableRow()
{
    // 삭제에 관한 내용을 다시 한번 확인하기 위해 메시지박스를 출력
    if(QMessageBox::warning(this, "Order Manager",
                            "Are you sure you want to delete it?",
                            QMessageBox::Yes | QMessageBox::No)
            == QMessageBox::Yes){
        // Yes를 클릭할 경우 행을 삭제
        QString id = ui->orderIdLineEdit->text();   // 삭제를 위한 id를 변수로 저장
        orderQueryModel->setQuery(QString("CALL DELETE_SHOPPING('%1')")
                                  .arg(id));    // id로 DB를 삭제하는 쿼리문 실행
        clearLineEdit();    // 라인에디터 테이블 뷰 초기화
    }
}
