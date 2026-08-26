#include    <TrainsListWidget.h>
#include    <Logger.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainsListWidget::TrainsListWidget(TrainsListWidgetParams *params)
    : _params(params)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainsListWidget::show()
{
    if (!_params || !_params->vehicles_handler || !_params->is_visible)
    {
        LOG_INFO("TrainsListWidget: show() skipped - params=%p, handler=%p, is_visible=%d",
                 _params, _params ? _params->vehicles_handler : nullptr,
                 _params ? _params->is_visible : false);

        return;
    }

    updateCachedTrainsList();
    syncSelectionWithCurrentTrain();
    renderTrainsList();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainsListWidget::updateCachedTrainsList()
{
    auto *handler = _params->vehicles_handler;

    if (!handler)
    {
        return;
    }

    const auto &trains = handler->getTrainsInfo();
    _cached_trains_ids.clear();

    if (trains.empty())
    {
        return;
    }

    for (const auto &train : trains)
    {
        _cached_trains_ids.push_back(train.first_vehicle_id);
    }

    if (_selected_train_id >= 0)
    {
        bool found = false;

        for (int id : _cached_trains_ids)
        {
            if (id == _selected_train_id)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            _selected_train_id = -1;
        }
    }

    // Синхронизируем выделение с текущим поездом
    syncSelectionWithCurrentTrain();

    _initialized = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainsListWidget::syncSelectionWithCurrentTrain()
{
    auto *handler = _params->vehicles_handler;
    if (!handler)
        return;

    const int current_vehicle_id = handler->getCurrentVehicleIndex();

    // Проверяем, что current_vehicle_id есть в кэше
    bool found = false;
    for (int id : _cached_trains_ids)
    {
        if (id == current_vehicle_id)
        {
            found = true;
            break;
        }
    }

    if (found)
    {
        _selected_train_id = current_vehicle_id;
    }
    else
    {
        // Если текущий поезд не найден в списке, сбрасываем выделение
        _selected_train_id = -1;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainsListWidget::renderTrainsList()
{
    auto *handler = _params->vehicles_handler;
    auto *viewer_handler = _params->viewer_handler;

    if (!handler || !viewer_handler)
    {
        return;
    }

    if (_cached_trains_ids.empty())
    {
        return;
    }

    // Получаем информацию о текущем и управляемом вагонах
    const int current_vehicle_id = handler->getCurrentVehicleIndex();
    const int controlled_vehicle_id = handler->getControlledVehicleIndex();

    // Получаем train_id для текущего и управляемого вагонов
    const auto& vehicles = handler->getVehicles();

    int current_train_id = -1;
    int controlled_train_id = -1;

    if (current_vehicle_id >= 0 && static_cast<size_t>(current_vehicle_id) < vehicles.size())
    {
        current_train_id = vehicles[current_vehicle_id].train_id;
    }

    if (controlled_vehicle_id >= 0 && static_cast<size_t>(controlled_vehicle_id) < vehicles.size())
    {
        controlled_train_id = vehicles[controlled_vehicle_id].train_id;
    }

    const auto& trains = handler->getTrainsInfo();

    // Расчёт размеров окна
    const float item_height = ImGui::GetTextLineHeightWithSpacing();
    const float header_height = ImGui::GetFrameHeightWithSpacing() * 1.5f;
    const float button_height = ImGui::GetFrameHeightWithSpacing() * 1.5f;
    const float padding = 10.0f;

    // Минимальная высота - чтобы виджет был крупнее
    const float min_height = ImGui::GetIO().DisplaySize.y * 0.4f;  // 40% от экрана минимум

    // Максимальная высота - вплоть до низа экрана ниже виджета профиля пути
    const float top_y = 300.0f;
    const float max_height = std::max(0.0f, ImGui::GetIO().DisplaySize.y - top_y - 20.0f);

    // Высота списка: заголовок + элементы + кнопка + отступы
    const float list_height = _cached_trains_ids.size() * item_height;
    float total_height = header_height + list_height + button_height + padding * 3;

    // Применяем минимальную и максимальную высоту
    total_height = std::clamp(total_height, std::min(min_height, max_height), max_height);

    // Ширина окна
    const float window_width = 350.0f;

    // Позиционирование: справа, под виджетом профиля пути (встык к нему снизу)
    const ImVec2 display_size = ImGui::GetIO().DisplaySize;
    const ImVec2 window_pos(
        display_size.x - window_width - 20.0f,  // отступ справа 20px
        top_y
        );

    ImGui::SetNextWindowSize(ImVec2(window_width, total_height));
    ImGui::SetNextWindowPos(window_pos);

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoNavInputs;
    window_flags |= ImGuiWindowFlags_NoNavFocus;

    bool open = true;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, _params->hud_background);
    ImGui::Begin("##TrainsList", &open, window_flags);
    ImGui::PopStyleColor();

    // Заголовок с количеством поездов
    ImGui::Text("Поезда (%zu):", _cached_trains_ids.size());
    ImGui::Separator();

    // Список поездов с прокруткой
    const float scroll_height = std::max(
        0.0f,
        total_height - header_height - button_height - padding * 3
        );

    ImGui::BeginChild("##TrainsListScroll", ImVec2(0, scroll_height), true);

    // Делаем подсветку при наведении полупрозрачной
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.3f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2f, 0.2f, 0.2f, 0.2f));

    for (size_t i = 0; i < _cached_trains_ids.size(); ++i)
    {
        const int first_vehicle_id = _cached_trains_ids[i];

        // Находим имя поезда и его train_id (индекс в векторе trains)
        QString train_name = "Поезд";
        int train_id = -1;
        for (const auto& train : trains)
        {
            ++train_id;
            if (train.first_vehicle_id == first_vehicle_id)
            {
                train_name = train.train_name;
                break;
            }
        }

        // Формируем строку для отображения
        std::string display_text;
        if (train_name.isEmpty() || train_name == "Поезд")
        {
            display_text = QString("#%1 Поезд #%2").arg(train_id).arg(first_vehicle_id).toStdString();
        }
        else
        {
            display_text = QString("#%1 %2 (ID: %3)").arg(train_id).arg(train_name).arg(first_vehicle_id).toStdString();
        }

        // Определяем цвет
        ImVec4 text_color = _params->hud_text; // цвет текста по умолчанию из конфигурации

        // Сравниваем train_id поезда из списка с train_id текущего и управляемого вагонов
        if (train_id == current_train_id)
        {
            text_color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // ЖЁЛТЫЙ - текущий поезд
        }
        else if (train_id == controlled_train_id)
        {
            text_color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // ЗЕЛЁНЫЙ - управляемый поезд
        }

        ImGui::PushStyleColor(ImGuiCol_Text, text_color);
        ImGui::PushID(i);

        // Selectable с подсветкой при наведении
        const bool is_selected = (first_vehicle_id == _selected_train_id);
        if (ImGui::Selectable(display_text.c_str(), is_selected))
        {
            _selected_train_id = first_vehicle_id;
            selectTrain(first_vehicle_id);
        }

        // Подсказка при наведении
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Кликните для переключения на поезд %s",
                              train_name.isEmpty() ? QString("№%1").arg(first_vehicle_id).toStdString().c_str() : train_name.toStdString().c_str());
        }

        ImGui::PopID();
        ImGui::PopStyleColor();
    }

    ImGui::PopStyleColor(2);  // Убираем стили подсветки

    ImGui::EndChild();

    // Кнопка "Вернуться к управляемому поезду"
    const bool has_controlled = (controlled_train_id >= 0 && controlled_train_id != current_train_id);

    ImGui::Separator();

    if (has_controlled)
    {
        if (ImGui::Button("Вернуться к управляемому поезду", ImVec2(window_width - padding * 2, 0)))
        {
            if (controlled_vehicle_id >= 0)
            {
                selectTrain(controlled_vehicle_id);
            }
        }
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        ImGui::TextDisabled("Управляемый поезд: текущий");
        ImGui::PopStyleColor();
    }

    ImGui::End();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainsListWidget::selectTrain(int first_vehicle_id)
{
    auto *handler = _params->vehicles_handler;
    auto *viewer_handler = _params->viewer_handler;

    if (!handler || !viewer_handler)
    {
        LOG_INFO("ERROR!!!");
        return;
    }

    const int current_vehicle_id = handler->getCurrentVehicleIndex();

    if (current_vehicle_id != first_vehicle_id)
    {
        handler->setCurrentVehicle(first_vehicle_id);
        viewer_handler->changeCurrentVehicle();
        LOG_INFO("Swithced to train with first vehicle %d", first_vehicle_id);
    }
}
