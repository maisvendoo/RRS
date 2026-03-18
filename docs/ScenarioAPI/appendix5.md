## Приложение 5. Формат графика движения поезда

* Графики движения располагаются в каталоге timetable папки со сценарием. Этот каталог должен располагаться рядом с файлом main.lua
* График движения - это файл формата XML, имя которого должно совпадать с именем поезда, заданным в сценарии или присвоенным в процессе игры. Например, для поезда с именем "134" имя файла графика должно быть 134.xml

Пример графика ([сценарий trains_134_135](https://github.com/maisvendoo/RRS/tree/v1.9.0-devel/routes/experimental-polygon_v2.0/scenarios/trains_134_135) из каталога примеров)

```
<?xml version="1.0" encoding="UTF-8"?>
<Config>

	<Station>
		<Name>Станция A</Name>			
		<DepartureTime>12:00</DepartureTime>
		<TargetTraj>track_a_p2b</TargetTraj>
		<TargetCoord>1040.0</TargetCoord>
		<RemovalTraj>track_a-b_nd-22</RemovalTraj>
	</Station>

	<Station>
		<Name>Станция B</Name>
		<ArrivalTime>12:20</ArrivalTime>			
		<DepartureTime>12:25</DepartureTime>
		<TargetTraj>track_b_p4</TargetTraj>
		<TargetCoord>1250.0</TargetCoord>
		<ApproachTraj>track_a-b_4-2</ApproachTraj>
		<RemovalTraj>track_b-c_nd-22</RemovalTraj>
	</Station>

	<Station>
		<Name>Станция C</Name>
		<ArrivalTime>12:45</ArrivalTime>			
		<TargetTraj>track_c_p3</TargetTraj>
		<TargetCoord>1250.0</TargetCoord>
		<ApproachTraj>track_b-c_4-2</ApproachTraj>			
	</Station>	
	
</Config>
```

Параметры описания станции (путевой точки) в графике движения

|Параметр|Тип значения|Назначение|
|-|-|-|
|Name|Строка|Название станции|
|ArrivalTime|Строка hh:mm|Время прибытия|
|DepartureTime|Строка hh:mm|Время отправления|
|TargetTraj|Строка|Имя целевой траектории|
|TargetCoord|Число|Координата вдоль целевой траектории, м|
|ApproachTraj|Строка|Имя траектории, при достижении которой и от которой строится маршрут прибытия поезда (участок приближения)|
|RemovalTraj|Строка|Имя траектории, в направлении которой строится маршрут отправления (участок удаления)|
|IsRightPlatform|Логическое (1 или 0)|Наличие или отсутствие платформы справа от целевой траектории. Может не указываться, по-умолчанию платформа отсутствует|
|IsLeftPlatform|Логическое (1 или 0)|Наличие или отсутствие платформы слева от целевой траектории. Может не указываться, по-умолчанию платформа отсутствует|
|IsVisible|Логическое (1 или 0)|Отображение станции в графике движения. По-умолчанию, если данный параметры не указан, станция отображается|
