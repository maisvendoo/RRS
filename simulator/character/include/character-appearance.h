//------------------------------------------------------------------------------
//
//      Character appearance (внешний вид персонажа)
//      ТЗ "VneshnyVid" (редактор персонажа, как в Garry's Mod)
//
//      Модульная система: слоты экипировки (голова, волосы, торс, ноги,
//      обувь, головной убор, аксессуары), предметы со ссылками на модели
//      и материалами (цветовые варианты), пол персонажа, железнодорожная
//      форма, сохранение/загрузка набора, случайный внешний вид.
//      Редактор UI и превью - во вьювере; ядро - данные и совместимость.
//
//------------------------------------------------------------------------------

#ifndef     CHARACTER_APPEARANCE_H
#define     CHARACTER_APPEARANCE_H

#include    <QString>
#include    <QStringList>

#include    <cstddef>
#include <vector>

//------------------------------------------------------------------------------
/// Внешний вид персонажа
//------------------------------------------------------------------------------
class CharacterAppearance
{
public:

    /// Пол
    enum class Gender
    {
        Male = 0,
        Female = 1
    };

    /// Предмет экипировки
    struct Item
    {
        QString id = "";            ///< Имя предмета
        QString model = "";         ///< Модель
        QString material = "";      ///< Материал (цветовой вариант)
        QStringList hides;          ///< Слоты, которые предмет скрывает
    };

    CharacterAppearance() = default;

    /// Загрузка секции [Character] конфига: пол, форма, стартовый набор
    void loadConfig(QString cfg_path);

    /// Задать пол (сбрасывает несовместимые предметы)
    void setGender(Gender gender);

    /// Надеть предмет в слот (п.6, 10: совместимость)
    bool equip(const QString& slot, const Item& item);

    /// Снять слот
    void unequip(const QString& slot);

    /// Предмет в слоте (пусто - нет)
    const Item& getEquipped(const QString& slot) const;

    /// Одета железнодорожная форма (п.7)
    bool isInRailwayUniform() const;

    /// Случайный внешний вид (п.15): пол и случайные предметы из набора
    void randomize();

    /// Сохранение/загрузка набора (строка "slot=id:material;...")
    QString serialize() const;
    void deserialize(const QString& data);

    Gender getGender() const;

    /// Доступные слоты
    static const QStringList& slots();

private:

    Gender gender_ = Gender::Male;

    /// Экипировка: слот -> предмет
    std::vector<std::pair<QString, Item>> equipment_;

    /// Базовые цвета формы (п.8)
    QStringList uniform_colors_ = {"dark_blue", "grey", "black"};

    const Item empty_item_;
};

#endif // CHARACTER_APPEARANCE_H
