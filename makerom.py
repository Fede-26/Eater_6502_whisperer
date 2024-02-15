rom = bytearray([0xEA] * 0x8000)

with open("rom.bin", "wb") as f:
    f.write(rom)