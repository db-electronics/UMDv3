import os
import glob
import struct
from crccheck.crc import Crc

# works with the CRC32 algorithm used by STM32 microcontrollers but is stupid slow
def crc32_stm32(data, poly=0x04C11DB7, crc=0xFFFFFFFF):
    # Loop over the data in 4-byte chunks
    # Pat Riley CRC on first 512 bytes = E6CF6C16 (little endian)
    # Pat Riley CRC on entire ROM = FA1AC95B
    for i in range(0, 512, 4):
        print(i)
        dword = int.from_bytes(data[i:i+4], 'little')
        crc = crc ^ dword
        for _ in range(32):
            if(crc & 0x80000000):
                crc = (crc << 1) ^ poly
            else:
                crc = crc << 1

    # Return the CRC value
    return crc

# STM32 CRC32 algorithm requires the data to be in little endian
def reverse_endianness(buf):
    dwords = [buf[i:i+4] for i in range(0, len(buf), 4)]
    reversed_dwords = [struct.pack('<I', struct.unpack('>I', dword)[0]) for dword in dwords]
    return b''.join(reversed_dwords)

def calculate_crc32(filename):
    buf = open(filename, 'rb').read()
    # crc = crc32_stm32(buf) & 0xFFFFFFFF
    crcStm32 = Crc(32, 0x04C11DB7, 0xFFFFFFFF, 0, False, False, False)
    buf = reverse_endianness(buf)
    crc = crcStm32.calc(buf)
    result = "%08X" % crc
    print(result)
    return result

# Create a file with the CRC32 checksum as the filename and the ROM name as the contents
def create_file(crc, rom_name, output_directory):
    with open(os.path.join(output_directory, f"{crc}.txt"), 'w') as f:
        f.write(rom_name)

def process_directory(directory, output_directory):
    for filename in glob.glob(os.path.join(directory, '*')):
        if os.path.isfile(filename):
            rom_name = os.path.basename(filename)
            print("processing", rom_name)
            crc = calculate_crc32(filename)
            create_file(crc, rom_name, output_directory)

process_directory('./Genesis', '../SD/UMD/Genesis')