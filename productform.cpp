#include "productform.h"
#include "ui_productform.h"

#include <QFile>
#include <QMessageBox>

ProductForm::ProductForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProductForm)
{
    ui->setupUi(this);

    connect(ui->tableWidget, SIGNAL(cellClicked(int,int)),
            this, SLOT(displayLineEdit(int,int)));
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


    loadData(); // csv 형색으로 저장한 파일을 불러오는 함수
}

/* 테이블위젯의 데이터를 소멸자에서 csv 형식으로 저장 */
ProductForm::~ProductForm()
{
    QFile file("productForm.txt");  // 저장할 파일명으로 QFile 객체 생성

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;     // 파일이 정상적으로 열리지 않으면 함수를 종료

    QTextStream out(&file); // 파일을 텍스트스트림 형태로 기록하기 위한 객체 생성
    int tableRowCount = ui->tableWidget->rowCount();
    // 반복 횟수로 사용할 행의 개수를 선언 및 초기화
    for (int i = 0; i < tableRowCount; i++) {
        // 스트림 연산자를 통해 쉼표로 데이터를 구분해서 저장, 마지막엔 줄바꿈 문자를 삽입
        out << ui->tableWidget->item(i, 0)->text() << ","
            << ui->tableWidget->item(i, 1)->text() << ","
            << ui->tableWidget->item(i, 2)->text() << ","
            << ui->tableWidget->item(i, 3)->text() << "\n";
    }
    file.close();   // 기록을 마친 파일은 닫음

    delete ui;  // 소멸자이므로 사용한 ui 객체의 메모리를 해제
}

/* csv 파일 데이터를 불러오고 테이블에 추가하는 함수 */
void ProductForm::loadData()
{
    QFile file("productForm.txt");  // 저장된 csv파일의 파일명으로 QFile 객체를 생성

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return; // 파일이 정상적으로 열리지 않을 경우 함수를 종료

    QTextStream in(&file);  // 파일을 텍스트스트림 형태로 다루기 위해 객체 생성
    while (!in.atEnd()) {   // 파일의 끝을 만날 때까지 반복
        QString line = in.readLine();   // 파일에서 한 줄을 읽어옴
        QStringList row = line.split(",");  // 쉼표로 구분해서 리스트에 저장
        if (!row.empty()) { // 파싱한 결과가 비어있지 않다면
            // 각 문자열을 QTableWidgetItem에 저장 및 item 생성
            QTableWidgetItem* productID = new QTableWidgetItem(row.at(0));
            QTableWidgetItem* productName = new QTableWidgetItem(row.at(1));
            QTableWidgetItem* productPrice = new QTableWidgetItem(row.at(2));
            QTableWidgetItem* productStock = new QTableWidgetItem(row.at(3));
            // 현재 테이블위젯의 행 개수를 1증가
            int tableRowCount = ui->tableWidget->rowCount();
            ui->tableWidget->setRowCount( tableRowCount + 1);
            // 추가된 행에 생성한 아이템들을 순서대로 저장
            ui->tableWidget->setItem(tableRowCount, 0, productID);
            ui->tableWidget->setItem(tableRowCount, 1, productName);
            ui->tableWidget->setItem(tableRowCount, 2, productPrice);
            ui->tableWidget->setItem(tableRowCount, 3, productStock);
        }
    }
    file.close();   // 데이터를 불러온 파일을 닫음

}

/* 고객 정보를 추가할 때 ID가 중복되지 않도록 생성하는 함수 */
int ProductForm::makeId()
{
    int lastColumn = ui->tableWidget->rowCount() - 1;   // 마지막 행의 값을 변수에 저장

    if( lastColumn < 0 )    // 데이터가 존재하지 않을 경우 2000번부터 ID를 부여
        return 2000;

    int creatId = ui->tableWidget->item(lastColumn, 0)->text().toInt() + 1;
    // 마지막 행의 ID값에 1을 추가한 값을 ID로 부여
    return creatId;     // 생성한 ID값을 반환
}

