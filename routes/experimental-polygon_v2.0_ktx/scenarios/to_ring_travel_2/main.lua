train2 = TrainData.new()
train2.name = "2"
train2.config = "vl60pk-1543-T65_17"
train2.traj = "track_a_p2b"
train2.coord = 1040
train2.dir = 1
train2.auto = true
setTrain(train2)

-- строим маршрут отправления поезду на испытательное кольцо
setTimeTrigger("+00:01", actionBuildTrainRoute(train2.traj, "track_akol_nk-6x1", train2.dir))

-- Функция построения маршрута на ближнюю кривую при её освобождении
function build_route_func(train_name, traj_name, is_busy)

	if traj_name == "track_akol_nk-6x1" and not is_busy then

		buildTrainRoute("track_akol_2x5-chk", "track_akol_nk-6x1", train2.dir)

	end

	return TRIG_SAFE
end

setTrigger(build_route_func)