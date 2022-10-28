#include "chatroomform.h"
#include "ui_chatroomform.h"
#include <QTcpSocket>
#include <QFile>
#include <QProgressDialog>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>

ChatRoomForm::ChatRoomForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatRoomForm)
{
    ui->setupUi(this);

    ui->ipLineEdit->setText("127.0.0.1");
    ui->portLineEdit->setText("19000");

    ui->messageLineEdit->setDisabled(true);
    ui->sentPushButton->setDisabled(true);
    ui->uploadPushButton->setDisabled(true);

    connect(ui->statusPushButton, SIGNAL(clicked()), this, SLOT(connectPushButton()));
    connect(ui->sentPushButton, SIGNAL(clicked()), SLOT(sendData()));
    connect(ui->messageLineEdit, SIGNAL(returnPressed()), this, SLOT(sendData()));

    chatSocket = new QTcpSocket(this);
    connect(chatSocket, &QAbstractSocket::errorOccurred, this,
            [=]{qDebug() << chatSocket->errorString(); });
    connect(chatSocket, SIGNAL(readyRead()), this, SLOT(receiveData()));
    connect(chatSocket, SIGNAL(disconnected()), this, SLOT(disconnectServer()));

    connect(ui->uploadPushButton, SIGNAL(clicked()), this, SLOT(sendFile()));

    uploadSocket = new QTcpSocket(this);
    connect(uploadSocket, SIGNAL(bytesWritten(qint64)), SLOT(goOnSend(qint64)));

    uploadProgressDialog = new QProgressDialog(0);
    uploadProgressDialog->setAutoClose(true);
    uploadProgressDialog->reset();


//    uploadFile = new UploadProtocol(false, nullptr, ui->ipLineEdit->text(), ui->portLineEdit->text().toUInt(), this);
//    connect(ui->uploadPushButton, SIGNAL(clicked()), uploadFile, SLOT(sendFile()));

    connect(ui->fileListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SIGNAL(clickedFileList(QListWidgetItem*)));

    downloadTotalSize = 0;
    downloadByteReceived = 0;

    downloadFileClient = new QTcpSocket(this);

    downloadFileClient->connectToHost(ui->ipLineEdit->text( ),
                                      ui->portLineEdit->text( ).toInt( ) + 200);
    connect(downloadFileClient, SIGNAL(readyRead()), this, SLOT(downloadFile()));

    downloadProgressDialog = new QProgressDialog(0);
    downloadProgressDialog->setAutoClose(true);
    downloadProgressDialog->reset();

    setWindowTitle(tr("Client Chatting Application"));

}

ChatRoomForm::~ChatRoomForm()
{
    chatSocket->close();
    uploadSocket->close();
    downloadFileClient->close();

    delete ui;
}

void ChatRoomForm::closeEvent(QCloseEvent*)
{
//    sendProtocol(Chat_LogOut, name->text().toStdString().data());
    chatSocket->write(Chat::Disconnect + (ui->nameLineEdit->text() + "(ID:"
                                         + ui->idLineEdit->text()).toUtf8() + ")");
    chatSocket->disconnectFromHost();
    if(chatSocket->state() != QAbstractSocket::UnconnectedState)
        chatSocket->waitForDisconnected();
}

/* 서버로부터 연결이 끊어졌을 때 실행되는 슬롯 함수 */
void ChatRoomForm::disconnectServer( )
{
    qDebug() << "disconnect";
    QMessageBox::critical(this, tr("Chatting Client"),
                          tr("Disconnect from Server"));
    ui->messageLineEdit->setDisabled(true);
    ui->sentPushButton->setDisabled(true);
    ui->uploadPushButton->setDisabled(true);
    ui->ipLineEdit->setDisabled(true);
    ui->portLineEdit->setDisabled(true);
    ui->idLineEdit->setDisabled(true);
    ui->nameLineEdit->setDisabled(true);

    ui->statusPushButton->setText(tr("Connect"));
}

