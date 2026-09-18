# ESL-HW3_Median & Scharr Filter with Stratus HLS

## Requirements

- CMake >= 3.8
- C++17 compatible compiler (e.g. g++)
- SystemC library
- Stratus HLS library

---

## Build

```bash
# 1. Enter the build directory
cd stratus

# 2. Clean build directory
make clean

# 3. Compile the project: 8 options
make sim_B                 # SystemC & behavioral simulation
make sim_V_BASIC           # RTL Basic simulation
make sim_V_DPA             # DPO Region Optimize simulation
make sim_V_UNROLL          # DPA + Unrolling
make sim_V_PIPE1           # DPA + Pipelining (Interval = 1)
make sim_V_PIPE2           # DPA + Pipelining (Interval = 2)
make sim_V_PIPE4           # DPA + Pipelining (Interval = 4)
make sim_V_PIPE_UNROLL     # DPA + Pipelining (Interval = 2) + Unrolling
```

> Repeat the make options in 3. and check the results in `bdw_work/sims/`, `bdw_work/modules`.

---

## Other Picture Results

for `512x512` cameraman picture, following:

```bash
# 1. Clean the build directory
make clean
# Process cameraman image
make {options}_cam
# replace options with 8 options above, ex: make sim_B_cam
```

## Directory Structure

```
.
├── files/
│   ├── *.h
│   ├── *.cpp
│   └── img/
│   │   ├── cameraman.bmp
│   │   └── lena_color_256.bmp
│   └── stratus/
│       ├── project.tcl
│       └── Makefile

```
