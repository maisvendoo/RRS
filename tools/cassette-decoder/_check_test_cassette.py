# -*- coding: utf-8 -*-
# Проверка test-cassette.kr читателем (зеркало логики декодера).
import hashlib
import struct
import sys
import zlib
from Crypto.Cipher import AES

MASTER_KEY = bytes([
    0x8e,0x12,0x7a,0x45,0xd3,0x6b,0x9f,0x21,
    0xc8,0x54,0x0e,0x77,0x3b,0xaa,0x91,0x2d,
    0x64,0xf0,0x18,0xc5,0x9e,0x37,0x72,0x0b,
    0xad,0x49,0x86,0xe1,0x53,0x0c,0xbf,0x25])

def unpkcs7(d):
    return d[:-d[-1]]

def varint(b, o):
    r = 0; sh = 0
    while True:
        x = b[o]; o += 1
        r |= (x & 0x7F) << sh; sh += 7
        if not (x & 0x80):
            break
    if r >= 1 << 63:
        r -= 1 << 64
    return (r >> 1) ^ (-1 if r & 1 else 0), o

data = open(sys.argv[1] if len(sys.argv) > 1 else
            "test-cassette.kr", "rb").read()
assert data[:16] == b"RRS-KR-CASSETTE\x00", "magic"
assert struct.unpack_from("<H", data, 16)[0] == 1

o = 23
serial = None
blocks = {}
counts = {}
channels = {}
integrity_ok = None
while o < len(data) - 12:
    start = o
    bid, ln = struct.unpack_from("<II", data, o)
    o += 8
    cipher = data[o:o + ln]
    crc = struct.unpack_from("<I", data, o + ln)[0]
    o += ln + 4
    assert crc == (zlib.crc32(data[start:o - 4]) & 0xFFFFFFFF), f"CRC блока {bid}"
    counts[bid] = counts.get(bid, 0) + 1

    if bid == 0:
        n = struct.unpack_from("<H", cipher, 0)[0]
        serial = cipher[2:2 + n].decode()
        continue

    key = hashlib.sha256(MASTER_KEY + serial.encode()).digest()
    iv = hashlib.sha256(serial.encode() +
                        struct.pack("<Q", start)).digest()[:16]
    plain = unpkcs7(AES.new(key, AES.MODE_CBC, iv).decrypt(cipher))

    if bid == 1:
        p = 2 + len(serial)  # serial пропускаем
        def rdstr(b, off):
            n = struct.unpack_from("<H", b, off)[0]
            return b[off + 2:off + 2 + n].decode(), off + 2 + n
        loco, p = rdstr(plain, p)
        start_s, p = rdstr(plain, p)
        p += 16
        acount = struct.unpack_from("<H", plain, p)[0]; p += 2
        for _ in range(acount):
            cid = struct.unpack_from("<H", plain, p)[0]; p += 2
            name, p = rdstr(plain, p)
            unit, p = rdstr(plain, p)
            p += 12
            channels[cid] = {"name": name, "rate": 2.0, "data": []}
    elif bid == 5:
        cid, t0, cnt = struct.unpack_from("<HfI", plain, 0)
        p = 10
        acc = 0.0
        for i in range(cnt):
            d, p = varint(plain, p)
            acc += d / 100.0
            channels[cid]["data"].append((t0 + i / channels[cid]["rate"], acc))
    elif bid == 9:
        master_hash = plain[:32]
        actual = hashlib.sha256(data[:start]).digest()
        sig = plain[40:72]
        sig_actual = hashlib.sha256(MASTER_KEY + actual + b"RRS-KR-SIG").digest()
        integrity_ok = (master_hash == actual and sig == sig_actual)

assert data[-12:-4] == b"KR-END\x00\x00", "футер"
file_crc = struct.unpack_from("<I", data, len(data) - 4)[0]
assert file_crc == (zlib.crc32(data[:-4]) & 0xFFFFFFFF), "CRC файла"

speed = channels[1]["data"]
vmax = max(v for _, v in speed)
print("Блоки:", {k: v for k, v in sorted(counts.items())})
print("Серийник:", serial)
print("Каналов:", len(channels))
for cid in (1, 3, 4, 6, 12):
    c = channels[cid]
    print(f"  канал {cid} {c['name']}: {len(c['data'])} сэмплов, "
          f"min={min(v for _, v in c['data']):.1f} "
          f"max={max(v for _, v in c['data']):.1f}")
er = channels[12]["data"]
tm = channels[3]["data"]
lead = sum(1 for i in range(len(er))
           if er[i][1] < tm[i][1] - 0.05)
max_diff = max(tm[i][1] - er[i][1] for i in range(len(er)))
# УР опережает ТМ в переходных эпизодах ступени (в установившемся
# режиме давления равны - как на реальной ленте)
print(f"УР опережает ТМ при ступенях: {lead} сэмплов, "
      f"макс. расхождение {max_diff:.2f} кгс/см2")
assert lead >= 5 and max_diff > 0.1, "УР не ведёт ТМ"
print(f"Макс. скорость: {vmax:.1f} км/ч")
print("Целостность (мастер-хеш + подпись):",
      "OK" if integrity_ok else "FAIL")
assert integrity_ok
print("ФАЙЛ КОРРЕКТЕН — готов к открытию в декодере")
