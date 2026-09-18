# ESL-HW4_Median & Scharr Filter with RISC-VP Platform

## Requirements

- CMake >= 3.8
- C++17 compatible compiler (e.g. g++)
- SystemC library
- Stratus HLS
- RISC-VP (https://github.com/agra-uni-bremen/riscv-vp.git)

---

## Build
I save the vp binary in `MedSch_riscv_vp/vp/build/bin/` folder, you can just use it without building virtual platform. Skip the Build, to Run.

Following the instructions if you want to build on your own:

```bash
# 1. Copy MedSch_riscv_vp/vp/src/ code into your RISC-VP Platform
cp MedSch_riscv_vp/vp/src/platform/medsch-acc <your-riscv-vp-path>/vp/src/platform/

# 2.In <your-riscv-vp-path>/vp/build, generate Makefile with CMake & install platform
cmake .. && make install
```
---

## Run

After building, go back to `MedSch_riscv_vp` folder, and run the software:

```bash
cd sw/medsch/

make all
# Default: cameraman.bmp
make sim 

# Other Images: replace lena_color_256.bmp with your images
make sim IMG=lena_color_256.bmp
```

Output images will be generated in the same directory:
- `out_cameraman.bmp`
- `out_lena_color_256.bmp`

---

## Directory Structure

```
.MedSch_riscv_vp/
├── sw/
│   ├── Makefile
│   ├── Makefile.common
│   └── medsch/ # software sources
│       ├── cameraman.bmp
│       ├── lena_color_256.bmp
│       ├── main
│       ├── main.cpp
│       ├── Makefile
│       ├── out_cameraman.bmp
│       ├── out_lena_color_256.bmp
│       └── Testbench.h
└── vp/
    ├── build/
    │   └── bin/ # medsch-acc platform binary
    └── src 
        └── platform/ # medsch-acc platform sources
```
