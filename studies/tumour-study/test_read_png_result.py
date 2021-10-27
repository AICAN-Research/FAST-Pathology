import matplotlib.pyplot as plt
import numpy as np


I = plt.imread("./pred_tumour_seg_1.png")

print(I.shape)

print(I.dtype, type(I))
print(np.unique(I))