#ifndef PLOTTER_H
#define PLOTTER_H

#include <QtGui/QWidget>
#include <QPixmap>
#include <QMap>
#include <QMouseEvent>
#include <QToolButton>

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

    QSize minimumSize() const;//设置最小窗口
    QSize sizeHint() const;//设置最合理窗口布局
private slots:
    void zoomIn();//放大
    void zoomOut();//缩小

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    void updateRubberBandRegion();
    void refreshPixmap();
    void drawGrid(QPainter *painter);
    void drawCurves(QPainter *painter);

     enum { Margin = 50 };//B边框

     QToolButton *zoomInButton;
     QToolButton *zoomOutButton;

    QVector<PlotSettings> zoomStack;
    int curZoom;

    QPixmap pixmap;
    QMap<int, QVector<QPointF> > curveMap;

    bool rubberBandIsShown;//是否显示矩形
    QRect rubberBandRect;//矩形大小
};

class PlotSettings
{
public:
    PlotSettings();

    void scroll(int dx, int dy);
    void adjust();

    double spanX() const { return maxX - minX; }//左右跨度
    double spanY() const { return maxY - minY; }//上下跨度

    int numXTicks;//刻度线数量
    int numYTicks;

    double minX;//X轴最小最大
    double maxX;

    double minY;//Y轴最小最大
    double maxY;

private:
    static void adjustAxis(double &min, double &max, int &numTicks);
};

#endif // PLOTTER_H
