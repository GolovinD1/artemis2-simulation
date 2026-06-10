import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.widgets import Button
from datetime import datetime, timedelta


df_sim = pd.read_csv('../data/my_simulation.csv').dropna()
df_real = pd.read_csv('../data/real_artemis2.csv').dropna()

df_sim = df_sim.iloc[::10].reset_index(drop=True)
df_real = df_real.iloc[::10].reset_index(drop=True)

# Переводим метры в километры
x_sim = df_sim['X'] / 1000.0
y_sim = df_sim['Y'] / 1000.0
x_moon_sim = df_sim['MoonX'] / 1000.0
y_moon_sim = df_sim['MoonY'] / 1000.0

x_real = df_real['X'] / 1000.0
y_real = df_real['Y'] / 1000.0
x_moon_real = df_real['MoonX'] / 1000.0
y_moon_real = df_real['MoonY'] / 1000.0

mission_start = datetime(2026, 4, 2, 0, 0)


# СЦЕНА

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 8))
fig.patch.set_facecolor('black')
plt.subplots_adjust(bottom=0.15)

camera_limit = 500000 

for ax, title in zip([ax1, ax2], ['Моя симуляция', 'Настоящий путь (NASA Horizons)']):
    ax.set_facecolor('black')
    ax.set_xlim(-camera_limit, camera_limit)
    ax.set_ylim(-camera_limit, camera_limit)
    ax.set_aspect('equal')
    ax.set_title(title, color='white', fontsize=14, pad=15)
    ax.tick_params(colors='white')


# 3. ОБЪЕКТЫ

# ЛЕВЫЙ ЭКРАН
earth1 = plt.Circle((0, 0), 6371, color='dodgerblue', zorder=5)
moon1 = plt.Circle((0, 0), 1737, color='lightgray', zorder=4)
ax1.add_patch(earth1)
ax1.add_patch(moon1)
tail1, = ax1.plot([], [], color='orange', lw=1.5, alpha=0.7, label='След Ориона')
ship1, = ax1.plot([], [], 'o', color='red', markersize=6, zorder=6, label='Орион')
time_text1 = ax1.text(0.05, 0.95, '', transform=ax1.transAxes, color='white', fontsize=12)
speed_text1 = ax1.text(0.05, 0.90, '', transform=ax1.transAxes, color='white', fontsize=12)
ax1.legend(facecolor='black', labelcolor='white', loc='lower left')

# ПРАВЫЙ ЭКРАН
earth2 = plt.Circle((0, 0), 6371, color='dodgerblue', zorder=5)
moon2 = plt.Circle((0, 0), 1737, color='lightgray', zorder=4)
ax2.add_patch(earth2)
ax2.add_patch(moon2)
tail2, = ax2.plot([], [], color='springgreen', lw=1.5, alpha=0.7, label='След NASA')
ship2, = ax2.plot([], [], 'o', color='white', markersize=6, zorder=6, label='Орион (Реальный)')
time_text2 = ax2.text(0.05, 0.95, '', transform=ax2.transAxes, color='white', fontsize=12)
speed_text2 = ax2.text(0.05, 0.90, '', transform=ax2.transAxes, color='white', fontsize=12)
ax2.legend(facecolor='black', labelcolor='white', loc='lower left')


max_frames = max(len(df_sim), len(df_real))

def update(frame):
    # ЛЕВЫЙ ЭКРАН
    idx1 = min(frame, len(df_sim) - 1)
    moon1.center = (x_moon_sim.iloc[idx1], y_moon_sim.iloc[idx1])
    
    if idx1 == 0:
        tail1.set_data(x_sim.iloc[:2], y_sim.iloc[:2])
    else:
        tail1.set_data(x_sim.iloc[:idx1+1], y_sim.iloc[:idx1+1])
        
    ship1.set_data([x_sim.iloc[idx1]], [y_sim.iloc[idx1]])
    
    sim_date = mission_start + timedelta(minutes=int(df_sim['Minute'].iloc[idx1]))
    time_text1.set_text(f'Дата: {sim_date.strftime("%Y-%m-%d %H:%M")}')
    speed_text1.set_text(f'Скорость: {df_sim["Speed"].iloc[idx1]:.2f} км/с')

    # ПРАВЫЙ ЭКРАН
    idx2 = min(frame, len(df_real) - 1)
    moon2.center = (x_moon_real.iloc[idx2], y_moon_real.iloc[idx2])
    
    if idx2 == 0:
        tail2.set_data(x_real.iloc[:2], y_real.iloc[:2])
    else:
        tail2.set_data(x_real.iloc[:idx2+1], y_real.iloc[:idx2+1])
        
    ship2.set_data([x_real.iloc[idx2]], [y_real.iloc[idx2]])
    
    # Читаем точную дату из файла C++
    time_text2.set_text(f'Дата:{df_real["Date"].iloc[idx2]}')
    # Читаем точную скорость из файла C++ (без каких-либо конвертаций)
    speed_text2.set_text(f'Скорость: {df_real["Speed"].iloc[idx2]:.2f} км/с')

    if frame == max_frames - 1:
        ani.pause()

    return moon1, tail1, ship1, time_text1, speed_text1, moon2, tail2, ship2, time_text2, speed_text2

ani = FuncAnimation(fig, update, frames=max_frames, interval=5, blit=True)

ax_button = plt.axes([0.45, 0.05, 0.1, 0.05])
btn = Button(ax_button, 'Повторить', color='#333333', hovercolor='#555555')
btn.label.set_color('white')

def replay(event):
    tail1.set_data([], [])
    tail2.set_data([], [])
    ani.frame_seq = ani.new_frame_seq()
    ani.resume()

btn.on_clicked(replay)

plt.show()