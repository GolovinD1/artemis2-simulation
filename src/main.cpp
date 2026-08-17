#include <QApplication>
#include "simulation_window.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    QApplication app(argc, argv);
    SimulationWindow window;

    window.setWindowTitle("Artemis 2 - Orbital Simulation");
    window.resize(1000, 800);

    window.show();

    return app.exec();
}