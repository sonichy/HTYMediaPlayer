#ifndef DIALOGURL_H
#define DIALOGURL_H

#include <QDialog>

namespace Ui {
class DialogURL;
}

class DialogURL : public QDialog
{
    Q_OBJECT

public:
    explicit DialogURL(QWidget *parent = 0);
    ~DialogURL();
    Ui::DialogURL *ui;

private:

};

#endif // DIALOGURL_H
