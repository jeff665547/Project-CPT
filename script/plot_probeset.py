import matplotlib.pyplot as plt
import pandas as pd
import sys

if( len(sys.argv) < 2 or len(sys.argv) > 3 ):
    print("Usage: python plot_probeset.py <probeset_id> [sample_name]")
    print("Files of <probeset_id>: raw/qtl/trans/summz data must be in the same directory with this executable script")
    sys.exit(1)

probeset_id = sys.argv[1]
sample_name = sys.argv[2] if len(sys.argv) > 2 else ""

# Get the input arguments
input_file1 = probeset_id + "_raw.txt"
input_file2 = probeset_id + "_qtl.txt"
input_file3 = probeset_id + "_trans.txt"
input_file4 = probeset_id + "_summz.txt"

# Read the first input file
df1 = pd.read_csv(input_file1, sep="\t")
df2 = pd.read_csv(input_file2, sep="\t")
df3 = pd.read_csv(input_file3, sep="\t")

pids = df1["PID"].unique()

# Set the x and y axis data for the first scatter plot
x1 = df1["Channel0"]
y1 = df1["Channel1"]

x2 = df2["Channel0"]
y2 = df2["Channel1"]

x3 = df3["Channel0"]
y3 = df3["Channel1"]

# Create the first scatter plot
fig, ax = plt.subplots(1 + len(pids), 4, figsize=(20, 5 * (1 + len(pids))))
ax[0, 0].scatter(x1, y1, color="blue", s=3)
ax[0, 1].scatter(x2, y2, color="blue", s=3)
ax[0, 2].scatter(x3, y3, color="blue", s=3)

if sample_name != "":
    ax[0, 0].scatter(x1[df1["Sample"] == sample_name], y1[df1["Sample"] == sample_name], color="red", s=15)
    ax[0, 1].scatter(x2[df2["Sample"] == sample_name], y2[df2["Sample"] == sample_name], color="red", s=15)
    ax[0, 2].scatter(x3[df3["Sample"] == sample_name], y3[df3["Sample"] == sample_name], color="red", s=15)

ax[0, 0].set_title("Raw Data")
ax[0, 0].set_xlabel("Channel 0")
ax[0, 0].set_ylabel("Channel 1")
ax[0, 0].set_xlim([min(x1), max(x1)])
ax[0, 0].set_ylim([min(y1), max(y1)])

ax[0, 1].set_title("Quantiled Data")
ax[0, 1].set_xlabel("Channel 0")
ax[0, 1].set_ylabel("Channel 1")
ax[0, 1].set_xlim([min(x2), max(x2)])
ax[0, 1].set_ylim([min(y2), max(y2)])

ax[0, 2].set_title("Transformated Data")
ax[0, 2].set_xlabel("Channel 0")
ax[0, 2].set_ylabel("Channel 1")
ax[0, 2].set_xlim([-3.5, 3.5])
ax[0, 2].set_ylim([min(y3), max(y3)])

for i in range(len(pids)):
    pid = pids[i]

    df1_pid = df1[df1["PID"] == pid]
    df2_pid = df2[df2["PID"] == pid]
    df3_pid = df3[df3["PID"] == pid]

    x1_pid = df1_pid["Channel0"]
    y1_pid = df1_pid["Channel1"]

    x2_pid = df2_pid["Channel0"]
    y2_pid = df2_pid["Channel1"]

    x3_pid = df3_pid["Channel0"]
    y3_pid = df3_pid["Channel1"]

    ax[i + 1, 0].scatter(x1_pid, y1_pid, color="blue", s=3)
    ax[i + 1, 1].scatter(x2_pid, y2_pid, color="blue", s=3)
    ax[i + 1, 2].scatter(x3_pid, y3_pid, color="blue", s=3)

    if sample_name != "":
        ax[i + 1, 0].scatter(x1_pid[df1_pid["Sample"] == sample_name], y1_pid[df1_pid["Sample"] == sample_name], color="red", s=15)
        ax[i + 1, 1].scatter(x2_pid[df2_pid["Sample"] == sample_name], y2_pid[df2_pid["Sample"] == sample_name], color="red", s=15)
        ax[i + 1, 2].scatter(x3_pid[df3_pid["Sample"] == sample_name], y3_pid[df3_pid["Sample"] == sample_name], color="red", s=15)
 
    ax[i + 1, 0].set_ylabel(pid)
    ax[i + 1, 0].set_xlim([min(x1_pid), max(x1_pid)])
    ax[i + 1, 0].set_ylim([min(y1_pid), max(y1_pid)])

    ax[i + 1, 1].set_xlim([min(x2_pid), max(x2_pid)])
    ax[i + 1, 1].set_ylim([min(y2_pid), max(y2_pid)])

    ax[i + 1, 2].set_xlim([-3.5, 3.5])
    ax[i + 1, 2].set_ylim([min(y3_pid), max(y3_pid)])

    fig.delaxes(ax[i + 1][3])

