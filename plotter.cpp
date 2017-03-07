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

    //zoomInButton->setEnabled(false);
    //zoomInButton->hide();
    //zoomOutButton->hide();

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
    };//������ɫ
    PlotSettings settings = zoomStack[curZoom];
    QRect rect(Margin, Margin,
               width() - 2 * Margin, height() - 2 * Margin);//��������
    if (!rect.isValid())//�������̫С
        return;

    painter->setClipRect(rect.adjusted(+1, +1, -1, -1));//ֻ����Ч���򣬳��������ڻ滭

    QMapIterator<int, QVector<QPointF> > i(curveMap);//����ȡ�����ڵ���������
    while (i.hasNext())
    {
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
    //���ð�ťλ��
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
//����ɿ�
void Plotter::mouseReleaseEvent(QMouseEvent *event)
{
    //������������Ƿ���ʾ��Ƥ����ο�
    if ((event->button() == Qt::LeftButton) && rubberBandIsShown)
    {
        rubberBandIsShown = false;//���ּ�����
        updateRubberBandRegion();

        QRect rect = rubberBandRect.normalized();
        if (rect.width() < 4 || rect.height() < 4)//�������̫С���ߣ���С��4���ز�ִ�в���
            return;
        rect.translate(-Margin, -Margin);//���ƽ�ƣ�����ԭ��ͻ�ͼ��ԭ��һ��

        PlotSettings prevSettings = zoomStack[curZoom];
        PlotSettings settings;
        double dx = prevSettings.spanX() / (width() - 2 * Margin);//ÿһ�����ض�Ӧ�Ŀ̶�
        double dy = prevSettings.spanY() / (height() - 2 * Margin);
        settings.minX = prevSettings.minX + dx * rect.left();//��ǰ����ϵ��С���ֵ
        settings.maxX = prevSettings.minX + dx * rect.right();
        settings.minY = prevSettings.maxY - dy * rect.bottom();
        settings.maxY = prevSettings.maxY - dy * rect.top();
        settings.adjust();

        zoomStack.resize(curZoom + 1);//����Ҫ�������������Գ����Ŵ�
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
    int numTicks = numDegrees / 15;//�����Ŀ̶�

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
    adjustAxis(minX, maxX, numXTicks);//X�����һ��
    adjustAxis(minY, maxY, numYTicks);//Y�����һ��
}

void PlotSettings::adjustAxis(double &min, double &max, int &numTicks)
{
    const int MinTicks = 4;//������С�̶���
    double grossStep = (max - min) / MinTicks;//��󲽳�
    //��󲽳�236 log236 = 2.37291��ȡ���2�� 10�Ķ��η� 100�� Ȼ���ڼ��㱶��200,500
    double step = std::pow(10.0, std::floor(std::log10(grossStep)));

    if (5 * step < grossStep) //���5��С��ȡ5
    {
        step *= 5;
    } else if (2 * step < grossStep)
    {
        step *= 2;
    }

    //�̶���
    numTicks = int(std::ceil(max / step)/*����*/ - std::floor(min / step)/*����*/);
    if (numTicks < MinTicks)
        numTicks = MinTicks;
    min = std::floor(min / step) * step;//�µ���Сֵ
    max = std::ceil(max / step) * step;
}

void Plotter::zoomIn()//�Ŵ�
{
    if (curZoom < zoomStack.count() - 1) //���������ж���setting
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
