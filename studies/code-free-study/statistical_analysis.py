import numpy as np
import pandas as pd
from tabulate import tabulate
np.set_printoptions(suppress=True)
np.set_printoptions(threshold=np.inf)


loc = "./build/results_neural-network-runtime.csv"
df = pd.read_csv(loc, sep=";", header=0)

nb_iters = 10 + 1

# engines = np.unique(df["Engine"])
IEs = ["OpenVINO CPU", "OpenVINO GPU", "TensorRT"]

ret = []
for i in range(3):
    tmp = df.iloc[int(i * nb_iters):int((i + 1) * nb_iters), :]
    print("----")
    print(tmp)
    print("-")
    print(np.mean(tmp, axis=0), np.std(tmp, axis=0))
    print()

#print(df)