/* 프로토콜과 데이터를 매개변수로 서버에 소켓을 전송하는 함수 */
void ChatRoomForm::writeSocket(char type, QByteArray message)
{
    QByteArray outByteArray;    // 전송할 데이터를 저장할 바이트어레이 변수 선언
    outByteArray.append(type);  // 프로토콜을 바이트배열로 저장
    outByteArray.append(message);   // 보낼 메시지를 바이트배열로 저장

    chatSocket->write(outByteArray);    // 저장한 데이터를 소켓으로 서버에 전송
    chatSocket->flush();    // 다음에 소켓을 사용하기 위해 남은 데이터를 초기화
    while(chatSocket->waitForBytesWritten());   // 소켓이 쓰여질 때까지 대기
}

void ChatRoomForm::connectPushButton()
{
    QString name = ui->nameLineEdit->text();
    QString buttonText = ui->statusPushButton->text();

    if( "Connect" == buttonText ) {
        if(name.length()){
            chatSocket->connectToHost(ui->ipLineEdit->text(), ui->portLineEdit->text().toInt());
            ui->statusPushButton->setText("Enter");
        }
    } else if( "Enter" == buttonText ) {
        ui->messageLineEdit->setEnabled(true);
        ui->sentPushButton->setEnabled(true);
        ui->uploadPushButton->setEnabled(true);
        ui->ipLineEdit->setDisabled(true);
        ui->portLineEdit->setDisabled(true);
        ui->idLineEdit->setDisabled(true);
        ui->nameLineEdit->setDisabled(true);
//        ui->chattingTextEdit->append("Enter the chat room");

        chatSocket->write(Chat::Enter + name.toUtf8());
        ui->statusPushButton->setText("Leave");
    } else if( "Leave" == buttonText){
        ui->messageLineEdit->setDisabled(true);
        ui->sentPushButton->setDisabled(true);
        ui->uploadPushButton->setDisabled(true);
        ui->ipLineEdit->setEnabled(true);
        ui->portLineEdit->setEnabled(true);
        ui->idLineEdit->setEnabled(true);
        ui->nameLineEdit->setEnabled(true);
        ui->chattingTextEdit->append("Chat Room Ended.");

        chatSocket->write(Chat::Leave + name.toUtf8());
        ui->statusPushButton->setText("Enter");
    }

}

