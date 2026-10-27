//------------------------------------------------------------------------------
//
//      Track irregularities profile for RRS
//      Track condition level: scales irregularity magnitudes
//
//------------------------------------------------------------------------------

#ifndef     TRACK_CONDITION_H
#define     TRACK_CONDITION_H

#include    "track-profile-export.h"

namespace track
{

//------------------------------------------------------------------------------
/// Состояние участка пути. Влияет на амплитуду и количество неровностей:
/// Excellent - практически идеальный путь, Critical - опасные неровности
//------------------------------------------------------------------------------
enum class Condition
{
    Excellent = 0,
    Good = 1,
    Normal = 2,
    Worn = 3,
    Poor = 4,
    Critical = 5
};

/// Множитель амплитуд неровностей для уровня состояния
TRACKPROFILE_EXPORT double conditionScale(Condition condition);

/// Число уровней состояния
constexpr int condition_count = 6;

/// Разбор уровня по имени ("excellent"..."critical").
/// При ошибке возвращает Normal и ok = false
TRACKPROFILE_EXPORT Condition conditionFromString(const char* name,
                                                  bool* ok = nullptr);

/// Имя уровня ("excellent"..."critical")
TRACKPROFILE_EXPORT const char* conditionToString(Condition condition);

} // namespace track

#endif // TRACK_CONDITION_H
