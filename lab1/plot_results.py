#!/usr/bin/env python3
"""
График пропускной способности коллектора метрик
SpacemiT K1 RISC-V Benchmark Results
Использование: python plot_results.py
"""

import matplotlib.pyplot as plt
import numpy as np

# Данные замеров (млн оп/сек) - SpacemiT K1, GCC 14.2.0, C++20
threads = [1, 2, 4, 8, 16]

# Этап 0: Однопоточный baseline (только для 1 потока)
stage0_baseline = [9.48, None, None, None, None]

# Этап 1: Пустой лок (без записи)
stage1_empty = [5.56, 1.90, 1.99, 1.58, 1.58]

# Этап 1: Единый общий лок
stage1_mutex = [4.51, 1.12, 1.24, 0.99, 0.95]

# Этап 2: Шардированный лок (16 групп)
stage2_sharded = [3.22, 2.49, 2.44, 2.42, 2.31]

# Этап 3: Thread-Local состояние
stage3_tls = [2.94, 5.89, 10.48, 19.45, 18.42]

# Этап 4: Двойная буферизация
stage4_double_buf = [3.68, 6.79, 13.11, 23.17, 21.88]

# Создание графика
plt.figure(figsize=(14, 9))

# Линия идеального линейного масштабирования (от baseline)
if stage0_baseline[0] > 0:
    ideal_scaling = [stage0_baseline[0] * t for t in threads]
    plt.plot(threads, ideal_scaling, 'k--', label='Идеальное масштабирование', alpha=0.5, linewidth=2)

# Построение графиков для каждого этапа
plt.plot(threads, stage1_empty, 'o-', label='Этап 1: Пустой лок', linewidth=2.5, markersize=9, color='#FF6B6B')
plt.plot(threads, stage1_mutex, 's-', label='Этап 1: Единый лок', linewidth=2.5, markersize=9, color='#C44569')
plt.plot(threads, stage2_sharded, '^-', label='Этап 2: Шардированный лок', linewidth=2.5, markersize=9, color='#FFA502')
plt.plot(threads, stage3_tls, 'd-', label='Этап 3: Thread-Local (TLS)', linewidth=2.5, markersize=9, color='#26C281')
plt.plot(threads, stage4_double_buf, 'v-', label='Этап 4: Двойная буферизация', linewidth=2.5, markersize=9, color='#4ECDC4')

# Настройки графика
plt.xlabel('Число потоков', fontsize=14, fontweight='bold')
plt.ylabel('Пропускная способность (млн оп/сек)', fontsize=14, fontweight='bold')
plt.title('Производительность коллектора метрик\nSpacemiT K1 RISC-V • GCC 14.2.0 • C++20', fontsize=16, fontweight='bold', pad=20)
plt.grid(True, alpha=0.3, linestyle='--')
plt.legend(loc='upper left', fontsize=11, framealpha=0.95)
plt.xticks(threads, fontsize=12)
plt.yticks(fontsize=12)

# Добавим аннотации для ключевых точек
# Пик Stage 4 на 8 потоках
plt.annotate(f'{stage4_double_buf[3]:.2f}', 
             xy=(8, stage4_double_buf[3]), 
             xytext=(9, stage4_double_buf[3] + 2),
             fontsize=10, 
             bbox=dict(boxstyle='round,pad=0.5', facecolor='yellow', alpha=0.7))

# Падение mutex на 16 потоках
plt.annotate(f'{stage1_mutex[4]:.2f}', 
             xy=(16, stage1_mutex[4]), 
             xytext=(14, stage1_mutex[4] - 2),
             fontsize=10,
             bbox=dict(boxstyle='round,pad=0.5', facecolor='pink', alpha=0.7))

# Сохранение графика
plt.tight_layout()
plt.savefig('benchmark_results.png', dpi=300, bbox_inches='tight')
print("✓ График сохранён в benchmark_results.png")

