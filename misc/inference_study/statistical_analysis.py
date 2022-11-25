import numpy as np
import pandas as pd
from tabulate import tabulate
import os
from statsmodels import stats
import pingouin
import scikit_posthocs as sp
from statsmodels.stats.multitest import multipletests
from statsmodels.stats.multicomp import pairwise_tukeyhsd
from scipy.stats import shapiro
from statsmodels.formula.api import ols
np.set_printoptions(suppress=True)
np.set_printoptions(threshold=np.inf)

path = "./results_windows2/"  # results_windows, results_windows2, results
filename = "neural-network-runtimes-case-"

paths = ["./results/", "./results_windows/", "./results_windows2/"]
all_cases = [["1", "2", "3", "4", "1_inceptionv3"], ["1", "1_inceptionv3"], ["1", "1_inceptionv3"]]

machines = ["ubuntu", "win1", "win2"]
total_runtimes = []
curr_engine_and_device = []

for path, cases, machine in zip(paths, all_cases, machines):

    print("#"*20)
    print("Current machine: ", machine)
    print("#"*20)

    for i in cases:
        loc = path + filename + str(i) + ".csv"
        print("\n" + "Case: " + str(i) + "\n" + "/"*100)
        df = pd.read_csv(loc, sep=";", header=0)

        data_head = df.head()
        header = np.array(df.columns)
        data = np.array(df)

        #print(tabulate(data, header="keys", tablefmt="psql"))
        #print(df.columns)

        for engine in np.unique(data[:, header == "Engine"]):
            print("\n" + "#"*60)
            print("Engine: " + engine)
            curr = np.squeeze(data[:, header == "Engine"])
            curr = data[curr == engine, :]
            for device in np.unique(curr[:, header == "Device Type"]):
                if (path == "./results_windows2/") & (device == "ANY1"):
                    continue
                curr_engine_and_device.append(i + "_" + engine + "_" + device + " / " + machine)
                print("-")
                print("Device: " + str(device))
                tmp = np.squeeze(curr[:, header == "Device Type"])
                tmp = curr[tmp == device, :]
                for measurement in header[3:]:
                    if "STD" in measurement:
                        continue
                    print("_")
                    print(measurement)

                    if measurement == "Total":
                        total = tmp[:, header == measurement] / 1000
                        total_runtimes.append(total)
                    else:
                        total = tmp[:, header == measurement]
                    dev = np.std(total) #1.95 * np.std(total)/np.sqrt(len(total))
                    print(np.mean(total), dev)

                    # print("total CI (mean and standard error): ", np.mean(total), "+-", np.std(total)/np.sqrt(len(total)))
                    # print("total CI: ", [np.mean(total) - dev, np.mean(total) + dev])

total_runtimes = np.squeeze(np.array(total_runtimes, dtype=np.float32), axis=-1)

# test if data is normal
res_normal = np.zeros((len(total_runtimes)), dtype=np.float32)
for i in range(len(total_runtimes)):
    res_normal[i] = np.round(shapiro(total_runtimes[i])[1], 4)

print(res_normal)
print(multipletests(res_normal, method="fdr_bh")[1])

df = pd.DataFrame(total_runtimes, curr_engine_and_device)

# test significance between total runtime between IEs (assuming normal)
res = np.round(sp.posthoc_ttest(total_runtimes, p_adjust="fdr_by"), 4)

print(total_runtimes.shape)
print(curr_engine_and_device)

# Use Tukey's method for multiple testing instead (works well with groups of the same number of samples)
print(total_runtimes.flatten().dtype)
tmp = np.repeat(range(len(curr_engine_and_device)), 10)
print(tmp.dtype)

print(df)

# perform multiple pairwise comparison (Tukey HSD)
m_comp = pairwise_tukeyhsd(endog=total_runtimes.flatten(), groups=np.repeat(range(len(curr_engine_and_device)), 10), alpha=0.05)
print(m_comp)

all_pvalues = -1 * np.ones((len(curr_engine_and_device), len(curr_engine_and_device)), dtype=np.float32)

ps = m_comp.pvalues
cnt = 0
for i in range(len(curr_engine_and_device)):
    for j in range(i + 1, len(curr_engine_and_device)):
        all_pvalues[i, j] = ps[cnt]
        cnt += 1
all_pvalues = np.round(all_pvalues, 4)#.astype(np.str)
print(all_pvalues.shape)
all_pvalues = all_pvalues[:-1, 1:]
print(all_pvalues.shape)

names = curr_engine_and_device.copy()
names = [n.replace(" / ", "_") for n in names]
names = [n.replace("win1", "lowend") for n in names]
names = [n.replace("win2", "highend") for n in names]
for i, n in enumerate(names):
    if ("TensorRT" in n) or ("CUDA" in n):
        continue
    names[i] = n.replace("ANY_", "CPU_")
#names = [n.replace("ANY_", "CPU_") for n in names]
names = [n.replace("_", "-") for n in names]

for i, n in enumerate(names):
    tmp = n.split_custom("-")
    if "inceptionv3" in n:
        names[i] = n.replace("-inceptionv3", "\_InceptionV3")
        #names[i] = n.replace("-inceptionv3", "") + "-InceptionV3"
    #elif ((tmp[0] == "1") and ("inceptionv3" not in n)):
    #    names[i] = n + "-MobileNetV2"

for i, n in enumerate(names):
    tmp = n.split_custom("-")
    if tmp[3] == "ubuntu":
        if tmp[0] == "1":
            if tmp[2] == "ANY0":
                names[i] = n.replace("-" + tmp[2], "-" + "RTX2070")
            else:
                names[i] = n.replace("-" + tmp[2], "-" + "P5000")
        else:
            if "ANY" in tmp[2]:
                names[i] = n.replace("-" + tmp[2], "-" + "P5000")
    elif tmp[3] == "highend":
        if tmp[2] == "ANY0":
            names[i] = n.replace("-" + tmp[2], "-" + "MaxQ")

col_names = names.copy()
col_names = ["\textbf{\rot{\multicolumn{1}{r}{" + n + "}}}" for n in col_names]

print(tabulate(all_pvalues))

out_pd = pd.DataFrame(data=all_pvalues, index=names[:-1], columns=col_names[1:])
stack = out_pd.stack()
stack[(0 < stack) & (stack <= 0.001)] = '\cellcolor{green!25}$<$0.001'
#stack[(0 < stack) & (stack <= 0.001)] = '\cellcolor{green!25}$<$0.001'
#stack[(stack > 0.001) & (stack < 0.05)] = '\cellcolor{green!25}$<$0.001'
for i in range(stack.shape[0]):
    try:
        curr = stack[i]
        #print(curr)
        if (float(curr) > 0.0011) & (float(curr) < 0.05):
            stack[i] = '\cellcolor{green!50}$' + str(np.round(stack[i], 3))
        elif (float(curr) >= 0.05):
            stack[i] = '\cellcolor{red!25}$' + str(np.round(stack[i], 3))
    except Exception:
        #print(curr)
        continue

#stack[stack == "-1.0"] = "-"
#stack[stack == "0.001"] = "<0.001"
out_pd = stack.unstack()
out_pd = out_pd.replace(0.0, " ")
out_pd = out_pd.replace(-1.0, "-")
#out_pd = out_pd.replace(0.001, '\cellcolor{green!25}$<$0.001')

with open("./test_latex_table.txt", "w") as pfile:
    pfile.write("{}".format(out_pd.to_latex(escape=False, column_format="r" + "c"*all_pvalues.shape[1], 
        bold_rows=True)))


print(sum(multipletests(res_normal, method="fdr_bh")[1] < 0.05))
