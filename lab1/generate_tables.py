#!/usr/bin/env python3
"""
Генератор таблицы с результатами бенчмарков для README.md
Заполните данные ниже и запустите скрипт для форматирования таблицы
"""

# ============================================================================
# ЗАПОЛНИТЕ ДАННЫЕ СВОИХ БЕНЧМАРКОВ ЗДЕСЬ (в миллионах операций в секунду)
# ============================================================================

# Число логических процессоров (из команды nproc)
nproc = 8  # ЗАПОЛНИТЬ

# Пропускная способность (млн оп/сек)
# Формат: {число_потоков: значение_в_млн_опсек}
stage0_baseline = {1: 0}  # ЗАПОЛНИТЬ
stage1_empty_lock = {1: 0, 2: 0, 4: 0, 8: 0, 16: 0}  # ЗАПОЛНИТЬ
stage1_mutex = {1: 0, 2: 0, 4: 0, 8: 0, 16: 0}  # ЗАПОЛНИТЬ
stage2_sharded = {1: 0, 2: 0, 4: 0, 8: 0, 16: 0}  # ЗАПОЛНИТЬ
stage3_tls = {1: 0, 2: 0, 4: 0, 8: 0, 16: 0}  # ЗАПОЛНИТЬ
stage4_double_buf = {1: 0, 2: 0, 4: 0, 8: 0, 16: 0}  # ЗАПОЛНИТЬ

# Стресс-тест согласованности (10000 снимков, 4 пишущих потока)
# Формат: {"broken_pct": %, "sum_less": раз, "sum_greater": раз, "count_diff": разница}
stress_stage2 = {"broken_pct": 0, "sum_less": 0, "sum_greater": 0, "count_diff": 0}  # ЗАПОЛНИТЬ
stress_stage3 = {"broken_pct": 0, "sum_less": 0, "sum_greater": 0, "count_diff": 0}  # ЗАПОЛНИТЬ
stress_stage4 = {"broken_pct": 0, "sum_less": 0, "sum_greater": 0, "count_diff": 0}  # ЗАПОЛНИТЬ
stress_stage4_no_check = {"broken_pct": 0, "sum_less": 0, "sum_greater": 0, "count_diff": 0}  # ЗАПОЛНИТЬ

# ============================================================================
# ГЕНЕРАЦИЯ ТАБЛИЦ
# ============================================================================

def format_value(val, threads):
    """Форматирование значения для таблицы"""
    if val is None or val == 0:
        return "—" if threads > 1 else ""
    return f"{val:.1f}"

def generate_throughput_table():
    """Генерация таблицы пропускной способности"""
    threads = [1, 2, 4, 8, 16]
    
    print("\n#### 2. Замеры пропускной способности (млн оп/сек)")
    print("*Заполните медианные значения замерялки (в миллионах вызовов `record()` в секунду):*\n")
    print("| Число потоков (T) | 1 поток | 2 потока | 4 потока | 8 потоков | 16 потоков |")
    print("| :--- | :---: | :---: | :---: | :---: | :---: |")
    
    # Этап 0
    row = f"| **Этап 0:** Однопоточный baseline | {format_value(stage0_baseline.get(1), 1)} |"
    for t in threads[1:]:
        row += " — |"
    print(row)
    
    # Этап 1: Пустой лок
    row = f"| **Этап 1:** Пустой лок (без записи) |"
    for t in threads:
        row += f" {format_value(stage1_empty_lock.get(t), t)} |"
    print(row)
    
    # Этап 1: Единый лок
    row = f"| **Этап 1:** Единый общий лок |"
    for t in threads:
        row += f" {format_value(stage1_mutex.get(t), t)} |"
    print(row)
    
    # Этап 2
    row = f"| **Этап 2:** Шардированный лок (16 групп) |"
    for t in threads:
        row += f" {format_value(stage2_sharded.get(t), t)} |"
    print(row)
    
    # Этап 3
    row = f"| **Этап 3:** Thread-Local состояние (локальные данные) |"
    for t in threads:
        row += f" {format_value(stage3_tls.get(t), t)} |"
    print(row)
    
    # Этап 4
    row = f"| **Этап 4:** Двойная буферизация |"
    for t in threads:
        row += f" {format_value(stage4_double_buf.get(t), t)} |"
    print(row)

