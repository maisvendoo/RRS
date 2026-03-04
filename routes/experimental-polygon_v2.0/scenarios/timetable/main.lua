setTime("11:58")

train134 = TrainData.new()
train134.name = "134"
train134.config = "vl60pk-1543-T65_17"
train134.traj = "track_a_p2b"
train134.coord = 1040.0
train134.dir = 1
train134.auto = true 

setTrain(train134)

train135 = TrainData.new()
train135.name = "135"
train135.config = "vl60pk-1543-T65_17"
train135.traj = "track_c_p2"
train135.coord = 30.0
train135.dir = -1
train135.auto = true 

--setTrain(train135)

-- 134
--setTimeTrigger("11:59", actionBuildTrainRoute(train134.traj, "track_a-b_nd-22", train134.dir))

--setOnTrajBusyTrigger("track_a-b_4-2", actionBuildTrainRoute("track_a-b_4-2", "track_b_p4", train134.dir))

--setTimeTrigger("12:24", actionBuildTrainRoute("track_b_p4", "track_b-c_nd-22", train134.dir))

--setOnTrajBusyTrigger("track_b-c_4-2", actionBuildTrainRoute("track_b-c_4-2", "track_c_p3", train134.dir))

-- 135
--setTimeTrigger("11:59", actionBuildTrainRoute(train135.traj, "track_b-c_21-chd", train135.dir))

--setOnTrajBusyTrigger("track_b-c_1-3", actionBuildTrainRoute("track_b-c_1-3", "track_b_p3", train135.dir))

--setTimeTrigger("12:24", actionBuildTrainRoute("track_b_p3", "track_a-b_21-chd", train135.dir))

--setOnTrajBusyTrigger("track_a-b_1-3", actionBuildTrainRoute("track_a-b_1-3", "track_a_p1a", train135.dir))