# Вывод данных в виде таблицы для проверки
print("\n" + "="*85)
print("ТАБЛИЦА РЕЗУЛЬТАТОВ")
print("="*85)
print(f"{'Этап':<42} | {' | '.join([f'{t:>6}T' for t in threads])}")
print("-" * 85)
print(f"{'Baseline (однопоточный)':<42} | {stage0_baseline[0]:>7.2f} | {'—':>7} | {'—':>7} | {'—':>7} | {'—':>7}")
print(f"{'Этап 1: Пустой лок':<42} | {' | '.join([f'{v:>7.2f}' if v else '   —   ' for v in stage1_empty])}")
print(f"{'Этап 1: Единый лок':<42} | {' | '.join([f'{v:>7.2f}' if v else '   —   ' for v in stage1_mutex])}")
print(f"{'Этап 2: Шардированный лок':<42} | {' | '.join([f'{v:>7.2f}' if v else '   —   ' for v in stage2_sharded])}")
print(f"{'Этап 3: Thread-Local':<42} | {' | '.join([f'{v:>7.2f}' if v else '   —   ' for v in stage3_tls])}")
print(f"{'Этап 4: Двойная буферизация':<42} | {' | '.join([f'{v:>7.2f}' if v else '   —   ' for v in stage4_double_buf])}")
print("="*85)

# Расчёт коэффициента масштабирования (speedup)
print("\n" + "="*85)
print("КОЭФФИЦИЕНТ МАСШТАБИРОВАНИЯ (Speedup относительно 1 потока)")
print("="*85)
for name, data, baseline in [
    ("Этап 3: Thread-Local", stage3_tls, stage3_tls[0]),
    ("Этап 4: Двойная буферизация", stage4_double_buf, stage4_double_buf[0])
]:
    if baseline > 0:
        speedups = [d / baseline if d else 0 for d in data]
        efficiencies = [(s / t) * 100 for s, t in zip(speedups, threads)]
        print(f"\n{name}:")
        print(f"  Speedup:     {' | '.join([f'{s:>6.2f}x' for s in speedups])}")
        print(f"  Efficiency:  {' | '.join([f'{e:>6.1f}%' for e in efficiencies])}")

# Интересные наблюдения
print("\n" + "="*85)
print("КЛЮЧЕВЫЕ НАБЛЮДЕНИЯ (RISC-V SpacemiT K1)")
print("="*85)

mutex_slowdown = stage0_baseline[0] / stage1_mutex[4]
print(f"• Mutex 16T: {mutex_slowdown:.1f}x медленнее baseline (катастрофический коллапс)")

stage3_peak_speedup = stage3_tls[3] / stage3_tls[0]
stage4_peak_speedup = stage4_double_buf[3] / stage4_double_buf[0]
print(f"• Stage 3 пик на 8T: {stage3_peak_speedup:.2f}x speedup (эффективность {(stage3_peak_speedup/8)*100:.1f}%)")
print(f"• Stage 4 пик на 8T: {stage4_peak_speedup:.2f}x speedup (эффективность {(stage4_peak_speedup/8)*100:.1f}%)")

stage4_vs_stage3_ratio = stage4_double_buf[3] / stage3_tls[3]
print(f"• Stage 4 vs Stage 3 на 8T: {stage4_vs_stage3_ratio:.2f}x быстрее (неожиданно!)")
print(f"  → Seq_cst протокол на RISC-V K1 работает эффективнее, чем relaxed на x86")

scaling_8_to_16_s3 = stage3_tls[4] / stage3_tls[3]
scaling_8_to_16_s4 = stage4_double_buf[4] / stage4_double_buf[3]
print(f"• Масштабирование 8T→16T: Stage 3 = {scaling_8_to_16_s3:.2f}x, Stage 4 = {scaling_8_to_16_s4:.2f}x")
print(f"  → Замедление после 8 потоков (вероятно 8 физических ядер на K1)")

print("="*85)
print("\n✓ Анализ завершён!\n")
