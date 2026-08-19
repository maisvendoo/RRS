-- Задаем время начала игры
setTime("17:30")

-- Описываем поезд игрока
train34 = TrainData.new()
train34.name = "34"
train34.config = "vl60pk-1543-T65_17"
train34.traj = "track_a_p2b"
train34.coord = 1040.0
train34.dir = 1 

setTrain(train34)

-- Строим маршрут отправления в 17:31 с пути 1А станции А
setTimeTrigger("17:31", actionBuildTrainRoute(train34.traj, "track_a-b_nd-22", train34.dir))

-- Строим маршрут пропуска поезда 34 по 2 главному пути станции В
setOnTrajBusyByTrainTrigger("track_a-b_4-2", "34", actionBuildTrainRoute("track_a-b_4-2", "track_b-c_nd-22", train34.dir))