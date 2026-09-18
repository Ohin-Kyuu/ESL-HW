import cv2
import numpy as np

img1 = cv2.imread("img/box.png", cv2.IMREAD_GRAYSCALE)
img2 = cv2.imread("img/box_in_scene.png", cv2.IMREAD_GRAYSCALE)

# Query: 256 points ; Destination : 256 points
orb = cv2.ORB_create(nfeatures=256)
kp1, des1 = orb.detectAndCompute(img1, None)
kp2, des2 = orb.detectAndCompute(img2, None)


# if not 256 points enough, then pad 0
def pad_descriptors(des, n=256):
    if des is None:
        return np.zeros((n, 32), dtype=np.uint8)
    if des.shape[0] < n:
        padding = np.zeros((n - des.shape[0], 32), dtype=np.uint8)
        return np.vstack((des, padding))
    return des[:n]


des1 = pad_descriptors(des1)
des2 = pad_descriptors(des2)

np.savetxt("data/query.txt", des1, fmt="%d", delimiter=",")
np.savetxt("data/database.txt", des2, fmt="%d", delimiter=",")

print("Loaded：query.txt, database.txt")
