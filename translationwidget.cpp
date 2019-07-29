/*
 *@file:   translationwidget.cpp
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
#include "translationwidget.h"
#include <QDebug>

/*
 *@brief:   构造函数，完成平移容器的初始化
 *@author:  缪庆瑞
 *@date:    2016.10.8
 *@param:   list:需要进行平移操作的部件列表,放进该容器的所有部件大小必须一致
 *@param:   parent:父对象
 */
TranslationWidget::TranslationWidget(QList<QWidget *> &list, QWidget *parent) :
    QWidget(parent),posX(0),posY(0),miniumMoveX(10),maximumMoveX(40)
{
    //初始化鼠标横坐标相对位置以及移动距离
    cursorX=0;
    moveX=0;
    currentPageIndex = 0;//初始化显示页
    moveEnabled = true;//组件默认可以平移

    widgetList = list;
    pageNum = widgetList.size();//部件的个数
    if(pageNum<1)
    {
        qDebug()<<"No Widget In TranslationWidget!!!";
        return ;
    }
    pageWidth = widgetList.at(0)->width();
    //this->resize(pageWidth,widgetList.at(0)->height());
    this->setFixedSize(pageWidth,widgetList.at(0)->height());//固定容器大小与部件一致

    for(int i=0;i<pageNum;i++)
    {
        widgetList.at(i)->setParent(this);
        if(i !=0)
        {
            widgetList.at(i)->move(pageWidth,0);//初始窗口只显示第一个页面，其他页面移到显示区域外
        }
    }
    //初始化动画组
    initAnimationGroup();
}

TranslationWidget::~TranslationWidget()
{
}
/*
 *@brief:   初始化动画组 动画对象只在初始化时申请内存
 *@author:  缪庆瑞
 *@date:    2017.12.5
 */
void TranslationWidget::initAnimationGroup()
{
    singleAnimation = new QPropertyAnimation(this);
    parallelAnimation1 = new QPropertyAnimation(this);
    parallelAnimation2 = new QPropertyAnimation(this);
    singleAnimation->setPropertyName("pos");//设置动画属性 以位置为标准
    parallelAnimation1->setPropertyName("pos");
    parallelAnimation2->setPropertyName("pos");
    singleAnimation->setEasingCurve(QEasingCurve::OutCubic);//设置缓和曲线
    parallelAnimation1->setEasingCurve(QEasingCurve::OutCubic);
    parallelAnimation2->setEasingCurve(QEasingCurve::OutCubic);
    animationGroup = new QParallelAnimationGroup(this);//并行动画组
    animationGroup->addAnimation(parallelAnimation1);
    animationGroup->addAnimation(parallelAnimation2);
    transitioned = false;//该变量标记并行动画组结束是否产生了状态迁移
    connect(animationGroup,SIGNAL(finished()),this,SLOT(animationFinished()));
}
/*
 *@brief:   单部件动画，用于只涉及一个部件的还原动画。
 * 该方法目前只在鼠标释放处理函数中针对widgetList的首尾页使用。
 *@author:  缪庆瑞
 *@date:    2017.12.5
 *@param:   targetIndex:动画的目标部件的索引
 *@param:   duration:动画的持续时间(ms)
 */
void TranslationWidget::singleAnimationShow(int targetIndex, int duration)
{
    QWidget * targetWidget = widgetList.at(targetIndex);
    //设置动画的目标部件　注:动画在running期间不可以更改目标对象
    singleAnimation->setTargetObject(targetWidget);
    singleAnimation->setStartValue(targetWidget->pos());//开始值:当前位置
    singleAnimation->setEndValue(QPoint(posX,posY));
    singleAnimation->setDuration(duration);//持续时间
    singleAnimation->start();//采用默认参数,动画结束后不删除动画对象
}
/*
 *@brief:   并行动画，适用于两个部件动画状态转换
 *@author:  缪庆瑞
 *@date:    2017.12.5
 *@updte:   2019.3.2
 *@param:   curTargetIndex:当前动画目标部件索引
 *@param:   nextTargetIndex:下一动画目标部件索引
 *@param:   direct:表示动画的方向　向左或向右
 *@param:   isTransitioned:表示是否完成了状态转换,即当前页面是否改变
 *@param:   duration:动画的持续时间(ms)
 */
