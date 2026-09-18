# ESL-HW1_Median & Scharr Filter (Pro1 & Pro2)

## Requirements

- CMake >= 3.8
- C++17 compatible compiler (e.g. g++)
- SystemC library

---

## Build

```bash
# 1. Create and enter the build directory
cd pro1 # or cd pro2
mkdir build && cd build

# 2. Generate Makefile with CMake
cmake ..

# 3. Compile the project
make
```

> Repeat the steps above in both `pro1/` and `pro2/` directories.

---

## Run

After building, process images via make targets:

```bash
# Process cameraman image
make cam

# Process lena image
make lena
```

Output images will be generated in the `build/` directory:
- `out_cameraman.bmp`
- `out_lena_color_256.bmp`

You can also run the executable manually:

```bash
# Pro1
./pro1 <input_image> <output_image>

# Pro2
./pro2 <input_image> <output_image>

# Example
./pro1 ../img/cameraman.bmp ./out_cameraman.bmp
./pro2 ../img/lena_color_256.bmp ./out_lena_color_256.bmp
```

---

## Verify Output

Compare output images against the golden reference to check correctness:

```bash
# Check cameraman output
make check_cam

# Check lena output
make check_lena
```

If the output matches the golden reference, you will see `Files ... are identical`.

---

## Directory Structure

```
.
├── pro1/
│   ├── CMakeLists.txt
│   ├── *.cpp
│   └── build/             # Generated after cmake
├── pro2/
│   ├── CMakeLists.txt
│   ├── *.cpp
│   └── build/             # Generated after cmake
├── img/
│   ├── cameraman.bmp
│   └── lena_color_256.bmp
└── golden/
    ├── gold_cameraman.bmp
    └── gold_lena_color_256.bmp
```
