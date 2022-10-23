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
            SLOT(displayItem(int,int)));    // 클릭한 셀을 라인에디터에 보여주는 기능
    connect(ui->idLineEdit, SIGNAL(returnPressed()), this,
            SLOT(returnPressedSearching()));    // 아이디에 맞는 줄을 활성화시키는 기능
    // 라인에디터에 따라 맞는 열을 검색하고 활성화시키는 기능
    connect(ui->nameLineEdit, SIGNAL(returnPressed()),
            this, SLOT(on_searchPushButton_clicked()));
    connect(ui->phoneNumLineEdit, SIGNAL(returnPressed()),
            this, SLOT(on_searchPushButton_clicked()));
    connect(ui->addressLineEdit, SIGNAL(returnPressed()),
            this, SLOT(on_searchPushButton_clicked()));

    loadData();     // csv 형식으로 저장한 파일을 불러오는 함수
}

/* 기록한 데이터를 소멸자에서 저장 */
ClientForm::~ClientForm()
{
    QFile file("clientForm.txt");   // 저장할 파일명으로 QFile 객체를 생성

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;     // 파일이 정상적으로 열리지 않으면 함수를 종료

    QTextStream out(&file);     // 텍스트스트림형태로 기록하기 위해 파일을 오픈
    int tableRowCount = ui->tableWidget->rowCount();
    // 반복횟수를 위해 변수에 행의 개수를 저장
    for (int i = 0; i < tableRowCount; i++) {
        // 연산자를 통해 쉼표로 데이터를 구분해서 저장, 마지막엔 줄바꿈 문자 삽입
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

    if( lastColumn <= 0 )   // 데이터가 없을 경우 1000번부터 ID를 부여
        return 1000;

    int creatId = ui->tableWidget->item(lastColumn, 0)->text().toInt() + 1;
    // 마지막 행의 0번째 열인 ID값을 가져와서 +1을 추가한 값을 부여
    return creatId;     // 생성한 ID값을 리턴
}

/* 행의 값을 통해 선택된 행의 데이터를 라인에디터에 표시하는 슬롯함수 */
void ClientForm::displayItem(int row,int column)
{
    Q_UNUSED(column);   // 사용하지 않을 column값을 명시적으로 나타냄
    // 시그널에서 받아온 행의 값으로 테이블위젯의 값을 가져온 후 라인에디터에 저장
    ui->idLineEdit->setText(ui->tableWidget->item(row, 0)->text());
    ui->nameLineEdit->setText(ui->tableWidget->item(row, 1)->text());
    ui->phoneNumLineEdit->setText(ui->tableWidget->item(row, 2)->text());
    ui->addressLineEdit->setText(ui->tableWidget->item(row, 3)->text());
}

/* 클리어 버튼이 눌렸을 때 모든 라인에디터를 클리어하는 슬롯함수 */
void ClientForm::on_clearPushButton_clicked()
{
    // 화면의 모든 라인에디터를 클리어
    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->phoneNumLineEdit->clear();
    ui->addressLineEdit->clear();
}

/* 라인에디터의 값들을 테이블위젯의 마지막 행에 추가하는 함수 */
void ClientForm::on_addPushButton_clicked()
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
    // 테이블위젯에 공백을 추가하지 않기 위해 문자열의 길이를 조건문 처리
    // 폰 번호는 이미 -가 2개 형식으로 표시되기 때문에 2개이면 아무것도 입력되지 않은 상태
    if ( name.length() <= 0 || phoneNum.length() <= 2
         || address.length() <= 0 ) {
        // 비어있는 라인에디터가 있을 경우 경고 메시지 출력
        QMessageBox::warning(this, "Client Manager",
                             "Check your input again.", QMessageBox::Ok);
        return;     // 경고메시지를 출력 후 함수 종료
    }
    else {      // 모든 라인에디터에 정상적으로 값이 있을 경우
        // 생성한 ID값과 라인에디터 값들로 테이블위젯아이템을 생성
        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(makeId()));
        QTableWidgetItem* nameItem = new QTableWidgetItem(name);
        QTableWidgetItem* phoneNumItem = new QTableWidgetItem(phoneNum);
        QTableWidgetItem* addressItem = new QTableWidgetItem(address);
        // 테이블위젯에 아이템을 추가하기 위해 행의 수를 1증가
        int tableRowCount = ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount( tableRowCount + 1);
        // 행을 증가시킨 테이블위젯에 각 아이템들을 저
        ui->tableWidget->setItem(tableRowCount, 0, idItem);
        ui->tableWidget->setItem(tableRowCount, 1, nameItem);
        ui->tableWidget->setItem(tableRowCount, 2, phoneNumItem);
        ui->tableWidget->setItem(tableRowCount, 3, addressItem);
    }
}

