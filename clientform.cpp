#include "clientform.h"
#include "ui_clientform.h"

#include <QFile>
#include <QMessageBox>
#include <QTableView>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

/* 생성자에서 ui에 대한 슬롯을 연결하고 저장한 csv데이터를 불러옴 */
ClientForm::ClientForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientForm)
{
    ui->setupUi(this);

    connect(ui->tableView, SIGNAL(clicked(QModelIndex)),this,
            SLOT(displayLineEdit(QModelIndex)));    // 클릭한 셀의 행 데이터를 라인에디터에 보여주는 기능
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

    clientQueryModel = new QSqlQueryModel;
    clientQueryModel->setQuery("SELECT * FROM CLIENT");
    clientQueryModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
    clientQueryModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
    clientQueryModel->setHeaderData(2, Qt::Horizontal, tr("Phone Number"));
    clientQueryModel->setHeaderData(3, Qt::Horizontal, tr("Address"));

    ui->tableView->setModel(clientQueryModel);

}

/* 기록한 데이터를 소멸자에서 저장 */
ClientForm::~ClientForm()
{
    delete ui;  // 소멸자이므로 사용한 ui 객체를 메모리 해제
}

/* 행의 값을 통해 선택된 행의 데이터를 라인에디터에 표시하는 슬롯함수 */
void ClientForm::displayLineEdit(const QModelIndex &index)
{
    // 시그널에서 받아온 행의 값으로 테이블위젯의 값을 가져온 후 라인에디터에 저장
    QString id = clientQueryModel->data(clientQueryModel->index(index.row(), 0)).toString();
    QString name = clientQueryModel->data(clientQueryModel->index(index.row(), 1)).toString();
    QString phoneNum = clientQueryModel->data(clientQueryModel->index(index.row(), 2)).toString();
    QString address = clientQueryModel->data(clientQueryModel->index(index.row(), 3)).toString();

    ui->idLineEdit->setText(id);
    ui->nameLineEdit->setText(name);
    ui->phoneNumLineEdit->setText(phoneNum);
    ui->addressLineEdit->setText(address);

}

/* 클리어 버튼이 눌렸을 때 모든 라인에디터를 클리어하는 슬롯함수 */
void ClientForm::clearLineEdit()
{
    // 화면의 모든 라인에디터를 클리어
    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->phoneNumLineEdit->clear();
    ui->addressLineEdit->clear();

    clientQueryModel->setQuery("SELECT * FROM CLIENT");
    ui->tableView->setModel(clientQueryModel);
}

/* 라인에디터의 값들을 테이블위젯의 마지막 행에 추가하는 슬롯 함수 */
void ClientForm::addTableRow()
{
    // 라인 에디터의 값들을 문자열 변수에 저장
    QString name = ui->nameLineEdit->text();
    QString phoneNum = ui->phoneNumLineEdit->text();
    QString address = ui->addressLineEdit->text();
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
        clientQueryModel->setQuery(QString("CALL INSERT_CLIENT('%1', '%2', '%3')")
                                   .arg(name).arg(phoneNum).arg(address));
        clientQueryModel->setQuery("SELECT * FROM CLIENT");
        ui->tableView->setModel(clientQueryModel);

    }
}

/* ID를 통해 검색하고 행을 활성화하는 슬롯 함수 */
void ClientForm::selectReturnPressedId()
{
    QString id = ui->idLineEdit->text();
    clientQueryModel->setQuery(QString("SELECT * FROM CLIENT WHERE CLIENT_ID = %1").arg(id));
    QString name = clientQueryModel->data(clientQueryModel->index(0, 1)).toString();
    ui->nameLineEdit->setText(name);
    ui->tableView->setModel(clientQueryModel);

}

/* 라인에디터에서 returnPressed 시그널을 발생시켰을 때 검색기능을 수행하기 위한 함수 */
void ClientForm::selectReturnPressedLineEdit()
{
    // 신호를 발생시킨 라인에디터를 변수에 저장
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(sender());
    if(lineEdit == nullptr)     return;     // 라인에디터가 존재하지 않을 경우 함수 종료

    // 라인에디터의 값이 저장되는 행을 객체 이름을 통해 조건문으로 처리하여 번호를 저장
    if( lineEdit->objectName() == "nameLineEdit" ){
        QString name = ui->nameLineEdit->text();
        clientQueryModel->setQuery(QString("SELECT * FROM CLIENT WHERE CLIENT_NAME = '%1'").arg(name));
    } else if( lineEdit->objectName() == "phoneNumLineEdit" ){
        QString phoneNum = ui->phoneNumLineEdit->text();
        clientQueryModel->setQuery(QString("SELECT * FROM CLIENT WHERE PHONE_NUMBER = '%1'").arg(phoneNum));
    } else if( lineEdit->objectName() == "addressLineEdit" ){
        QString address = ui->addressLineEdit->text();
        clientQueryModel->setQuery(QString("SELECT * FROM CLIENT WHERE ADDRESS = '%1'").arg(address));
    } else {
        return;     // 해당하지 않을 경우 함수를 종료
    }
    ui->tableView->setModel(clientQueryModel);

}

/* 테이블위젯의 값을 변경하는 슬롯함수 */
void ClientForm::modifyTableRow()
{
    // 라인 에디터의 값들을 문자열 변수에 저장
    QString id = ui->idLineEdit->text();
    QString name(ui->nameLineEdit->text());
    QString phoneNum(ui->phoneNumLineEdit->text());
    QString address(ui->addressLineEdit->text());
    // 값을 저장한 라인에디터들을 초기화
    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->phoneNumLineEdit->clear();
    ui->addressLineEdit->clear();

    clientQueryModel->setQuery(QString("CALL UPDATE_CLIENT(%1, '%2', '%3', '%4')")
                               .arg(id).arg(name).arg(phoneNum).arg(address));
    clientQueryModel->setQuery("SELECT * FROM CLIENT");
    ui->tableView->setModel(clientQueryModel);

}

/* 테이블위젯의 선택된 행을 삭제하는 슬롯함수 */
void ClientForm::removeTableRow()
{
    // 삭제에 관한 내용을 다시 한번 확인하기 위해 메시지박스를 출력
    if(QMessageBox::warning(this, tr("Client Manager"),
                            tr("Are you sure you want to delete it?"),
                            QMessageBox::Yes | QMessageBox::No)
            == QMessageBox::Yes){   // Yes를 클릭할 경우 행을 삭제
        QString id = ui->idLineEdit->text();
        clientQueryModel->setQuery(QString("CALL DELETE_CLIENT('%1')").arg(id));
    }
    clientQueryModel->setQuery("SELECT * FROM CLIENT");
    ui->tableView->setModel(clientQueryModel);
    clearLineEdit();

}


