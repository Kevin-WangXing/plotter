#include <QtGui/QApplication>
#include "plotter.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Plotter plotter;

    int numPoints = 100;//100�������
    QVector<QPointF> points0;//�������ߣ���һ��
    QVector<QPointF> points1;//�������ߣ��ڶ���

    for(int x = 0; x < numPoints; x++)
    {
        points0.append(QPointF(x, uint(qrand()) % 100));
        points1.append(QPointF(x, uint(qrand()) % 100));
    }

    plotter.setCurveData(0, points0);
    plotter.setCurveData(1, points1);

    PlotSettings settings;
    settings.minX = 0.0;
    settings.maxX = 100.0;
    settings.minY = 0.0;
    settings.maxY = 100.0;

    plotter.setPlotSettings(settings);

    plotter.show();

    return a.exec();
}
