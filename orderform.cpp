#include "orderform.h"
#include "ui_orderform.h"

#include <QFile>
#include <QTableWidgetItem>
#include <QMessageBox>

#include <searchingdialog.h>

/* 생성자에서 ui에 대한 시그널과 슬롯을 연결 */
OrderForm::OrderForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrderForm)
{
    ui->setupUi(this);

    connect(ui->tableWidget, SIGNAL(cellClicked(int,int)),
            this, SLOT(displayItem(int,int)));  // 클릭한 셀의 행 데이터를 라인에디터에 보여주는 기능
    // 라인에디터에 따라 맞는 열을 검색하고 결과를 활성화시키는 기능
    connect(ui->clientNameLineEdit, SIGNAL(returnPressed()),
            this, SLOT(returnPressedSearching()));
    connect(ui->clientPhoneNumLineEdit, SIGNAL(returnPressed()),
            this, SLOT(returnPressedSearching()));
    connect(ui->clientAddressLineEdit, SIGNAL(returnPressed()),
            this, SLOT(returnPressedSearching()));
    connect(ui->productNameLineEdit, SIGNAL(returnPressed()),
            this, SLOT(returnPressedSearching()));
    connect(ui->productPriceLineEdit, SIGNAL(returnPressed()),
            this, SLOT(returnPressedSearching()));
    connect(ui->amountLineEdit, SIGNAL(returnPressed()),
            this, SLOT(returnPressedSearching()));

    connect(ui->orderIdLineEdit, SIGNAL(returnPressed()),
            this, SLOT(on_searchPushButton_clicked())); // 아이디를 검색하고 행을 활성화하는 기능
    // 스핀박스 값이 바뀔 때 총액을 설정하는 기능을 람다함수로 구현
    connect(ui->quantitySpinBox, &QSpinBox::valueChanged, this, [=](int i){
        quint32 amount = ui->productPriceLineEdit->text().toUInt() * i;
        ui->amountLineEdit->setText(QString::number(amount));});

    connect(ui->selectPushButton, SIGNAL(clicked()), this, SIGNAL(clickedSearchButton()));

}

/* 기록한 데이터를 소멸자에서 csv파일에 저장 */
OrderForm::~OrderForm()
{
    QFile file("orderForm.txt");    // 저장할 파일명으로 QFile 객체를 생성

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return; // 파일을 정상적으로 열지 못했을 경우 함수를 종료

    QTextStream out(&file); // 텍스트스트림으로 쓰기위해 파일을 열어줌
    int tableRowCount = ui->tableWidget->rowCount();
    // 반복 횟수를 위한 행의 수를 변수에 저장
    for (int i = 0; i < tableRowCount; i++) {
        // 쉼표로 데이터를 구분하고 스트림 연산자를 통해 데이터를 저장, 마지막엔 줄바꿈 문자를 삽입
        out << ui->tableWidget->item(i, 0)->text() << ","
            << ui->tableWidget->item(i, 1)->text() << ","
            << ui->tableWidget->item(i, 2)->text() << ","
            << ui->tableWidget->item(i, 8)->text() << "\n";
    }
    file.close();   // 기록이 끝난 파일을 닫아줌

    delete ui;  // 소멸자이므로 사용이 끝난 ui객체를 메모리 해제
}

