# ESL-Final: Hamming Match using HLS & RISC-VP Platform

## Requirements

- CMake >= 3.8
- C++17 compatible compiler (e.g. g++)
- Python3
- SystemC library
- Stratus HLS
- RISC-VP (https://github.com/agra-uni-bremen/riscv-vp.git)

---

## Stratus HLS 
In `hls/`, there are two version of Hamming Match simulation
- `hls/hls/` : Hamming Match without Memory Bank
- `hls/hls_mb` : Hamming Match with Memory Bank

You can see the HLS result report in `stratus/bdw_work/` under each simulation folder.

---
For Stratus HLS synthesizing, do the following to run:
```bash
# 1. Enter the build directory
cd stratus

# 2. Clean build directory
make clean

# 3. Compile the project: 8 options
make sim_B                 # SystemC & behavioral simulation
make sim_V_BASIC           # RTL Basic simulation
make sim_V_DPO             # DPO Region Optimize simulation
make sim_V_UNROLL          # DPO + Unrolling
make sim_V_PIPE1           # DPO + Pipelining (Interval = 1)
make sim_V_PIPE2           # DPO + Pipelining (Interval = 2)
make sim_V_PIPE4           # DPO + Pipelining (Interval = 4)
make sim_V_PIPE_UNROLL     # DPO + Pipelining (Interval = 2) + Unrolling
```

> Repeat the make options in 3. and check the results in `bdw_work/sims/`, `bdw_work/modules`.
---

## RISC-VP Platform
I save the vp binary in `riscv_vp/vp/build/bin/` folder, you can just use it without building virtual platform. Skip the Build, to Run.

### Build
Following the instructions if you want to build on your own:

```bash
# 1. Copy riscv_vp/vp/ code into your RISC-VP Platform
cp riscv_vp/vp/ <your-riscv-vp-path>/vp/src/platform/riscv-vp-acc-hm/

# 2.In <your-riscv-vp-path>/vp/build, generate Makefile with CMake & install platform
cmake .. && make install
```
---

### Run
After building, go back to `riscv_vp` folder, and run the software:

```bash
cd sw/hm/

make sim # Run Hamming Match RISC-VP Platform 
```

Output Match will be generated in the same directory:
- `results.csv`
---

## Python
To Generate 256 query & train descriptors from image, using python.

### Run
```bash
python3 gen_desc.py  # Generate descriptors from image
python3 gen_desc_header.py # Generate C++ header file for descriptors
python3 result_img.py # plot result.csv from Hardware to original image
```

Output Results will be generated in the each directory:
- `data/*.txt, *.h`: gen_desc.py & gen_desc_header.py generated 
- `result/matches.png`: result_img.py result


## SystemC 
To simulate same operation in HLS using SystemC:

### Build
```bash
cd systemC/

# Create build directory 
mkdir build && cd build 

# CMake Build 
cmake .. && make 
```
### Run

```bash
# In build directory, make run to simulate
cd systemC/build
make run
```
