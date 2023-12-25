import matplotlib.patches as mpatches
import matplotlib.pyplot as plt
import pandas as pd
import sys

if( len(sys.argv) != 6 ):
    print("Usage: python plot_genotyping.py <probeset_id> [ps_file] [mdl_file] [genotype_file] [evaluated_file]")
    sys.exit(1)

# Get the input arguments
probeset_id = sys.argv[1]
ps_file = sys.argv[2]
mdl_file = sys.argv[3]
genotype_file = sys.argv[4]
evaluated_file = sys.argv[5]

probeset_index = -1

# read the ps file and find the index of given probeset_id
with open(ps_file, "r") as f:
    for i, line in enumerate(f):
        if i == 0:
            continue
        if line.strip() == probeset_id:
            probeset_index = i - 1
            break

if probeset_index == -1:
    print("Probeset ID not found")
    exit(1)

# read the mdl file and find the matched index of with probeset_index
with open(mdl_file, "r") as f:
    for i, line in enumerate(f):
        if i == 0:
            continue
        if i - 1 == probeset_index:
            mdl_line = line.strip().split("\t")
            if len(mdl_line) > 1:
                mean_AA, std_AA = mdl_line[1].split("|")
                mean_AB, std_AB = mdl_line[2].split("|")
                mean_BB, std_BB = mdl_line[3].split("|")
            break

# Initialize variables to store sample data and ACC information
sample_data = {}
acc_info = {}

# Open the input file and read the data
with open(evaluated_file, "r") as f:
    # Read the header line and split it into sample names
    header = f.readline().strip().split("\t")
    sample_names = header[2:]

    # Loop through the remaining lines in the file
    for line in f:
        # Split the line into columns
        cols = line.strip().split("\t")

        # Get the probeset ID from the first column
        current_probeset_id = cols[0]

        # If this is the probeset we're looking for, process the sample data
        if current_probeset_id == probeset_id:
            # Split the line into components
            acc_components = cols[1].split("|")

            # Store the ACC information using the component names as the keys
            acc_info["All"] = "All: " + acc_components[0]
            acc_info["0/0"] = "0/0: " + acc_components[1]
            acc_info["0/1"] = "0/1: " + acc_components[2]
            acc_info["1/1"] = "1/1: " + acc_components[3]

            # Loop through the sample names and extract the data
            for i, sample_name in enumerate(sample_names):
                # Get the sample data from the current column
                sample_data_str = cols[i + 2]

                # Split the sample data into tf, call, and vcf
                tf, call, vcf = sample_data_str.split("|")

                # Store the sample data using the sample name as the key
                sample_data[sample_name] = {"tf": tf, "call": call, "vcf": vcf}

# Print the sample data and ACC information
# print('Sample data:')
# print(sample_data)
# print('ACC information:')
# print(acc_info)

fig, ax = plt.subplots(1, 3, figsize=(30, 10))

# Set main plot title
fig.suptitle(probeset_id, fontsize = 20)

ax[0].set_title("CPT Calls", fontsize = 20)
ax[1].set_title("Genotypes", fontsize = 20)
ax[2].set_title("VCF Calls", fontsize = 20)

with open(genotype_file, "r") as f:
    df = pd.read_csv(f, sep="\t", names=["x_axis", "y_axis", "genotype", "confidence", "probeset_name", "sample_name"], header=0)

psid_list = [probeset_id]

df = df[df["probeset_name"].isin(psid_list)]
df = df[df["sample_name"].isin(sample_data.keys())]

x = df["x_axis"]
y = df["y_axis"]

# Define color map
colors = { -1: "gray", 0: "green", 1: "blue", 2: "red"}

# Define color and edge color for each point
colors_vcf = [colors[int(sample_data[sample]["vcf"])] for sample in df["sample_name"]]
colors_cen = [colors[int(sample_data[sample]["call"])] for sample in df["sample_name"]]

# Create the second scatter plot
ax[0].scatter(x, y, c=colors_cen, label=colors_cen, s=50)
ax[1].scatter(x, y, c=colors_vcf, edgecolors=colors_cen, s=50)
ax[2].scatter(x, y, c=colors_vcf, label=colors_vcf, s=50)

ax[0].set_xlabel("X")
ax[0].set_ylabel("Y")
ax[0].set_xlim([-3.5, 3.5])
ax[0].set_ylim([min(y), max(y)])

ax[1].set_xlabel("X")
ax[1].set_ylabel("Y")
ax[1].set_xlim([-3.5, 3.5])
ax[1].set_ylim([min(y), max(y)])

ax[2].set_xlabel("X")
ax[2].set_ylabel("Y")
ax[2].set_xlim([-3.5, 3.5])
ax[2].set_ylim([min(y), max(y)])

# Add a legend to each subplot
color_00 = mpatches.Patch(color=colors[0], label='0/0')
color_01 = mpatches.Patch(color=colors[1], label='0/1')
color_11 = mpatches.Patch(color=colors[2], label='1/1')

ax[2].legend(handles=[color_00, color_01, color_11], bbox_to_anchor=(1, 1.15), fontsize=15)

acc = acc_info["All"] + "\n" + acc_info["0/0"] + "\n" + acc_info["0/1"] + "\n" + acc_info["1/1"]
plt.gcf().text( 0.11, 0.9, acc, fontsize = 15) 

# plot the center of each cluster with mean_AA, mean_AB, mean_BB as x value
# y value use avage of y

mean_y = sum(y) / len(y)

ax[0].scatter(
    [float(mean_AA), float(mean_AB), float(mean_BB)],
    [mean_y, mean_y, mean_y],
    color="black",
    s=15,
)

ax[1].scatter(
    [float(mean_AA), float(mean_AB), float(mean_BB)],
    [mean_y, mean_y, mean_y],
    color="black",
    s=15,
)

ax[2].scatter(
    [float(mean_AA), float(mean_AB), float(mean_BB)],
    [mean_y, mean_y, mean_y],
    color="black",
    s=15,
)

# plot standard deviation of each cluster
# plot a circle with radius of std_AA, std_AB, std_BB
# the circle center with mean_AA, mean_AB, mean_BB as x value
# y value use avage of y

ax[0].add_patch(
    plt.Circle((float(mean_AA), mean_y), float(std_AA), color="black", fill=False)
)
ax[0].add_patch(
    plt.Circle((float(mean_AB), mean_y), float(std_AB), color="black", fill=False)
)
ax[0].add_patch(
    plt.Circle((float(mean_BB), mean_y), float(std_BB), color="black", fill=False)
)

ax[1].add_patch(
    plt.Circle((float(mean_AA), mean_y), float(std_AA), color="black", fill=False)
)
ax[1].add_patch(
    plt.Circle((float(mean_AB), mean_y), float(std_AB), color="black", fill=False)
)
ax[1].add_patch(
    plt.Circle((float(mean_BB), mean_y), float(std_BB), color="black", fill=False)
)

ax[2].add_patch(
    plt.Circle((float(mean_AA), mean_y), float(std_AA), color="black", fill=False)
)
ax[2].add_patch(
    plt.Circle((float(mean_AB), mean_y), float(std_AB), color="black", fill=False)
)
ax[2].add_patch(
    plt.Circle((float(mean_BB), mean_y), float(std_BB), color="black", fill=False)
)

# Save image to png file
plt.savefig(f"{probeset_id}_genotyping.png", bbox_inches='tight')
