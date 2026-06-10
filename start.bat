@echo off
:: Включаем поддержку русского языка в консоли
chcp 65001 > nul 


echo       ЗАПУСК C++ КОДА - ВЫЧИСЛЕНИЯ

echo.

:: Ищем скомпилированный файл
  if exist "..\x64\Debug\artemis2.exe" (
    "..\x64\Debug\artemis2.exe"
) else (
    echo Файл не найден! 
    pause
    exit /b
)

echo.

echo       ЗАПУСК PYTHON КОДА - ВИЗУАЛИЗАЦИЯ

echo.

:: Запускаем скрипт из папки scripts

cd scripts
python plot_orbit.py
cd ..

pause