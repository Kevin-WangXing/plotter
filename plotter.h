#ifndef PLOTTER_H
#define PLOTTER_H

#include <QtGui/QWidget>
#include <QPixmap>
#include <QMap>
#include <QMouseEvent>

class PlotSettings;

class Plotter : public QWidget
{
    Q_OBJECT

public:
    Plotter(QWidget *parent = 0);
    ~Plotter();

    void setPlotSettings(const PlotSettings &settings);

    void setCurveData(int id, const QVector<QPointF> &data);//��������
    void clearCurve(int id);

    QSize minimumSize() const;//������С����
    QSize sizeHint() const;//����������ڲ���

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

private:
    void updateRubberBandRegion();
    void refreshPixmap();
    void drawGrid(QPainter *painter);
    void drawCurves(QPainter *painter);

     enum { Margin = 50 };//B�߿�


    QVector<PlotSettings> zoomStack;
    int curZoom;

    QPixmap pixmap;
    QMap<int, QVector<QPointF> > curveMap;

    bool rubberBandIsShown;//�Ƿ���ʾ����
    QRect rubberBandRect;//���δ�С
};

class PlotSettings
{
public:
    PlotSettings();

    double spanX() const { return maxX - minX; }//���ҿ��
    double spanY() const { return maxY - minY; }//���¿��

    int numXTicks;//�̶�������
    int numYTicks;

    double minX;//X����С���
    double maxX;

    double minY;//Y����С���
    double maxY;
};

#endif // PLOTTER_H
