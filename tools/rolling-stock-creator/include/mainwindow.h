#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "model-viewer.h"
#include "scene-model.h"
#include "train-project.h"
#include "vehicle-schema.h"

#include <QDomDocument>
#include <QMainWindow>
#include <QMap>
#include <QStringList>

#include <vector>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QMenu;
class QPlainTextEdit;
class QPushButton;
class QDoubleSpinBox;
class QSpinBox;
class QTabWidget;
class QTableWidget;
class QTableWidgetItem;
class QTreeWidget;
class QTreeWidgetItem;
class QWidget;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
/// Редактор характеристик и 3D-инструмент подвижного состава
/// (ТЗ «ПРОГА-ЭКСПОРТЕР и редактор характеристик ПС»)
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    // --- Вкладка «Характеристики» ---
    /// Открыть XML-конфиг ПС
    void slotOpenConfig();

    /// Сохранить конфиг в текущий файл
    void slotSaveConfig();

    /// Сохранить конфиг в новый файл
    void slotSaveAsConfig();

    /// Открыть текущий файл в системном редакторе
    void slotOpenInSystemEditor();

    /// Выбрана секция в дереве — построить форму
    void slotSectionSelected(QTreeWidgetItem* item, int column);

    /// Добавить экземпляр повторяющейся секции (CabElement)
    void slotAddCabElement();

    /// Удалить текущий экземпляр повторяющейся секции
    void slotRemoveCurrentSectionInstance();

    // --- Проект .trainproject (ТЗ п.30-31) ---
    /// Новый проект из шаблона
    void slotNewProject();

    /// Открыть проект
    void slotOpenProject();

    /// Сохранить проект (в текущий файл)
    void slotSaveProject();

    /// Сохранить проект в новый файл
    void slotSaveProjectAs();

    /// Открыт недавний проект
    void slotOpenRecentProject();

    // --- Вкладка «Модель» (ТЗ п.1-4, 19-20) ---
    /// Загрузить glTF-модель ПС
    void slotLoadModel();

    /// Автоматическое распознавание ролей по именам
    void slotAutoAssignRoles();

    /// Запуск/остановка 3D-вьюпорта
    void slotToggle3D();

    /// Переключение варианта LOD
    void slotLodChanged(int index);

    /// Переключение сетки
    void slotGridToggled(bool checked);

    /// Переключение превью коллизий
    void slotCollisionToggled(bool checked);

    /// Переключение проекции (перспектива/ортография)
    void slotProjectionChanged(int index);

    /// Фокус камеры на выбранном объекте
    void slotFocusSelected();

    /// Переименовать display-имя выбранного объекта
    void slotRenameSelected();

    /// Изменён флажок видимости меша в дереве
    void slotObjectItemChanged(QTreeWidgetItem* item, int column);

    // --- Вкладка «Физические точки» (ТЗ п.14) ---
    void slotAddPoint();
    void slotRemovePoint();
    void slotPointChanged(int row, int column);

    // --- Вкладка «Экспорт» (ТЗ п.28-29) ---
    /// Выбор папки назначения пакета
    void slotBrowseExportDir();

    /// Собрать пакет: «Перенести в игру»
    void slotExport();

