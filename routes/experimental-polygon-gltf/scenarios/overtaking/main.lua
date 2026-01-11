-- Пассажирский перед 2-м участком приближения к станции Б
train1 = TrainData.new()
train1.name = "65"
train1.config = "vl60pk-1543-T65_17"
train1.traj = "route1_0018_189"
train1.coord = 1790.0
train1.dir = 1

setTrain(train1)

-- Грузовой на 3-м пути станции Б
train2 = TrainData.new()
train2.name = "2001"
train2.config = "vl60k-1737-frEmpties"
train2.traj = "branch1_0004_2_246_259"
train2.coord = 1085.0
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

-- Маршрут отправления поезда 2001 по удалению за 65-м
function train2001_route_build(train_name, traj_name, is_busy)

	-- Если поезд 65 освободил 1-й участок удаления
	if not is_busy and train_name == "65" and traj_name == STB_rmv1 then
		-- Строим маршрут поезду 2001
		buildRoute(train2.traj, STB_rmv1, train2.dir)
		return TRIG_DELETE
	end

	return TRIG_SAFE
end

setTrigger(train2001_route_build)