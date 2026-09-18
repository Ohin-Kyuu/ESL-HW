import csv

import cv2
import matplotlib.pyplot as plt

img1 = cv2.imread("img/box.png", cv2.IMREAD_GRAYSCALE)
img2 = cv2.imread("img/box_in_scene.png", cv2.IMREAD_GRAYSCALE)

orb = cv2.ORB_create(nfeatures=256)
kp1, _ = orb.detectAndCompute(img1, None)
kp2, _ = orb.detectAndCompute(img2, None)

print(f"Keypoints: img1={len(kp1)}, img2={len(kp2)}")

# Read HW results
matches = []
with open("result/results.csv", "r") as f:
    for row in csv.DictReader(f):
        q = int(row["query_idx"])
        t = int(row["best_train_idx"])
        d = float(row["min_hamming_dist"])
        # skip padded zero-descriptor
        if q < len(kp1) and t < len(kp2):
            matches.append(cv2.DMatch(q, t, d))

THRESHOLD = 80  # min_hamming_dist threshold
good = [m for m in matches if m.distance <= THRESHOLD]

print(f"Total matches: {len(matches)} | Good (dist≤{THRESHOLD}): {len(good)}")

# Show
out = cv2.drawMatches(
    img1, kp1, img2, kp2, good, None, flags=cv2.DrawMatchesFlags_NOT_DRAW_SINGLE_POINTS
)

cv2.imwrite("result/matches.png", out)

plt.figure(figsize=(16, 6))
plt.imshow(out)
plt.axis("off")
plt.tight_layout()
plt.savefig("result/matches.png", dpi=150)
plt.show()
