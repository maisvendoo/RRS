--[[ 

Мой первый сценарий
для Russian Railway Simulator

]]

-- Задаем время начала игры
setTime("17:30")

-- Описываем поезд игрока
my_train = TrainData.new()
my_train.name = "ВЛ60пк"
my_train.config = "vl60pk-1543"
my_train.traj = "track_a_p1a"
my_train.coord = 1690.0
my_train.dir = 1 

-- Устанавливаем поезд игрока
setTrain(my_train)

-- Строим маршрут отправления в 17:31 с пути 1А станции А
setTimeTrigger("17:31", actionBuildTrainRoute("track_a_p1a", "track_a-b_nd-22", my_train.dir))

-- Строим маршрут приема на 4 путь станции В по факту занятия второго четного приближения
setOnTrajBusyTrigger("track_a-b_4-2", actionBuildTrainRoute("track_a-b_4-2", "track_b_p4", my_train.dir))
