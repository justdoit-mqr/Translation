/*
 *@file:   translationwidget.h
 *@author: 缪庆瑞
 *@date:   2016.10.8  根据鼠标事件，实现内部部件发生平移的容器
 *@update:  2017.12.5　加入平移动画效果
 *@update:  2019.3.1    提供外界设置容器当前页的接口
 *@brief:  该组件本质上是一个容器，使用时只需将要实现平移效果的界面列表(widgetlist)传递给
 *该容器。容器内部实现根据鼠标事件平移界面，并在鼠标释放时结合qt动画实现缓和的滑动效果，
 * 此外也留出了与外部通信的信号接口，实现内部界面完成状态迁移，外部根据信号做一些界面
 * 切换的逻辑处理。
 * 注意：该平移组件容器的核心是根据不同鼠标事件(按下,移动,释放)处理，完成平移操作。平移界
 * 面列表(widgetlist)作为该容器的子对象，如果子对象本身也重写了鼠标事件处理函数，那么按照
 * 事件传递顺序会先传给获得焦点的窗口部件(这里是子对象)，只有子对象事件处理函数忽略该事件
 * 时，才会上传给父对象的事件处理函数。所以如果子对象重写了事件处理函数，那么一定要在该
 * 函数最后加上event->ignore()
 */
#ifndef TRANSLATIONWIDGET_H
#define TRANSLATIONWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QtGlobal>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QEasingCurve>

class TranslationWidget : public QWidget
{
    Q_OBJECT

public:
    enum ANIMATION_DIRECT //枚举类型表示动画的方向
    {
        ANIMATION_RIGHT,  //向右
        ANIMATION_LEFT  //向左
    };
    explicit TranslationWidget(QList<QWidget *> &list,QWidget *parent = 0);
    ~TranslationWidget();

    int getCurrentPageIndex(){return currentPageIndex;}//获取当前页的索引
    QWidget *getCurrentPage(){return widgetList.at(currentPageIndex);}//获取当前页
    void setCurrentPage(int index,bool isAnimation=false);//设置当前显示页
    bool getMoveEnabled(){return moveEnabled;}//获取当前组件是否可以平移
    void setMoveEnabled(bool moveEnabled);//设置组件是否可以平移

private:
    void initAnimationGroup();//初始化动画组
    void singleAnimationShow(int targetIndex,int duration=100);//单动画展示
    void parallelAnimationShow(int curTargetIndex,int nextTargetIndex,
                               ANIMATION_DIRECT direct,bool isTransitioned,int duration=250);//并行动画展示

signals:
    void currentPageChanged(int index);//动画结束当前页改变发送此信号

public slots:
    void animationFinished();

protected:
    virtual void mousePressEvent(QMouseEvent *event);//重写鼠标按下事件处理
    virtual void mouseReleaseEvent(QMouseEvent *event);//重写鼠标释放事件处理
    virtual void mouseMoveEvent(QMouseEvent *event);//重写鼠标移动事件处理
    //virtual void paintEvent(QPaintEvent *event);//重写绘图事件

private:
    //需要进行平移操作的部件列表，顺序即从左至右
    QList<QWidget *> widgetList;
    bool moveEnabled;//标识部件组是否可以平移
    //实现动画
    QPropertyAnimation *singleAnimation;//单动画
    QPropertyAnimation *parallelAnimation1;
    QPropertyAnimation *parallelAnimation2;
    QParallelAnimationGroup *animationGroup;//并行动画组
    bool transitioned;//标记是否由动画产生了状态迁移,即是否完成了页面的切换
    //使用常变量取代宏
    const int posX;//页面左上顶点的x坐标　move()方法参数就是左上顶点
    const int posY;//页面左上顶点的y坐标
    const int miniumMoveX;//最小的水平移动像素，以此值确定是否进行平移状态转换，越小越灵敏
    const int maximumMoveX;//首尾部件的最大移动距离

    int cursorX;//鼠标按下时指针的横坐标（相对于该容器本身）
    int moveX;//横轴方向移动的像素个数

    int currentPageIndex;//界面当前显示页号
    int pageNum;//页数　表示容器中的部件个数
    int pageWidth;//页面的宽度

};

#endif // TRANSLATIONWIDGET_H