/* 선택된 행의 값을 통해 데이터를 라인에디터로 표시하는 슬롯 함수*/
void ProductForm::displayLineEdit(int row,int column)
{
    Q_UNUSED(column);   // 사용하지 않는 column값을 명시적으로 표시
    // 시그널에서 받아온 행의 값으로 테이블위젯의 값을 라인에디터에 저장
    ui->idLineEdit->setText(ui->tableWidget->item(row, 0)->text());
    ui->nameLineEdit->setText(ui->tableWidget->item(row, 1)->text());
    ui->priceLineEdit->setText(ui->tableWidget->item(row, 2)->text());
    ui->stockLineEdit->setText(ui->tableWidget->item(row, 3)->text());
}

/* 모든 라인에디터를 클리어하는 슬롯 함수*/
void ProductForm::clearLineEdit()
{
    // 화면의 모든 라인에디터를 클리어
    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->priceLineEdit->clear();
    ui->stockLineEdit->clear();
}

/* 라인에디터의 값들을 테이블위젯 마지막 행에 추가하는 슬롯 함수 */
void ProductForm::addTableRow()
{
    // 라인에디터의 값들을 문자열로 저장
    QString name(ui->nameLineEdit->text());
    QString price(ui->priceLineEdit->text());
    QString stock(ui->stockLineEdit->text());
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
        // ID를 생성하고 저장한 라인에디터 값으로 테이블위젯아이템을 생성
        QTableWidgetItem* idItem =
                new QTableWidgetItem(QString::number(makeId()));
        QTableWidgetItem* nameItem = new QTableWidgetItem(name);
        QTableWidgetItem* priceItem = new QTableWidgetItem(price);
        QTableWidgetItem* stockItem = new QTableWidgetItem(stock);
        // 테이블위젯의 행의 수를 읽어온 후 1증가 시켜서 저장
        int tableRowCount = ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount( tableRowCount + 1);
        // 생성한 아이템들을 추가된 행에 저장
        ui->tableWidget->setItem(tableRowCount, 0, idItem);
        ui->tableWidget->setItem(tableRowCount, 1, nameItem);
        ui->tableWidget->setItem(tableRowCount, 2, priceItem);
        ui->tableWidget->setItem(tableRowCount, 3, stockItem);
    }
}

/* ID를 통해 테이블위젯을 검색하고 행을 활성화하는 슬롯 함수 */
void ProductForm::selectReturnPressedId()
{
    QString searchingText(ui->idLineEdit->text());  // ID를 문자열로 저장
    QList<QTableWidgetItem*> searchingResult;   // 검색 결과를 저장하기 위한 리스트 선언
    searchingResult = ui->tableWidget->findItems(searchingText,
                                                 Qt::MatchFixedString);
    // 정확히 일치하는 결과를 검색
    if(searchingResult.isEmpty())     return; // 검색 결과가 없으면 함수 종료

    int searchingRow = searchingResult.first()->row();
    // ID는 중복되지 않기 때문에 하나의 검색 결과만 존재
    // 가장 첫 번째 결과가 전체 결과이므로 이를 결과를 가져옴
    int searchingColumn = searchingResult.first()->column();
    // ID 라인에디터 열의 값인 0과 검색 결과의 열이 일치할 경우 조건문 수행
    if ( 0 == searchingColumn ){
        ui->tableWidget->selectRow(searchingRow);   // 검색 결과의 행을 활성화
        displayLineEdit(searchingRow, 0);   // 활성화된 행을 라인에디터에 출력
    }
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
        lineEditColumn = 1;
    } else if( lineEdit->objectName() == "priceLineEdit" ){
        lineEditColumn = 2;
    } else if( lineEdit->objectName() == "stockLineEdit" ){
        lineEditColumn = 3;
    } else {
        return;     // 조건에 해당하지 않을 경우 함수를 종료
    }

    QString searchingText(lineEdit->text());    // 검색할 내용을 라인에디터 값으로부터 가져옴
    // 이전 검색 리스트가 비어있거나 현재 검색 내용과 일치하지 않다면 다시 리스트를 생성
    if ( searchingList.isEmpty() || searchingList.first()->text().compare(
             searchingText, Qt::CaseInsensitive)) {
        // 새롭게 검색 결과 리스트를 갱신한 후 저장
        searchingList = ui->tableWidget->findItems(searchingText,
                                                   Qt::MatchFixedString);
        if(searchingList.isEmpty())     return;     // 갱신한 검색 결과가 없을 경우 함수 종료
    }
    // 검색 결과 리스트의 가장 위의 값을 가져와서 사용, 가져온 값은 리스트에서 제거됨
    QTableWidgetItem* searchingResult(searchingList.takeFirst());
    // 리스트에서 가져온 값의 행이 다르다면 다음 값을 가져옴
    while ( searchingResult->column() != lineEditColumn ) {
        if(searchingList.isEmpty())     return;
        // 다음 검색 결과를 가져오기 전에 리스트가 비어 있으면 함수 종료
        searchingResult = searchingList.takeFirst();    // 다음 검색 결과를 저장
    }

    int searchingRow = searchingResult->row();  // 행을 활성화하기 위한 행의 값
    // 라인에디터와 맞는 열을 가져왔는지 확인하기 위한 열의 값
    int searchingCoulmn = searchingResult->column();
    // 라인에디터의 열의 값과 검색 결과의 열의 값이 일치할 경우 조건문 수행
    if ( searchingCoulmn == lineEditColumn )
    {
        ui->tableWidget->selectRow(searchingRow);   // 검색 결과의 행을 활성화
        displayLineEdit(searchingRow, 0);       // 검색 결과의 값을 라인에디터에 저장
    }
}

