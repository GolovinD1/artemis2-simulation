#define BOOST_TEST_MODULE ArtemisTests
#include <boost/test/included/unit_test.hpp>

#include "../include/math_types.h"
#include "../include/physics.h"

//Тесты для вектора
BOOST_AUTO_TEST_SUITE(MathTests)

BOOST_AUTO_TEST_CASE(VectorLengthAndOps) {
    Vector v1{3000.0, 4000.0, 0.0};

    // Проверяем вычисление длины вектора
    BOOST_CHECK_EQUAL(v1.length(), 5000.0);

    Vector v2{1000.0, 1000.0, 1000.0};
    Vector v3 = v1 + v2;

    // Проверяем перегрузку оператора сложения
    BOOST_CHECK_EQUAL(v3.x, 4000.0);
    BOOST_CHECK_EQUAL(v3.y, 5000.0);
    BOOST_CHECK_EQUAL(v3.z, 1000.0);
}

BOOST_AUTO_TEST_SUITE_END()

//тесты для RK4
BOOST_AUTO_TEST_SUITE(PhysicsTests)

BOOST_AUTO_TEST_CASE(RK4StepValidation) {
    State initial;
    initial.position = {6500000.0, 0.0, 0.0}; // Позиция по X
    initial.velocity = {0.0, 7000.0, 0.0};    // Скорость по Y
    initial.time = 0.0;

    Vector moon_pos{384000000.0, 0.0, 0.0};
    double dt = 1.0;                          // Шаг симуляции - 1сек

    State next_state = rk4_step(initial, moon_pos, dt);

    //Время должно корректно увеличиться на dt
    BOOST_CHECK_EQUAL(next_state.time, 1.0);

    //Позиция по Y должна стать больше нуля
    BOOST_CHECK(next_state.position.y > 0.0);

    //Ускорение свободного падения тянет корабль к центру Земли,
    // поэтому скорость по X должна стать отрицательной
    BOOST_CHECK(next_state.velocity.x < 0.0);

}

BOOST_AUTO_TEST_SUITE_END()