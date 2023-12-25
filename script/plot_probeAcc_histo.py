
import sys
import pandas as pd
import matplotlib.pyplot as plt

# Read input file
input = sys.argv[1]
df = pd.read_csv(input, header=None, sep='\t', names=['id', 'acc'])

# Plot histogram
plt.hist(df['acc'], bins=25, range=(0,100))
plt.xlabel('Accuracy Score')
plt.ylabel('Frequency')
plt.title('Accuracy Score Histogram')
plt.savefig(input.split('.')[0] + '.png', bbox_inches="tight")
