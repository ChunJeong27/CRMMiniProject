#include "productform.h"
#include "ui_productform.h"

#include <QFile>
#include <QMessageBox>
#include <QSqlQueryModel>

ProductForm::ProductForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProductForm)
{
    ui->setupUi(this);

    connect(ui->tableView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(displayLineEdit(QModelIndex)));
    // 클릭한 셀의 행 데이터를 라인에디터에 보여주는
    connect(ui->clearPushButton, SIGNAL(clicked()),
            this, SLOT(clearLineEdit()));   // 모든 라인에디터를 초기하는 기능
    connect(ui->addPushButton, SIGNAL(clicked()), this, SLOT(addTableRow()));
    // 라인에디터의 데이터를 테이블위젯에 추가하는 기능 연결
    connect(ui->searchPushButton, SIGNAL(clicked()),
            this, SLOT(selectReturnPressedId()));
    // 버튼 시그널에 대해 id를 검색하는 기능 연결
    connect(ui->modifyPushButton, SIGNAL(clicked()),
            this, SLOT(modifyTableRow()));  // 선택된 테이블 행을 변경하는 기능 연결
    connect(ui->removePushButton, SIGNAL(clicked()),
            this, SLOT(removeTableRow()));  // 선택된 테이블 행을 삭제하는 기능 연결
    //
    connect(ui->idLineEdit, SIGNAL(returnPressed()),
            this, SLOT(selectReturnPressedId()));
    connect(ui->nameLineEdit, SIGNAL(returnPressed()),
            this, SLOT(selectReturnPressedLineEdit()));
    connect(ui->priceLineEdit, SIGNAL(returnPressed()),
            this, SLOT(selectReturnPressedLineEdit()));
    connect(ui->stockLineEdit, SIGNAL(returnPressed()),
            this, SLOT(selectReturnPressedLineEdit()));

    productQueryModel = new QSqlQueryModel;
    productQueryModel->setQuery("SELECT * FROM PRODUCT");
    productQueryModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
    productQueryModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
    productQueryModel->setHeaderData(2, Qt::Horizontal, tr("Price"));
    productQueryModel->setHeaderData(3, Qt::Horizontal, tr("Stock"));

    ui->tableView->setModel(productQueryModel);

}

/* 테이블위젯의 데이터를 소멸자에서 csv 형식으로 저장 */
ProductForm::~ProductForm()
{
    delete ui;  // 소멸자이므로 사용한 ui 객체의 메모리를 해제
}

/* 선택된 행의 값을 통해 데이터를 라인에디터로 표시하는 슬롯 함수*/
void ProductForm::displayLineEdit(const QModelIndex &index)
{
    // 시그널에서 받아온 행의 값으로 테이블위젯의 값을 라인에디터에 저장
    QString id = productQueryModel->data(productQueryModel->index(index.row(), 0)).toString();
    QString name = productQueryModel->data(productQueryModel->index(index.row(), 1)).toString();
    QString price = productQueryModel->data(productQueryModel->index(index.row(), 2)).toString();
    QString stock = productQueryModel->data(productQueryModel->index(index.row(), 3)).toString();

    ui->idLineEdit->setText(id);
    ui->nameLineEdit->setText(name);
    ui->priceLineEdit->setText(price);
    ui->stockLineEdit->setText(stock);
}

/* 모든 라인에디터를 클리어하는 슬롯 함수*/
void ProductForm::clearLineEdit()
{
    // 화면의 모든 라인에디터를 클리어
    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->priceLineEdit->clear();
    ui->stockLineEdit->clear();

    productQueryModel->setQuery("SELECT * FROM PRODUCT");
}

/* 라인에디터의 값들을 테이블위젯 마지막 행에 추가하는 슬롯 함수 */
void ProductForm::addTableRow()
{
    // 라인에디터의 값들을 문자열로 저장
    QString name = ui->nameLineEdit->text();
    QString price = ui->priceLineEdit->text();
    QString stock = ui->stockLineEdit->text();
    // 값을 저장한 라인에디터들을 초기화
    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->priceLineEdit->clear();
    ui->stockLineEdit->clear();
    // 테이블위젯에 빈 셀을 추가하지 않기 위해 문자열의 길이로 조건문을 처리
    // 글자수가 0보다 같거나 작으면 라인에디터가 비어있다고 판단
    if ( name.length() <= 0 || price.length() <=0 || stock.length() <= 0 ) {
        QMessageBox::warning(this, tr("Product Manager"),
                             tr("Check your input again."), QMessageBox::Ok);
        // 비어있는 라인에디터가 있을 경우 경고 메시지 출력
        return;     // 경고메시지 출력 후 함수 종료
    }
    else {  // 모든 라인에디터 값이 존재할 경우
        productQueryModel->setQuery(QString("CALL INSERT_PRODUCT('%1', '%2', '%3')")
                                   .arg(name).arg(price).arg(stock));
        productQueryModel->setQuery("SELECT * FROM PRODUCT");
    }
}

