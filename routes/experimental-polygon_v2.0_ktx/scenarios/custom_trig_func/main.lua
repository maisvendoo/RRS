-- Устанавливаем поезд 102
train102 = TrainData.new()
train102.name = "102"
train102.config = "vl60pk-1543-T65_17"
train102.traj = "track_b_p4"
train102.coord = 1250.0
train102.dir = 1 
train102.auto = true

setTrain(train102)

-- Устанавливаем поезд 104
train104 = TrainData.new()
train104.name = "104"
train104.config = "vl60pk-1543-T65_17"
train104.traj = "track_a-b_6-4"
train104.coord = 1900.0
train104.dir = 1 
train104.auto = true

setTrain(train104)

-- Второй участок приближения к станции В
local stB_CHP2 = "track_a-b_4-2"
-- Первый участок приближения к станции В
local stB_CHP1 = "track_a-b_2-ch"
-- Первый участок удаления от станции В
local stB_CHU1 = "track_b-c_nd-22"

setTimeTrigger("+00:00:36", actionBuildTrainRoute(train102.traj, stB_CHU1, train102.dir))

-- Специальная функция для позиционного триггера
function train104_appr(train_name, traj_name, is_busy)

	-- Если занята траектория stB_CHP2
	if is_busy and traj_name == stB_CHP2 then

		-- Проверяем состояние участка удаления
		local traj_state = getTrajState(stB_CHU1)

		-- Если он занят или включен в маршрут
		if traj_state.is_busy or traj_state.in_route then

			-- Ставим триггер на его оcвобожение с целью таки построить маршрут
			setOnTrajFreeTrigger(stB_CHU1, actionBuildTrainRoute(stB_CHP1, stB_CHU1, train104.dir))

		else

			-- Если всё ок, строим маршрут
			buildTrainRoute(stB_CHP1, stB_CHU1, train104.dir)

		end

		-- Влюбом случае тригер удаляем, он отработал
		return TRIG_DELETE

	end

	-- Оставляем триггер если он не сработал
	return TRIG_SAFE

end

setTrigger(train104_appr)