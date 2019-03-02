#include "form.h"
#include "ui_form.h"
#include <QFrame>
#include <QLabel>

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    initTranslationWidget();
    ui->gridLayout->addWidget(translationWidget);

}

Form::~Form()
{
    delete ui;
}

void Form::initTranslationWidget()
{
    QFrame *frame0 = new QFrame();
    frame0->resize(510,306);//设置页面大小 与窗口等大
    frame0->setStyleSheet("background-color:yellow;");//设置背景颜色
    QLabel *page0 = new QLabel("Page0",frame0);
    page0->move(125,210);//页面下方添加label标签
    QFrame *frame1 = new QFrame();
    frame1->resize(510,306);
    frame1->setStyleSheet("background-color:cyan;");
    QLabel *page1 = new QLabel("Page1",frame1);
    page1->move(125,210);
    QFrame *frame2 = new QFrame();
    frame2->resize(510,306);
    frame2->setStyleSheet("background-color:orange;");
    QLabel *page2 = new QLabel("Page2",frame2);
    page2->move(125,210);
    QFrame *frame3 = new QFrame();
    frame3->resize(510,306);
    frame3->setStyleSheet("background-color:red;");
    QLabel *page3 = new QLabel("Page3",frame3);
    page3->move(125,210);
    QFrame *frame4 = new QFrame();
    frame4->resize(510,306);
    frame4->setStyleSheet("background-color:green;");
    QLabel *page4 = new QLabel("Page4",frame4);
    page4->move(125,210);

    QList<QWidget *> list;
    list.append(frame0);
    list.append(frame1);
    list.append(frame2);
    list.append(frame3);
    list.append(frame4);
    translationWidget = new TranslationWidget(list,this);
    //translationWidget->setMoveEnabled(false);
}
//设置当前页
void Form::on_pushButton_clicked()
{
    int page = ui->spinBox->value();
    translationWidget->setCurrentPage(page,true);
}
