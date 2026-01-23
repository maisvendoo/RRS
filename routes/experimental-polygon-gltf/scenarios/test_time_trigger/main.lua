-- Устанавливаем время
setTime("23:59")

-- Устанавливаем поезд перед входным Ч станции А
train1 = TrainData.new()
train1.name = "VL60pk"
train1.config = "vl60pk-1543"
train1.traj = "route1_0001_1"
train1.coord = 785.0
train1.dir = 1

setTrain(train1)

-- Устанавливаем триггер на 00:00
setAbsTimeTrigger("00:00", actionBuildRoute(train1.traj, "route1_0005_13", train1.dir))