void TranslationWidget::parallelAnimationShow(int curTargetIndex, int nextTargetIndex, ANIMATION_DIRECT direct, bool isTransitioned, int duration)
{
    QWidget *curTargetWidget = widgetList.at(curTargetIndex);
    QWidget *nextTargetWidget = widgetList.at(nextTargetIndex);
    /* 假如上一个动画还没运行结束，此时不可以直接改变动画对象的目标部件。
     * 而如果调用stop()则会导致上一动画被迫终止，部件的位置停留在stop时的
     * 位置造成混乱。所以这里使用的方式是设置动画的当前时间为整个动画的持续
     * 时间，也就是让上一动画立即完成使命结束，进而正常的开启下一动画。
     */
    if(animationGroup->state()==QAbstractAnimation::Running)
    {
        //让动画立即完成使命进而结束
        animationGroup->setCurrentTime(animationGroup->totalDuration());
    }
    //设置当前动画的目标部件　注:动画在running期间不可以更改目标对象
    parallelAnimation1->setTargetObject(curTargetWidget);
    parallelAnimation1->setDuration(duration);//持续时间
    parallelAnimation1->setStartValue(curTargetWidget->pos());//开始值:当前位置

    parallelAnimation2->setTargetObject(nextTargetWidget);//设置下一动画的目标部件
    parallelAnimation2->setDuration(duration);//持续时间
    parallelAnimation2->setStartValue(nextTargetWidget->pos());//开始值:当前位置

    int tempPageWidth;//根据方向确定部件的移动位置
    tempPageWidth = (direct==ANIMATION_LEFT)?-pageWidth:pageWidth;
    if(isTransitioned)//完成了状态转换　即当前部件随动画移出，下一部件移入
    {
        parallelAnimation1->setEndValue(QPoint(tempPageWidth,posY));
        parallelAnimation2->setEndValue(QPoint(posX,posY));
        /*理论上讲，需要动画结束才真正实现了页面切换。但现在在该函数以及
         * mouseMoveEvent事件处理函数中添加了对动画状态的判断，即能够保证
         * 每一个动画只要开始一定能正常结束，所以在动画开始之前就改变页面索引号
         * 也不会出错*/
        currentPageIndex = nextTargetIndex;
        transitioned = true;
    }
    else//未完成状态转换　即当前部件随动画复原，下一部件移出
    {
        parallelAnimation1->setEndValue(QPoint(posX,posY));
        parallelAnimation2->setEndValue(QPoint(tempPageWidth,posY));
        transitioned = false;
    }
    //并行动画组
    animationGroup->start();//采用默认参数,动画结束后不删除动画对象
}
/*
 *@brief:   设置当前显示页
 *@author:  缪庆瑞
 *@date:    2017.12.7
 *@param:   index:当前页索引号
 *@param:   isAnimation:是否带动画
 */
void TranslationWidget::setCurrentPage(int index, bool isAnimation)
{
    if(index<0 || index>=pageNum || index == currentPageIndex)
    {
        //qDebug()<<"the page index out the range.";
        return ;
    }
    if(isAnimation)//带动画显示
    {
        if(currentPageIndex < index)//向左移动
        {
            //先把当前页和要显示的页之间所有的部件左移，确保之后动画的运动方向正确
            for(int i=currentPageIndex+1;i<index;i++)
            {
                widgetList.at(i)->move(-pageWidth,posY);
            }
            parallelAnimationShow(currentPageIndex,index,ANIMATION_LEFT,true);
        }
        else if(currentPageIndex > index)//向右
        {
            //先把当前页和要显示的页之间所有的部件右移，确保之后动画的运动方向正确
            for(int i=index+1;i<currentPageIndex;i++)
            {
                widgetList.at(i)->move(pageWidth,posY);
            }
            parallelAnimationShow(currentPageIndex,index,ANIMATION_RIGHT,true);
        }
    }
    else//不带动画，直接平移
    {
        for(int i=0;i<pageNum;i++)
        {
            if(i<index)//左边
            {
                widgetList.at(i)->move(-pageWidth,posY);
            }
            else if(i==index)
            {
                widgetList.at(i)->move(posX,posY);
                currentPageIndex = index;
            }
            else if(i>index)//右边
            {
                widgetList.at(i)->move(pageWidth,posY);
            }
        }
    }
}
/*
 *@brief:   设置移动使能
 *@author:  缪庆瑞
 *@date:    2017.12.7
 *@param:   moveEnabled: true=可以移动　false=不可以移动
 */