/* 저장된 csv파일에서 데이터를 불러오는 함수 */
void OrderForm::loadData()
{
    QFile file("orderForm.txt");    // 저장된 csv파일을 열기
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return; // 파일이 정상적으로 열리지 않으면 함수 종료

    QTextStream in(&file);  // 텍스트스트림으로 읽기 위해 파일을 열어줌
    while (!in.atEnd()) {   // 파일의 끝을 만날 때까지 반복
        QString line = in.readLine();   // 파일의 한 줄을 문자열로 읽어옴
        QStringList row = line.split(",");  // 쉼표로 구분하여 리스트에 저장
        if (!row.isEmpty()) {     // 리스트가 비어있지 않을 경우
            // 리스트의 저장된 문자열을 각 테이블위젯아이템으로 생성
            QTableWidgetItem* orderIdItem = new QTableWidgetItem(row.at(0));
            QTableWidgetItem* clientIdItem = new QTableWidgetItem(row.at(1));
            QTableWidgetItem* productIdItem = new QTableWidgetItem(row.at(2));
            QTableWidgetItem* orderQuantityItem =
                    new QTableWidgetItem(row.at(3));
            // 불러온 고객 ID와 제품 ID로 각 관리 클래스에서 정보를 가져오기 위해 시그널 방출
            emit searchedClient(0, row.at(1));
            emit searchedProduct(0, row.at(2));
            // 위 시그널을 통해 가져온 데이터로 각 테이블위젯아이템을 생성
            QTableWidgetItem* clientNameItem =
                    new QTableWidgetItem(clientName);
            QTableWidgetItem* clientPhoneNumItem =
                    new QTableWidgetItem(clientPhoneNum);
            QTableWidgetItem* clientAddressItem =
                    new QTableWidgetItem(clientAddress);
            QTableWidgetItem* productNameItem =
                    new QTableWidgetItem(productName);
            QTableWidgetItem* productPriceItem =
                    new QTableWidgetItem(productPrice);
            // 제품 가격과 수량으로 총액을 계산하고 아이템으로 생성
            quint32 amount = productPrice.toUInt() * row.at(3).toUInt();
            QTableWidgetItem* orderAmountItem =
                    new QTableWidgetItem(QString::number(amount));
            // 현재 테이블위젯의 행 수에서 1을 증가시켜 행의 수를 설정
            int tableRowCount = ui->tableWidget->rowCount();
            ui->tableWidget->setRowCount( tableRowCount + 1);
            // 생성된 아이템들을 테이블위젯에 저장
            ui->tableWidget->setItem(tableRowCount, 0, orderIdItem);
            ui->tableWidget->setItem(tableRowCount, 1, clientIdItem);
            ui->tableWidget->setItem(tableRowCount, 2, productIdItem);
            ui->tableWidget->setItem(tableRowCount, 3, clientNameItem);
            ui->tableWidget->setItem(tableRowCount, 4, clientPhoneNumItem);
            ui->tableWidget->setItem(tableRowCount, 5, clientAddressItem);
            ui->tableWidget->setItem(tableRowCount, 6, productNameItem);
            ui->tableWidget->setItem(tableRowCount, 7, productPriceItem);
            ui->tableWidget->setItem(tableRowCount, 8, orderQuantityItem);
            ui->tableWidget->setItem(tableRowCount, 9, orderAmountItem);
        }
    }
    on_clearPushButton_clicked();   // 라인에디터를 클리어하기 위해 함수를 호출
    file.close();   // 데이터를 모두 불러온 후 파일을 닫아줌
}

/* 주문 정보를 추가할 때 중복되지 않도록 ID를 생성하는 함수 */
int OrderForm::makeId()
{
    int lastColumn = ui->tableWidget->rowCount() - 1;   // 테이블위젯의 마지막 행을 가져옴

    if( lastColumn < 0 )   // 데이터가 없을 경우 3000번부터 ID를 부여
        return 3000;

    int creatId = ui->tableWidget->item(lastColumn, 0)->text().toInt() + 1;
    // 마지막 행의 0번째 열의 ID값을 가져온 후 +1을 추가한 값을 부여
    return creatId;     // 생성된 ID값을 리턴
}

