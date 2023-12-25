import sys
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

# Read input csv file
input = sys.argv[1]
df1 = pd.read_csv(input)
df2 = df1.copy()

# figsize
fig = plt.figure(figsize=(18, 10))
ax1 = plt.subplot2grid((1, 3), (0, 0), colspan=2)
ax2 = plt.subplot2grid((1, 3), (0, 2))

# replace tm value from 0 with 62.5 and remve 0 in tm column
df1['tm'] = df1['tm'].replace(0, 62.5)
df2 = df2[df2['tm'] != 0]

# Plot heatmap and color with value from 40 to 85 with coolwarm
heatmap_data = pd.pivot_table(df1, values='tm', index=['y'], columns=['x'])
sns.heatmap( heatmap_data, ax=ax1, cmap='coolwarm', vmin=40, vmax=85)
ax1.set_title('Heatmap of TM')
ax1.axis('off')

# Plot density plot with sns set x limit from 40 to 85
sns.histplot(df2['tm'], ax=ax2, kde=False, bins=100)
ax2.set_xlim(40, 85)
ax2.set_ylabel('Count')
ax2.set_xlabel('TM')
ax2.set_title('Density Plot of TM')

# add df2['tm']'s mean and median to plot
ax2.axvline(df2['tm'].mean(), color='red', linestyle='dashed', linewidth=1)
ax2.axvline(df2['tm'].median(), color='green', linestyle='dashed', linewidth=1)
ax2.legend({'Mean: {:.2f}'.format(df2['tm'].mean()):df2['tm'].mean(),'Median: {:.2f}'.format(df2['tm'].median()):df2['tm'].median()})

# Set title and axis labels
fig.suptitle(input.split('.')[0], fontsize=14)

# Save to png
fig.tight_layout()
fig.subplots_adjust(top=0.93)
plt.savefig(input.split('.')[0] + '.png', bbox_inches="tight")
