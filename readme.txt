translation是基于Qt开发的一个平移组件，主要应用在嵌入式触摸显示屏上，通过手触滑动切换页面显示。

该组件本质上是一个容器，使用时只需将要实现平移效果的界面列表(widgetList)传递给该容器。容器内部对鼠标事件(按下,移动,释放)进行不同的算法处理，最终通过move()完成内部部件的平移，并在鼠标释放时结合qt动画实现缓和的滑动效果。此外该组件也留出了几个对外接口，可以在外部直接调用完成页面的切换，同时还提供一个信号，向外界报告状态迁移的结果，外部可以根据信号做一些界面切换的逻辑处理。

该组件的所有功能主要由一个类TranslationWidget实现，只需在类对象初始化时传递平移部件的列表即可。对外提供了如下接口：
int getCurrentPageIndex(){return currentPageIndex;}//获取当前页的索引
QWidget *getCurrentPage(){return widgetList.at(currentPageIndex);}//获取当前页
void setCurrentPage(int index,bool isAnimation=false);//设置当前显示页
bool getMoveEnabled(){return moveEnabled;}//获取当前组件是否可以平移
void setMoveEnabled(bool moveEnabled);//设置组件是否可以平移
可以在外界调用setCurrentPage()接口函数直接切换到指定的页面，并且可以选择是否带有动画效果。setMoveEnabled()是用来使能或者禁用组件平移功能，当禁用掉平移功能后，该组件则主要可以作为一个淡如淡出的动画容器。

作者联系方式：@为-何-而来(新浪微博)