/* 시그널에서 발생한 행의 값을 통해 테이블위젯의 행을 라인에디터에 표시하는 슬롯 함수 */
void OrderForm::displayItem(int row,int column)
{
    Q_UNUSED(column);   // 사용하지 않는 열 변수를 명시적으로 표시
    // 테이블위젯에서 행에 맞는 값을 가져온 후 라인에디터에 저장
    ui->orderIdLineEdit->setText(ui->tableWidget->item(row, 0)->text());
    ui->clientNameLineEdit->setText(ui->tableWidget->item(row, 3)->text());
    ui->clientPhoneNumLineEdit->setText(ui->tableWidget->item(row, 4)->text());
    ui->clientAddressLineEdit->setText(ui->tableWidget->item(row, 5)->text());
    ui->productNameLineEdit->setText(ui->tableWidget->item(row, 6)->text());
    ui->productPriceLineEdit->setText(ui->tableWidget->item(row, 7)->text());
    ui->quantitySpinBox->setValue(ui->tableWidget->item(row, 8)->text().toInt());
    ui->amountLineEdit->setText(ui->tableWidget->item(row, 9)->text());
}

/* 클리어 버튼이 눌렸을 때 모든 라인에디터를 초기화하는 슬롯 함수 */
void OrderForm::on_clearPushButton_clicked()
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

/* 라인에디터의 값들을 데이블위젯에 마지막 행에 추가하는 슬롯 함수 */
void OrderForm::on_addPushButton_clicked()
{
    if( ui->quantitySpinBox->text().toUInt() > productStock.toUInt()){
        QMessageBox::critical(this, "Order Manager",
                              "Order quantity exceeded product inventory.");
        return;
    }
    // 다이얼로그에서 저장한 멤버변수의 값들을 테이블위젯아이템으로 생성
    // 아이디는 중복되지 않도록 함수를 통해 마지막 ID에서 +1한 값을 사용
    QTableWidgetItem* orderIdItem =
            new QTableWidgetItem(QString::number(makeId()));
    QTableWidgetItem* clientIdItem = new QTableWidgetItem(clientId);
    QTableWidgetItem* productIdItem = new QTableWidgetItem(productId);

    QTableWidgetItem* clientNameItem = new QTableWidgetItem(clientName);
    QTableWidgetItem* clientPhoneNumItem =
            new QTableWidgetItem(clientPhoneNum);
    QTableWidgetItem* clientAddressItem = new QTableWidgetItem(clientAddress);

    QTableWidgetItem* productNameItem = new QTableWidgetItem(productName);
    QTableWidgetItem* productPriceItem = new QTableWidgetItem(productPrice);
    // 수량, 총액은 스핀박스와 라인에디터의 값으로 아이템을 생성
    QTableWidgetItem* orderQuantityItem = new QTableWidgetItem(ui->quantitySpinBox->text());
    quint32 amount = productPrice.toUInt() * ui->quantitySpinBox->text().toUInt();
    QTableWidgetItem* orderAmountItem = new QTableWidgetItem(QString::number(amount));
    // 테이블위젯의 마지막 행에 데이터를 추가하기 위해 현재 행의 수를 가져와 +1한 후 설정
    int tableRowCount = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount( tableRowCount + 1);
    // 각 아이템들을 테이블위젯의 행에 맞게 저장
    ui->tableWidget->setItem(tableRowCount, 0, orderIdItem);
    ui->tableWidget->setItem(tableRowCount, 1, clientIdItem);
    ui->tableWidget->setItem(tableRowCount, 2, productIdItem);
    ui->tableWidget->setItem(tableRowCount, 3, clientNameItem);
    ui->tableWidget->setItem(tableRowCount, 4, clientPhoneNumItem);
    ui->tableWidget->setItem(tableRowCount, 5, clientAddressItem);
    ui->tableWidget->setItem(tableRowCount, 6, productNameItem);
    ui->tableWidget->setItem(tableRowCount, 7, productPriceItem);
    ui->tableWidget->setItem(tableRowCount, 8, orderQuantityItem);
    ui->tableWidget->setItem(tableRowCount, 9, orderAmountItem);
}

