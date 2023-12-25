import matplotlib.patches as mpatches
import matplotlib.pyplot as plt
import pandas as pd
import sys

if( len(sys.argv) != 2 ):
    print("Usage: python plot_evaluatedbox.py [evaluated_file]")
    sys.exit(1)

# Get the input arguments
evaluated_file = sys.argv[1]

probeset_acc = []
sample_acc = []

# Open the input file and read the data
with open(evaluated_file, "r") as f:
    # Read the header line and split it into sample names
    header = f.readline().strip().split("\t")
    sample_names = header[2:]

    # Loop through the remaining lines in the file
    for line in f:
        # Split the line into columns
        cols = line.strip().split("\t")

        if cols[1] == ":All|Samples:":
            
            # Loop through the sample names and extract the data
            for i, sample_name in enumerate(sample_names):
                sample_data_str = cols[i + 2]

                acc_components = sample_data_str.split("|")
                sample_acc.append(float(acc_components[0].strip("%"))/100)

            continue

        # Split the line into components
        acc_components = cols[1].split("|")
        probeset_acc.append(float(acc_components[0].strip("%"))/100)

fig, ax = plt.subplots(1, 2, figsize=(20, 10))

# Set main plot title
fig.suptitle(evaluated_file, fontsize = 20)

# plot boxplot for probeset accuracy
ax[0].boxplot(probeset_acc)
ax[0].set_title(f"Probeset Accuracy, Median Acc: {round(pd.Series(probeset_acc).median(), 3)}", fontsize = 16)
ax[0].set_ylabel("Accuracy", fontsize = 16)

# plot boxplot for sample accuracy
ax[1].boxplot(sample_acc)
ax[1].set_title(f"Sample Accuracy, Median Acc: {round(pd.Series(sample_acc).median(), 3)}", fontsize = 16)
ax[1].set_ylabel("Accuracy", fontsize = 16)

# Set the x-axis tick labels
ax[0].set_xticklabels([" "])
ax[1].set_xticklabels([" "])

# Set the y-axis limits
ax[0].set_ylim([0, 1])
ax[1].set_ylim([0, 1])

# Set the y-axis tick labels
ax[0].set_yticks([0, 0.25, 0.5, 0.75, 1])
ax[1].set_yticks([0, 0.25, 0.5, 0.75, 1])

# Set the y-axis tick labels
ax[0].tick_params(labelsize = 14)
ax[1].tick_params(labelsize = 14)

# Add a horizontal line at y = 0.5
ax[0].axhline(y = 0.5, color = 'r', linestyle = '--')
ax[1].axhline(y = 0.5, color = 'r', linestyle = '--')

# Add a horizontal line at y = 0.75
ax[0].axhline(y = 0.75, color = 'g', linestyle = '--')
ax[1].axhline(y = 0.75, color = 'g', linestyle = '--')

# Add a horizontal line at y = 0.9
ax[0].axhline(y = 0.9, color = 'b', linestyle = '--')
ax[1].axhline(y = 0.9, color = 'b', linestyle = '--')

# Add a horizontal line at y = 0.95
ax[0].axhline(y = 0.95, color = 'c', linestyle = '--')
ax[1].axhline(y = 0.95, color = 'c', linestyle = '--')

# Add a horizontal line at y = 0.99
ax[0].axhline(y = 0.99, color = 'm', linestyle = '--')
ax[1].axhline(y = 0.99, color = 'm', linestyle = '--')

# Add a horizontal line at y = 1.0
ax[0].axhline(y = 1.0, color = 'k', linestyle = '--')
ax[1].axhline(y = 1.0, color = 'k', linestyle = '--')

# Add a legend
red_patch = mpatches.Patch(color = 'red', label = '0.5')
green_patch = mpatches.Patch(color = 'green', label = '0.75')
blue_patch = mpatches.Patch(color = 'blue', label = '0.9')
cyan_patch = mpatches.Patch(color = 'cyan', label = '0.95')
magenta_patch = mpatches.Patch(color = 'magenta', label = '0.99')
black_patch = mpatches.Patch(color = 'black', label = '1.0')
ax[0].legend(handles = [red_patch, green_patch, blue_patch, cyan_patch, magenta_patch, black_patch], fontsize = 14)
ax[1].legend(handles = [red_patch, green_patch, blue_patch, cyan_patch, magenta_patch, black_patch], fontsize = 14)

# Save image to png file
plt.savefig(f"{evaluated_file.split('.')[0]}.png", bbox_inches='tight')
