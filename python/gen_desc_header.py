import os

INPUTS = [
    {
        "input_file": "data/train.txt",
        "array_name": "train_descriptors",
        "output_file": "data/train_desc.h",
    },
    {
        "input_file": "data/query.txt",
        "array_name": "query_descriptors",
        "output_file": "data/query_desc.h",
    },
]

for cfg in INPUTS:
    with open(cfg["input_file"], "r") as fin:
        lines = fin.readlines()

    guard = os.path.basename(cfg["output_file"])
    guard = guard.upper().replace(".", "_")

    with open(cfg["output_file"], "w") as fout:
        fout.write(f"#ifndef {guard}\n")
        fout.write(f"#define {guard}\n\n")

        fout.write(f"static const unsigned char {cfg['array_name']}[256][32] = {{\n")

        for line in lines[:256]:
            vals = [v.strip() for v in line.split(",")]
            fout.write("    {" + ", ".join(vals) + "},\n")

        fout.write("};\n\n")
        fout.write(f"#endif // {guard}\n")

    print(f"Generated {cfg['output_file']}")