/* ID를 통해 테이블 위젯을 검색하고 행을 활성화하는 함수 */
void OrderForm::on_searchPushButton_clicked()
{
    QString searchingText(ui->orderIdLineEdit->text()); // ID값을 문자열로 저장
    QList<QTableWidgetItem*> searchingResult;   // 검색 결과를 저장하기 위한 리스트 변수 생성
    // 문자열이 정확히 일치하는 검색 결과를 저장
    searchingResult = ui->tableWidget->findItems(searchingText,
                                                 Qt::MatchFixedString);
    if(searchingResult.isEmpty())     return; // 검색 결과가 존재하지 않으면 함수를 종료

    int searchingRow = searchingResult.first()->row();  // 행을 활성화하기 위한 행의 값
    // 라인에디터와 맞는 값을 가져왔는지 확인하기 위한 열의 값
    int searchingColumn = searchingResult.first()->column();
    // ID 라인에디터의 열의 값인 0과 검색 결과의 열이 일치할 경우 조건문 수행
    if ( 0 == searchingColumn ){
        ui->tableWidget->selectRow(searchingRow);   // 검색 결과의 행을 활성화
        displayItem(searchingRow, 0);   // 검색 결과의 값을 라인에디터에 저장
    }
}

/* 테이블위젯의 값을 변경하는 함수 */
void OrderForm::on_modifyPushButton_clicked()
{
    // 현재 선택된 행의 값을 변수에 저장
    int tableCurrentRow = ui->tableWidget->currentRow();
    // 다이얼로그로부터 가져온 멤버 변수에 저장된 값을 아이템으로 생성
    QTableWidgetItem* clientIdItem = new QTableWidgetItem(clientId);
    QTableWidgetItem* productIdItem = new QTableWidgetItem(productId);
    // 다이얼로그로부터 라인에디터에 저장한 값들을 가져온 후 아이템으로 생성
    QTableWidgetItem* clientNameItem =
            new QTableWidgetItem(ui->clientNameLineEdit->text());
    QTableWidgetItem* clientPhoneNumItem =
            new QTableWidgetItem(ui->clientPhoneNumLineEdit->text());
    QTableWidgetItem* clientAddressItem =
            new QTableWidgetItem(ui->clientAddressLineEdit->text());

    QTableWidgetItem* productNameItem =
            new QTableWidgetItem(ui->productNameLineEdit->text());
    QTableWidgetItem* productPriceItem =
            new QTableWidgetItem(ui->productPriceLineEdit->text());
    // 스핀박스와 라인에디터에 수정된 값을 가져온 후 아이템으로 생성
    QTableWidgetItem* orderQuantityItem =
            new QTableWidgetItem(ui->quantitySpinBox->text());
    QTableWidgetItem* orderAmountItem =
            new QTableWidgetItem(ui->amountLineEdit->text());
    // 활성화된 행에 열에 맞는 각 아이템들을 저장
    ui->tableWidget->setItem(tableCurrentRow, 1, clientIdItem);
    ui->tableWidget->setItem(tableCurrentRow, 2, productIdItem);
    ui->tableWidget->setItem(tableCurrentRow, 3, clientNameItem);
    ui->tableWidget->setItem(tableCurrentRow, 4, clientPhoneNumItem);
    ui->tableWidget->setItem(tableCurrentRow, 5, clientAddressItem);
    ui->tableWidget->setItem(tableCurrentRow, 6, productNameItem);
    ui->tableWidget->setItem(tableCurrentRow, 7, productPriceItem);
    ui->tableWidget->setItem(tableCurrentRow, 8, orderQuantityItem);
    ui->tableWidget->setItem(tableCurrentRow, 9, orderAmountItem);
}

/* 테이블위젯의 선택된 행을 삭제하는 함수 */
void OrderForm::on_removePushButton_clicked()
{
    // 삭제에 관한 내용을 다시 한번 확인하기 위해 메시지박스를 출력
    if(QMessageBox::warning(this, "Order Manager",
                            "Are you sure you want to delete it?",
                            QMessageBox::Yes | QMessageBox::No)
            == QMessageBox::Yes){
        // Yes를 클릭할 경우 행을 삭제
        ui->tableWidget->removeRow(ui->tableWidget->currentRow());
    }

}

