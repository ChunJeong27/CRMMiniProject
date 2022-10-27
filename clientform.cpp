#include "clientform.h"
#include "ui_clientform.h"

#include <QFile>
#include <QMessageBox>

/* 생성자에서 ui에 대한 슬롯을 연결하고 저장한 csv데이터를 불러옴 */
ClientForm::ClientForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientForm)
{
    ui->setupUi(this);

    connect(ui->tableWidget, SIGNAL(cellClicked(int,int)),this,
            SLOT(displayLineEdit(int,int)));    // 클릭한 셀의 행 데이터를 라인에디터에 보여주는 기능
    connect(ui->clearPushButton, SIGNAL(clicked()),
            this, SLOT(clearLineEdit()));   // 모든 라인에디터를 초기하는 기능
    connect(ui->addPushButton, SIGNAL(clicked()), this, SLOT(addTableRow()));
    // 라인에디터의 데이터를 테이블위젯에 추가하는 기능
    connect(ui->searchPushButton, SIGNAL(clicked()),
            this, SLOT(selectReturnPressedId()));
    // 버튼 시그널에 대해 id를 검색하는 기능
    connect(ui->modifyPushButton, SIGNAL(clicked()),
            this, SLOT(modifyTableRow()));
    // 선택된 테이블 행을 변경하는 기능
    connect(ui->removePushButton, SIGNAL(clicked()),
            this, SLOT(removeTableRow()));
    connect(ui->idLineEdit, SIGNAL(returnPressed()), this,
            SLOT(selectReturnPressedId()));    // 아이디에 맞는 줄을 활성화시키는 기능
    // 라인에디터에 따라 맞는 열을 검색하고 활성화시키는 기능
    connect(ui->nameLineEdit, SIGNAL(returnPressed()),
            this, SLOT(selectReturnPressedLineEdit()));
    connect(ui->phoneNumLineEdit, SIGNAL(returnPressed()),
            this, SLOT(selectReturnPressedLineEdit()));
    connect(ui->addressLineEdit, SIGNAL(returnPressed()),
            this, SLOT(selectReturnPressedLineEdit()));

    loadData();     // csv 형식으로 저장한 파일을 불러오는 함수
}

/* 기록한 데이터를 소멸자에서 저장 */
ClientForm::~ClientForm()
{
    QFile file("clientForm.txt");   // 저장할 파일명으로 QFile 객체를 생성

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;     // 파일이 정상적으로 열리지 않으면 함수를 종료

    QTextStream out(&file); // 파일을 텍스트스트림 형태로 기록하기 위한 객체 생성
    int tableRowCount = ui->tableWidget->rowCount();
    // 반복횟수를 위해 변수에 행의 개수를 저장
    for (int i = 0; i < tableRowCount; i++) {
        // 스트림 연산자를 통해 쉼표로 데이터를 구분해서 저장, 마지막엔 줄바꿈 문자 삽입
        out << ui->tableWidget->item(i, 0)->text() << ","
            << ui->tableWidget->item(i, 1)->text() << ","
            << ui->tableWidget->item(i, 2)->text() << ","
            << ui->tableWidget->item(i, 3)->text() << "\n";
    }
    file.close();   // 기록을 마친 파일 닫기

    delete ui;  // 소멸자이므로 사용한 ui 객체를 메모리 해제
}

