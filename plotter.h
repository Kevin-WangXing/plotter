#ifndef PLOTTER_H
#define PLOTTER_H

#include <QtGui/QWidget>
#include <QPixmap>
#include <QMap>

class PlotSettings;

class Plotter : public QWidget
{
    Q_OBJECT

public:
    Plotter(QWidget *parent = 0);
    ~Plotter();

    void setPlotSettings(const PlotSettings &settings);

    void setCurveData(int id, const QVector<QPointF> &data);//设置曲线
    void clearCurve(int id);

protected:
    void paintEvent(QPaintEvent *event);

private:
    void refreshPixmap();
    void drawGrid(QPainter *painter);
    void drawCurves(QPainter *painter);

     enum { Margin = 50 };//B边框


    QVector<PlotSettings> zoomStack;
    int curZoom;

    QPixmap pixmap;
    QMap<int, QVector<QPointF> > curveMap;
};

class PlotSettings
{
public:
    PlotSettings();

    double spanX() const { return maxX - minX; }//左右跨度
    double spanY() const { return maxY - minY; }//上下跨度

    int numXTicks;//刻度线数量
    int numYTicks;

    double minX;//X轴最小最大
    double maxX;

    double minY;//Y轴最小最大
    double maxY;
};

#endif // PLOTTER_H
