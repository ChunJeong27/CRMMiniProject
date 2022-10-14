#include "chattingform.h"
#include "ui_chattingform.h"

ChattingForm::ChattingForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChattingForm)
{
    ui->setupUi(this);
}

ChattingForm::~ChattingForm()
{
    delete ui;
}
