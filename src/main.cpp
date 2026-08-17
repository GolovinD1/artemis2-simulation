#include <QApplication>
#include "simulation_window.h"
#include <windows.h>

int main(int argc, char *argv[])
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    QApplication app(argc, argv);
    SimulationWindow window;

    window.setWindowTitle("Artemis 2 - Orbital Simulation");
    window.resize(1000, 800);

    window.show();

    return app.exec();
}