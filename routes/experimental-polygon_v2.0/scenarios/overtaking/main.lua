-- Устанавливаем поезд 1
train1 = TrainData.new()
train1.name = "1"
train1.config = "vl60pk-1543-T65_17"
train1.traj = "track_a-b_6-4"
train1.coord = 1900.0
train1.dir = 1 
train1.auto = true

setTrain(train1)

-- Устанавливаем поезд 3
train3 = TrainData.new()
train3.name = "3"
train3.config = "vl60pk-1543-T65_17"
train3.traj = "track_a-b_12-10"
train3.coord = 1900.0
train3.dir = 1 
train3.auto = true

setTrain(train3)

-- Второй участок приближения к станции В
local stB_CHP2 = "track_a-b_4-2"
-- Первый участок приближения к станции В
local stB_CHP1 = "track_a-b_2-ch"
-- Первый участок удаления от станции В
local stB_CHU1 = "track_b-c_nd-22"
-- Второй участок удаления от станции В
local stB_CHU2 = "track_b-c_22-20"
-- 4 путь станции В
local stB_p4 = "track_b_p4"
-- Стрелочный участок от стрелки 12 до 4 пути станции В
local sw12_H4 = "track_b_12-p4"

-- Строим маршрут приема поезду 1 на 4 путь станции В
setOnTrajBusyTrigger(stB_CHP2, actionBuildTrainRoute(stB_CHP1, stB_p4, train1.dir))


-- Действия, выполняемые при освобождении поездом 1 стрелочного участка 12 - путь 4
function set_appr_train3()

	-- Строим маршрут пропуска поезду 3
	setOnTrajBusyTrigger(stB_CHP2, actionBuildTrainRoute(stB_CHP1, stB_CHU1, train3.dir))

end

setOnTrajFreeTrigger(sw12_H4, set_appr_train3)


setOnTrajFreeTrigger(stB_CHU1, actionBuildTrainRoute(stB_p4, stB_CHU1, train1.dir))