# -*- coding: utf-8 -*-
"""
Генератор тестовой кассеты .kr (формат CassetteRecorderSystem).

Симулирует рейс ~10 минут: разгон, ход по перегону, зона ограничения
40 км/ч с зарегистрированным превышением, торможение к станции,
стоянка, повторный разгон. Пишет файл test-cassette.kr рядом со
страницей расшифровки (index.html) для проверки декодера.

Требует: pip install pycryptodome
"""
import hashlib
import math
import os
import struct
import zlib
from Crypto.Cipher import AES

# Мастер-ключ формата — тот же, что в kr-crypto CassetteRecorderSystem
MASTER_KEY = bytes([
    0x8e, 0x12, 0x7a, 0x45, 0xd3, 0x6b, 0x9f, 0x21,
    0xc8, 0x54, 0x0e, 0x77, 0x3b, 0xaa, 0x91, 0x2d,
    0x64, 0xf0, 0x18, 0xc5, 0x9e, 0x37, 0x72, 0x0b,
    0xad, 0x49, 0x86, 0xe1, 0x53, 0x0c, 0xbf, 0x25])
MAGIC = b"RRS-KR-CASSETTE\x00"
END_MARKER = b"KR-END\x00\x00"

BLOCK_BOOTSTRAP = 0
BLOCK_SESSION = 1
BLOCK_ANALOG = 5
BLOCK_DISCRETE = 6
BLOCK_BUTTON = 7
BLOCK_PATH = 8
BLOCK_INTEGRITY = 9
BLOCK_SIGNALS = 10

ANALOG = [  # id, name, unit, min, max, rate
    (1, "ActualSpeed", "km/h", 0, 400, 2.0),
    (2, "AllowedSpeed", "km/h", 0, 400, 2.0),
    (3, "BrakePipePressure", "kgf/cm2", 0, 10, 2.0),
    (4, "BrakeCylinderPressure", "kgf/cm2", 0, 6, 2.0),
    (5, "MainReservoirPressure", "kgf/cm2", 0, 10, 2.0),
    (6, "TractionMotorCurrent", "A", 0, 2000, 2.0),
    (7, "OverheadLineVoltage", "V", 0, 40000, 2.0),
    (8, "TractionForce", "tf", 0, 100, 2.0),
    (9, "BrakeForce", "tf", 0, 100, 2.0),
    (10, "ControllerPosition", "pos", 0, 60, 2.0),
    (11, "Acceleration", "m/s2", 0, 5, 2.0),
    (12, "EqualizingReservoirPressure", "kgf/cm2", 0, 6, 2.0),
    (13, "Direction", "dir", -1, 1, 2.0),
    (14, "VigilanceLevel", "%", 0, 100, 2.0),
    (15, "EPTMode", "mode", 0, 2, 2.0),
]
DISCRETE = ["EPK", "RB", "SAUT", "TSKBM", "Compressor", "Fan", "SandSystem",
            "TractionMode", "RegenMode", "EmergencyBrake", "Horn", "Doors",
            "LightsOn", "HeaterOn", "RoofEquipment", "BV",
    "ControlGenerator", "EPKKey", "Whistle"]
BIT = {name: i for i, name in enumerate(DISCRETE)}

SAMPLE_DT = 0.5          # 2 Гц
FLUSH_EVERY = 10.0       # сэмплов на чанк (имитация сброса на диск)
SERIAL = "test-kr-0001"


def pkcs7(data):
    pad = 16 - (len(data) % 16)
    return data + bytes([pad]) * pad


def put_varint(value):
    zz = (value << 1) ^ (~0 if value < 0 else 0)
    if zz < 0:
        zz += 1 << 64
    out = bytearray()
    while zz >= 0x80:
        out.append((zz & 0xFF) | 0x80)
        zz >>= 7
    out.append(zz)
    return bytes(out)


def s(txt):
    b = txt.encode("utf-8")
    return struct.pack("<H", len(b)) + b


