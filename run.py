import os
import subprocess
import sys

def run_step(cmd, step_name):
    """Вспомогательная функция для запуска команд с проверкой ошибок"""
    print("\n")
    print(f"Этап: {step_name}")
    print("\n")
    
    result = subprocess.run(cmd)
    if result.returncode != 0:
        print(f"\nОшибка! Выполнение прервано на этапе: {step_name}")
        sys.exit(1)

def main():
    if not os.path.exists(os.path.join("build", "CMakeCache.txt")):
        run_step(["cmake", "-B", "build"], "Первичная конфигурация проекта")

    run_step(["cmake", "--build", "build"], "Сборка C++ кода")

    # Поиск и запуск исполняемого файла
    exe_name = "artemis_sim.exe" if os.name == 'nt' else "artemis_sim"
    exe_path = None
    
    for root, dirs, files in os.walk("build"):
        if exe_name in files:
            exe_path = os.path.join(root, exe_name)
            break
            
    if not exe_path:
        print(f"\nОшибка! Не удалось найти скомпилированный файл {exe_name} в папке build/")
        sys.exit(1)

    run_step([exe_path], "Вычисление физики и генерация CSV")

    # Запуск скрипта визуализации
    script_path = os.path.join("scripts", "plot_orbit.py")
    if not os.path.exists(script_path):
        print(f"\nОшибка! Не найден скрипт визуализации по пути: {script_path}")
        sys.exit(1)
        
    run_step([sys.executable, script_path], "Отрисовка (Python)")

if __name__ == "__main__":
    main()