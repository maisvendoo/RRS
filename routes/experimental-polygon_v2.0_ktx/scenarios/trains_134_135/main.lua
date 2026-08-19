setTime("11:59")

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

setTrain(train135)