/* 서버에서 보낸 채팅 데이터를 프로토콜을 통해 받는 슬롯 함수 */
void ChatRoomForm::receiveData()
{
    QTcpSocket *chatSocket = qobject_cast<QTcpSocket*>(sender());
    // 네트워크 소켓을 보낸 객체를 sender로 불러온 후 객체로 선언
    if(chatSocket->bytesAvailable() > BLOCK_SIZE)     return;
    // 소켓의 데이터가 읽을 수 있는 블록 사이즈보다 크다면 읽어올 수 없기 때문에 함수를 종료
    QByteArray byteArray = chatSocket->read(BLOCK_SIZE);    // 소켓으로부터 데이터를 읽어서 변수로 선언

    char header = byteArray.at(0);   // 클라이언트에서 수행할 동작을 갖고 있는 첫 1byte를 변수로 선언
    QString body = byteArray.remove(0, 1);  // 데이터가 들어있는 헤더 이후의 값을 변수로 선언

    QString id = ui->idLineEdit->text();    // 서버에 전송할 id를 변수로 선언
    QString name = ui->nameLineEdit->text();    // 서버에 전송할 이름을 변수로 선언

    switch(header){
    case Chat::Connect: // 연결 프로토콜에 대한 클라이언트 응답
        writeSocket(Chat::Connect, name.toUtf8()
                    + "@" + id.toUtf8());
        // 서버에 등록하기 위한 이름과 id데이터를 형식에 맞게 전송
        break;

    case Chat::Enter:   // 입장 프로토콜에 대한 클라이언트 동작
    {
        QList<QString> dataList = body.split("/");  // 기호로 구분된 데이터를 나눠 리스트로 저장
        qDebug() << dataList;

        ui->chattingTextEdit->append(dataList.takeFirst());
        // 첫 번째 데이터인 채팅 메시지 전달
        int clientCnt = dataList.takeFirst().toInt(); // 채팅방 리스트 개수를 변수로 선언
        ui->clientListWidget->clear();  // 채팅방의 접속 리스트를 갱신하기 위해 초기화
        for( int i = 0; i < clientCnt; i++){  // 채팅방 리스트 개수만큼 반복
            QListWidgetItem* nameItem =
                    new QListWidgetItem(dataList.takeFirst());
            // 채팅방 리스트 목록을 아이템으로 선언
            ui->clientListWidget->addItem(nameItem);    // 생성한 아이템을 리스트위젯에 추가
        }
        int fileCnt = dataList.takeFirst().toInt(); // 파일의 리스트 개수를 변수로 선언
        for( int i = 0; i < fileCnt; i++){  // 파일리스트 개수만큼 반복
            QListWidgetItem* fileItem =
                    new QListWidgetItem(dataList.takeFirst());
            // 파일리스트 목록을 아이템으로 선언
            ui->fileListWidget->addItem(fileItem);  // 생성한 아이템을 리스트위젯에 추가
        }
    } break;

    case Chat::Message: // 메시지 프로토콜에 대한 동작
        ui->chattingTextEdit->append(body); // 데이터를 채팅방에 출력
        break;

    case Chat::Invite:  // 초대 프로토콜에 대한 클라이언트 동작
    {
        ui->messageLineEdit->setEnabled(true);  // 메시지 입력 라인에디터 활성화
        ui->sentPushButton->setEnabled(true);   // 보내기 버튼 활성화
        ui->uploadPushButton->setEnabled(true); // 파일 업로드 버튼 활성화
        ui->ipLineEdit->setDisabled(true);
        ui->portLineEdit->setDisabled(true);
        ui->idLineEdit->setDisabled(true);
        ui->nameLineEdit->setDisabled(true);
        ui->statusPushButton->setText("Leave"); // 다음 동작을 나타내는 텍스트로 변경
//        ui->chattingTextEdit->append("Invited to chat room by server.");

        writeSocket(Chat::Invite, "Invited " + name.toUtf8());
        // 동작을 마친 후 로그 기록을 위한 메시지를 서버로 전송
    } break;

    case Chat::Banish:  // 강퇴 프로토콜에 대한 클라이언트 동작
    {
        ui->messageLineEdit->setDisabled(true); // 메시지 입력 라인에디터 비활성화
        ui->sentPushButton->setDisabled(true);  // 보내기 버튼 비활성화
        ui->uploadPushButton->setDisabled(true);    // 파일 업로드 버튼 비활성화
        ui->ipLineEdit->setEnabled(true);
        ui->portLineEdit->setEnabled(true);
        ui->idLineEdit->setEnabled(true);
        ui->nameLineEdit->setEnabled(true);
        ui->statusPushButton->setText("Enter"); // 다음 동작을 나타내는 텍스트로 변경
        ui->chattingTextEdit->append("Terminated in chat rooms from the server.");

        writeSocket(Chat::Banish, "Kicked out " + name.toUtf8());
        // 동작을 마친 후 로그 기록을 위한 메시지를 서버로 전송
    } break;

//    case Chat::Disconnect:  // 연결 해제에 대한 클라이언트 동작
//        ui->statusPushButton->setText("Connect");   //
//        break;

    case Chat::ClientList:  // 클라이언트 리스트 갱신 프로토콜에 대한 동작
    {
        QList<QString> dataList = body.split("/");
        // 클라이언트 리스트를 기호로 구분하고 리스트로 선언
        qDebug() << dataList;

        ui->clientListWidget->clear();  // 리스트를 갱신하기 전에 초기화
        foreach(QString list, dataList){    // 리스트의 개수만큼 반복
            QListWidgetItem* nameItem = new QListWidgetItem(list);
            // 리스트의 내용으로 리스트 아이템을 생성
            ui->clientListWidget->addItem(nameItem);    // 생성한 아이템을 리스트위젯에 저장
        }
    } break;

    case Chat::FileList:
        QList<QString> fileList = body.split("/");
        // 파일 리스트를 기호로 구분하고 리스트로 선언
        qDebug() << fileList;

        ui->fileListWidget->clear();  // 리스트를 갱신하기 전에 초기화
        foreach(QString list, fileList){    // 리스트의 개수만큼 반복
            QListWidgetItem* nameItem = new QListWidgetItem(list);
            // 리스트의 내용으로 리스트 아이템을 생성
            ui->fileListWidget->addItem(nameItem);    // 생성한 아이템을 리스트위젯에 저장
        }
        break;
    }
}