private:
    /// Построить интерфейс
    void createInterface();

    /// Вкладка характеристик (существующий редактор XML)
    QWidget* createCharacteristicsTab();

    /// Вкладка 3D-модели
    QWidget* createModelTab();

    /// Вкладка физических точек
    QWidget* createPointsTab();

    /// Вкладка экспорта
    QWidget* createExportTab();

    /// Заполнить дерево секций по схеме и документу
    void rebuildTree();

    /// Построить форму редактирования секции
    void buildSectionForm(const SectionSpec& spec, QDomElement section);

    /// Прочитать значение поля из элемента секции
    QString fieldValue(const QDomElement& section, const FieldSpec& field,
                       bool& found) const;

    /// Записать значение поля в элемент секции (создаёт тег при отсутствии)
    void setFieldValue(QDomElement& section, const FieldSpec& field,
                       const QString& value);

    /// Значение поля изменено редактором — записать в DOM
    void onFieldEdited(const FieldSpec* field, const QString& value);

    /// Проверка: обязательные ключи [Vehicle] и диапазоны min/max
    QStringList validateConfig() const;

    /// Подсветка редакторов, вышедших за пределы min/max
    void highlightRanges() const;

    /// Сохранить документ в файл (QXmlStreamWriter, прочие секции как есть)
    bool writeConfigFile(const QString& file_path) const;

    /// Сериализовать документ в строку (для экспорта пакета)
    QByteArray configToByteArray() const;

    /// Записать DOM-дерево через QXmlStreamWriter
    void writeDomNode(QXmlStreamWriter& writer, const QDomNode& node) const;

    void updateStatusBar();

    void setModified(bool modified);

    // --- Модель и вьюпорт ---
    /// Загрузить модель и обновить вкладку «Модель»
    bool loadModelFile(const QString& path);

    /// Перестроить дерево объектов модели
    void rebuildObjectTree();

    /// Обновить превью коллизий (по текущим ролям)
    void refreshCollisionPreview();

    /// Обновить маркеры физических точек во вьюпорте
    void refreshPointMarkers();

    /// Показать модель в 3D (запуск вьюпорта)
    bool showModelIn3D();

    /// Применить сохранённые свойства узлов (роли/имена/видимость)
    void applySavedNodeProperties();

    // --- Точки ---
    /// Заполнить таблицу точек
    void rebuildPointsTable();

    /// Прочитать точки из таблицы
    std::vector<PhysPoint> pointsFromTable() const;

    /// Точка по строке таблицы
    PhysPoint pointFromRow(int row) const;

    // --- Конфиг и роли ---
    /// Записать/обновить секцию [Collision] в документе по ролям
    bool applyCollisionToConfig();

    /// Записать/обновить секции [Cabine] по точкам-камерам
    bool applyCameraPointsToConfig();

    // --- Проект ---
    /// Собрать текущее состояние интерфейса в объект проекта
    void collectProjectFromUi();

    /// Применить проект к интерфейсу
    void applyProjectToUi();

    /// Заголовок окна с именем файла проекта
    void updateWindowTitle();

    /// Список недавних проектов (QSettings)
    static QStringList recentProjects();

    /// Запомнить проект в списке недавних
    static void pushRecentProject(const QString& path);

    /// Обновить меню недавних проектов
    void updateRecentMenu();

private:
    /// XML-документ конфига (все секции, правки применяются сразу)
    QDomDocument doc;

    /// Путь к открытому файлу
    QString filePath;

    /// Есть несохранённые изменения
    bool isModified = false;

    /// Дерево секций
    QTreeWidget* sectionsTree = nullptr;

    /// Контейнер формы справа
    QWidget* formWidget = nullptr;

    /// Заголовок формы
    QLabel* formTitle = nullptr;

    /// Кнопка добавления экземпляра секции (для multiple)
    QPushButton* addInstanceButton = nullptr;

    /// Кнопка удаления текущего экземпляра секции (для multiple)
    QPushButton* removeInstanceButton = nullptr;

    /// Спецификация текущей отображаемой секции
    const SectionSpec* currentSpec = nullptr;

    /// Элемент текущей отображаемой секции (может быть null для отсутствующей)
    QDomElement currentSectionElement;

    /// Индекс экземпляра для multiple-секций (-1 — не multiple)
    int currentInstanceIndex = -1;

    /// Редакторы значений по имени ключа (для подсветки и записи)
    struct FieldEditor
    {
        const FieldSpec* spec = nullptr;
        QLineEdit* line_edit = nullptr;
        QSpinBox* int_edit = nullptr;
        QDoubleSpinBox* double_edit = nullptr;
        QPushButton* bool_button = nullptr;
        bool bool_value = false;
    };

    QMap<QString, FieldEditor> fieldEditors;

    // --- Вкладки ---
    QTabWidget* tabWidget = nullptr;

    // --- Вкладка «Модель» ---
    QTreeWidget* objectTree = nullptr;
    QLabel* modelStatusLabel = nullptr;
    QPushButton* toggle3DButton = nullptr;
    QComboBox* lodCombo = nullptr;
    QComboBox* projectionCombo = nullptr;
    QCheckBox* gridCheck = nullptr;
    QCheckBox* collisionCheck = nullptr;

    /// Блокировка обработки сигналов при программной перестройке дерева
    bool objectTreeUpdating = false;

    // --- Вкладка «Физические точки» ---
    QTableWidget* pointsTable = nullptr;

    /// Блокировка обработки сигналов при программной перестройке таблицы
    bool pointsTableUpdating = false;

    // --- Вкладка «Экспорт» ---
    QLineEdit* exportDirEdit = nullptr;
    QLineEdit* exportNameEdit = nullptr;
    QCheckBox* copyModelCheck = nullptr;
    QPlainTextEdit* exportLog = nullptr;

    // --- 3D и данные проекта ---
    SceneModel scene;          ///< Загруженная glTF-модель ПС
    ModelViewer viewer;        ///< 3D-вьюпорт (vsg, отдельное окно)
    TrainProject project;      ///< Текущий проект .trainproject
    QString projectFilePath;   ///< Путь к файлу проекта

    /// Меню недавних проектов
    QMenu* recentMenu = nullptr;
};

#endif // MAINWINDOW_H