/* 기록된 데이터를 불러오고 테이블위젯에 추가하는 함수 */
void ClientForm::loadData()
{
    QFile file("clientForm.txt");   // 저장된 파일을 읽기위해 열기
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;     // 파일을 정상적으로 열지 못했을 경우 함수 종료

    QTextStream in(&file);  // 텍스트스트림으로 파일을 다루기 위해 열기
    while (!in.atEnd()) {   // 파일을 끝을 만날 때까지 반복
        QString line = in.readLine();   // 파일의 한 줄을 읽어옴
        QStringList row = line.split(",");  // csv파일을 쉼표로 구분해서 QString으로 저장
        if (!row.empty()) {     // 파싱한 QList가 비어있지 않다면
            // 각 문자열을 저장한 QTableWidgetItem을 생성
            QTableWidgetItem* clientID = new QTableWidgetItem(row.at(0));
            QTableWidgetItem* clientName = new QTableWidgetItem(row.at(1));
            QTableWidgetItem* clientPhoneNum = new QTableWidgetItem(row.at(2));
            QTableWidgetItem* clientAddress = new QTableWidgetItem(row.at(3));
            // 현재 테이블위젯을 행의 개수를 한 줄 증가시켜서 설정
            int tableRowCount = ui->tableWidget->rowCount();
            ui->tableWidget->setRowCount( tableRowCount + 1);
            // 추가한 행에 생성한 아이템들을 순서대로 저장
            ui->tableWidget->setItem(tableRowCount, 0, clientID);
            ui->tableWidget->setItem(tableRowCount, 1, clientName);
            ui->tableWidget->setItem(tableRowCount, 2, clientPhoneNum);
            ui->tableWidget->setItem(tableRowCount, 3, clientAddress);
        }
    }
    file.close();   // 데이터를 불러오는 작업을 마친 파일을 닫음

}

/* 고객 정보를 추가할 때 중복되지 않도록 ID를 생성하는 함수 */
int ClientForm::makeId()
{
    int lastColumn = ui->tableWidget->rowCount() - 1;   // 마지막 행의 값을 가져옴

    if( lastColumn < 0 )   // 데이터가 없을 경우 1000번부터 ID를 부여
        return 1000;

    int creatId = ui->tableWidget->item(lastColumn, 0)->text().toInt() + 1;
    // 마지막 행의 0번째 열인 ID값을 가져와서 +1을 추가한 값을 부여
    return creatId;     // 생성한 ID값을 리턴
}

/* 행의 값을 통해 선택된 행의 데이터를 라인에디터에 표시하는 슬롯함수 */
void ClientForm::displayLineEdit(int row,int column)
{
    Q_UNUSED(column);   // 사용하지 않을 column값을 명시적으로 나타냄
    // 시그널에서 받아온 행의 값으로 테이블위젯의 값을 가져온 후 라인에디터에 저장
    ui->idLineEdit->setText(ui->tableWidget->item(row, 0)->text());
    ui->nameLineEdit->setText(ui->tableWidget->item(row, 1)->text());
    ui->phoneNumLineEdit->setText(ui->tableWidget->item(row, 2)->text());
    ui->addressLineEdit->setText(ui->tableWidget->item(row, 3)->text());
}

/* 클리어 버튼이 눌렸을 때 모든 라인에디터를 클리어하는 슬롯함수 */
void ClientForm::clearLineEdit()
{
    // 화면의 모든 라인에디터를 클리어
    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->phoneNumLineEdit->clear();
    ui->addressLineEdit->clear();
}

