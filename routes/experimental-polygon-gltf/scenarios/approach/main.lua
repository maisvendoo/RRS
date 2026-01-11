train1 = TrainData.new()
train1.name = "65"
train1.config = "vl60pk-1543-T65_17"
train1.traj = "route1_0019_208"
train1.coord = 1790.0
train1.dir = 1

setTrain(train1)


function approach_build_route(train_name, traj_name, is_busy)
	
	if is_busy then
		if traj_name == "route1_0020_226" then
			buildRoute("route1_0020_226", "route1_0028_263", train1.dir)
			return TRIG_DELETE
		end
	end

	return TRIG_SAFE
end

setTrigger(approach_build_route)