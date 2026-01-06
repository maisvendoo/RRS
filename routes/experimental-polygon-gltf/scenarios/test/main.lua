-- setTime("06:10")
-- setDate("06.06.2025")

-- Задаем дату и время начала игры
setDateTime("06.06.1982 17:30")

-- Устанавливаем поезд игрока на 4 путь станции Б
train1 = TrainData.new()
train1.name = "VL60pk"
train1.config = "vl60pk-1543"
train1.traj = "branch1_0003_2_x246_x259"
train1.coord = 1085.0
train1.dir = 1

setTrain(train1)

-- Установка стрелок по маршруту
delay(10.0)
switchBwd("00071")
delay(10.0)
switchFwd("00072")
delay(10.0)
switchBwd("00026")

-- Открываем выходной сигнал Ч4 на станции Б
delay(10.0)
openSignal("00098")

-- Закрываем выходной сигнал Ч4 на станции Б
delay(10.0)
closeSignal("00098")