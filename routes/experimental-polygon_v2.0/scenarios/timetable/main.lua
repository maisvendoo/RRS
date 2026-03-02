setTime("11:58")

train134 = TrainData.new()
train134.name = "134"
train134.config = "vl60pk-1543-T65_17"
train134.traj = "track_a_p2b"
train134.coord = 1040.0
train134.dir = 1
train134.auto = true 

setTrain(train134)

setTimeTrigger("11:59", actionBuildTrainRoute(train134.traj, "track_a-b_nd-22", train143.dir))

setOnTrajBusyTrigger("track_a-b_4-2", actionBuildTrainRoute("track_a-b_4-2", "track_b_p4", train134.dir))