/* ID를 통해 테이블위젯을 검색하고 행을 활성화하는 슬롯 함수 */
void ProductForm::selectReturnPressedId()
{
    QString id = ui->idLineEdit->text();
    productQueryModel->setQuery(QString("SELECT * FROM PRODUCT WHERE PRODUCT_ID = %1").arg(id));
    QString name = productQueryModel->data(productQueryModel->index(0, 1)).toString();
    QString phoneNum = productQueryModel->data(productQueryModel->index(0, 2)).toString();
    QString address = productQueryModel->data(productQueryModel->index(0, 3)).toString();
    ui->nameLineEdit->setText(name);
    ui->priceLineEdit->setText(phoneNum);
    ui->stockLineEdit->setText(address);
}

/* 여러 라인에디터에서 returnPressed 시그널을 발생시켰을 때 검색기능을 수행하기 위한 슬롯 함수 */
void ProductForm::selectReturnPressedLineEdit()
{
    // 신호를 발생시킨 라인에디터를 동적 형변환을 통해 변수에 저장
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(sender());
    if(lineEdit == nullptr)     return;     // 라인에디터가 존재하지 않을 경우 함수 종료

    int lineEditColumn(0);  // 라인에디터의 값과 대응되는 행을 저장하기 위한 변수
    // 라인에디터의 값이 저장되는 행을 객체 이름을 통해 조건문으로 처리하여 번호를 저장
    if( lineEdit->objectName() == "nameLineEdit" ){
        QString name = ui->nameLineEdit->text();
        productQueryModel->setQuery(QString("SELECT * FROM PRODUCT WHERE PRODUCT_NAME = '%1'").arg(name));
    } else if( lineEdit->objectName() == "priceLineEdit" ){
        QString price = ui->priceLineEdit->text();
        productQueryModel->setQuery(QString("SELECT * FROM PRODUCT WHERE PRICE = '%1'").arg(price));
    } else if( lineEdit->objectName() == "stockLineEdit" ){
        QString stock = ui->stockLineEdit->text();
        productQueryModel->setQuery(QString("SELECT * FROM PRODUCT WHERE ADDRESS = '%1'").arg(stock));
    } else {
        return;     // 조건에 해당하지 않을 경우 함수를 종료
    }

}

/* 테이블위젯 행의 값을 변경하는 슬롯 함수 */
void ProductForm::modifyTableRow()
{
    // 라인 에디터의 값들을 문자열 변수에 저장
    QString id = ui->idLineEdit->text();
    QString name = ui->nameLineEdit->text();
    QString price = ui->priceLineEdit->text();
    QString stock = ui->stockLineEdit->text();
    // 값을 저장한 라인에디터들을 초기화
    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->priceLineEdit->clear();
    ui->stockLineEdit->clear();

    productQueryModel->setQuery(QString("CALL UPDATE_PRODUCT(%1, '%2', '%3', '%4')")
                               .arg(id).arg(name).arg(price).arg(stock));
    productQueryModel->setQuery("SELECT * FROM PRODUCT");
}

/* 선택된 테이블 위젯의 행을 삭제하는 슬롯 함수 */
void ProductForm::removeTableRow()
{
    if(QMessageBox::warning(this, tr("Product Manager"),
                            tr("Are you sure you want to delete it?"),
                             QMessageBox::Yes | QMessageBox::No)
            == QMessageBox::Yes){
        QString id = ui->idLineEdit->text();
        productQueryModel->setQuery(QString("CALL DELETE_PRODUCT('%1')").arg(id));
    }
        // Yes를 선택할 경우 행을 삭제
    productQueryModel->setQuery("SELECT * FROM PRODUCT");
    clearLineEdit();
}

/* 테이블위젯에서 열로 데이터를 구분하고 검색하는 슬롯 함수 */
void ProductForm::searching(QString columnName, QString searchingText)
{
    if( "ID" == columnName ){ // 항목 값에 따라 인자값을 다르게 시그널 발생
        productQueryModel->setQuery(QString("SELECT * FROM PRODUCT WHERE PRODUCT_ID = %1").arg(searchingText));
    } else if( "Name" == columnName ){
        productQueryModel->setQuery(QString("SELECT * FROM PRODUCT WHERE PRODUCT_NAME = '%1'").arg(searchingText));
    } else if( "Price" == columnName ){
        productQueryModel->setQuery(QString("SELECT * FROM PRODUCT WHERE PRICE = '%1'").arg(searchingText));
    } else if( "Stock" == columnName ){
        productQueryModel->setQuery(QString("SELECT * FROM PRODUCT WHERE STOCK = '%1'").arg(searchingText));
    } else {
        return;
    }
    QList<QString> searchResults;
    int resultRow = productQueryModel->rowCount();
    for (int i = 0; i < resultRow; i++ ){
        searchResults << productQueryModel->data(productQueryModel->index(i, 0)).toString()
                      << productQueryModel->data(productQueryModel->index(i, 1)).toString()
                      << productQueryModel->data(productQueryModel->index(i, 2)).toString()
                      << productQueryModel->data(productQueryModel->index(i, 3)).toString();
    }
    productQueryModel->setQuery("SELECT * FROM PRODUCT");
    emit returnSearching(searchResults);
    // 검색 결과를 저장한 리스트를 시그널을 통해 orderForm으로 전달
}
