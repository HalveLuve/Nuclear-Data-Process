import struct
import os
import scipy.io as sio
import numpy as np

lut_file = '../data/LUT.mat'
lut = sio.loadmat(lut_file)
lut = lut['LUT'].squeeze(1)

path = '../data/data0423_group_2_4test.ldat'  # 共含483314个event
save_path_1 = '../data/0-63'
if not os.path.exists(save_path_1):
    os.makedirs(save_path_1)
save_path_2 = '../data/256-319'
if not os.path.exists(save_path_2):
    os.makedirs(save_path_2)
# 判断大小端
cmd = 'echo -n I | od -o | head -n1 | cut -f2 -d" " | cut -c6'
out = os.popen(cmd)
output = int(out.readlines()[0][0])
pattern = None
# 2*uint8_t, 1*long long, 1*float, 1*int
if output == 1:
    pattern = '<2B1q1f1i'
elif output == 0:
    pattern = '>2B1q1f1i'

cnt_1 = 0
cnt_2 = 0
with open(path, 'rb') as f:
    while True:
        event = np.zeros((8, 8), dtype=np.float32)
        line = f.read(18)
        if not line:
            break
        line_tuple = struct.unpack(pattern, line)
        # tuple: (非0SiPM总数, 序号, 时间戳, 能量读数, SiPM电路ID)
        num = line_tuple[0]
        idx = line_tuple[1]
        timestamp = line_tuple[2]
        energy = line_tuple[3]

        sipm = line_tuple[4]
        arr = lut[sipm % 64]
        event[arr // 8][arr % 8] = energy

        while idx < num-1:
            line = f.read(18)
            if not line:
                break
            line_tuple = struct.unpack(pattern, line)

            # tuple: (非0SiPM总数, 序号, 时间戳, 能量读数, SiPM电路ID)
            num = line_tuple[0]
            idx = line_tuple[1]
            timestamp = line_tuple[2]
            energy = line_tuple[3]

            sipm = line_tuple[4]
            arr = lut[sipm % 64]
            event[arr // 8][arr % 8] = energy

            if idx == num-1:
                print(f"\rProcessed events {cnt_1+cnt_2}", end=" ")
                if sipm < 63:
                    cnt_1 += 1
                    np.save(os.path.join(save_path_1, f'event-{cnt_1}.npy'), event)
                elif 255 < sipm < 320:
                    cnt_2 += 1
                    np.save(os.path.join(save_path_2, f'event-{cnt_2}.npy'), event)
        # print(line_tuple)

