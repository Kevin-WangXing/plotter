#ifndef PLOTTER_H
#define PLOTTER_H

#include <QtGui/QWidget>

class PlotSettings;

class Plotter : public QWidget
{
    Q_OBJECT

public:
    Plotter(QWidget *parent = 0);
    ~Plotter();

    void setPlotSettings(const PlotSettings &settings);

protected:
    void paintEvent(QPaintEvent *event);

private:
    void drawGrid(QPainter *painter);
    void drawCurves(QPainter *painter);

     enum { Margin = 50 };//B�߿�


    QVector<PlotSettings> zoomStack;
    int curZoom;
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
