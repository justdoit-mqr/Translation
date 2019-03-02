#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include "translationwidget.h"

namespace Ui {
class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();
    void initTranslationWidget();//初始化平移容器

private slots:
    void on_pushButton_clicked();

private:
    Ui::Form *ui;
    TranslationWidget *translationWidget;
};

#endif // FORM_H