/* 라인에디터의 값들을 테이블위젯의 마지막 행에 추가하는 슬롯 함수 */
void ClientForm::addTableRow()
{
    // 라인 에디터의 값들을 문자열 변수에 저장
    QString name(ui->nameLineEdit->text());
    QString phoneNum(ui->phoneNumLineEdit->text());
    QString address(ui->addressLineEdit->text());
    // 값을 저장한 라인에디터들을 초기화
    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->phoneNumLineEdit->clear();
    ui->addressLineEdit->clear();
    // 테이블위젯에 빈 셀을 추가하지 않기 위해 문자열의 길이로 조건문 처리
    // 폰 번호는 이미 -가 2개 형식으로 표시되기 때문에 2개이면 아무것도 입력되지 않은 상태
    if ( name.length() <= 0 || phoneNum.length() <= 2
         || address.length() <= 0 ) {
        // 비어있는 라인에디터가 있을 경우 경고 메시지 출력
        QMessageBox::warning(this, tr("Client Manager"),
                             tr("Check your input again."), QMessageBox::Ok);
        return;     // 경고메시지를 출력 후 함수 종료
    }
    else {      // 모든 라인에디터에 정상적으로 값이 있을 경우
        // 생성한 ID값과 라인에디터 값들로 테이블위젯아이템을 생성
        QTableWidgetItem* idItem =
                new QTableWidgetItem(QString::number(makeId()));
        QTableWidgetItem* nameItem = new QTableWidgetItem(name);
        QTableWidgetItem* phoneNumItem = new QTableWidgetItem(phoneNum);
        QTableWidgetItem* addressItem = new QTableWidgetItem(address);
        // 테이블위젯에 아이템을 추가하기 위해 행의 수를 1증가
        int tableRowCount = ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount( tableRowCount + 1);
        // 행을 증가시킨 테이블위젯에 각 아이템들을 저장
        ui->tableWidget->setItem(tableRowCount, 0, idItem);
        ui->tableWidget->setItem(tableRowCount, 1, nameItem);
        ui->tableWidget->setItem(tableRowCount, 2, phoneNumItem);
        ui->tableWidget->setItem(tableRowCount, 3, addressItem);
    }
}

/* ID를 통해 검색하고 행을 활성화하는 슬롯 함수 */
void ClientForm::selectReturnPressedId()
{
    QString searchingText(ui->idLineEdit->text());  // ID값을 문자열로 저장
    QList<QTableWidgetItem*> searchingResult;       // 검색 결과를 리스트로 저장하기 위한 변수
    // 정확히 일치하는 검색 결과를 저장
    searchingResult = ui->tableWidget->findItems(searchingText,
                                                 Qt::MatchFixedString);
    if(searchingResult.isEmpty())     return;     // 검색 결과가 없을 경우 함수 종료

    int searchingRow = searchingResult.first()->row();  // 행을 활성화하기 위한 행의 값
    // 라인에디터와 맞는 값을 가져왔는지 확인하기 위한 열의 값
    int searchingColumn = searchingResult.first()->column();
    // ID 라인에디터의 열의 값인 0과 검색 결과의 열이 일치할 경우 조건문 수행
    if ( 0 == searchingColumn ){
        // ID이므로 검색 결과가 하나이기 때문에 가장 첫 번째 값의 행 값을 사용
        ui->tableWidget->selectRow(searchingRow);   // 검색된 값의 행을 활성화
        displayLineEdit(searchingRow, 0);   // 검색된 값을 라인에디터에 출력
    }
}

