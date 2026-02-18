-- Задаем время начала игры
setTime("17:30")

-- Описываем поезд игрока
my_train = TrainData.new()
my_train.name = "ВЛ60пк"
my_train.config = "vl60pk-1543"
my_train.traj = "track_a_p2b"
my_train.coord = 1040.0
my_train.dir = 1 

-- Устанавливаем поезд игрока
setTrain(my_train)

-- Пользовательская action-функция
function my_action()
	
	buildTrainRoute("track_a_p2b", "track_a-b_nd-22", 1)

	buildTrainRoute("track_a-b_n-1", "track_a_p5b", -1)
	
end

-- Ставим временной триггер с пользовательским action
setTimeTrigger("17:30:10", my_action)

-- Построение двух сквозных маршрутов пропуска
function dual_approach(traj_CHP1, traj_CHU1, traj_NP1, traj_NU1)
    
    -- Объявляем функцию БЕЗ параметров
    function action()

    	-- Строим маршрут от траектории traj_CHP1 до traj_CHU1 в направлении 1
		buildTrainRoute(traj_CHP1, traj_CHU1, 1)
		-- Строим маршрут от траектории traj_NP1 до traj_NU1 в направлении -1
		buildTrainRoute(traj_NP1, traj_NU1, -1)

	end	

	-- Возвращаем функцию
	return action

end

setTimeTrigger("17:30:20", dual_approach("track_a-b_2-ch", "track_b-c_nd-22", "track_b-c_n-1", "track_a-b_21-chd"))

setTimeTrigger("17:30:20", dual_approach("track_b-c_2-ch", "track_c_m3-nd", "track_c_m1-n", "track_b-c_21-chd"))