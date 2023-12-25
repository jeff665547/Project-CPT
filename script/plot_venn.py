import sys
from matplotlib import pyplot as plt
from matplotlib_venn import venn2

# Read in input files from argv
file1 = sys.argv[1]
file2 = sys.argv[2]

name1 = file1.split('.')[0]
name2 = file2.split('.')[0]

# Read in data from files
with open(file1, 'r') as f1, open(file2, 'r') as f2:
    data1 = set([line.strip() for line in f1.readlines()[1:]])
    data2 = set([line.strip() for line in f2.readlines()[1:]])

# Plot Venn diagram
venn2([data1, data2], (name1, name2))
plt.savefig(f'{name1}_{name2}.png', bbox_inches='tight')
