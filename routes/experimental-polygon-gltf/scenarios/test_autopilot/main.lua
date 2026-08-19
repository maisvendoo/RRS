-- Пассажирский 65 на 3-м пути станции А
train1 = TrainData.new()
train1.name = "65"
train1.config = "vl60pk-1543-T65_17"
train1.traj = "branch1_0002_2_12_22"
train1.coord = 785.0
train1.dir = 1

setTrain(train1)

-- Пассажирский 64 на 4-м пути станции В
train2 = TrainData.new()
train2.name = "64"
train2.config = "vl60pk-1543-T65_17"
train2.traj = "branch1_0005_2_x481_x493"
train2.coord = 15.0
train2.dir = -1

setTrain(train2)

-- Пассажирский 62 на 3-м пути станции В
train3 = TrainData.new()
train3.name = "62"
train3.config = "vl60pk-1543-T65_17"
train3.traj = "branch1_0006_2_481_493"
train3.coord = 15.0
train3.dir = -1

setTrain(train3)

-- Пассажирский  67 на 1-м пути станции A
train4 = TrainData.new()
train4.name = "67"
train4.config = "vl60pk-1543-T65_17"
train4.traj = "route1_0005_13"
train4.coord = 785.0
train4.dir = 1

setTrain(train4)

-- Автоматический пропуск всего и вся по станции Б
local station_B_odd_appr2 = "route1_0019_208"
local station_B_odd_rmv1 = "route1_0028_263"

setTrigger(autoApproach(station_B_odd_appr2, station_B_odd_rmv1, 1))

local station_B_even_appr2 = "route2_0029_279"
local station_B_even_rmv1 = "route2_0020_226"

setTrigger(autoApproach(station_B_even_appr2, station_B_even_rmv1, -1))

-- Триггер строящий маршрут отправления поезду 64 по отправлению поезда 65
function build_route64(train_name, traj_name, is_busy)
	
	if not is_busy then
		-- Поезд 65 уехал с занимаегого им пути
		if traj_name == train1.traj and train_name == "65" then
			-- строим маршрут отправления поезду 64
			buildRoute(train2.traj, "route2_0039_461", train2.dir)
			-- просим грохнуть этот триггер, более он не нужен
			return TRIG_DELETE
		end
	end

	-- Если ситуация не наша - оставляем триггер жить
	return TRIG_SAFE
end

-- Регистрируем триггер в системе
setTrigger(build_route64)

-- Триггер строящий маршрут отправления поезду 62 по удалению поезда 64
function build_route62(train_name, traj_name, is_busy)
	
	if not is_busy then
		-- Поезд 64 покинул ЧУ2
		if traj_name == "route2_0038_443" and train_name == "64" then
			-- строим маршрут отправления поезду 62
			buildRoute(train3.traj, "route2_0039_461", train3.dir)
			-- просим грохнуть этот триггер, более он не нужен
			return TRIG_DELETE
		end
	end

	-- Если ситуация не наша - оставляем триггер жить
	return TRIG_SAFE
end

-- Регистрируем триггер в системе
setTrigger(build_route62)

-- Триггер строящий маршрут отправления поезду 67 по удалению поезда 65
function build_route67(train_name, traj_name, is_busy)
	
	if not is_busy then
		-- Поезд 65 покинул У1
		if traj_name == "route1_0009_26" and train_name == "65" then
			-- строим маршрут отправления поезду 67
			buildRoute(train4.traj, "route1_0009_26", train4.dir)
			-- просим грохнуть этот триггер, более он не нужен
			return TRIG_DELETE
		end
	end

	-- Если ситуация не наша - оставляем триггер жить
	return TRIG_SAFE
end

-- Регистрируем триггер в системе
setTrigger(build_route67)

-- Триггер строящий маршрут прибытия поезду 64 на станцию А
function arrival_route64(train_name, traj_name, is_busy)
	
	if is_busy then
		-- Поезд 64 занял П2
		if traj_name == "route2_0010_42" and train_name == "64" then
			-- строим маршрут прибытия поезду 64
			buildRoute("route2_0009_26", "branch1_0001_2_x12_x22", train2.dir)
			-- просим грохнуть этот триггер, более он не нужен
			return TRIG_DELETE
		end
	end

	-- Если ситуация не наша - оставляем триггер жить
	return TRIG_SAFE
end

-- Регистрируем триггер в системе
setTrigger(arrival_route64)

-- Триггер строящий маршрут прибытия поезду 62 на станцию А
function arrival_route62(train_name, traj_name, is_busy)
	
	if is_busy then
		-- Поезд 62 занял П2
		if traj_name == "route2_0010_42" and train_name == "62" then
			-- строим маршрут прибытия поезду 64
			buildRoute("route2_0009_26", "branch1_0001_2_12_22", train3.dir)
			-- просим грохнуть этот триггер, более он не нужен
			return TRIG_DELETE
		end
	end

	-- Если ситуация не наша - оставляем триггер жить
	return TRIG_SAFE
end

-- Регистрируем триггер в системе
setTrigger(arrival_route62)

-- Триггер строящий маршрут прибытия поезду 65 на станцию В
function arrival_route65(train_name, traj_name, is_busy)
	
	if is_busy then
		-- Поезд 65 занял П2
		if traj_name == "route1_0038_443" and train_name == "65" then
			-- строим маршрут прибытия поезду 64
			buildRoute("route1_0039_461", "branch1_0006_2_481_493", train1.dir)
			-- просим грохнуть этот триггер, более он не нужен
			return TRIG_DELETE
		end
	end

	-- Если ситуация не наша - оставляем триггер жить
	return TRIG_SAFE
end

-- Регистрируем триггер в системе
setTrigger(arrival_route65)

-- Триггер строящий маршрут прибытия поезду 67 на станцию В
function arrival_route67(train_name, traj_name, is_busy)
	
	if is_busy then
		-- Поезд 67 занял П2
		if traj_name == "route1_0038_443" and train_name == "67" then
			-- строим маршрут прибытия поезду 64
			buildRoute("route1_0039_461", "branch1_0005_2_x481_x493", train4.dir)
			-- просим грохнуть этот триггер, более он не нужен
			return TRIG_DELETE
		end
	end

	-- Если ситуация не наша - оставляем триггер жить
	return TRIG_SAFE
end

-- Регистрируем триггер в системе
setTrigger(arrival_route67)