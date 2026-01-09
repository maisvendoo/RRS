-- setTime("06:10")
-- setDate("06.06.2025")

-- Задаем дату и время начала игры
setDateTime("06.06.1982 18:30")

-- Устанавливаем грузовой поезд "туда", перед станцией А
train_freight = TrainData.new()
train_freight.name = "freight"
train_freight.config = "vl60k-1737-frEmpties"
train_freight.traj = "route1_0001_1"
train_freight.coord = 785.0
train_freight.dir = 1

setTrain(train_freight)

-- Устанавливаем пассажирский поезд "обратно", перед станцией В
train_pass = TrainData.new()
train_pass.name = "freight"
train_pass.config = "vl60pk-1543-T65_17"
train_pass.traj = "route2_0047_497"
train_pass.coord = 15.0
train_pass.dir = -1

setTrain(train_pass)

-- Устанавливаем поезд на 1 путь станции Б
train1 = TrainData.new()
train1.name = "VL60pk"
train1.config = "vl60pk-1543"
train1.traj = "route1_0024_247"
train1.coord = 1085.0
train1.dir = 1

setTrain(train1)

-- Устанавливаем поезд на 3 путь станции Б
--[[train2 = TrainData.new()
train2.name = "VL60pk_3"
train2.config = "vl60pk-1543"
train2.traj = "branch1_0004_2_246_259"
train2.coord = 1085.0
train2.dir = 1

setTrain(train2)--]]

-- Устанавливаем поезд на 2 путь станции Б
train3 = TrainData.new()
train3.name = "VL60k"
train3.config = "VL60k-1737"
train3.traj = "route2_0024_247"
train3.coord = 185.0
train3.dir = -1

setTrain(train3)

-- Устанавливаем поезд на 4 путь станции Б
--[[train4 = TrainData.new()
train4.name = "VL60pk_4"
train4.config = "vl60pk-1543"
train4.traj = "branch1_0003_2_x246_x259"
train4.coord = 185.0
train4.dir = -1

setTrain(train4)--]]

-- Маршрут по первому пути - туда
delay(5.0)
buildRoute("route1_0001_1", "route1_0043_482", 1)
-- Маршрут по второму пути - обратно
delay(3.0)
buildRoute("route2_0047_497", "route2_0005_13", -1)