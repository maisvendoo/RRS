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
--
-------------------------------------------------------------------
function actionBuildRoute(traj_begin, traj_end, dir)

	function action()
		buildRoute(traj_begin, traj_end, dir)	
	end

	return action
end