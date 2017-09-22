#include "dialogurl.h"
#include "ui_dialogurl.h"

DialogURL::DialogURL(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogURL)
{
    ui->setupUi(this);
}

DialogURL::~DialogURL()
{
    delete ui;
}
