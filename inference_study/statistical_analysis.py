import numpy as np
import pandas as pd
from tabulate import tabulate
import os

path = "./results/"
filename = "neural-network-runtimes-case-"

cases = list(range(1, 5)) + ["1_batch", "1_inceptionv3"]

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
                else:
                    total = tmp[:, header == measurement]
                dev = 1.95 * np.std(total)/np.sqrt(len(total))
                print(np.mean(total), dev)

                # print("total CI (mean and standard error): ", np.mean(total), "+-", np.std(total)/np.sqrt(len(total)))
                # print("total CI: ", [np.mean(total) - dev, np.mean(total) + dev])

    #print()
    #print(header)