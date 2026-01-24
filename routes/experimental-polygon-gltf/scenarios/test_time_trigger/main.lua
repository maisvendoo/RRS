-- Устанавливаем время
setTime("23:59")

-- Устанавливаем поезд на 3 путь станции А
train1 = TrainData.new()
train1.name = "VL60pk_1"
train1.config = "vl60pk-1543"
train1.traj = "branch1_0002_2_12_22"
train1.coord = 785.0
train1.dir = 1

setTrain(train1)

-- Устанавливаем поезд на 2 путь станции А
train2 = TrainData.new()
train2.name = "VL60pk_2"
train2.config = "vl60pk-1543"
train2.traj = "route2_0005_13"
train2.coord = 785.0
train2.dir = 1

setTrain(train2)

-- Устанавливаем триггер на 00:00 игрового времени
setAbsTimeTrigger("00:00", actionBuildRoute(train1.traj, "route1_0009_26", train1.dir))

-- Устанавливаем триггер на +00:02 от времени старта игры
setAbsTimeTrigger("+00:02", actionBuildRoute(train2.traj, "route2_0009_26", train2.dir))