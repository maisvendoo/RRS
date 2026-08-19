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

-- Автоматически строим маршрут пропуска поезда по кольцу
setAutoApproachTrigger("track_akol_4x3-2x5", "track_akol_nk-6x1", train2.dir)