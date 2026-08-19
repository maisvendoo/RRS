-- Устанавливаем время
setTime("23:59")

-- Устанавливаем поезд на 3 путь станции А
train1 = TrainData.new()
train1.name = "VL60pk_1"
train1.config = "vl60pk-1543"
train1.traj = "branch1_0002_2_12_22"
train1.coord = 785.0
train1.dir = 1
train1.auto = true

setTrain(train1)

-- Устанавливаем поезд на 2 путь станции А
train2 = TrainData.new()
train2.name = "VL60pk_2"
train2.config = "vl60pk-1543"
train2.traj = "route2_0005_13"
train2.coord = 785.0
train2.dir = 1
train2.auto = true

setTrain(train2)

-- Устанавливаем поезд на 2 путь станции А
train3 = TrainData.new()
train3.name = "VL60pk_3"
train3.config = "vl60pk-1543"
train3.traj = "branch1_0001_2_x12_x22"
train3.coord = 785.0
train3.dir = 1
train3.auto = true

setTrain(train3)

-- Устанавливаем триггер на 00:01 игрового времени
setTimeTrigger("00:01", actionBuildRoute(train3.traj, "route1_0009_26", train3.dir))

-- Устанавливаем триггер на момент +20 секунд от времени старта игры
setTimeTrigger("+00:00:20", actionBuildRoute(train2.traj, "route2_0009_26", train2.dir))

-- Отправление поезда 1
function train1_dep(train_name, traj_name, is_busy)
	
	-- Когда поезд 3 освободит 1 участок удаления
	if not is_busy and train_name == train3.name and traj_name == "route1_0009_26" then

		-- Построить ему маршрут через 1 минуту после этого
		setPostEventTimeTrigger("+00:01", actionBuildRoute(train1.traj, "route1_0009_26", train1.dir))		

		return TRIG_DELETE

	end	

	return TRIG_SAFE

end

setTrigger(train1_dep)