/* 버튼 시그널이 발생했을 때 서버에 메시지를 보내는 슬롯 함수 */
void ChatRoomForm::sendData()
{
    QString message = ui->messageLineEdit->text();  // 라인에디터로부터 메시지를 변수로 선언
    ui->messageLineEdit->clear();   // 메시지를 복사한 라인에디터를 초기화

    if(message.length()){   // 메시지가 존재하면 조건문을 실행
        chatSocket->write(Chat::Message + message.toUtf8());    // 메시지를 서버로 전송
        ui->chattingTextEdit->append("Me : " + message);
        // 클라이언트의 채팅방에도 동일한 메시지를 출력
    }
}

/* 파일 데이터를 보내기 위한 초기 설정 슬롯 함수 */
void ChatRoomForm::sendFile()   // 파일을 열고 경로를 포함한 파일 이름을 서버로부터 가져옴
{
    loadSize = 0;   // 매번 서버로 보낼 바이트의 크기
    byteToWrite = 0;    // 보낼 데이터의 크기
    totalSize = 0;  // 총 파일의 크기
    outBlock.clear();   // 서버로 보낼 바이트 어레이를 초기화

    QString filename = QFileDialog::getOpenFileName(this);  // 파일의 경로를 받아올 다이얼로그
    file = new QFile(filename); // 받아온 다이얼로그로 QFile 변수로 동적 할당
    file->open(QFile::ReadOnly);    // 파일을 읽기 전용으로 열기

    qDebug() << QString("file %1 is opened").arg(filename);
    uploadProgressDialog->setValue(0);  // 첫 전송은 데이터를 보내지 않음으로 0으로 초기화

    if (!isSent) {  // 첫 번째 전송에서 서버와 연결하고 시그널을 발생시킴
        uploadSocket->connectToHost(ui->ipLineEdit->text( ),
                                  ui->portLineEdit->text( ).toInt( ) + 100);
        // ip는 채팅서버와 동일하게 사용하고 포트는 100을 더해서 사용
        isSent = true;  // 조건문을 한 번만 수행하기 위해 true를 저장
    }

    byteToWrite = totalSize = file->size(); // 파일의 크기를 총 크기, 기록할 파일 크기로 저장
    loadSize = 1024;    // 서버로 전송할 바이트의 크기를 설정

    QDataStream out(&outBlock, QIODevice::WriteOnly);
    // 서버로 보낼 바이트 배열 변수를 데이터 스트림 변수로 열어줌
    out << qint64(0) << qint64(0) << filename;
    // qint64(0) 바이트 공간을 확보해놓기 위해 사용, 마지막 위치에 파일 이름을 저장

    totalSize += outBlock.size();   // 파일 크기와 기록할 파일 크기에 첫 바이트 블록의 크기를 추가
    byteToWrite += outBlock.size();

    out.device()->seek(0);  // 총 크기와 기록할 크기를 저장하기 위해 데이터 스트림의 기록위치를 가장 처음으로 변경
    out << totalSize << qint64(outBlock.size());    // 총 파일 크기와 전송할 데이터의 크기를 저장

    uploadSocket->write(outBlock);  // 읽어온 파일을 서버로 전송

    uploadProgressDialog->setMaximum(totalSize);
    // 총 파일 크기를 다이얼로그의 최대 크기로 설정
    uploadProgressDialog->setValue(totalSize - byteToWrite);
    // 총 크기 - 기록한 바이트 수로 남은 진행 상황을 다이얼로그로 표시
    uploadProgressDialog->show();   // 다이얼로그를 화면에 표시

    qDebug() << QString("Sending file %1").arg(filename);
}

