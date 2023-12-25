import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import sys

import warnings

warnings.filterwarnings("ignore")

if len(sys.argv) != 2:
    print("Usage: python plot_NPcall.py [np_qtl.txt]")
    sys.exit(1)

np_file = sys.argv[1]
sample_name = np_file.split("/")[-1].split("_qtl")[0]

df = pd.read_csv(np_file, sep="\t")
fig, ax = plt.subplots(2, 2, figsize=(10, 10))

x = df["Channel0"]  # CG
y = df["Channel1"]  # AT

min = min(min(np.log(x)), min(np.log(y)))
max = max(max(np.log(x)), max(np.log(y)))

ax[0, 0].scatter(
    np.log(x[df["AT/CG"] == "AT"]),
    np.log(y[df["AT/CG"] == "AT"]),
    color="red",
    s=3,
    alpha=0.3,
)

ax[0, 1].scatter(
    np.log(x[df["AT/CG"] == "CG"]),
    np.log(y[df["AT/CG"] == "CG"]),
    color="green",
    s=3,
    alpha=0.3,
)

ax[1, 0].scatter(
    np.log(x[df["AT/CG"] == "AT"]),
    np.log(y[df["AT/CG"] == "AT"]),
    color="red",
    s=3,
    alpha=0.3,
)

ax[1, 0].scatter(
    np.log(x[df["AT/CG"] == "CG"]),
    np.log(y[df["AT/CG"] == "CG"]),
    color="green",
    s=3,
    alpha=0.3,
)

ax[0, 0].set_title("NP porbe of AT type")
ax[0, 0].set_xlabel("Channel 0")
ax[0, 0].set_ylabel("Channel 1")
ax[0, 0].set_xlim([min, max])
ax[0, 0].set_ylim([min, max])

ax[0, 1].set_title("NP porbe of CG type")
ax[0, 1].set_xlabel("Channel 0")
ax[0, 1].set_ylabel("Channel 1")
ax[0, 1].set_xlim([min, max])
ax[0, 1].set_ylim([min, max])

acc = sum((df["AT/CG"] == "AT") & (x > y)) + sum((df["AT/CG"] == "CG") & (x < y))
acc = round(acc * 100 / df.shape[0], 2)

ax[1, 0].set_title(f"NP Call Rate: {acc}%")
ax[1, 0].set_xlabel("Channel 0")
ax[1, 0].set_ylabel("Channel 1")
ax[1, 0].set_xlim([min, max])
ax[1, 0].set_ylim([min, max])
ax[1, 0].plot([min, max], [min, max], color="black", ls="--")

dots = np.array([x, y]).T
contrast = (dots[:, 0] - dots[:, 1]) / (dots[:, 0] + dots[:, 1])

d_at = contrast[df["AT/CG"] == "AT"]
d_cg = contrast[df["AT/CG"] == "CG"]

dqc_at = round(
    np.count_nonzero(d_at > (d_cg.mean() + 2 * d_cg.std())) / len(d_at) * 100, 2
)
dqc_cg = round(
    np.count_nonzero(d_cg < (d_at.mean() - 2 * d_at.std())) / len(d_cg) * 100, 2
)

ax[1, 1].hist(
    d_at, bins=201, range=(-1, 1), alpha=0.3, density=True, label="AT", color="red"
)
ax[1, 1].hist(
    d_cg, bins=201, range=(-1, 1), alpha=0.3, density=True, label="CG", color="green"
)

ax[1, 1].set_title(f"DQC of AT/CG: {dqc_at}% / {dqc_cg}%")
ax[1, 1].set_xlabel("Contrast")
ax[1, 1].set_ylabel("Probability Density")
ax[1, 1].set_xlim([-1, 1])
ax[1, 1].legend(loc="upper right", fancybox=True, framealpha=0.5)

fig.suptitle(f"{sample_name}")
fig.tight_layout(rect=[0, 0.1, 1, 0.97])
plt.savefig(f"{sample_name}.png", bbox_inches="tight")

print(f"{sample_name}: Call Rate: {acc}%, DQC(AT/CG): {dqc_at}% / {dqc_cg}%")