void TranslationWidget::setMoveEnabled(bool moveEnabled)
{
    this->moveEnabled = moveEnabled;
}
/*
 *@brief:   动画结束的槽，此处只连接并行动画组
 *@author:  缪庆瑞
 *@date:    2017.12.7
 */
void TranslationWidget::animationFinished()
{
    if(transitioned)
    {
        emit currentPageChanged(currentPageIndex);
    }
}
/*
 *@brief:   鼠标按下事件处理
 *@author:  缪庆瑞
 *@date:    2016.10.8
 *@param:   event:鼠标事件
 */
void TranslationWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)//左键按下
    {
        cursorX=event->pos().x();//获取鼠标按下时相对容器的横坐标值
        //qDebug()<<"press:"<<cursorX;
    }
}
/*
 *@brief:   鼠标释放事件处理 显示页面的切换
 *@author:  缪庆瑞
 *@date:    2016.10.8
 *@param:   event:鼠标事件
 */
void TranslationWidget::mouseReleaseEvent(QMouseEvent *event)
{
    //qDebug()<<"release:"<<event->pos().x();
    if(moveX == 0)//仅仅按下鼠标后释放，没有移动事件发生
    {
        return ;
    }
    if(event->button()==Qt::LeftButton)//左键释放
    {
        //qDebug()<<"释放前的界面:"<<currentPageIndex;
        if(widgetList.size() == 1)//只有一个部件
        {
            singleAnimationShow(0);//回到原位置
        }
        else //多个部件
        {
            if(currentPageIndex == 0)//首页
            {
                if(moveX>0)//首页右移 不会进行状态转换
                {
                    singleAnimationShow(0);//首页回到原位置
                }
                else //首页左移
                {
                    if(qAbs(moveX)<miniumMoveX) //不会进行状态转换 动画实际右移
                    {
                        parallelAnimationShow(0,1,ANIMATION_RIGHT,false);
                    }
                    else //会进行状态转换　动画实际左移
                    {
                        parallelAnimationShow(0,1,ANIMATION_LEFT,true);
                    }
                }
            }
            else if(currentPageIndex == pageNum-1)//尾页
            {
                if(moveX<0)//尾页左移　不会进行状态转换
                {
                    singleAnimationShow(pageNum-1);//尾页回到原位置
                }
                else //尾页右移
                {
                    if(qAbs(moveX)<miniumMoveX) //不会进行状态转换 动画实际左移
                    {
                        parallelAnimationShow(pageNum-1,pageNum-2,ANIMATION_LEFT,false);
                    }
                    else //会进行状态转换　动画实际右移
                    {
                        parallelAnimationShow(pageNum-1,pageNum-2,ANIMATION_RIGHT,true);
                    }
                }
            }
            else // 中间页
            {
                if(moveX<0)//中间页左移
                {
                    if(qAbs(moveX)<miniumMoveX) //不会进行状态转换 动画实际右移
                    {
                        parallelAnimationShow(currentPageIndex,currentPageIndex+1,ANIMATION_RIGHT,false);
                    }
                    else //会进行状态转换　动画实际左移
                    {
                        parallelAnimationShow(currentPageIndex,currentPageIndex+1,ANIMATION_LEFT,true);
                    }
                }
                else //中间页右移
                {
                    if(qAbs(moveX)<miniumMoveX) //不会进行状态转换 动画实际左移
                    {
                        parallelAnimationShow(currentPageIndex,currentPageIndex-1,ANIMATION_LEFT,false);
                    }
                    else //会进行状态转换　动画实际右移
                    {
                        parallelAnimationShow(currentPageIndex,currentPageIndex-1,ANIMATION_RIGHT,true);
                    }
                }
            }
        }
        moveX = 0;//释放鼠标后将横轴方向移动的像素个数设为０
        //qDebug()<<"释放后的界面:"<<currentPageIndex;
    }
}
/*
 *@brief:   鼠标移动事件处理 同时移动窗口页面
 *@author:  缪庆瑞
 *@date:    2016.10.8
 *@param:   event:鼠标事件
 */
