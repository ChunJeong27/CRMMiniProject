#ifndef CHATTINGFORM_H
#define CHATTINGFORM_H

#include <QWidget>

namespace Ui {
class ChattingForm;
}

class ChattingForm : public QWidget
{
    Q_OBJECT

public:
    explicit ChattingForm(QWidget *parent = nullptr);
    ~ChattingForm();

private:
    Ui::ChattingForm *ui;
};

#endif // CHATTINGFORM_H
