import numpy as np
import pandas as pd
from tabulate import tabulate
np.set_printoptions(suppress=True)
np.set_printoptions(threshold=np.inf)
np.set_printoptions(precision=8, suppress=False)
import pandas as pd
pd.options.display.float_format = '{:.8f}'.format


loc = "./results-pipeline-runtime.csv"
df = pd.read_csv(loc, sep=";", header=0)

nb_iters = 10
IEs = ["OpenVINO", "TensorRT"]

ret = []
for i in range(2):
    tmp = df.iloc[int(i * nb_iters):int((i + 1) * nb_iters), :]
    print("----")
    print(tmp)
    print("-")
    print(np.mean(tmp, axis=0) / (10**9), np.std(tmp, axis=0) / (10**9))
    print()