class Writer:
    def __init__(self, serial):
        self.buf = bytearray()
        self.serial = serial
        header = bytearray(MAGIC)
        header += struct.pack("<HB", 1, 1)
        header += struct.pack("<I", zlib.crc32(bytes(header)) & 0xFFFFFFFF)
        self.buf += header
        self._block(BLOCK_BOOTSTRAP, s(serial), plain=True)

    def session_key(self):
        return hashlib.sha256(MASTER_KEY + self.serial.encode()).digest()

    def _block(self, block_id, payload, plain=False):
        if plain:
            cipher = payload
        else:
            key = self.session_key()
            iv = hashlib.sha256(self.serial.encode() +
                                struct.pack("<Q", len(self.buf))).digest()[:16]
            cipher = AES.new(key, AES.MODE_CBC, iv).encrypt(pkcs7(payload))
        block = struct.pack("<II", block_id, len(cipher)) + cipher
        block += struct.pack("<I", zlib.crc32(block) & 0xFFFFFFFF)
        self.buf += block

    def session_info(self, loco, start, mass_t, axles):
        p = s(self.serial) + s(loco) + s(start)
        p += struct.pack("<dd", mass_t, axles)
        p += struct.pack("<H", len(ANALOG))
        for cid, name, unit, lo, hi, rate in ANALOG:
            p += struct.pack("<H", cid) + s(name) + s(unit)
            p += struct.pack("<fff", lo, hi, rate)
        p += struct.pack("<H", len(DISCRETE))
        for name in DISCRETE:
            p += s(name)
        # Хвост метаданных: поезд/бригада/состав
        p += s("804")            # номер поезда
        p += s("00099")          # табельный машиниста
        p += s("Грузовой")       # категория
        p += s("1737")           # номер локомотива
        p += struct.pack("<d", 166.0 / 4.0)  # усл. вагоны
        self._block(BLOCK_SESSION, p)

    def analog_chunk(self, cid, t0, deltas):
        p = struct.pack("<HfI", cid, t0, len(deltas))
        for d in deltas:
            p += put_varint(d)
        self._block(BLOCK_ANALOG, p)

    def discrete_chunk(self, events):
        p = struct.pack("<I", len(events))
        for t, mask in events:
            p += struct.pack("<fI", t, mask)
        self._block(BLOCK_DISCRETE, p)

    def button_chunk(self, events):
        p = struct.pack("<I", len(events))
        for e in events:
            p += struct.pack("<ddf", e[0], e[1], e[2])   # t, coord, speed
            p += struct.pack("<H", e[3])                  # action_id
            p += s(e[4]) + s(e[5])                        # name, param
            p += struct.pack("<BBf", e[6], e[7], e[8])    # prev, next, dur
        self._block(BLOCK_BUTTON, p)

    def path_chunk(self, records):
        p = struct.pack("<I", len(records))
        for t, coord, grad, curv, lim in records:
            p += struct.pack("<ddfff", t, coord, grad, curv, lim)
        self._block(BLOCK_PATH, p)

    def signal_chunk(self, records):
        """Сигнальная информация: ближайший светофор впереди по ходу.
        aspect: 0 - красный, 1 - жёлтый, 2 - зелёный"""
        p = struct.pack("<I", len(records))
        for t, coord, aspect, dist, kind in records:
            p += struct.pack("<fdBIB", t, coord, aspect, dist, kind)
        self._block(BLOCK_SIGNALS, p)

    def finish(self):
        master_hash = hashlib.sha256(bytes(self.buf)).digest()
        key = self.session_key()
        fingerprint = hashlib.sha256(key).digest()[:8]
        signature = hashlib.sha256(MASTER_KEY + master_hash +
                                   b"RRS-KR-SIG").digest()
        self._block(BLOCK_INTEGRITY, master_hash + fingerprint + signature)
        self.buf += END_MARKER
        self.buf += struct.pack("<I", zlib.crc32(bytes(self.buf)) & 0xFFFFFFFF)
        return bytes(self.buf)


