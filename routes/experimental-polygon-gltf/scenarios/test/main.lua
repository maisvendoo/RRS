-- setTime("06:10")
-- setDate("06.06.2025")

-- Задаем дату и время начала игры
setDateTime("06.06.1982 17:30")

-- Устанавливаем поезд игрока на 1 путь станции Б
train1 = TrainData.new()
train1.name = "VL60pk"
train1.config = "vl60pk-1543"
train1.traj = "route1_0024_247"
train1.coord = 1085.0
train1.dir = 1

setTrain(train1)

-- Устанавливаем поезд игрока на 3 путь станции Б
--[[train2 = TrainData.new()
train2.name = "VL60pk"
train2.config = "vl60pk-1543"
train2.traj = "branch1_0004_2_246_259"
train2.coord = 1085.0
train2.dir = 1

setTrain(train2)--]]

-- Устанавливаем поезд игрока на 2 путь станции Б
train3 = TrainData.new()
train3.name = "VL60pk"
train3.config = "vl60pk-1543"
train3.traj = "route2_0024_247"
train3.coord = 1085.0
train3.dir = 1

setTrain(train3)

-- Устанавливаем поезд игрока на 4 путь станции Б
--[[train4 = TrainData.new()
train4.name = "VL60pk"
train4.config = "vl60pk-1543"
train4.traj = "branch1_0003_2_x246_x259"
train4.coord = 1085.0
train4.dir = 1

setTrain(train4)--]]

buildRoute("route1_0001_1", "route2_0047_497", 1)
delay(1.0)
buildRoute("route1_0047_497", "route2_0001_1", -1)