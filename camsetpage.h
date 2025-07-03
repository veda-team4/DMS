#ifndef CAMSETPAGE_H
#define CAMSETPAGE_H

#include <QWidget>

namespace Ui {
class CamSetPage;
}

class CamSetPage : public QWidget
{
    Q_OBJECT

public:
    explicit CamSetPage(QWidget *parent = nullptr);
    ~CamSetPage();

private:
    Ui::CamSetPage *ui;
};

#endif // CAMSETPAGE_H
