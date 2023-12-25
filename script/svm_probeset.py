
import sys
import numpy as np
import pandas as pd
import concurrent.futures
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from collections import Counter
from sklearn.model_selection import train_test_split
from sklearn import svm
from sklearn import metrics

is_plotting = False

def process_pid(pid, data):
    # Check if there are at least two unique values in the data
    X = [v[:2] for v in data.values()]  # x and y values
    y = [v[2] for v in data.values()]  # z values

    if len(y) == 0:
        return -1
    
    class_counts = Counter(y)
    if any(count < 2 for count in class_counts.values()):
        return -1

    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, stratify=y)

    # Check if y_train only contains one unique value
    if len(set(y_train)) == 1:
        return -1

    # Create an instance of the SVM model
    clf = svm.SVC(kernel='rbf', gamma=0.7, C=1.0)  # Gaussian Kernel

    # Train the model using the training data
    clf.fit(X_train, y_train)

    # Make predictions using the test data
    y_pred = clf.predict(X_test)

    # Calculate the accuracy of the model
    acc = round(metrics.accuracy_score(y_test, y_pred) * 100, 2)

    if is_plotting:
        # Train the model using the training data
        clf.fit(X, y)
        plt.figure()

        # Create a grid to evaluate model
        x_min, x_max = min([x[0] for x in X]), max([x[0] for x in X])
        y_min, y_max = min([x[1] for x in X]), max([x[1] for x in X])
        xx, yy = np.meshgrid(np.arange(x_min, x_max, 0.2),
                             np.arange(y_min, y_max, 0.2))

        # Form a 2D grid
        grid = np.c_[xx.ravel(), yy.ravel()]

        # Make predictions using the test data
        Z = clf.predict(grid)

        # Reshape Z to match the shape of xx
        Z = Z.reshape(xx.shape)

        # Check if Z is at least a 2x2 array
        if Z.shape[0] < 2 or Z.shape[1] < 2:
            return acc

        # Plot decision boundary and margins
        plt.contour(xx, yy, Z, colors='k', levels=[-1, 0, 1], alpha=0.5, linestyles=['--', '-', '--'])

        # Plot the original data points with different colors for different labels
        plt.scatter([x[0] for x in X], [x[1] for x in X], c=y, cmap='viridis', s=20)

        # Plot support vectors
        plt.scatter(clf.support_vectors_[:, 0], clf.support_vectors_[:, 1], s=100, facecolors='none', edgecolors='k')

        plt.gcf().text( 0.13, 0.85, 'O: Support vectors', fontsize=10 )
        plt.gcf().text( 0.13, 0.81, f'Acc: {acc}%', fontsize=10 )

        plt.xlim(-3.5, 3.5)
        plt.ylim(yy.min(), yy.max())
        plt.xticks(())
        plt.yticks(())
        plt.title(pid)

        # Plot support vectors
        plt.savefig(f"{pid}_svm.png", bbox_inches='tight')
        plt.close()

    return acc

if( len(sys.argv) < 4 or len(sys.argv) > 5 ):
    print("Usage: python svm_probeset.py [probeset_list] [genotyped_file] [evaluated_file] [plot_probeset(true|false, default=false)]")
    sys.exit(1)

probesetl_file = sys.argv[1]
genotyped_file = sys.argv[2]
evaluated_file = sys.argv[3]
is_plotting = True if len(sys.argv) > 5 and sys.argv[4].lower() == "true" else False

datas = {}

print("Reading probeset list file...")
with open(probesetl_file, "r") as f:
    for line in f:
        if line != "probeset_id":
            datas[line.strip()] = {}
        continue

print("Reading evaluated file...")
with open(evaluated_file, "r") as f:
    header = f.readline().strip().split("\t")
    sample_names = header[2:]

    for line in f:
        cols = line.strip().split("\t")

        if cols[1] != ":All|Samples:" and cols[0] in datas:
            for i, sample_name in enumerate(sample_names):
                label = cols[i + 2].split("|")[2]
                
                if label != "-1":
                    datas[cols[0]][sample_name] = [ 0, 0, int(label)]
            continue

count = 0
print("Reading genotyped file...")
with open(genotyped_file, "r") as f:
    for line in f:
        print(count, end="\r")
        count += 1
        cols = line.strip().split("\t")

        if cols[0] != "x_axis" and cols[0] != "a_allele" and cols[4] in datas and cols[5] in datas[cols[4]]:
            datas[cols[4]][cols[5]][0] = float(cols[0])
            datas[cols[4]][cols[5]][1] = float(cols[1])
        continue

# Process each PID in parallel
# with concurrent.futures.ProcessPoolExecutor(max_workers=8) as executor:
with concurrent.futures.ProcessPoolExecutor() as executor:
    acc_list = list(executor.map(process_pid, datas.keys(), datas.values()))

accs = []

# Write the accuracy to a file
with open(f'svm_probeset_acc.csv', 'w') as f:
    for pid, acc in zip(datas.keys(), acc_list):
        f.write(f"{pid},{acc}\n")

        if acc != -1:
            accs.append(acc)

fig, ax = plt.subplots(1, 1, figsize=(10, 10))

# plot boxplot for probeset accuracy
ax.boxplot(accs)
ax.set_title(f"Box of Accuracy (Acc), Median Acc: {round(pd.Series(accs).median(), 3)}%", fontsize = 16)
ax.set_ylabel("Acc", fontsize = 16)

ax.set_xticklabels([" "])
ax.tick_params(labelsize = 14)

# Add horizontal lines
ax.axhline(y = 10, color = 'r', linestyle = '--')
ax.axhline(y = 25, color = 'g', linestyle = '--')
ax.axhline(y = 50, color = 'b', linestyle = '--')
ax.axhline(y = 75, color = 'c', linestyle = '--')
ax.axhline(y = 90, color = 'm', linestyle = '--')
ax.axhline(y = 99, color = 'k', linestyle = '--')

# Add a legend
red_patch = mpatches.Patch(color = 'red', label = '1%')
green_patch = mpatches.Patch(color = 'green', label = '25%')
blue_patch = mpatches.Patch(color = 'blue', label = '50%')
cyan_patch = mpatches.Patch(color = 'cyan', label = '75%')
magenta_patch = mpatches.Patch(color = 'magenta', label = '90%')
black_patch = mpatches.Patch(color = 'black', label = '99%')
ax.legend(handles = [red_patch, green_patch, blue_patch, cyan_patch, magenta_patch, black_patch], fontsize = 14)

# Save image to png file
plt.savefig("svm_probeset_acc.png", bbox_inches='tight')