import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import cv2

# read data from file
data = np.loadtxt("./plt/heatmap.dat")
# h2 = cv2.getGaussianKernel(4, 1) * cv2.getGaussianKernel(4, 1).T
# data = cv2.filter2D(data, -1, h2, borderType=cv2.BORDER_REPLICATE)

# create heatmap
# plt.imshow(data, cmap='hot', interpolation='nearest')
sns.heatmap(data, square=True, cmap='jet')
plt.show()
