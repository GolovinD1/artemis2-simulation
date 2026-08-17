#include "simulation_window.h"
#include "physics.h"
#include <QWidget>
#include <QVBoxLayout>
#include <fstream>
#include <qcoreevent.h>
#include <qevent.h>
#include <sstream>
#include <QTimer>
#include <QMessageBox>
#include <iostream>
#include <QPushButton>


std::vector<State> parse_horizons(const std::string& filename) {
    std::vector<State> data;
    std::ifstream in(filename);

    if (!in.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::string line;
    while (std::getline(in, line)) {
        if (line == "$$SOE") break;
    }

    double currentTime = 0.0;
    while (std::getline(in, line)) {
        if (line == "$$EOE") break;
        std::string token;
        std::stringstream ss(line);

        std::getline(ss, token, ',');
        std::getline(ss, token, ','); std::string real_date_str = token;
        std::getline(ss, token, ','); double x = std::stod(token) * 1000.0;
        std::getline(ss, token, ','); double y = std::stod(token) * 1000.0;
        std::getline(ss, token, ','); double z = std::stod(token) * 1000.0;

        std::getline(ss, token, ','); double v_x = std::stod(token) * 1000.0;
        std::getline(ss, token, ','); double v_y = std::stod(token) * 1000.0;
        std::getline(ss, token, ','); double v_z = std::stod(token) * 1000.0;

        data.push_back(State{ Vector{x,y,z}, Vector{v_x,v_y,v_z}, currentTime, real_date_str });
        currentTime += 3600.0;
    }
    in.close();
    return data;
}

SimulationWindow::SimulationWindow(QWidget *parent)
    : QWidget{parent}
{
    current_step = 0;
    scene = new QGraphicsScene(-500000, -500000, 1000000, 700000);
    view = new QGraphicsView(scene);
    scene->setBackgroundBrush(Qt::black);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(view);
    this->setFixedSize(1000,810);

    view->fitInView(QRectF(-200000, -350000, 400000, 400000), Qt::KeepAspectRatio);
    view->setDragMode(QGraphicsView::ScrollHandDrag);
    view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->viewport()->installEventFilter(this);

    //Чекбоксы
    QHBoxLayout *layout_Down = new QHBoxLayout();
    QVBoxLayout *layout_Boxes = new QVBoxLayout();
    layout_Down->addLayout(layout_Boxes);
    layout_Down->addStretch();
    redBox = new QCheckBox("Красный Орион", this);
    whiteBox = new QCheckBox("Белый Орион", this);
    layout_Boxes->addWidget(redBox);
    layout_Boxes->addWidget(whiteBox);
    layout->addLayout(layout_Down);

    redBox->setChecked(true);
    whiteBox->setChecked(true);
    connect(redBox, &QCheckBox::toggled, this, [this](bool checked) {
        simulatedOrionItem->setVisible(checked);
        simulatedTailItem->setVisible(checked);

    });
    connect(whiteBox, &QCheckBox::toggled, this, [this](bool checked) {
        realOrionItem->setVisible(checked);
        realTailItem->setVisible(checked);
    });

    //Земля
    earthItem = new QGraphicsEllipseItem(-6371,-6371,12742,12742);
    earthItem->setPen(Qt::NoPen);
    earthItem->setBrush(Qt::blue);
    scene->addItem(earthItem);
    //Луна
    moonItem = new QGraphicsEllipseItem(-1737*2,-1737*2,3474*2,3474*2);
    moonItem->setPen(Qt::NoPen);
    moonItem->setBrush(QColor(0x808080));
    scene->addItem(moonItem);

    moon_data = parse_horizons(RESOURCES_PATH "moon.txt");

    orion_nasa_data = parse_horizons(RESOURCES_PATH "orion.txt");

    if (moon_data.empty() || orion_nasa_data.empty()){
        QMessageBox::critical(this, "Ошибка", "Файлы не найдены!");
        return;
    }

    //поиск конца работы двигателей (TLI)
    size_t peak_velocity_index = 0;
    double max_v = 0.0;

    size_t search_limit = static_cast<size_t>(orion_nasa_data.size() * 0.75);

    for (size_t i = 0; i < search_limit; ++i) {
        double current_v = orion_nasa_data[i].velocity.length();
        if (current_v > max_v) {
            max_v = current_v;
            peak_velocity_index = i;
        }
    }

    tli_index = peak_velocity_index + 40;

    std::cout << "\nПик скорости найден на минуте: " << peak_velocity_index << "\n";
    std::cout << "Двигатели выключаются на минуте: " << tli_index << "\n";

    if (tli_index >= orion_nasa_data.size() || tli_index >= moon_data.size()) {
        QMessageBox::critical(this, "Ошибка", "Файлы слишком короткие для отступа!\n");
        return;
    }

    moonItem->setPos(moon_data[0].position.x/1000, moon_data[0].position.y/1000);

    //настоящий Орион
    realOrionItem = new QGraphicsEllipseItem(-1500*2,-1500*2,3000*2,3000*2);
    realOrionItem->setPen(Qt::NoPen);
    realOrionItem->setBrush(Qt::white);
    scene->addItem(realOrionItem);
    realOrionItem->setPos(orion_nasa_data[0].position.x/1000, orion_nasa_data[0].position.y/1000);

    //Орион, мною посчитанный
    simulatedOrionItem = new QGraphicsEllipseItem(-1500*2,-1500*2,3000*2,3000*2);
    simulatedOrionItem->setPen(Qt::NoPen);
    simulatedOrionItem->setBrush(Qt::red);
    scene->addItem(simulatedOrionItem);
    simulatedOrionItem->setPos(orion_nasa_data[0].position.x/1000, orion_nasa_data[0].position.y/1000);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &SimulationWindow::updatePhysicsStep);
    timer->start(10);

    //Телеметрия

    infoText = new QLabel(this);
    infoText->setGeometry(400, 600, 600, 150);
    QFont font("Consolas");
    font.setPointSize(13);
    font.setBold(true);
    infoText->setStyleSheet("background: transparent;");
    infoText->setFont(font);


    //Траектории

    simulatedTail.moveTo(orion_nasa_data[0].position.x/1000, orion_nasa_data[0].position.y/1000);
    realTail.moveTo(orion_nasa_data[0].position.x/1000, orion_nasa_data[0].position.y/1000);

    simulatedTailItem = new QGraphicsPathItem(simulatedTail);
    realTailItem = new QGraphicsPathItem(realTail);

    simulatedTailItem->setPen(QPen(Qt::red, 0));
    realTailItem->setPen(QPen(Qt::white, 0));

    scene->addItem(simulatedTailItem);
    scene->addItem(realTailItem);

    QPushButton *resetBtn = new QPushButton("Повтор", this);
    layout_Down->addWidget(resetBtn);
    connect(resetBtn, &QAbstractButton::clicked, this, &SimulationWindow::resetSimulation);
    resetBtn->setFixedSize(120,30);
    this->setStyleSheet(
        //Общий фон нижней панели
        "QWidget {"
        "    background-color: #1a1a1a;"
        "}"

        //Стили для чекбоксов
        "QCheckBox {"
        "    color: #e0e0e0;"
        "    font-size: 13px;"
        "    padding: 2px 5px;"
        "}"

        "QCheckBox::indicator {"
        "    width: 15px;"
        "    height: 15px;"
        "}"
        "QMessageBox QLabel {"
        "    color: white;"
        "}"
        //Стили для кнопки рестарта
        "QPushButton {"
        "    background-color: #333333;"
        "    color: white;"
        "    border: 1px solid #555555;"
        "    border-radius: 4px;"
        "    padding: 6px 15px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #4a4a4a;"
        "    border: 1px solid #777777;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #222222;"
        "}"
        );
}

