-- Пассажирский перед 2-м участком приближения к станции Б
train1 = TrainData.new()
train1.name = "65"
train1.config = "vl60pk-1543-T65_17"
train1.traj = "route1_0018_189"
train1.coord = 1790.0
train1.dir = 1

setTrain(train1)

-- Пассажирский перед 2-м участком приближения к станции Б
train2 = TrainData.new()
train2.name = "64"
train2.config = "vl60pk-1543-T65_17"
train2.traj = "route2_0030_297"
train2.coord = 1790.0
train2.dir = -1

setTrain(train2)

local station_B_odd_appr2 = "route1_0019_208"
local station_B_odd_rmv1 = "route1_0028_263"

setTrigger(autoApproach(station_B_odd_appr2, station_B_odd_rmv1, 1))

local station_B_even_appr2 = "route2_0029_279"
local station_B_even_rmv1 = "route2_0020_226"

setTrigger(autoApproach(station_B_even_appr2, station_B_even_rmv1, -1))