# --------------------------------------------------------------------------
# Симуляция рейса (загрузка ~3400 т, ВЛ60к)
# --------------------------------------------------------------------------
def simulate():
    n = int(600 / SAMPLE_DT) + 1            # 600 с, 2 Гц
    times = [i * SAMPLE_DT for i in range(n)]

    # Профиль пути: пикет/уклон/лимит по контрольным точкам
    path_ctrl = [
        (0.0,    0.0,   2.0, 0.0,   70.0),   # старт, подъём
        (60.0,   700.0,  4.0, 0.0,   70.0),
        (140.0,  2200.0, 0.0, 0.003, 70.0),  # вершина, кривая R=330
        (220.0,  4100.0, -3.0, 0.0,  70.0),  # спуск
        (300.0,  6000.0, -2.0, 0.0,   70.0),
        (330.0,  6700.0, -2.0, 0.0,   40.0),  # ЗОНА 40 (работы)
        (450.0,  8600.0, -2.0, 0.0,   40.0),
        (480.0,  9200.0,  0.0, 0.0,   70.0),  # конец зоны
        (540.0,  10400.0, 0.0, 0.0,  70.0),
        (560.0,  10800.0, 0.0, 0.0,   25.0),  # станция
        (600.0,  11200.0, 0.0, 0.0,  25.0),
    ]

    def path_at(t):
        for i in range(len(path_ctrl) - 1):
            a, b = path_ctrl[i], path_ctrl[i + 1]
            if a[0] <= t < b[0]:
                k = (t - a[0]) / (b[0] - a[0])
                return tuple(a[j] + k * (b[j] - a[j]) for j in range(5))
        return path_ctrl[-1]

    # Целевая скорость машиниста (с опозданием перед зоной 40 - нарушение)
    def target_speed(t):
        if t < 20:
            return min(60.0, t * 3.0)          # разгон
        if t < 320:
            return 60.0
        if t < 470:                             # зона 40: начал поздно
            return 40.0 if t > 355 else 60.0    # 330..355 ещё 60!
        if t < 530:
            return 40.0
        if t < 555:
            return max(0.0, 40.0 - (t - 530) * 1.8)   # к станции
        if t < 585:
            return 0.0                          # стоянка
        return min(25.0, (t - 585) * 2.5)       # разгон

    speeds, allowed, tm, tc, nm, cur, volt, f_tr, f_br, pos, accel, er, \
    direction, vigilance, ept = \
        [], [], [], [], [], [], [], [], [], [], [], [], [], [], []
    discrete, buttons, path_rec, signal_rec = [], [], [], []

    # Светофор впереди по ходу: пикет 7160 м. Аспекты АЛСН:
    # 0=К 1=КЖ 2=Ж 3=З 4=Б. Демо нарушений: перекрытие З->КЖ (461),
    # сбой кодов З->Б (250..255), К (470..575, проезд), скатывание 590
    SIG_COORD = 7160.0

    def sig_aspect(t):
        if 250.0 <= t < 255.0:
            return 4  # сбой: Б при З
        if t < 461.0:
            return 3  # З
        if t < 470.0:
            return 1  # КЖ (перекрытие с З)
        if t < 473.0:
            return 0  # К
        if t < 478.0:
            return 4  # Б: выключение К (демо)
        if t < 575.0:
            return 0  # К
        return 3

    v = 0.0
    coord = 0.0
    mask = 0
    prev_tm, prev_nm, prev_tc, prev_er = 5.0, 9.0, 0.0, 5.0
    epk_on = False

    def setbit(name, on):
        nonlocal mask
        if on:
            mask |= 1 << BIT[name]
        else:
            mask &= ~(1 << BIT[name])

    for i, t in enumerate(times):
        vt = target_speed(t)
        # Разгон/торможение с ограничением ускорения
        a = max(-0.7, min(0.5, (vt - v) * 0.5))
        if 590.0 <= t < 596.0:
            vt = -2.0  # скатывание назад
        if t > 550 and v < 1.0 and t < 589.0:
            v = 0.0
            a = 0.0
        v = max(0.0, v + a * SAMPLE_DT)
        accel.append(round(a, 2))
        # Направление: демо скатывания назад на 590-596 с
        if 590.0 <= t < 596.0:
            direction.append(-1.0)
        else:
            direction.append(1.0)
        # Бодрость ТСКБМ: спадает без РБ, РБ (каждые 60 с) восстанавливает
        vigilance.append(round(max(20.0, 100.0 - (t % 60.0) * 1.2), 0))
        # ЭПТ: служебный режим при торможении с 530-й
        ept.append(1.0 if (a < -0.05 and t >= 530.0) else 0.0)
        speeds.append(round(v, 1))

        _, _, _, _, lim = path_at(t)
        allowed.append(lim)

        # Торможение к скорости (ступень ТМ)
        braking = a < -0.05
        target_tm = 4.3 if braking else (4.0 if a < -0.4 else 5.0)
        # УР ведёт ТМ: машинист снижает УР, ТМ следует за ним
        er_rate = 0.08 if target_tm < prev_er else 0.03
        prev_er += max(-er_rate, min(er_rate, target_tm - prev_er))
        er.append(round(prev_er, 2))

        tm_delta = 0.06 if target_tm < prev_tm else 0.02
        prev_tm += max(-tm_delta, min(tm_delta, target_tm - prev_tm))
        tm.append(round(prev_tm, 2))

        # ТЦ следует за глубиной ступени
        target_tc = max(0.0, (5.0 - prev_tm)) * 1.6
        prev_tc += max(-0.15, min(0.15, target_tc - prev_tc))
        tc.append(round(prev_tc, 2))

        # НМ: проседает при торможении, компрессор качает
        load = 0.4 if braking else 0.05
        prev_nm += (9.0 - prev_nm) * 0.02 - load * 0.03
        prev_nm = min(9.4, max(7.2, prev_nm))
        nm.append(round(prev_nm, 2))

        # Тяга: ток от ускорения/скорости
        traction = a > 0.05 and t < 555
        tr_ok = (0 <= t < 310) or (480 <= t < 530) or t >= 585
        i_motor = 0.0
        if traction and tr_ok:
            i_motor = min(950.0, 300.0 + 900.0 * a / 0.5 * (1 - v / 90.0))

        # ДЕМО-НАРУШЕНИЕ: торможение при тяге (356-364 с, позднее
        # торможение к зоне 40 при включённой тяге)
        if 356.0 <= t < 364.0:
            i_motor = 420.0
        cur.append(round(i_motor, 0))
        volt.append(25000.0 if t > 15 else 0.0)   # подъём токоприёмника
        f_tr.append(round(i_motor * 0.018, 1))
        f_br.append(round(prev_tc * 12.0, 1))
        pos.append(0 if not traction else min(15, int(4 + a * 10)))

        coord += v / 3.6 * SAMPLE_DT

        # Путь (1 Гц)
        if i % 2 == 0:
            _, _, grad, curv, lim2 = path_at(t)
            path_rec.append((t, round(coord, 1), round(grad, 1),
                             round(curv, 5), lim2))

        # Светофор впереди (1 Гц): позиция/показание ближайшего сигнала
        if i % 2 == 0:
            signal_rec.append((t, SIG_COORD, sig_aspect(t),
                                max(0, round(SIG_COORD - coord)),
                                0))  # kind 0 = светофор

        # Дискретные события
        new_mask = mask
        events_here = []

        if t >= 10 and not epk_on:
            epk_on = True
            events_here.append(("Тумблер ЭПК", "Вкл", 0, 1))
        if i == 40:
            events_here.append(("Свисток", "", 0, 1))
        if i == 42:
            events_here.append(("Свисток", "", 1, 0))
        # РБ каждые ~60 с
        if i > 0 and i % 120 == 0:
            events_here.append(("Кнопка РБ", "", 0, 1))
        if i > 0 and i % 120 == 1:
            events_here.append(("Кнопка РБ", "", 1, 0))
        # Позиции КМ при смене режима
        if i in (0, 40, 320, 328, 356, 470, 528, 556, 584, 600, 620):
            events_here.append(("Ручка КМ",
                                f"позиция {pos[-1]:.0f}", 0, pos[-1]))
        # Песок на подъёме после стоянки
        if i == 604:
            events_here.append(("Песочная система", "Вкл", 0, 1))
        if i == 640:
            events_here.append(("Песочная система", "Выкл", 1, 0))

        for name, param, was, now in events_here:
            buttons.append((t, round(coord, 1), round(v, 1), 0,
                            name, param, was, now, 0.3 if "РБ" in name or
                            "Свисток" in name else 0.0))

        # Маска состояний
        # ДЕМО-НАРУШЕНИЕ: срыв ЭПК на 500-506 с при движении
        epk_now = epk_on and not (500.0 <= t < 506.0)
        setbit("EPK", epk_now)
        setbit("TractionMode", bool(cur[-1] > 50))
        setbit("RegenMode", False)
        setbit("Compressor", nm[-1] < 8.9)
        setbit("Fan", epk_on)
        setbit("SAUT", not (100.0 <= t < 160.0))
        setbit("TSKBM", not (200.0 <= t < 240.0))
        setbit("ControlGenerator", epk_on)
        setbit("EPKKey", epk_now)
        setbit("Whistle", i in (41, 42))
        setbit("SandSystem", 604 <= i <= 640)
        setbit("EmergencyBrake", False)
        setbit("Horn", i in (40, 41))
        setbit("RoofEquipment", t > 15)
        setbit("BV", t > 18)

        if new_mask != mask or (i % 20 == 0):
            discrete.append((t, mask))
        new_mask = mask

    return (times, speeds, allowed, tm, tc, nm, cur, volt, f_tr, f_br,
            pos, accel, er, direction, vigilance, ept, discrete, buttons,
            path_rec, signal_rec)


