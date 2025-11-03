import pandas as pd
import matplotlib.pyplot as plt
import os

# Назва файлу з результатами
RESULTS_FILE = "results.csv"
PLOTS_DIR = "plots"

def create_plots(df):
    """
    Створює та зберігає графіки на основі DataFrame з результатами.
    """
    if not os.path.exists(PLOTS_DIR):
        os.makedirs(PLOTS_DIR)

    # Унікальні типи баз даних для побудови
    db_types = df['DB_Type'].unique()
    
    # === Графік: Продуктивність (Операції за 10 сек) ===
    plt.figure(figsize=(12, 7))
    
    for db_type in db_types:
        subset = df[df['DB_Type'] == db_type]
        plt.plot(subset['DB_Size'], subset['Operations'], marker='o', label=db_type)

    plt.title('Продуктивність: Кількість операцій за 10 секунд', fontsize=16)
    plt.xlabel('Розмір бази даних (кількість рядків)', fontsize=12)
    plt.ylabel('Кількість операцій (більше - краще)', fontsize=12)
    plt.xscale('log') # Логарифмічна шкала для розміру
    plt.yscale('log') # Логарифмічна шкала для операцій
    plt.grid(True, which="both", ls="--")
    plt.legend()
    
    plot_path_perf = os.path.join(PLOTS_DIR, 'benchmark_performance.png')
    plt.savefig(plot_path_perf)
    print(f"Графік продуктивності збережено: {plot_path_perf}")
    plt.close()


def main():
    """
    Головна функція: читає CSV та запускає створення графіків.
    """
    try:
        df = pd.read_csv(RESULTS_FILE)
    except FileNotFoundError:
        print(f"Помилка: Файл '{RESULTS_FILE}' не знайдено.")
        print("Будь ласка, спершу запустіть C++ програму для бенчмаркінгу, щоб згенерувати цей файл.")
        return
    except pd.errors.EmptyDataError:
        print(f"Помилка: Файл '{RESULTS_FILE}' порожній.")
        return

    print("CSV з результатами успішно завантажено:")
    print(df)
    print("\nСтворення графіків...")
    create_plots(df)
    print(f"\nГотово. Графік знаходиться в теці '{PLOTS_DIR}'.")

if __name__ == "__main__":
    main()