#pragma once
#ifndef CAB_ELEMENTS_H
#define CAB_ELEMENTS_H

#include <cstdint>
#include <string>
#include <vector>

//------------------------------------------------------------------------------
// Интерактивные органы кабины ПС (ТЗ "Взаимодействие с элементами кабины").
// Конфиг [CabElement] читается и сервером (реестр-заглушка), и клиентом:
// клиент пикингом мышью находит меш (ModelName) и инжектит штатную
// клавишу управления устройством в поток клавиатуры на сервер
//------------------------------------------------------------------------------

struct CabElement
{
    std::string name;               ///< Имя для подсказки (Alt)
    std::string hint;               ///< Что делает элемент
    std::string model_name;         ///< Имя меша в 3D-модели кабины
    std::string type;               ///< Toggle / Button / Lever / Gauge
    int signal_id = -1;             ///< Анимационный сигнал состояния (-1 - нет)
    std::uint16_t key_on = 0;       ///< Клавиша включения/шага вперёд (ЛКМ)
    std::uint16_t mod_on = 0;       ///< Модификатор включения
    std::uint16_t key_off = 0;      ///< Клавиша выключения/шага назад (ПКМ)
    std::uint16_t mod_off = 0;      ///< Модификатор выключения
    bool interactable = true;       ///< Есть клавиатурный мост (клик работает)

    /// Дискретные состояния для тултипа: имена через ';' (кран 395 -
    /// 7 положений, реверс - назад/нейтраль/вперёд) либо число
    /// позиций (КМЭ - номер позиции)
    std::string state_names = "";
    int state_positions = 0;

    /// Второй сигнал (у реверса: SignalID - ось, SignalID2 - вставлена
    /// ли рукоятка)
    int signal_id2 = -1;

    /// Шкала сигнала для тултипа: "norm" (0..1, по умолчанию),
    /// "index" (сырая позиция 0..N: кран 395, КМЭ),
    /// "centered" (диапазон с нулём в центре: реверс)
    std::string state_mode = "";
};

/// Загрузка секций [CabElement] конфига ПС
std::vector<CabElement> loadCabElements(const std::string& cfg_path);

/// Состояние подсказки кабины для отрисовки (заполняет пикинг,
/// читает MyGui; один поток UI)
struct CabTooltipState
{
    bool active = false;
    float x = 0.0f;
    float y = 0.0f;
    std::string title;
    std::string state_text;
    std::string action_text;
};

CabTooltipState& cabTooltip();

#endif // CAB_ELEMENTS_H