/* 라인에디터에서 returnPressed 시그널을 발생시켰을 때 검색기능을 수행하기 위한 슬롯 함수*/
void OrderForm::returnPressedSearching()
{
    // 신호를 발생시킨 라인에디터를 변수에 저장
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(sender());
    if(lineEdit == nullptr)     return;     // 라인에디터가 존재하지 않을 경우 함수 종료

    int lineEditColumn(0);  // 라인에디터의 값과 대응되는 행을 저장하기 위한 변수
    // 라인에디터의 값이 저장되는 행을 객체 이름을 통해 조건문으로 처리하여 번호를 저장
    if( lineEdit->objectName() == "clientNameLineEdit" ){
        lineEditColumn = 3;
    } else if( lineEdit->objectName() == "clientPhoneNumLineEdit" ){
        lineEditColumn = 4;
    } else if( lineEdit->objectName() == "clientAddressLineEdit" ){
        lineEditColumn = 5;
    } else if( lineEdit->objectName() == "productNameLineEdit" ){
        lineEditColumn = 6;
    } else if( lineEdit->objectName() == "productPriceLineEdit" ){
        lineEditColumn = 7;
    } else if( lineEdit->objectName() == "amountLineEdit" ){
        lineEditColumn = 9;
    } else {
        return;     // 해당하는 객체 이름이 존재하지 않을 경우 함수 종료
    }

    QString searchingText(lineEdit->text());    // 검색할 내용의 문자열을 라인에디터로부터 가져옴
    // 이전 검색 리스트가 비어있거나 현재 검색 내용과 일치하지 않으면 다시 검색 리스트를 생성
    if ( searchingList.isEmpty() || searchingList.first()->text().compare(
             searchingText, Qt::CaseInsensitive)) {
        // 새롭게 검색 결과 리스트를 갱신한 후 저장
        searchingList = ui->tableWidget->findItems(searchingText,
                                                   Qt::MatchFixedString);
        if(searchingList.isEmpty())     return;     // 갱신한 검색 결과가 없을 경우 함수를 종료
    }
    // 검색 결과 리스트의 가장 위의 값을 가져와서 사용, 가져온 값은 리스트에서 제거됨
    QTableWidgetItem* searchingResult(searchingList.takeFirst());
    // 리스트에서 가져온 값의 행이 다르다면 다음 값을 가져옴
    while ( searchingResult->column() != lineEditColumn ) {
        if(searchingList.isEmpty())     return;     // 검색 결과 리스트가 비어 있다면 함수를 종료
        searchingResult = searchingList.takeFirst();    // 다음 검색 결과를 저장
    }

    int searchingRow = searchingResult->row();  // 행을 활성화하기 위한 행의 값
    // 라인에디터와 맞는 열을 가져왔는지 확인하기 위한 열의 값
    int searchingColumn = searchingResult->column();
    // 라인에디터의 열의 값과 검색 결과의 열의 값이 일치할 경우 조건문 수행
    if ( searchingColumn == lineEditColumn )
    {
        ui->tableWidget->selectRow(searchingRow);   // 검색 결과의 행을 활성화
        displayItem(searchingRow, 0);       // 검색 결과의 값을 라인에디터에 저장
    }
}

void OrderForm::addClientResult(QList<QString> result)
{
    clientId = result.at(0);
    clientName = result.at(1);
    clientPhoneNum = result.at(2);
    clientAddress = result.at(3);
    ui->clientNameLineEdit->setText(result.at(1));
    ui->clientPhoneNumLineEdit->setText(result.at(2));
    ui->clientAddressLineEdit->setText(result.at(3));
}

void OrderForm::addProductResult(QList<QString> result)
{
    productId = result.at(0);
    productName = result.at(1);
    productPrice = result.at(2);
    productStock = result.at(3);
    ui->productNameLineEdit->setText(result.at(1));
    ui->productPriceLineEdit->setText(result.at(2));
}
