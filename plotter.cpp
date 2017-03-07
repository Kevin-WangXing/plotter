#include "plotter.h"

#include <QPen>
#include <QPainter>
#include <cmath>

Plotter::Plotter(QWidget *parent)
    : QWidget(parent)
{

    setBackgroundRole(QPalette::Dark);
    setAutoFillBackground(true);
    setPlotSettings(PlotSettings());

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);
    rubberBandIsShown = false;

    zoomInButton = new QToolButton(this);
    zoomInButton->setIcon(QIcon(":/images/zoomin.png"));
    zoomInButton->adjustSize();
    connect(zoomInButton, SIGNAL(clicked()), this, SLOT(zoomIn()));

    zoomOutButton = new QToolButton(this);
    zoomOutButton->setIcon(QIcon(":/images/zoomout.png"));
    zoomOutButton->adjustSize();
    connect(zoomOutButton, SIGNAL(clicked()), this, SLOT(zoomOut()));

    //resize(600, 500);
}

Plotter::~Plotter()
{

}

PlotSettings::PlotSettings()//默认值
{
    minX = 0.0;
    maxX = 10.0;
    numXTicks = 5;

    minY = 0.0;
    maxY = 10.0;
    numYTicks = 5;

}

void Plotter::setPlotSettings(const PlotSettings &settings)
{
    zoomStack.clear();
    zoomStack.append(settings);
    curZoom = 0;

    //zoomInButton->setEnabled(false);
    //zoomInButton->hide();
    //zoomOutButton->hide();

    refreshPixmap();
}

void Plotter::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    //drawGrid(&painter);
    //drawCurves(&painter);//屏内绘图，即使没有resizeEvent函数，改标窗口大小，绘图跟着变化
    painter.drawPixmap(0, 0, pixmap);

    if(rubberBandIsShown)
    {
        painter.setPen(palette().light().color());//画笔
        painter.drawRect(rubberBandRect.normalized()
                         .adjusted(0, 0, -1, -1));//矩形
    }
}

void Plotter::drawGrid(QPainter *painter)
{
    QRect rect(Margin, Margin,
               width() - 2 * Margin, height() - 2 * Margin);
    if (!rect.isValid())
        return;

    PlotSettings settings = zoomStack[curZoom];
    QPen quiteDark = palette().dark().color().light();
    QPen light = palette().light().color();

    for (int i = 0; i <= settings.numXTicks; ++i)
    {
        int x = rect.left() + (i * (rect.width() - 1)
                               / settings.numXTicks);
        double label = settings.minX + (i * settings.spanX()
                                        / settings.numXTicks);
        painter->setPen(quiteDark);
        painter->drawLine(x, rect.top(), x, rect.bottom());
        painter->setPen(light);
        painter->drawLine(x, rect.bottom(), x, rect.bottom() + 5);
        painter->drawText(x - 50, rect.bottom() + 5, 100, 20,
                          Qt::AlignHCenter | Qt::AlignTop,
                          QString::number(label));
    }

    for (int j = 0; j <= settings.numYTicks; ++j)
    {
        int y = rect.bottom() - (j * (rect.height() - 1)
                                   / settings.numYTicks);
        double label = settings.minY + (j * settings.spanY()
                                          / settings.numYTicks);
        painter->setPen(quiteDark);
        painter->drawLine(rect.left(), y, rect.right(), y);
        painter->setPen(light);
        painter->drawLine(rect.left() - 5, y, rect.left(), y);
        painter->drawText(rect.left() - Margin, y - 10, Margin - 5, 20,
                          Qt::AlignRight | Qt::AlignVCenter,
                          QString::number(label));
    }
}

