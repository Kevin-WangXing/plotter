#include "plotter.h"

#include <QPen>
#include <QPainter>

Plotter::Plotter(QWidget *parent)
    : QWidget(parent)
{
    setBackgroundRole(QPalette::Dark);
    setAutoFillBackground(true);
    setPlotSettings(PlotSettings());

    resize(600, 500);
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

    refreshPixmap();
}

void Plotter::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    //drawGrid(&painter);
    painter.drawPixmap(0, 0, pixmap);
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

    for (int j = 0; j <= settings.numYTicks; ++j) {
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
    while (i.hasNext()) {
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
