#pragma once
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem> // Для отрисовки хвостов-траекторий
#include <QGraphicsTextItem> // Для вывода текста (скорость, дата)
#include <QTimer>
#include <QLabel>
#include <vector>
#include "physics.h"
#include <QCheckBox>


class SimulationWindow : public QWidget {
    Q_OBJECT

public:
    SimulationWindow(QWidget *parent = nullptr);

private slots:
    void updatePhysicsStep();
    void resetSimulation();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    // холст
    QGraphicsScene *scene;
    QGraphicsView *view;

    // тела
    QGraphicsEllipseItem *earthItem;
    QGraphicsEllipseItem *moonItem;

    // орион
    QGraphicsEllipseItem *simulatedOrionItem; // красный
    QGraphicsEllipseItem *realOrionItem;      // белый

    // траектории
    QGraphicsPathItem *simulatedTailItem; // оранжевый след
    QGraphicsPathItem *realTailItem;// зеленый след
    QPainterPath realTail;
    QPainterPath simulatedTail;

    // текстовая телеметрия
    QLabel *infoText;

    // данные
    QTimer *timer;
    std::vector<State> moon_data;
    std::vector<State> orion_nasa_data;

    QCheckBox *redBox;
    QCheckBox *whiteBox;

    State simulated_orion;
    size_t current_step;
    size_t tli_index;
    double dt_physics = 0.1;
    const double REENTRY_RADIUS = 6371000.0 + 120000.0; // Радиус Земли 6371 км + атмосфера 120 км
    int steps_per_minute = 600;
};