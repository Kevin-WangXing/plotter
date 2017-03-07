#include <QtGui/QApplication>
#include "plotter.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Plotter plotter;

    PlotSettings settings;
    settings.minX = 0.0;
    settings.maxX = 100.0;
    settings.minY = 0.0;
    settings.maxY = 100.0;

    plotter.setPlotSettings(settings);

    plotter.show();

    return a.exec();
}