void SimulationWindow::updatePhysicsStep()
{
    int speed_multiplier = 7;

    for (int m = 0; m < speed_multiplier; ++m) {
        Vector current_moon_pos;
        if (current_step < moon_data.size()) {
            current_moon_pos = moon_data[current_step].position;
        }
        else {
            current_moon_pos = moon_data.back().position;
        }

        moonItem->setPos(current_moon_pos.x/1000, current_moon_pos.y/1000);
        if (current_step < orion_nasa_data.size()){
            realOrionItem->setPos(orion_nasa_data[current_step].position.x/1000, orion_nasa_data[current_step].position.y/1000);
            realTail.lineTo(orion_nasa_data[current_step].position.x/1000,orion_nasa_data[current_step].position.y/1000);
        }

        if(current_step<=tli_index)
        {
            simulatedOrionItem->setPos(orion_nasa_data[current_step].position.x/1000, orion_nasa_data[current_step].position.y/1000);
            simulatedTail.lineTo(orion_nasa_data[current_step].position.x/1000,orion_nasa_data[current_step].position.y/1000);
            simulated_orion = orion_nasa_data[current_step];
        }else{
            for(size_t j = 0; j < steps_per_minute; j++){
                simulated_orion = rk4_step(simulated_orion, current_moon_pos, dt_physics);
            }
            simulatedOrionItem->setPos(simulated_orion.position.x/1000, simulated_orion.position.y/1000);
            simulatedTail.lineTo(simulated_orion.position.x/1000,simulated_orion.position.y/1000);
        }

        double distance_to_earth = simulated_orion.position.length();

        if (distance_to_earth <= REENTRY_RADIUS) {
            QMessageBox::information(this, "Отчет о миссии", QString("Орион вошел в атмосферу Земли на %1 минуте миссии").arg(current_step));
            timer->stop();
            return;
        }

        if (current_step > tli_index + 43200) {
            QMessageBox::information(this, "Отчет о миссии", QString("Орион промахнулся мимо Земли и улетел в открытый космос"));
            timer->stop();
            return;
        }

        current_step++;
    }

    simulatedTailItem->setPath(simulatedTail);
    realTailItem->setPath(realTail);

    int safe_step = 0;
    QString dashboardText = "<table cellspacing='0' cellpadding='10' style='border: 2px solid #00FF00; background-color: #001500;'><tr><td>";

    if(whiteBox->isChecked()){

        if(current_step < orion_nasa_data.size())   safe_step = current_step;
        else safe_step = orion_nasa_data.size() - 1;

        int days = safe_step/1440;
        int hours = (safe_step/60) % 24;
        int mins = safe_step%60;
        dashboardText += QString("<span style='color: white;'>■</span> <span style='color: green;'>Орион (Реальный): Время полета (T+): %1 дн. %2 ч. %3 мин. </span>").arg(days, 2, 10, QChar('0')).arg(hours, 2, 10, QChar('0')).arg(mins, 2, 10, QChar('0')) + QString("<br><span style='color: green;'>Скорость: %1 км/с</span>").arg(orion_nasa_data[safe_step].velocity.length()/1000);
    }
    if(redBox->isChecked()){
        int days = current_step / 1440;
        int hours = (current_step / 60) % 24;
        int mins = current_step % 60;
        dashboardText += QString("<br><span style='color: red;'>■</span> <span style='color: green;'>Орион (Симуляция): Время полета (T+): %1 дн. %2 ч. %3 мин. </span>").arg(days, 2, 10, QChar('0')).arg(hours, 2, 10, QChar('0')).arg(mins, 2, 10, QChar('0')) + QString("<br><span style='color: green;'>Скорость: %1 км/с</span>").arg(simulated_orion.velocity.length()/1000);
    }
    dashboardText += "</td></tr></table>";
    infoText->setText(dashboardText);
}

void SimulationWindow::resetSimulation(){
    current_step = 0;
    simulated_orion = orion_nasa_data[0];

    realTail.clear();
    simulatedTail.clear();
    simulatedTail.moveTo(orion_nasa_data[0].position.x/1000, orion_nasa_data[0].position.y/1000);
    realTail.moveTo(orion_nasa_data[0].position.x/1000, orion_nasa_data[0].position.y/1000);

    timer->start(10);
}


bool SimulationWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == view->viewport() && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        if (wheelEvent->angleDelta().y() > 0) {
            view->scale(1.1, 1.1);
        } else if (wheelEvent->angleDelta().y() < 0) {
            view->scale(0.9, 0.9);
        }
        return true;
    }
    return QWidget::eventFilter(obj, event);
}
