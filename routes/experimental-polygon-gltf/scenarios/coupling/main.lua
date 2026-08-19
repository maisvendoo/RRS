--setTime("12:00")

train1 = TrainData.new()
train1.name = "vl60pk"
train1.config = "vl60pk-1543"
train1.traj = "route1_0005_13"
train1.coord = 785.0
train1.dir = 1

setTrain(train1)

train2 = TrainData.new()
train2.name = "0000"
train2.config = "train-T65_17"
train2.traj = "route1_0005_13"
train2.coord = 750.0
train2.dir = 1

setTrain(train2)

-- 2-й участок приближения к станции Б
STB_appr2 = "route1_0019_208"
-- 1-й участок удаления от станции Б
STB_rmv1 = "route1_0028_263"

-- Маршрут пропуска пассажирского поезда 65
function train65_route_build(train_name, traj_name, is_busy)

	-- Если поезд 65 занял 2-й участок приближения
	if is_busy and train_name == "65" and traj_name == STB_appr2 then
		-- Строим ему маршрут
		buildRoute("route1_0020_226", STB_rmv1, train1.dir)
		-- Просим удалить этот триггер
		return TRIG_DELETE
	end
	
	-- Сохраняем тригер, пока он не отработал
	return TRIG_SAFE
end

-- Устанавливаем триггер
setTrigger(train65_route_build)