void ClientForm::on_searchPushButton_clicked()
{
    QString searchingText = ui->idLineEdit->text();
    QList<QTableWidgetItem*> searchingResult;
    searchingResult = ui->tableWidget->findItems(searchingText, Qt::MatchFixedString);

    if(searchingResult.empty())     return;

    int searchingRow = searchingResult.first()->row();
    ui->tableWidget->selectRow(searchingRow);
    displayItem(searchingRow, 0);
}

void ClientForm::on_modifyPushButton_clicked()
{
    int tableCurrentRow = ui->tableWidget->currentRow();

    QTableWidgetItem* clientName = new QTableWidgetItem(ui->nameLineEdit->text());
    QTableWidgetItem* clientPhoneNum = new QTableWidgetItem(ui->phoneNumLineEdit->text());
    QTableWidgetItem* clientAddress = new QTableWidgetItem(ui->addressLineEdit->text());

    ui->tableWidget->setItem(tableCurrentRow, 1, clientName);
    ui->tableWidget->setItem(tableCurrentRow, 2, clientPhoneNum);
    ui->tableWidget->setItem(tableCurrentRow, 3, clientAddress);
}

void ClientForm::on_removePushButton_clicked()
{
    if(QMessageBox::warning(this, "Client Manager", "Are you sure you want to delete it?",
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        ui->tableWidget->removeRow(ui->tableWidget->currentRow());
}

void ClientForm::returnPressedSearching()
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(sender());
    if(lineEdit == nullptr)     return;

    int lineEditType(0);
    if( lineEdit->objectName() == "nameLineEdit" ){
        lineEditType = 1;
    } else if( lineEdit->objectName() == "phoneNumLineEdit" ){
        lineEditType = 2;
    } else if( lineEdit->objectName() == "addressLineEdit" ){
        lineEditType = 3;
    } else {
        return;
    }

    QString searchingText = lineEdit->text();
    if ( searchingList.empty()
         || searchingList.first()->text().compare(searchingText, Qt::CaseInsensitive)) {
        searchingList = ui->tableWidget->findItems(searchingText, Qt::MatchFixedString);
        if(searchingList.empty())     return;
    }

    QTableWidgetItem* searchingResult = searchingList.takeFirst();
    while ( searchingResult->column() != lineEditType ) {
        if(searchingList.empty())     return;
        searchingResult = searchingList.takeFirst();
    }

    int searchingRow = searchingResult->row();
    int searchingCoulmn = searchingResult->column();
    if ( searchingCoulmn == lineEditType )
    {
        ui->tableWidget->selectRow(searchingRow);
        displayItem(searchingRow, 0);
    }
}

void ClientForm::searching(int type, QString content)
{
    QList<QTableWidgetItem*> searchingResult;
    searchingResult = ui->tableWidget->findItems(content, Qt::MatchFixedString);

    if(searchingResult.empty())     return;

    QList<QString> returnResult;
    foreach( QTableWidgetItem* item, searchingResult ){
        if( item->column() == type ){
            int searchingRow = item->row();
            returnResult << ui->tableWidget->item(searchingRow, 0)->text()
                         << ui->tableWidget->item(searchingRow, 1)->text()
                         << ui->tableWidget->item(searchingRow, 2)->text()
                         << ui->tableWidget->item(searchingRow, 3)->text();
        }
    }

    emit returnSearching(returnResult);
}

void ClientForm::checkIdName(QString name, QString id){
    QList<QTableWidgetItem*> searchingResult;
    searchingResult = ui->tableWidget->findItems(id, Qt::MatchFixedString);

    if(searchingResult.empty())     return;

    if(ui->tableWidget->item(searchingResult.first()->row(), 1)->text() == name){
        emit checkedIdName(true);
    } else {
        emit checkedIdName(false);
    }

}
