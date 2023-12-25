
import sys
import pandas as pd
import matplotlib.pyplot as plt

# Read in all input files
pids1 = sys.argv[1]
pids2 = sys.argv[2]
pids3 = sys.argv[3]
input = sys.argv[4]

filepaths = [pids1, pids2, pids3]
pids = [pd.read_csv(filepath, header=None, names=['pid']) for filepath in filepaths]
dfs = [pd.read_csv(input) for filepath in filepaths]

for i in range(len(dfs)):
    dfs[i].loc[~dfs[i]['pid'].isin(pids[i]['pid']), 'tm'] = 0

# Extract the "tm" column from each file and store them in a list
tms = [df[df['tm'] != 0]['tm'] for df in dfs]

# Create a list of colors to use for each histogram
colors = ['red', 'green', 'blue']

# Combine all histograms into one plot using matplotlib's subplots function
fig, ax = plt.subplots(figsize=(10, 13))
legend = {}

# Plot each histogram using matplotlib's hist function, passing in the extracted "tm" column and the corresponding color
for i in range(len(tms)):
    ax.hist(tms[i], color=colors[i], alpha=0.5, bins=200)
    ax.axvline(tms[i].mean(), color=colors[i], linestyle='dashed', linewidth=1)
    legend.update({filepaths[i].split('.')[0] + ' Mean: {:.2f}'.format(tms[i].mean()) : tms[i].mean()})

# Set the y-axis limits to be the same for all histograms using matplotlib's ylim function
ax.set_xlim(40, 85)
ax.set_ylabel('Count')
ax.set_xlabel('TM')
ax.set_title('Density Plot of TM')

# Show the plot using matplotlib's show function
fig.tight_layout()
fig.subplots_adjust(top=0.93)

plt.legend(legend)
plt.savefig('probe_tm_overlapHisto.png', bbox_inches="tight")
