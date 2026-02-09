setTime("12:00")

-- Устанавливаем поезд с четного направления
train1 = TrainData.new()
train1.name = "34"
train1.config = "vl60pk-1543-T65_17"
train1.traj = "track_a-b_6-4"
train1.coord = 1900.0
train1.dir = 1 
train1.auto = true

setTrain(train1)

-- Устанавливаем поезд с нечетного направления
train2 = TrainData.new()
train2.name = "35"
train2.config = "vl60pk-1543-T65_17"
train2.traj = "track_b-c_3-5"
train2.coord = 50.0
train2.dir = -1 
train2.auto = true

setTrain(train2)

-- Автоматический пропуск поездов в четном направлении
setAutoApproachTrigger("track_a-b_4-2", "track_b-c_nd-22", 1)

-- Автоматический пропуск поездов в нечетном направлении
setAutoApproachTrigger("track_b-c_1-3", "track_a-b_21-chd", -1)