import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import sys

import warnings

warnings.filterwarnings("ignore")

if len(sys.argv) != 2:
    print("Usage: python plot_marker_call.py [cen_qtl.txt]")
    sys.exit(1)

mk_file = sys.argv[1]
sample_name = mk_file.split("/")[-1].split("_qtl")[0]

df = pd.read_csv(mk_file, sep="\t")
fig, ax = plt.subplots(3, 2, figsize=(20, 30))

x = df["X"]
y = df["Y"]

a0 = np.array(df["Channel0"] / df["Channel0"].max())
a1 = np.array(df["Channel1"] / df["Channel1"].max())

c0 = np.zeros((len(a0), 4))
c0[:, 0] = 1.0
c0[:, 3] = a0

c1 = np.zeros((len(a1), 4))
c1[:, 1] = 1.0
c1[:, 3] = a1

ax[0, 0].scatter(x, y, s=30, c=c0, marker="s")
ax[0, 1].scatter(x, y, s=30, c=c1, marker="s")

for i in range(0, 6):
    ax[0, 0].axhline(y=7.5 + (i * 8), color="black", ls="-")
    ax[0, 0].axvline(x=7.5 + (i * 8), color="black", ls="-")

    ax[0, 1].axhline(y=7.5 + (i * 8), color="black", ls="-")
    ax[0, 1].axvline(x=7.5 + (i * 8), color="black", ls="-")

ax[0, 0].set_title("Marker AM1")
ax[0, 0].set_xlim([-1, max(x) + 1])
ax[0, 0].set_ylim([-1, max(y) + 1])
ax[0, 0].axison = False

ax[0, 1].set_title("Marker AM2/3")
ax[0, 1].set_xlim([-1, max(x) + 1])
ax[0, 1].set_ylim([-1, max(y) + 1])
ax[0, 1].axison = False

x = df["Channel0"]  # AM1
y = df["Channel1"]  # AM2/3

min = min(min(np.log(x)), min(np.log(y)))
max = max(max(np.log(x)), max(np.log(y)))

ax[1, 0].scatter(
    np.log(x[df["Channels"] == 0]),
    np.log(y[df["Channels"] == 0]),
    color="red",
    s=3,
    alpha=0.3,
)

ax[1, 1].scatter(
    np.log(x[df["Channels"] == 1]),
    np.log(y[df["Channels"] == 1]),
    color="green",
    s=3,
    alpha=0.3,
)

ax[2, 0].scatter(
    np.log(x[df["Channels"] == 0]),
    np.log(y[df["Channels"] == 0]),
    color="red",
    s=3,
    alpha=0.3,
)

ax[2, 0].scatter(
    np.log(x[df["Channels"] == 1]),
    np.log(y[df["Channels"] == 1]),
    color="green",
    s=3,
    alpha=0.3,
)

ax[1, 0].set_title("Marker AM1")
ax[1, 0].set_xlabel("Channel 0")
ax[1, 0].set_ylabel("Channel 1")
ax[1, 0].set_xlim([min, max])
ax[1, 0].set_ylim([min, max])

ax[1, 1].set_title("Marker AM2/3")
ax[1, 1].set_xlabel("Channel 0")
ax[1, 1].set_ylabel("Channel 1")
ax[1, 1].set_xlim([min, max])
ax[1, 1].set_ylim([min, max])

acc = sum((df["Channels"] == 0) & (x > y)) + sum((df["Channels"] == 1) & (x < y))
acc = round(acc * 100 / df.shape[0], 2)

ax[2, 0].set_title(f"Marker Call Rate: {acc}%")
ax[2, 0].set_xlabel("Channel 0")
ax[2, 0].set_ylabel("Channel 1")
ax[2, 0].set_xlim([min, max])
ax[2, 0].set_ylim([min, max])
ax[2, 0].plot([min, max], [min, max], color="black", ls="--")

dots = np.array([x, y]).T
contrast = (dots[:, 0] - dots[:, 1]) / (dots[:, 0] + dots[:, 1])

d_0 = contrast[df["Channels"] == 0]
d_1 = contrast[df["Channels"] == 1]

dqc_0 = round(np.count_nonzero(d_0 > (d_1.mean() + 2 * d_1.std())) / len(d_0) * 100, 2)
dqc_1 = round(np.count_nonzero(d_1 < (d_0.mean() - 2 * d_0.std())) / len(d_1) * 100, 2)

ax[2, 1].hist(
    d_0, bins=201, range=(-1, 1), alpha=0.3, density=True, label="AM1", color="red"
)
ax[2, 1].hist(
    d_1, bins=201, range=(-1, 1), alpha=0.3, density=True, label="AM2/3", color="green"
)

ax[2, 1].set_title(f"DQC of Channels: {dqc_0}% / {dqc_1}%")
ax[2, 1].set_xlabel("Contrast")
ax[2, 1].set_ylabel("Probability Density")
ax[2, 1].set_xlim([-1, 1])
ax[2, 1].legend(loc="upper right", fancybox=True, framealpha=0.5)

fig.suptitle(f"{sample_name}")
fig.tight_layout(rect=[0, 0.1, 1, 0.97])
plt.savefig(f"{sample_name}_call.png", bbox_inches="tight")

print(f"{sample_name}: Call Rate: {acc}%, DQC(Channels): {dqc_0}% / {dqc_1}%")