void Plotter::drawCurves(QPainter *painter)
{
    static const QColor colorForIds[6] =
    {
        Qt::red, Qt::green, Qt::blue, Qt::cyan, Qt::magenta, Qt::yellow
    };//曲线颜色
    PlotSettings settings = zoomStack[curZoom];
    QRect rect(Margin, Margin,
               width() - 2 * Margin, height() - 2 * Margin);//绘制区域
    if (!rect.isValid())//如果区域太小
        return;

    painter->setClipRect(rect.adjusted(+1, +1, -1, -1));//只画有效区域，超过区域不在绘画

    QMapIterator<int, QVector<QPointF> > i(curveMap);//将点取出放在迭代器里面
    while (i.hasNext())
    {
        i.next();

        int id = i.key();//容器里的值都是成对的，key第几条曲线
        QVector<QPointF> data = i.value();//value第几条曲线上得值
        QPolygonF polyline(data.count());//设置多段线（把每一个点都连起来）

                //把每一个点都加进去
        for (int j = 0; j < data.count(); ++j)
        {
            double dx = data[j].x() - settings.minX;//每一个点的i坐标减去最左边，就是要画出的位置
            double dy = data[j].y() - settings.minY;
            double x = rect.left() + (dx * (rect.width() - 1)
                                         / settings.spanX());//计算每个点的坐标
            double y = rect.bottom() - (dy * (rect.height() - 1)
                                           / settings.spanY());
            polyline[j] = QPointF(x, y);//每个点都绘制出来
        }
        painter->setPen(colorForIds[uint(id) % 6]);//画刷第几条线用第几个颜色
        painter->drawPolyline(polyline);//画线
    }
}

void Plotter::refreshPixmap()
{
    pixmap = QPixmap(size());

    QPainter painter(&pixmap);
    painter.initFrom(this);
    drawGrid(&painter);
    drawCurves(&painter);
    update();
}

void Plotter::setCurveData(int id, const QVector<QPointF> &data)
{
    curveMap[id] = data;
}

void Plotter::clearCurve(int id)
{
    curveMap.remove(id);
}
//因为是屏外绘图，如果不加入该函数改变窗口大小，曲线不跟着动
void Plotter::resizeEvent(QResizeEvent *event)
{
    //设置按钮位置
    int x = width() - (zoomInButton->width()
                       + zoomOutButton->width() + 10);
    zoomInButton->move(x, 5);
    zoomOutButton->move(x + zoomInButton->width() + 5, 5);
    refreshPixmap();
}

QSize Plotter::minimumSize() const
{
    return QSize(6 * Margin, 4 * Margin);
}

QSize Plotter::sizeHint() const
{
    return QSize(12 * Margin, 8 * Margin);
}

void Plotter::mousePressEvent(QMouseEvent *event)
{
    //绘图区里面按下才起作用
    QRect rect(Margin, Margin,
               width() - 2 * Margin, height() - 2 * Margin);

    if (event->button() == Qt::LeftButton) //鼠标左键点击
    {
        if (rect.contains(event->pos())) //鼠标按下的位置在绘图区里面
        {
            rubberBandIsShown = true;
            rubberBandRect.setTopLeft(event->pos());
            rubberBandRect.setBottomRight(event->pos());
            updateRubberBandRegion();
            //setCursor(Qt::CrossCursor);
        }
    }
}

void Plotter::mouseMoveEvent(QMouseEvent *event)
{
    if (rubberBandIsShown) //检查是不是要画
    {
        updateRubberBandRegion();
        rubberBandRect.setBottomRight(event->pos());//改变右下角
        updateRubberBandRegion();
    }
}