# Read the second input file but extract the first row as an idependent string
# first row will be like
# "#19327	-0.414398|0.0965864	0.189408|0.206045	0.600288|0.0834446"
# get the column 2 to 4 and split by "|"
# first vaule as mean, second value as std
# mean and std will store to variables, col 2 to 4 indecate to "AA", "AB", "BB"
# the input file data will begain from the second row

is_mdl = False

with open(input_file4, "r") as f:
    first_line = f.readline().strip().split("\t")
    if(len(first_line) > 1):
        mean_AA, std_AA = first_line[1].split("|")
        mean_AB, std_AB = first_line[2].split("|")
        mean_BB, std_BB = first_line[3].split("|")
        is_mdl = True

    df4 = pd.read_csv(f, sep="\t", names=["Sample", "X", "Y"], header=0)

# Set the x and y axis data for the second scatter plot
x4 = df4["X"]
y4 = df4["Y"]

# Create the second scatter plot
ax[0, 3].scatter(x4, y4, color="blue", s=3)

if sample_name != "":
    ax[0, 3].scatter(x4[df4["Sample"] == sample_name], y4[df4["Sample"] == sample_name], color="red", s=15)

ax[0, 3].set_title("Summarized Data")
ax[0, 3].set_xlabel("X")
ax[0, 3].set_ylabel("Y")
ax[0, 3].set_xlim([-3.5, 3.5])
ax[0, 3].set_ylim([min(y4), max(y4)])



if(is_mdl):
    # plot the center of each cluster with mean_AA, mean_AB, mean_BB as x value
    # y value use avage of y3
    
    mean_y = sum(y4) / len(y4)
    ax[0, 3].scatter([float(mean_AA), float(mean_AB), float(mean_BB)], [mean_y, mean_y, mean_y], color="green", s=15)
    
    # plot standard deviation of each cluster
    # plot a circle with radius of std_AA, std_AB, std_BB
    # the circle center with mean_AA, mean_AB, mean_BB as x value
    # y value use avage of y3
    
    ax[0, 3].add_patch(plt.Circle((float(mean_AA), mean_y), float(std_AA), color='green', fill=False))
    ax[0, 3].add_patch(plt.Circle((float(mean_AB), mean_y), float(std_AB), color='green', fill=False))
    ax[0, 3].add_patch(plt.Circle((float(mean_BB), mean_y), float(std_BB), color='green', fill=False))

if sample_name != "":
    # Set main plot title
    fig.suptitle(f'{probeset_id} with {sample_name}')
    fig.tight_layout(rect=[0, 0.1, 1, 0.97])

    # Save image to png file
    plt.savefig(f'{probeset_id}_{sample_name}.png', bbox_inches='tight')
else:
    # Set main plot title
    fig.suptitle(f'{probeset_id}')
    fig.tight_layout(rect=[0, 0.1, 1, 0.97])

    # Save image to png file
    plt.savefig(f'{probeset_id}.png', bbox_inches='tight')
