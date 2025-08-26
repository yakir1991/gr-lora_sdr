#!/usr/bin/env python3
"""Generate static permutation tables for LoRa interleaver."""

MAX_SF = 12
MIN_SF = 7
CW_LEN = 8

print("/* Auto-generated file. Do not edit manually. */")
print("#ifndef LORA_INTERLEAVER_TABLES_H")
print("#define LORA_INTERLEAVER_TABLES_H")
print("#include <stdint.h>\n")
print("static const uint8_t LORA_INTERLEAVE_PERM[%d][%d][%d][%d] = {" % (MAX_SF - MIN_SF + 1, MAX_SF, CW_LEN, MAX_SF))
for sf in range(MIN_SF, MAX_SF + 1):
    print("    { /* sf = %d */" % sf)
    for sf_app in range(1, MAX_SF + 1):
        print("        { /* sf_app = %d */" % sf_app)
        for i in range(CW_LEN):
            row = []
            for j in range(MAX_SF):
                if j < sf_app and sf_app <= sf:
                    idx = (i + sf_app - j - 1) % sf_app
                    row.append(str(idx))
                else:
                    row.append("0")
            print("            { %s }," % ", ".join(row))
        print("        },")
    print("    },")
print("};\n")
print("#endif /* LORA_INTERLEAVER_TABLES_H */")
