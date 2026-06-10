#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <clocale>

#include "math_types.h"
#include "physics.h"


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

int main() {
    setlocale(LC_ALL, "rus");
    setlocale(LC_NUMERIC, "C");

    try {
        std::cout << "Загрузка Луны...\n";
        std::vector<State> moon_data = parse_horizons("data/moon.txt");

        std::cout << "Загрузка Ориона...\n";
        std::vector<State> orion_nasa_data = parse_horizons("data/orion.txt");

        if (moon_data.empty() || orion_nasa_data.empty()) return 1;

        //поиск конца работы двигателей (TLI)
        size_t peak_velocity_index = 0;
        double max_v = 0.0;

        size_t search_limit = orion_nasa_data.size() * 0.75;

        for (size_t i = 0; i < search_limit; ++i) {
            double current_v = orion_nasa_data[i].velocity.length();
            if (current_v > max_v) {
                max_v = current_v;
                peak_velocity_index = i;
            }
        }

        size_t tli_index = peak_velocity_index + 40;

        std::cout << "\nПик скорости найден на минуте: " << peak_velocity_index << "\n";
        std::cout << "Двигатели выключаются на минуте: " << tli_index << "\n";

        if (tli_index >= orion_nasa_data.size() || tli_index >= moon_data.size()) {
            std::cerr << "Ошибка: Файлы слишком короткие для отступа!\n";
            return 1;
        }

        std::ofstream out("data/my_simulation.csv");
        out << "X,Y,Z,Vx,Vy,Vz,MoonX,MoonY,MoonZ,Speed,Minute\n";
        std::ofstream out2("data/real_artemis2.csv");
        out2 << "X,Y,Z,Vx,Vy,Vz,MoonX,MoonY,MoonZ,Speed,Date\n";

        std::cout << "1: Запись реальной траектории\n";

        for (size_t i = 0; i < tli_index; ++i) {
            double v_km = orion_nasa_data[i].velocity.length() / 1000.0;
            out << orion_nasa_data[i].position.x << ","
                << orion_nasa_data[i].position.y << ","
                << orion_nasa_data[i].position.z << ","
                << orion_nasa_data[i].velocity.x << ","
                << orion_nasa_data[i].velocity.y << ","
                << orion_nasa_data[i].velocity.z << ","
                << moon_data[i].position.x << ","
                << moon_data[i].position.y << ","
                << moon_data[i].position.z << ","
                << v_km << "," 
                << i << "\n";  
        }

        for (size_t i = 0; i < orion_nasa_data.size(); ++i) {
            double v_km = orion_nasa_data[i].velocity.length() / 1000.0;
            out2 << orion_nasa_data[i].position.x << ","
                << orion_nasa_data[i].position.y << ","
                << orion_nasa_data[i].position.z << ","
                << orion_nasa_data[i].velocity.x << ","
                << orion_nasa_data[i].velocity.y << ","
                << orion_nasa_data[i].velocity.z << ","
                << moon_data[i].position.x << ","
                << moon_data[i].position.y << ","
                << moon_data[i].position.z << ","
                << v_km << ","  
                << orion_nasa_data[i].date << "\n";
        }

        std::cout << "2: Запуск симуляции до возвращения на Землю\n";

        State simulated_orion = orion_nasa_data[tli_index];
        double dt_physics = 0.1;
        int steps_per_minute = 600;

        // Радиус Земли 6371 км + атмосфера 120 км
        const double REENTRY_RADIUS = 6371000.0 + 120000.0;

        size_t current_step = tli_index;

        while (true) {

            Vector current_moon_pos;
            if (current_step < moon_data.size()) {
                current_moon_pos = moon_data[current_step].position;
            }
            else {
                current_moon_pos = moon_data.back().position;
            }

            double v_km = simulated_orion.velocity.length() / 1000.0;

            out << simulated_orion.position.x << ","
                << simulated_orion.position.y << ","
                << simulated_orion.position.z << ","
                << simulated_orion.velocity.x << ","
                << simulated_orion.velocity.y << ","
                << simulated_orion.velocity.z << ","
                << current_moon_pos.x << ","
                << current_moon_pos.y << ","
                << current_moon_pos.z << ","
                << v_km << ","
                << current_step << "\n";

            for (int j = 0; j < steps_per_minute; ++j) {
                simulated_orion = rk4_step(simulated_orion, current_moon_pos, dt_physics);
            }

            double distance_to_earth = simulated_orion.position.length();

            if (distance_to_earth <= REENTRY_RADIUS) {
                std::cout << "\nОрион вошел в атмосферу Земли на " << current_step << " минуте миссии" << "\n";
                break; 
            }

            if (current_step > tli_index + 43200) {
                std::cout << "\nОрион промахнулся мимо Земли и улетел в открытый космос\n";
                break;
            }

            current_step++;
        }

        std::cout << "Симуляция успешно завершена\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Симуляция прервана: " << e.what() << '\n';
        return 1;
    }

    return 0;
}