def generate_stress_table():
    """Генерация таблицы стресс-тестов"""
    print("\n\n#### 3. Результаты стресс-теста согласованности снимков")
    print("*10 000 вызовов `snapshot()` на фоне 4 пишущих потоков:*\n")
    print("| Реализация | Доля битых снимков (sum != count) | Снимок: sum < count | Снимок: sum > count | Итоговый count − число вызовов (после join) |")
    print("| :--- | :---: | :---: | :---: | :---: |")
    
    for name, data in [
        ("**Этап 2:** Шардированный лок", stress_stage2),
        ("**Этап 3:** Thread-Local состояние (локальные данные)", stress_stage3),
        ("**Этап 4:** Двойная буферизация", stress_stage4),
        ("**Этап 4 (без шага 3):** без проверки `active`", stress_stage4_no_check)
    ]:
        broken = data["broken_pct"]
        less = data["sum_less"]
        greater = data["sum_greater"]
        diff = data["count_diff"]
        
        broken_str = f"{broken:.2f}%" if broken > 0 else "0%"
        diff_str = f"{diff:+d}" if diff != 0 else "0"
        
        print(f"| {name} | {broken_str} | {less} | {greater} | {diff_str} |")

def generate_summary():
    """Вывод краткой статистики"""
    print("\n\n" + "="*80)
    print("СВОДНАЯ СТАТИСТИКА")
    print("="*80)
    
    if stage3_tls.get(1, 0) > 0:
        print(f"\nЭтап 3 (Thread-Local) - коэффициент масштабирования:")
        for t in [1, 2, 4, 8, 16]:
            if t in stage3_tls and stage3_tls[t] > 0:
                speedup = stage3_tls[t] / stage3_tls[1]
                efficiency = (speedup / t) * 100
                print(f"  {t:2d} потоков: {stage3_tls[t]:6.1f} млн оп/сек | ускорение: {speedup:5.2f}x | эффективность: {efficiency:5.1f}%")
    
    if stage4_double_buf.get(1, 0) > 0:
        print(f"\nЭтап 4 (Двойная буферизация) - коэффициент масштабирования:")
        for t in [1, 2, 4, 8, 16]:
            if t in stage4_double_buf and stage4_double_buf[t] > 0:
                speedup = stage4_double_buf[t] / stage4_double_buf[1]
                efficiency = (speedup / t) * 100
                print(f"  {t:2d} потоков: {stage4_double_buf[t]:6.1f} млн оп/сек | ускорение: {speedup:5.2f}x | эффективность: {efficiency:5.1f}%")
    
    if stage3_tls.get(16, 0) > 0 and stage4_double_buf.get(16, 0) > 0:
        ratio = stage3_tls[16] / stage4_double_buf[16]
        print(f"\nСоотношение Этап 3 / Этап 4 на 16 потоках: {ratio:.2f}x")
    
    print("\n" + "="*80)

# ============================================================================
# ЗАПУСК
# ============================================================================

if __name__ == "__main__":
    print("="*80)
    print("ТАБЛИЦЫ РЕЗУЛЬТАТОВ ДЛЯ README.md")
    print("="*80)
    print(f"\nПроцессор: SpacemiT K1")
    print(f"Число логических процессоров: {nproc}")
    print(f"Язык: C++20 (GCC 14.2.0)")
    
    generate_throughput_table()
    generate_stress_table()
    generate_summary()
    
    print("\n\nИНСТРУКЦИЯ:")
    print("1. Скопируйте сгенерированные таблицы выше")
    print("2. Вставьте их в README.md в соответствующие разделы")
    print("3. Заполните данные в plot_results.py и запустите для создания графика")