def main():
    (times, speeds, allowed, tm, tc, nm, cur, volt, f_tr, f_br, pos,
     accel, er, direction, vigilance, ept, discrete, buttons,
     path_rec, signal_rec) = simulate()

    w = Writer(SERIAL)
    w.session_info("Test-VL60k-1737", "2026-08-21 09:15:00", 3400.0, 166)

    series = [speeds, allowed, tm, tc, nm, cur, volt, f_tr, f_br, pos,
              accel, er, direction, vigilance, ept]

    # Чанки по FLUSH_EVERY секунд (имитация периодического сброса)
    chunk_len = int(FLUSH_EVERY / SAMPLE_DT)
    for start in range(0, len(times), chunk_len):
        end = min(start + chunk_len, len(times))
        t0 = times[start]
        for cid, values in enumerate(series, start=1):
            deltas = [round(values[start] * 100)]
            for k in range(start + 1, end):
                deltas.append(round((values[k] - values[k - 1]) * 100))
            w.analog_chunk(cid, t0, deltas)

        ev = [(t, m) for t, m in discrete
              if t >= t0 and t < t0 + FLUSH_EVERY]
        if ev:
            w.discrete_chunk(ev)
        pev = [b for b in buttons if t0 <= b[0] < t0 + FLUSH_EVERY]
        if pev:
            w.button_chunk(pev)
        prev = [p for p in path_rec if t0 <= p[0] < t0 + FLUSH_EVERY]
        if prev:
            w.path_chunk(prev)

        sig = [s for s in signal_rec if t0 <= s[0] < t0 + FLUSH_EVERY]
        if sig:
            w.signal_chunk(sig)

    data = w.finish()

    out = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                       "test-cassette.kr")
    with open(out, "wb") as f:
        f.write(data)

    viol = sum(1 for i in range(len(times))
               if allowed[i] > 0 and speeds[i] > allowed[i] + 1)
    print(f"Создан файл: {out}")
    print(f"Размер: {len(data)} байт")
    print(f"Сэмплов на канал: {len(times)} (600 с, 2 Гц)")
    print(f"Событий дискретных: {len(discrete)}, нажатий: {len(buttons)}, "
          f"записей пути: {len(path_rec)}")
    print(f"Сэмплов с превышением: {viol} "
          f"({viol * SAMPLE_DT:.0f} с) - видны в протоколе декодера")
    print("Откройте tools/cassette-decoder/index.html и выберите файл.")


if __name__ == "__main__":
    main()