/* 파일 내용을 연속적으로 전송하는 슬롯 함수 */
void ChatRoomForm::goOnSend(qint64 numBytes)
{
    byteToWrite -= numBytes;    // 기록할 데이터를 빼줌으로써 기록할 데이터를 갱신
    outBlock = file->read(qMin(byteToWrite, numBytes));
    // 기록할 남은 바이트와 이전에 기록한 바이트 수 중 더 적은 값으로 파일에서 전송할 데이터를 읽어옴
    uploadSocket->write(outBlock);  // 파일에서 읽어온 데이터를 서버로 전송
    // 다이얼로그에 총 파일 크기와 전송한 파일 크기를 설정
    uploadProgressDialog->setMaximum(totalSize);
    uploadProgressDialog->setValue(totalSize - byteToWrite);

    if (byteToWrite == 0) { // 기록할 바이트가 0이면 파일 전송 완료
        qDebug("File sending completed!");
        uploadProgressDialog->reset();  // 전송을 완료했으므로 다이얼로그 초기화
    }
}

/* 서버에서 파일을 다운로드하기 위한 함수 */
void ChatRoomForm::downloadFile()
{
    qDebug("Downloading file ...");

    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    // 데이터를 보낸 소켓을 변수로 선언
//    QString ip = socket->peerAddress().toString();
//    QString fileName;   // 파일이름을 저장할 지역변수 선언

    if(downloadByteReceived == 0) {
        // 데이터를 처음 받을 때 총 파일의 크기, 받은 파일의 크기, 파일의 정보를 가져옴
        downloadProgressDialog->reset();    // 화면에 표시할 다이얼로그를 초기화
        downloadProgressDialog->show(); // 다이얼로그 출력

        QDataStream in(socket); // 소켓을 데이터 스트림으로 사용
        in >> downloadTotalSize >> downloadByteReceived >> downloadFilename;
        // 3개의 변수에 차례대로 데이터를 저장
        downloadProgressDialog->setMaximum(downloadTotalSize);
        // 파일의 총 크기를 다이얼로그 최대값으로 설정
        QFileInfo info(downloadFilename);   // 경로를 포함한 파일이름으로 파일 정보를 가져옴
        QString currentFileName = info.fileName();  // 파일 정보에서 이름만 문자열로 저장
        qDebug() << currentFileName;

        downloadNewFile = new QFile("client/" + currentFileName);
        // client 폴더에 파일을 저장하기 위해 QFile 객체로 동적 할당
//        downloadNewFile = new QFile(currentFileName);
        downloadNewFile->open(QIODevice::WriteOnly);    // 다운로드할 파일을 쓰기 전용으로 열기

    } else {
        qDebug("read?");
        downloadInBlock = socket->readAll();    // 소켓의 모든 데이터를 읽어옴

        downloadByteReceived += downloadInBlock.size();
        // 읽어온 데이터 크기만큼 받은 데이터를 추가
        downloadNewFile->write(downloadInBlock);    // 읽어온 데이터를 파일에 기록
        downloadNewFile->flush();   // 파일에 남은 데이터를 해제
    }

    downloadProgressDialog->setValue(downloadByteReceived);
    // 데이터를 받은 만큼 다이얼로그에 값으로 설정

    if(downloadByteReceived == downloadTotalSize){
        qDebug() << QString("%1 receive completed")/*.arg(filename)*/;
//        ui->textEdit->append(tr("%1 receive completed").arg(filename));

        QFileInfo info(downloadFilename);
        QString currentFileName = info.fileName();

        writeSocket(Chat::FileDownload, currentFileName.toUtf8());

        downloadInBlock.clear();
        downloadByteReceived = 0;
        downloadTotalSize = 0;
        downloadProgressDialog->reset();
        downloadProgressDialog->hide();
        downloadNewFile->close();

        delete downloadNewFile;
    }
}
