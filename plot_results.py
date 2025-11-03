import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import matplotlib.ticker as ticker

df = pd.read_csv('results.csv')
df['DB_Size'] = df['DB_Size'].astype(int)

plt.figure(figsize=(12, 7))

sns.lineplot(
    data=df,
    x='DB_Size',
    y='Operations_Per_10s',
    hue='DB_Type',
    marker='o',
    dashes=False,
    palette='deep',
    linewidth=2
)

plt.xscale('log')

plt.xticks(
    df['DB_Size'].unique(),
    labels=[f'{x:,}' for x in df['DB_Size'].unique()]
)
plt.gca().get_xaxis().set_major_formatter(ticker.FuncFormatter(lambda x, pos: f'{int(x):,}'))

plt.yscale('log')

formatter = ticker.ScalarFormatter(useMathText=True)
formatter.set_scientific(True)

formatter.set_powerlimits((-1, 9))
plt.gca().yaxis.set_major_formatter(formatter)

plt.title('Database Performance Benchmark (50:10:5 Op1:Op2:Op3)', fontsize=16)
plt.xlabel('Database Size (Number of Students) - Log Scale', fontsize=12)
plt.ylabel('Operations Per 10 Seconds ($10^x$)', fontsize=12)

plt.legend(title='DB Structure', loc='upper right', frameon=True)
plt.grid(True, which="both", ls="--", c='0.7', alpha=0.6)
plt.tight_layout()

plt.savefig('result.png')
print("Plot saved as 'result.png'")
plt.show()