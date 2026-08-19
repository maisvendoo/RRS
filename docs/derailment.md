# Система схода с рельсов (ТЗ «Динамика ПС» п.11-13)

Постепенный сход по физическим датчикам — не случайное событие и не
телепортация. Машина состояний `VehicleDerailment`
(`simulator/vehicle/vehicle-derailment`), управляемая поперечной
динамикой (Y/Q, смещения осей) и вертикальной (разгрузка колёс):

```
OnTrack -> WheelLiftWarning -> AxleDerailed -> BogieDerailed -> VehicleDerailed
```

- **Подъём гребня** (перевод в AxleDerailed оси): устойчивое время
  (FlangeClimbTime, умолчание 50 мс) выполнения любого условия:
  - Y/Q оси выше предела (YQLimit; 0 — типовой предел Надаля 0.8) при
    смещении оси к гребню (> 0.5·DerailDisplacement);
  - колесо почти разгружено вертикальной динамикой
    (AxisLoadFactor < WheelUnloadLimit) и ось доведена до гребня
    (> DerailDisplacement).
- Кратковременные пики не сходят: таймер спадает вдвое быстрее набора.
- Сошло ≥ половины осей — BogieDerailed; все — VehicleDerailed.
- Сход необратим до явного сброса (`Vehicle::resetDerailment()`).
- События в журнал: `[DERAILMENT] Vehicle #N axle i DERAILED`.
- После схода: дополнительное сопротивление (DerailedResistance от веса,
  умолчание 0.15) + аварийное замедление в `integrationPreStep`
  (уточняется в ТЗ «Физика после схода»: шпалы/балласт/опрокидывание).

## Конфигурация ПЕ: секция `[Derailment]`

```xml
<Derailment>
    <Enabled>true</Enabled>
    <YQLimit>0.8</YQLimit>            <!-- 0 = предел Надаля -->
    <WheelUnloadLimit>0.2</WheelUnloadLimit>
    <FlangeClimbTime>0.05</FlangeClimbTime>
    <DerailDisplacement>0.02</DerailDisplacement>
    <DerailedResistance>0.15</DerailedResistance>
</Derailment>
```

## Верификация

`_verify/verify_derailment.py`: нормальная езда — нет схода; краткий пик
Y/Q (40 мс < 50 мс) не сход, таймер спадает; устойчивое Y/Q одной оси →
стадия AXLE; вторая ось тележки → BOGIE; разгруженное колесо на гребне
сходит; все оси → VEHICLE; сход необратим, сброс восстанавливает;
вертикальная разгрузка повышает Y/Q (связь систем).

Колебания ПС (Bounce/Roll/Pitch), подвеска, состояние пути, вибрации,
центробежные силы, влияние массы/погоды — реализованы этапами 2-6
(см. `docs/track-profile.md`, `docs/lateral-dynamics.md`).
