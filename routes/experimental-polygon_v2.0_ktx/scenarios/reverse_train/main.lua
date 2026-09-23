--[[

Пример разворота поезда с использованием
временных триггеров

]]

-- Задаем время начала игры
setTime("17:30")

-- Описываем поезд игрока (ВЛ60пк-1543 + 17 пассажирских вагонов)
my_train = TrainData.new()
my_train.name = "ВЛ60пк"
my_train.config = "vl60pk-1543-T65_17"
my_train.traj = "track_a_p1a"
my_train.coord = 1690.0
my_train.dir = 1

-- Устанавливаем поезд игрока
setTrain(my_train)

-- Через 10 секунд после старта разворачиваем поезд
setTimeTrigger("+00:00:10", actionReverseTrain(my_train.name))

-- Ещё через 10 секунд снова разворачиваем — поезд возвращается к исходному направлению
setTimeTrigger("+00:00:20", actionReverseTrain(my_train.name))