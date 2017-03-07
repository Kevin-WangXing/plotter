#include "plotter.h"

#include <QPen>
#include <QPainter>

Plotter::Plotter(QWidget *parent)
    : QWidget(parent)
{

    setBackgroundRole(QPalette::Dark);
    setAutoFillBackground(true);
    setPlotSettings(PlotSettings());

    rubberBandIsShown = false;

    //resize(600, 500);
}

Plotter::~Plotter()
{

}

PlotSettings::PlotSettings()//Ĭ��ֵ
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
    //drawCurves(&painter);//���ڻ�ͼ����ʹû��resizeEvent�������ı괰�ڴ�С����ͼ���ű仯
    painter.drawPixmap(0, 0, pixmap);

    if(rubberBandIsShown)
    {
        painter.setPen(palette().light().color());//����
        painter.drawRect(rubberBandRect.normalized()
                         .adjusted(0, 0, -1, -1));//����
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
    };//������ɫ
    PlotSettings settings = zoomStack[curZoom];
    QRect rect(Margin, Margin,
               width() - 2 * Margin, height() - 2 * Margin);//��������
    if (!rect.isValid())//�������̫С
        return;

    painter->setClipRect(rect.adjusted(+1, +1, -1, -1));//ֻ����Ч���򣬳��������ڻ滭

    QMapIterator<int, QVector<QPointF> > i(curveMap);//����ȡ�����ڵ���������
    while (i.hasNext()) {
        i.next();

        int id = i.key();//�������ֵ���ǳɶԵģ�key�ڼ�������
        QVector<QPointF> data = i.value();//value�ڼ��������ϵ�ֵ
        QPolygonF polyline(data.count());//���ö���ߣ���ÿһ���㶼��������

                //��ÿһ���㶼�ӽ�ȥ
        for (int j = 0; j < data.count(); ++j)
                {
            double dx = data[j].x() - settings.minX;//ÿһ�����i�����ȥ����ߣ�����Ҫ������λ��
            double dy = data[j].y() - settings.minY;
            double x = rect.left() + (dx * (rect.width() - 1)
                                         / settings.spanX());//����ÿ���������
            double y = rect.bottom() - (dy * (rect.height() - 1)
                                           / settings.spanY());
            polyline[j] = QPointF(x, y);//ÿ���㶼���Ƴ���
        }
        painter->setPen(colorForIds[uint(id) % 6]);//��ˢ�ڼ������õڼ�����ɫ
        painter->drawPolyline(polyline);//����
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
//��Ϊ�������ͼ�����������ú����ı䴰�ڴ�С�����߲����Ŷ�
void Plotter::resizeEvent(QResizeEvent *event)
{
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
    //��ͼ�����水�²�������
    QRect rect(Margin, Margin,
               width() - 2 * Margin, height() - 2 * Margin);

    if (event->button() == Qt::LeftButton) //���������
    {
        if (rect.contains(event->pos())) //��갴�µ�λ���ڻ�ͼ������
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
    if (rubberBandIsShown) //����ǲ���Ҫ��
    {
        updateRubberBandRegion();
        rubberBandRect.setBottomRight(event->pos());//�ı����½�
        updateRubberBandRegion();
    }
}

void Plotter::updateRubberBandRegion()
{
    QRect rect = rubberBandRect.normalized();//�����׼�������Ͻ�С�����½Ǵ�
        //ֻˢ�¾�������
    update(rect.left(), rect.top(), rect.width(), 1);//width����
    update(rect.left(), rect.top(), 1, rect.height());//1�߶�height����
    update(rect.left(), rect.bottom(), rect.width(), 1);
    update(rect.right(), rect.top(), 1, rect.height());
}
