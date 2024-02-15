rom = bytearray([0xEA] * 0x8000)

rom[0x7ffc] = 0x12
rom[0x7ffd] = 0x80

with open("rom.bin", "wb") as f:
    f.write(rom)