import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import sys

import warnings

warnings.filterwarnings("ignore")

if len(sys.argv) != 2:
    print("Usage: python plot_marker_slop.py [cen_qtl.txt]")
    sys.exit(1)

mk_file = sys.argv[1]
sample_name = mk_file.split("/")[-1].split("_qtl")[0]
sample_name = sample_name if len(sample_name) == 18 else sample_name + " "

df = pd.read_csv(mk_file, sep="\t")
fig, ax = plt.subplots(1, 2, figsize=(16, 8))

x = df["Channel0"]  # AM1
y = df["Channel1"]  # AM2/3

ax[0].scatter(
    x[df["Channels"] == 0],
    y[df["Channels"] == 0],
    color="red",
    s=3,
    alpha=0.3,
)

ax[0].scatter(
    x[df["Channels"] == 1],
    y[df["Channels"] == 1],
    color="green",
    s=3,
    alpha=0.3,
)

dots = np.array([x, y]).T

c0 = dots[df["Channels"] == 0]
c1 = dots[df["Channels"] == 1]

c0_slope, c0_intercept = np.polyfit(c0[:, 0], c0[:, 1], 1)
c1_slope, c1_intercept = np.polyfit(c1[:, 0], c1[:, 1], 1)

slope_contrast = c0_slope / c1_slope

min = min(min(x), min(y))
max = max(max(x), max(y))

ax[0].plot(
    [min, max],
    [c0_slope * min + c0_intercept, c0_slope * max + c0_intercept],
    color="black",
    alpha=0.5,
    ls="--",
    lw=2,
)

ax[0].plot(
    [min, max],
    [c1_slope * min + c1_intercept, c1_slope * max + c1_intercept],
    color="black",
    alpha=0.5,
    ls="--",
    lw=2,
)

slope_contrast = round(c0_slope / c1_slope, 2).astype(str)

c0_slope = round(c0_slope, 2).astype(str)
c1_slope = round(c1_slope, 2).astype(str)

slope_contrast = slope_contrast if len(slope_contrast) == 4 else slope_contrast + " "
c0_slope = c0_slope if len(c0_slope) == 4 else c0_slope + " "
c1_slope = c1_slope if len(c1_slope) == 4 else c1_slope + " "

ax[0].text(
    max - 0.31 * (max - min),
    max - 0.05 * (max - min),
    f"Slope (Channel 0): {c0_slope}",
    color="black",
    fontsize=12,
)

ax[0].text(
    max - 0.31 * (max - min),
    max - 0.1 * (max - min),
    f"Slope (Channel 1): {c1_slope}",
    color="black",
    fontsize=12,
)

ax[0].set_title(f"(Raw) Slope Contrast: {slope_contrast}")
ax[0].set_xlabel("Channel 0")
ax[0].set_ylabel("Channel 1")
ax[0].set_xlim([min, max])
ax[0].set_ylim([min, max])

ax[1].scatter(
    np.log(x[df["Channels"] == 0]),
    np.log(y[df["Channels"] == 0]),
    color="red",
    s=3,
    alpha=0.3,
)

ax[1].scatter(
    np.log(x[df["Channels"] == 1]),
    np.log(y[df["Channels"] == 1]),
    color="green",
    s=3,
    alpha=0.3,
)

dots = np.array([np.log(x), np.log(y)]).T

c0 = dots[df["Channels"] == 0]
c1 = dots[df["Channels"] == 1]

c0_slope_log, c0_intercept_log = np.polyfit(c0[:, 0], c0[:, 1], 1)
c1_slope_log, c1_intercept_log = np.polyfit(c1[:, 0], c1[:, 1], 1)

slope_contrast_log = c0_slope_log / c1_slope_log

min_log = np.log(min)
max_log = np.log(max)

ax[1].plot(
    [min_log, max_log],
    [
        c0_slope_log * min_log + c0_intercept_log,
        c0_slope_log * max_log + c0_intercept_log,
    ],
    color="black",
    alpha=0.5,
    ls="--",
    lw=2,
)

ax[1].plot(
    [min_log, max_log],
    [
        c1_slope_log * min_log + c1_intercept_log,
        c1_slope_log * max_log + c1_intercept_log,
    ],
    color="black",
    alpha=0.5,
    ls="--",
    lw=2,
)

slope_contrast_log = round(c0_slope_log / c1_slope_log, 2).astype(str)

c0_slope_log = round(c0_slope_log, 2).astype(str)
c1_slope_log = round(c1_slope_log, 2).astype(str)

slope_contrast_log = (
    slope_contrast_log if len(slope_contrast_log) == 4 else slope_contrast_log + " "
)
c0_slope_log = c0_slope_log if len(c0_slope_log) == 4 else c0_slope_log + " "
c1_slope_log = c1_slope_log if len(c1_slope_log) == 4 else c1_slope_log + " "

ax[1].text(
    max_log - 0.31 * (max_log - min_log),
    max_log - 0.05 * (max_log - min_log),
    f"Slope (Channel 0): {c0_slope_log}",
    color="black",
    fontsize=12,
)

ax[1].text(
    max_log - 0.31 * (max_log - min_log),
    max_log - 0.1 * (max_log - min_log),
    f"Slope (Channel 1): {c1_slope_log}",
    color="black",
    fontsize=12,
)

ax[1].set_title(f"(Log) Slope Contrast: {slope_contrast_log}")
ax[1].set_xlabel("Channel 0")
ax[1].set_ylabel("Channel 1")
ax[1].set_xlim([min_log, max_log])
ax[1].set_ylim([min_log, max_log])

print(
    f"{sample_name} (Natural/Log): (SlopeContrast/SlopeChannel0/SlopeChannel1): {slope_contrast} / {c0_slope} / {c1_slope} ;  {slope_contrast_log} / {c0_slope_log} / {c1_slope_log}"
)

fig.suptitle(f"{sample_name}")
fig.tight_layout(rect=[0, 0.1, 1, 0.97])
plt.savefig(f"{sample_name}_slop.png", bbox_inches="tight")
