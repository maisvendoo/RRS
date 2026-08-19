-- Устанавливаем поезд
train1 = TrainData.new()
train1.name = "65"
train1.config = "vl60pk-1543-T65_17"
train1.traj = "route1_0019_208"
train1.coord = 1790.0
train1.dir = 1

setTrain(train1)

-- Триггер строящий маршрут пропуска
function approach_build_route(train_name, traj_name, is_busy)
	
	-- Если мы только что заняли участок пути
	if is_busy then
		-- Если это первый участок приближения к станции Б
		if traj_name == "route1_0020_226" then
			-- строим маршрут пропуска по 1-му пути станции Б
			buildRoute("route1_0020_226", "route1_0028_263", train1.dir)
			-- просим грохнуть этот триггер, более он не нужен
			return TRIG_DELETE
		end
	end

	-- Если ситуация не наша - оставляем триггер жить
	return TRIG_SAFE
end

-- Регистрируем триггер в системе
setTrigger(approach_build_route)