/* 라인에디터에서 returnPressed 시그널을 발생시켰을 때 검색기능을 수행하기 위한 함수 */
void ClientForm::selectReturnPressedLineEdit()
{
    // 신호를 발생시킨 라인에디터를 변수에 저장
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(sender());
    if(lineEdit == nullptr)     return;     // 라인에디터가 존재하지 않을 경우 함수 종료

    int lineEditColumn(0);  // 라인에디터의 값과 대응되는 행을 저장하기 위한 변수
    // 라인에디터의 값이 저장되는 행을 객체 이름을 통해 조건문으로 처리하여 번호를 저장
    if( lineEdit->objectName() == "nameLineEdit" ){
        lineEditColumn = 1;
    } else if( lineEdit->objectName() == "phoneNumLineEdit" ){
        lineEditColumn = 2;
    } else if( lineEdit->objectName() == "addressLineEdit" ){
        lineEditColumn = 3;
    } else {
        return;     // 해당하지 않을 경우 함수를 종료
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
        if(searchingList.isEmpty())     return;     // 검색 결과 리스트가 비어 있으면 함수 종료
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

/* 테이블위젯의 값을 변경하는 슬롯함수 */
void ClientForm::modifyTableRow()
{
    // 현재 선택된 행의 값을 변수에 저장
    int tableCurrentRow = ui->tableWidget->currentRow();
    // 수정할 값을 라인에디터로부터 받아와서 각 아이템으로 생성
    QTableWidgetItem* clientNameItem =
            new QTableWidgetItem(ui->nameLineEdit->text());
    QTableWidgetItem* clientPhoneNumItem =
            new QTableWidgetItem(ui->phoneNumLineEdit->text());
    QTableWidgetItem* clientAddressItem =
            new QTableWidgetItem(ui->addressLineEdit->text());
    // 활성화된 행에 각 아이템들을 저장
    ui->tableWidget->setItem(tableCurrentRow, 1, clientNameItem);
    ui->tableWidget->setItem(tableCurrentRow, 2, clientPhoneNumItem);
    ui->tableWidget->setItem(tableCurrentRow, 3, clientAddressItem);
}

/* 테이블위젯의 선택된 행을 삭제하는 슬롯함수 */
void ClientForm::removeTableRow()
{
    // 삭제에 관한 내용을 다시 한번 확인하기 위해 메시지박스를 출력
    if(QMessageBox::warning(this, tr("Client Manager"),
                            tr("Are you sure you want to delete it?"),
                            QMessageBox::Yes | QMessageBox::No)
            == QMessageBox::Yes)
        // Yes를 클릭할 경우 행을 삭제
        ui->tableWidget->removeRow(ui->tableWidget->currentRow());
    clearLineEdit();
}


/* 열에 따라 테이블위젯에서 데이터를 검색하는 슬롯 함수*/
void ClientForm::searching(int coulumn, QString content)
{
    QList<QTableWidgetItem*> searchingResult;   // 검색결과를 저장하기 위한 리스트 생성
    // 시그널을 통해 받아온 문자열로 테이블위젯을 검색하고 결과를 리스트에 저장
    searchingResult = ui->tableWidget->findItems(content,
                                                 Qt::MatchFixedString);

    if(searchingResult.empty())     return; // 검색결과가 없을 경우 함수를 종료

    QList<QString> returnResult;    // 열에 맞는 검색결과를 반환하기 위한 변수 생성
    // 각 검색결과의 열을 확인하여 리스트에 저장
    foreach( QTableWidgetItem* item, searchingResult ){
        if( item->column() == coulumn ){   // 검색결과의 행과 데이터의 행이 일치할 경우
            int searchingRow = item->row();     // 결과리스트에 저장할 테이블위젯의 행을 저장
            // 행을 통해 결과리스트에 반환할 테이블위젯 데이터를 저장
            returnResult << ui->tableWidget->item(searchingRow, 0)->text()
                         << ui->tableWidget->item(searchingRow, 1)->text()
                         << ui->tableWidget->item(searchingRow, 2)->text()
                         << ui->tableWidget->item(searchingRow, 3)->text();
        }
    }
    emit returnSearching(returnResult); // 결과리스트를 시그널을 통해 orderForm으로 전달
}

/* 시그널로 받아온 이름과 ID를 client tableWidget에 존재하는지 확인하는 슬롯 함수 */
void ClientForm::searchIdName(QString name, QString id){
    QList<QTableWidgetItem*> searchingResult;   // 검색결과를 저장할 리스트 선언
    // 테이블위젯에서 ID를 검색하고 리스트에 저장
    searchingResult = ui->tableWidget->findItems(id, Qt::MatchFixedString);

    if(searchingResult.empty())     return; // 검색결과가 존재하지 않으면 함수를 종료
    // 검색결과의 행의 이름이 매개변수로 받아온 이름과 일치할 경우 Ture를 시그널로 방출
    if(ui->tableWidget->item(searchingResult.first()->row(), 1)->text() == name){
        emit checkedIdName(true);
    } else {
        emit checkedIdName(false);  // 일치하지 않을 경우 False를 방출
    }
}