void Plotter::updateRubberBandRegion()
{
    QRect rect = rubberBandRect.normalized();//坐标标准化，左上角小，右下角大
    //只刷新矩形区域
    update(rect.left(), rect.top(), rect.width(), 1);//width横线
    update(rect.left(), rect.top(), 1, rect.height());//1高度height竖线
    update(rect.left(), rect.bottom(), rect.width(), 1);
    update(rect.right(), rect.top(), 1, rect.height());
}
//鼠标松开
void Plotter::mouseReleaseEvent(QMouseEvent *event)
{
    //检查鼠标左键，是否显示橡皮筋（矩形框）
    if ((event->button() == Qt::LeftButton) && rubberBandIsShown)
    {
        rubberBandIsShown = false;//松手即不画
        updateRubberBandRegion();

        QRect rect = rubberBandRect.normalized();
        if (rect.width() < 4 || rect.height() < 4)//如果矩形太小，高，宽小于4像素不执行操作
            return;
        rect.translate(-Margin, -Margin);//鼠标平移，鼠标的原点和绘图的原点一致

        PlotSettings prevSettings = zoomStack[curZoom];
        PlotSettings settings;
        double dx = prevSettings.spanX() / (width() - 2 * Margin);//每一个像素对应的刻度
        double dy = prevSettings.spanY() / (height() - 2 * Margin);
        settings.minX = prevSettings.minX + dx * rect.left();//当前坐标系最小最大值
        settings.maxX = prevSettings.minX + dx * rect.right();
        settings.minY = prevSettings.maxY - dy * rect.bottom();
        settings.maxY = prevSettings.maxY - dy * rect.top();
        settings.adjust();

        zoomStack.resize(curZoom + 1);//向量要保存起来，可以持续放大
        zoomStack.append(settings);
        zoomIn();
    }
}
void Plotter::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        zoomIn();
        break;
    case Qt::Key_Minus:
        zoomOut();
        break;
    case Qt::Key_Left:
        zoomStack[curZoom].scroll(-1, 0);
        refreshPixmap();
        break;
    case Qt::Key_Right:
        zoomStack[curZoom].scroll(+1, 0);
        refreshPixmap();
        break;
    case Qt::Key_Down:
        zoomStack[curZoom].scroll(0, -1);
        refreshPixmap();
        break;
    case Qt::Key_Up:
        zoomStack[curZoom].scroll(0, +1);
        refreshPixmap();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}
void Plotter::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8;//
    int numTicks = numDegrees / 15;//滚动的刻度

    if (event->orientation() == Qt::Horizontal)
        {
        zoomStack[curZoom].scroll(numTicks, 0);
    } else
        {
        zoomStack[curZoom].scroll(0, numTicks);
    }
    refreshPixmap();
}

void PlotSettings::adjust()
{
    adjustAxis(minX, maxX, numXTicks);//X轴调用一次
    adjustAxis(minY, maxY, numYTicks);//Y轴调用一次
}

void PlotSettings::adjustAxis(double &min, double &max, int &numTicks)
{
    const int MinTicks = 4;//设置最小刻度线
    double grossStep = (max - min) / MinTicks;//最大步长
    //最大步长236 log236 = 2.37291，取结果2， 10的二次方 100， 然后在计算倍数200,500
    double step = std::pow(10.0, std::floor(std::log10(grossStep)));

    if (5 * step < grossStep) //如果5倍小于取5
    {
        step *= 5;
    } else if (2 * step < grossStep)
    {
        step *= 2;
    }

    //刻度线
    numTicks = int(std::ceil(max / step)/*上限*/ - std::floor(min / step)/*下限*/);
    if (numTicks < MinTicks)
        numTicks = MinTicks;
    min = std::floor(min / step) * step;//新的最小值
    max = std::ceil(max / step) * step;
}

void Plotter::zoomIn()//放大
{
    if (curZoom < zoomStack.count() - 1) //容器里面有多少setting
    {
        ++curZoom;
        zoomInButton->setEnabled(curZoom < zoomStack.count() - 1);
        zoomOutButton->setEnabled(true);
        zoomOutButton->show();
        refreshPixmap();

    }

}

void Plotter::zoomOut()
{
    if (curZoom > 0)
    {
        --curZoom;
        zoomOutButton->setEnabled(curZoom > 0);
        zoomInButton->setEnabled(true);
        zoomInButton->show();
        refreshPixmap();
    }
}

void PlotSettings::scroll(int dx, int dy)
{
    double stepX = spanX() / numXTicks;
    minX += dx * stepX;
    maxX += dx * stepX;

    double stepY = spanY() / numYTicks;
    minY += dy * stepY;
    maxY += dy * stepY;
}
