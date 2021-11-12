import matplotlib.pyplot as plt
import numpy as np


I = plt.imread("./pred_tumour_seg_from_FP_final.png")

print(I.shape)

print(I.dtype, type(I))
print(np.unique(I))

I = (I / np.amax(I)) * 255
I = I.astype(np.uint8)


#print(np.bincount(I))
print(np.histogram(I))


plt.imshow(I)
plt.show()

