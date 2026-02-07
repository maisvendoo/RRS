-------------------------------------------------------------------
--
--	Фабрика типовых триггеров
--
-------------------------------------------------------------------

function repeatAutoApproach(traj_begin, traj_end, dir)

	function approach_route_build(train_name, traj_name, is_busy)

		if not is_busy and traj_name == traj_end then
			buildRoute(traj_begin, traj_end, dir)
			return TRIG_DELETE
		end

		return TRIG_SAFE
	end
	
	return approach_route_build
end

-------------------------------------------------------------------
-- Автоматический сквозной пропуск поезда по станции
-------------------------------------------------------------------
function autoApproach(traj_begin, traj_end, dir)
	
	function approach_route_build(train_name, traj_name, is_busy)

		-- Если начальная траектория занята
		if is_busy and traj_name == traj_begin then

			-- Проверяем состояние конечной траектории
			local traj_state = getTrajState(traj_end) 

			-- Если она занята, или включена в другой маршрут
			if traj_state.is_busy or traj_state.in_route then

				logMessage("Trajectory " .. traj_end .. " is busy or in other route")
				-- Ставим триггер на освобождение и повторную попытку 
				setTrigger(repeatAutoApproach(getNextTraj(traj_begin, dir), traj_end, dir))				

			else

				logMessage("Try to build route from " .. traj_begin .. " to " .. traj_end)
				-- Строим маршрут от неё до конечной в заданном 
				-- направлении
				buildRoute(getNextTraj(traj_begin, dir), traj_end, dir)

			end

		end
		
		-- Сохраняем тригер в системе
		return TRIG_SAFE
	end

	return approach_route_build
end

-------------------------------------------------------------------
-- Функции-действия, для подстановки в триггеры
-- (Прямой вызов C-функции нельзя присвоить sol::function)
-------------------------------------------------------------------

-- Постровение маршрута
function actionBuildRoute(traj_begin, traj_end, dir)

	function action()
		buildRoute(traj_begin, traj_end, dir)	
	end

	return action
end

-- Постровение поездного маршрута
function actionBuildTrainRoute(traj_begin, traj_end, dir)

	function action()
		buildTrainRoute(traj_begin, traj_end, dir)	
	end

	return action
end

-- Постровение маневрового маршрута
function actionBuildShuntingRoute(traj_begin, traj_end, dir)

	function action()
		buildShuntingRoute(traj_begin, traj_end, dir)	
	end

	return action
end

-- Установить стрелки по маршруту
function actionSetSwitchsAlongRoute(traj_begin, traj_end, dir)

	function action()
		setSwitchsAlongRoute(traj_begin, traj_end, dir)	
	end

	return action
end

-- Открыть маневровый сигнал
function actionOpenShuntingSignal(conn_name, dir)

	function action()
		openShuntingSignal(conn_name, dir)
	end

	return action
end

-- Открыть пригласительны сигнал
function actionOpenCallSignal(conn_name, dir)

	function action()
		openCallSignal(conn_name, dir)
	end

	return action
end

-- Закрыть сигнал (любой)
function actionCloseSignal(conn_name, dir)

	function action()
		closeSignal(conn_name, dir)
	end

	return action
end

--------------------------------------------------------------------------------
--	Функции установки страндартных позиционных триггеров
-- (на занятие и освобождение заданной траектории)
--------------------------------------------------------------------------------

-- Занятие траектории произвольным поездом
function setOnTrajBusyTrigger(name_traj, action)


	function on_busy_trigger_func(train_name, traj_name, is_busy)

		if is_busy and traj_name == name_traj then

			action()			

			return TRIG_DELETE
		end

		return TRIG_SAFE
	end	

	setTrigger(on_busy_trigger_func)
end

-- Освобождение траектории произвольным поездом
function setOnTrajFreeTrigger(name_traj, action)


	function on_free_trigger_func(train_name, traj_name, is_busy)

		if not is_busy and traj_name == name_traj then

			action()			

			return TRIG_DELETE
		end

		return TRIG_SAFE
	end	

	setTrigger(on_free_trigger_func)
end

-- Занятие траектории заданным поездом 
function setOnTrajBusyByTrainTrigger(name_traj, name_train, action)


	function on_busy_trigger_func(train_name, traj_name, is_busy)

		if is_busy and traj_name == name_traj and train_name == name_train then

			action()			

			return TRIG_DELETE
		end

		return TRIG_SAFE
	end	

	setTrigger(on_busy_trigger_func)
end

-- Освобождение траектории заданным поездом 
function setOnTrajFreeByTrainTrigger(name_traj, name_train, action)


	function on_free_trigger_func(train_name, traj_name, is_busy)

		if not is_busy and traj_name == name_traj and train_name == name_train then

			action()			

			return TRIG_DELETE
		end

		return TRIG_SAFE
	end	

	setTrigger(on_free_trigger_func)
end