/* 테이블위젯 행의 값을 변경하는 슬롯 함수 */
void ProductForm::modifyTableRow()
{
    int tableCurrentRow = ui->tableWidget->currentRow();
    // 현재 선택된 행의 값을 변수로 선언
    // 수정할 값을 라인에디터로부터 받아온 후 테이블위젯아이템으로 생성
    QTableWidgetItem* productName =
            new QTableWidgetItem(ui->nameLineEdit->text());
    QTableWidgetItem* productPrice =
            new QTableWidgetItem(ui->priceLineEdit->text());
    QTableWidgetItem* productStock =
            new QTableWidgetItem(ui->stockLineEdit->text());
    // 활성화된 행에 각 아이템들을 저장함으로써 데이터를 변경
    ui->tableWidget->setItem(tableCurrentRow, 1, productName);
    ui->tableWidget->setItem(tableCurrentRow, 2, productPrice);
    ui->tableWidget->setItem(tableCurrentRow, 3, productStock);
}

/* 선택된 테이블 위젯의 행을 삭제하는 슬롯 함수 */
void ProductForm::removeTableRow()
{
    if(QMessageBox::warning(this, tr("Product Manager"),
                            tr("Are you sure you want to delete it?"),
                             QMessageBox::Yes | QMessageBox::No)
            == QMessageBox::Yes)
        // Yes를 선택할 경우 행을 삭제
        ui->tableWidget->removeRow(ui->tableWidget->currentRow());
    clearLineEdit();
}

/* 테이블위젯에서 열로 데이터를 구분하고 검색하는 슬롯 함수 */
void ProductForm::searching(int type, QString content)
{
    QList<QTableWidgetItem*> searchingResult;   // 검색 결과를 저장하기 위한 리스트 생성
    // 시그널을 통해 받은 문자열로 테이블위젯을 검색하고 결과를 리스트에 저장
    searchingResult = ui->tableWidget->findItems(content,
                                                 Qt::MatchFixedString);

    if(searchingResult.empty())     return; // 검색 결과가 없을 경우 함수를 종료

    QList<QString> returnResult;    // 열에 맞는 검색결과를 반환하기 위한 리스트 생성
    foreach( QTableWidgetItem* item, searchingResult ){
        // 검색 결과들을 열을 확인하고 리스트에 저장
        if( item->column() == type ){
            int searchingRow = item->row(); // 검색 결과의 행을 변수로 선언
            // 검색 결과를 리스트에 순서대로 저장
            returnResult << ui->tableWidget->item(searchingRow, 0)->text()
                         << ui->tableWidget->item(searchingRow, 1)->text()
                         << ui->tableWidget->item(searchingRow, 2)->text()
                         << ui->tableWidget->item(searchingRow, 3)->text();
        }
    }
    emit returnSearching(returnResult);
    // 검색 결과를 저장한 리스트를 시그널을 통해 orderForm으로 전달
}
