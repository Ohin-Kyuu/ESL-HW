import numpy as np
from PIL import Image


def compare_images(img1_path, img2_path):
    try:
        # 開啟圖片
        img1 = Image.open(img1_path)
        img2 = Image.open(img2_path)

        # 1. 檢查基本屬性
        if img1.size != img2.size:
            print(f"[-] 尺寸不同: {img1.size} vs {img2.size}")
            return
        if img1.mode != img2.mode:
            print(f"[-] 色彩模式不同: {img1.mode} vs {img2.mode}")
            return

        # 轉換為 Numpy 陣列進行數值比較
        arr1 = np.array(img1).astype(np.int16)
        arr2 = np.array(img2).astype(np.int16)

        # 2. 計算差異
        diff = arr1 - arr2
        dist = np.abs(diff)

        num_diff = np.count_nonzero(diff)

        if num_diff == 0:
            print("[+] 兩張圖片像素完全相同！")
        else:
            print(f"[*] 共有 {num_diff} 個像素點不同。")
            print(f"[*] 最大差異值 (Max Difference): {np.max(dist)}")
            print(f"[*] 平均絕對誤差 (MAE): {np.mean(dist):.4f}")
            print(f"[*] 均方根誤差 (RMSE): {np.sqrt(np.mean(diff**2)):.4f}")

            # 找到第一個不同點的位置
            coords = np.argwhere(diff != 0)
            print(f"[*] 第一個不同點座標 (y, x): {coords[0][:2]}")
            print(f"    - 檔案A 數值: {arr1[tuple(coords[0])]}")
            print(f"    - 檔案B 數值: {arr2[tuple(coords[0])]}")

            # (選用) 產出差異視覺化圖，這會把有差的地方變亮
            diff_img = Image.fromarray(np.uint8(dist * 255 / np.max(dist)))
            diff_img.save("diff_visualization.png")

    except Exception as e:
        print(f"[-] 發生錯誤: {e}")


if __name__ == "__main__":
    # 你可以直接在這裡填路徑，或用 command line 傳入
    path1 = "out_1.bmp"
    path2 = "out_2.bmp"
    compare_images(path1, path2)