void TranslationWidget::mouseMoveEvent(QMouseEvent *event)
{
    //qDebug()<<"move:"<<event->pos().x();
    //在上一次移动产生的动画结束前或者组件本身不可以移动的情况下，不处理新的移动
    if(animationGroup->state()==QAbstractAnimation::Running ||
            singleAnimation->state()==QAbstractAnimation::Running || !moveEnabled)
    {
        return;
    }
    //鼠标左键按下同时移动
    //不可以用event.button()==Qt::LeftButton来判断，因为在鼠标移动事件里该方法一直返回Qt::NoButton
    if(event->buttons()&Qt::LeftButton)
    {
        moveX = event->pos().x();//获取移动过程中的横轴坐标
        moveX = moveX-cursorX;//鼠标左键按下水平移动距离（左负 右正）
        if(qAbs(moveX)>pageWidth)//当鼠标拖动的距离大于一个页面长度时保证不会出现“下下”一个页面
        {
            moveX=(moveX>0)?pageWidth:-pageWidth;
        }
        if(widgetList.size() == 1)//只有一个部件
        {
            //实现一个页面不能再移动的效果
            if(qAbs(moveX) > maximumMoveX)//移动距离大于设置的像素数，则按设置的像素数移动
            {
                if(moveX<0)
                {
                    widgetList.at(0)->move(-maximumMoveX,posY);
                }
                else
                {
                    widgetList.at(0)->move(maximumMoveX,posY);
                }

            }
            else
            {
                widgetList.at(0)->move(moveX,posY);
            }
        }
        else //多个部件
        {
            if( currentPageIndex == 0)//首页
            {
                //移动首页
                if(moveX > maximumMoveX)
                {
                    widgetList.at(0)->move(maximumMoveX,posY);
                }
                else
                {
                    widgetList.at(0)->move(moveX,posY);
                }
                //移动后一页
                if(moveX<0)//向左移动
                {
                    widgetList.at(1)->move(moveX+pageWidth,posY); //后一个页面 水平左移
                }
                else //向右移动要保证之前左移过来的页面移出显示区域
                {
                    widgetList.at(1)->move(pageWidth,posY);//以免鼠标移动太快，有页面部分残留
                }
            }
            else if(currentPageIndex == pageNum-1)//尾页
            {
                //移动尾页
                if(moveX < -maximumMoveX)
                {
                    widgetList.at(pageNum-1)->move(-maximumMoveX,posY);
                }
                else
                {
                    widgetList.at(pageNum-1)->move(moveX,posY);
                }
                //移动前一页
                if(moveX>0)//向右移动
                {
                    widgetList.at(pageNum-2)->move(moveX-pageWidth,posY);//上一个页面 水平右移
                }
                else
                {
                    widgetList.at(pageNum-2)->move(-pageWidth,posY);//保证之前右移的过来的页面移出显示区域
                }
            }
            else //中间页
            {
                //移动当前页
                widgetList.at(currentPageIndex)->move(moveX,posY);
                if(moveX<0)//向左移动
                {
                    widgetList.at(currentPageIndex-1)->move(-pageWidth,posY);//保证之前右移的过来的页面移出显示区域
                    widgetList.at(currentPageIndex+1)->move(moveX+pageWidth,posY);//后一个页面 水平左移
                }
                else //向右移动
                {
                    widgetList.at(currentPageIndex-1)->move(moveX-pageWidth,posY);//上一个页面0 水平右移
                    widgetList.at(currentPageIndex+1)->move(pageWidth,posY);//保证之前左移的过来的页面移出显示区域
                }
            }
        }
    }
}
