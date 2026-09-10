#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "model-viewer.h"
#include "scene-model.h"
#include "train-project.h"
#include "vehicle-schema.h"

#include <QDomDocument>
#include <QMainWindow>
#include <QXmlStreamWriter>
#include <QMap>
#include <QStringList>

#include <vector>

class QCheckBox;
class QComboBox;
class QDragEnterEvent;
class QDropEvent;
class QLabel;
class QLineEdit;
class QListWidget;
class QMenu;
class QPlainTextEdit;
class QProcess;
class QPushButton;
class QDoubleSpinBox;
class QSpinBox;
class QTabWidget;
class QTableWidget;
class QTableWidgetItem;
class QTreeWidget;
class QTreeWidgetItem;
class QWidget;

class BlendImporter;
class TractionChart;

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

    /// Деструктор вынесен в .cpp: разрушение членов с
    /// vsg::ref_ptr (анимации) требует полных типов vsg
    ~MainWindow() override;

protected:
    /// Drag&Drop файлов .blend/.glb/.gltf на окно (импорт модели)
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

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

    /// Экспорт всех секций схемы характеристик в CSV
    /// (Секция;Ключ;Значение, подготовка к предпросмотру динамики,
    /// промт п.20 вторая часть)
    void slotExportCsv();

    /// Диалог «Проверить характеристики» (валидатор п.18):
    /// диапазоны схемы + логические проверки
    void slotValidateCharacteristics();

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

    /// Импорт .blend: headless-конвертация Blender -> GLB
    void slotImportBlend();

    /// Выбор пути к исполняемому файлу Blender
    void slotBrowseBlender();

    /// Конвертация .blend завершена
    void slotBlendFinished(bool ok, const QString& glb_path,
                           const QString& error);

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

    /// Переключение стрелок осей ПС (дебаг-вид, промт п.38)
    void slotAxesToggled(bool checked);

    /// Переключение бокса габарита 1Т (дебаг-вид, промт п.38)
    void slotGabaritToggled(bool checked);

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

    /// Сохранить центр масс в секцию [MassCenter] конфига
    void slotSaveMassCenter();

    /// Показать маркер центра масс в 3D-вьюпорте
    void slotShowMassCenter();

    // --- Вкладка «Анимации» (ТЗ п.5-6) ---
    /// Добавить анимацию вручную (по имени, в конфиг)
    void slotAnimAdd();

    /// Удалить выбранную анимацию из списка и конфига
    void slotAnimRemove();

    /// Выбрана анимация в списке — загрузить её параметры
    void slotAnimSelected();

    /// Изменены параметры анимации — записать в конфиг
    void slotAnimDataChanged();

    /// Проигрывать выбранную анимацию в 3D-вьюпорте
    void slotAnimPlay();

    /// Остановить проигрывание анимаций
    void slotAnimStop();

    // --- Тяговая характеристика (ТЗ п.6) ---
    /// Добавить строку-точку характеристики
    void slotTractiveAddRow();

    /// Удалить выбранную строку-точку характеристики
    void slotTractiveRemoveRow();

    /// Изменена ячейка таблицы характеристики — записать в конфиг
    void slotTractiveCellChanged(int row, int column);

    // --- Вкладка «Звук» (промт п.8-9, Sound Manager) ---
    /// Добавить звук в таблицу (секция <Sound>)
    void slotSoundAdd();

    /// Удалить выбранный звук из таблицы и конфига
    void slotSoundRemove();

    /// Выбрать файл звука для текущей строки (обзор)
    void slotSoundBrowseFile();

    /// Изменена ячейка таблицы звуков — перезаписать секции <Sound>
    void slotSoundCellChanged(int row, int column);

    /// Проверить существование файлов звуков (подсветка отсутствующих)
    void slotSoundCheckFiles();

    /// Конвертировать файл выбранного звука в OGG внешним ffmpeg
    void slotSoundConvertToOgg();

    /// Выбор пути к исполняемому файлу ffmpeg
    void slotBrowseFfmpeg();

    /// Выбор папки звуков
    void slotBrowseSoundDir();

    // --- Спец-инструменты (промт п.10-12) ---
    void slotSaveHoseParam();
    void slotLoadHoseParam();
    void slotSaveAngleCock();
    void slotLoadAngleCock();
    void slotSaveCouplingSA3();
    void slotLoadCouplingSA3();
    void slotSavePantographTool();
    void slotLoadPantographTool();

    // --- Кабины / CAB EDITER (промт п.17) ---
    /// Выбрана кабина в списке — загрузить позицию/направление
    void slotCabSelected();

    /// Изменены поля кабины — записать в Camera-точку таблицы
    void slotCabDataChanged();

    /// Записать все Camera-точки в секции [Cabine] конфига
    void slotCabApplyToConfig();

    // --- Дерево модели: поиск и теги (промт п.32-34) ---
    /// Фильтр дерева объектов по имени
    void slotFilterObjectTree(const QString& text);

    /// Присвоить метку выбранному узлу (сохраняется в проекте)
    void slotAssignNodeTag();

    /// Выделить все узлы с меткой из поля тега
    void slotSelectByTag();

    /// Применить роль выбранного узла ко всем видимым после фильтра
    void slotMassAssignRole();

    // --- Осевые нагрузки (промт п.15, вторая часть) ---
    /// Изменено ускорение — пересчитать динамическую нагрузку
    void slotAxleAccelChanged();

    // --- Проверка пакета и мастер (промт п.22, 41) ---
    /// Диалог «Проверка пакета» (Asset Validator)
    void slotValidatePackage();

    /// Мастер нового ПС (8 шагов)
    void slotRunWizard();

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

    /// Вкладка анимаций glTF-модели
    QWidget* createAnimationsTab();

    /// Вкладка звуков (Sound Manager, промт п.8-9)
    QWidget* createSoundTab();

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
    bool writeConfigFile(const QString& file_path);

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

    // --- Проверка характеристик (валидатор, промт п.18) ---
    /// Диапазоны схемы + логические проверки; errors/warnings — наружу
    void validateCharacteristics(QStringList* errors,
                                 QStringList* warnings) const;

    // --- Анимации ---
    /// Перестроить список анимаций (из модели и конфига)
    void refreshAnimationsList();

    /// Найти секцию <Animation Name="..."> в документе
    QDomElement findAnimationElement(const QString& name) const;

    // --- Тяговая характеристика ---
    /// Заполнить таблицу точек из секции <TractiveCurve>
    void rebuildTractiveTable();

    /// Обновить превью-график по таблице точек
    void refreshTractiveChart();

    /// Записать точки таблицы в секцию <TractiveCurve>
    void applyTractiveToConfig();

    // --- Центр масс ---
    /// Прочитать [MassCenter] в поля редактора
    void refreshMassCenterFields();

    /// Применить маркер ЦМ к работающему вьюпорту
    void refreshComMarker();

    // --- Звук (промт п.8-9) ---
    /// Заполнить таблицу звуков из секций <Sound> конфига
    void rebuildSoundsTable();

    /// Перезаписать секции <Sound> по содержимому таблицы
    void applySoundsToConfig();

    /// Создать строку таблицы звуков со всеми редакторами
    void fillSoundRow(int row, const QString& name, const QString& file,
                      double volume, double pitch, bool loop, bool is3d,
                      const QString& variants);

    /// Папка поиска звуковых файлов (поле вкладки либо папка конфига)
    QString soundSearchDir() const;

    /// Существует ли звуковой файл (абсолютный/относительно папки звуков)
    bool soundFileExists(const QString& file_path,
                         QString* found_path = nullptr) const;

    // --- Кабины (CAB EDITER, промт п.17) ---
    /// Перестроить список кабин по Camera-точкам таблицы
    void refreshCabList();

    // --- Осевые нагрузки (промт п.15) ---
    /// Перестроить таблицу нагрузок по осям из [Vehicle]
    void refreshAxleLoadTable();

    // --- Проверка пакета (Asset Validator, промт п.22) ---
    /// Полный набор проверок: errors блокируют экспорт, warnings — нет
    void validatePackage(QStringList* errors, QStringList* warnings) const;

    // --- Дерево модели: поиск/теги (промт п.32-34) ---
    /// Применить фильтр к дереву объектов (скрытие несовпадающих)
    void applyObjectFilter();

    /// Собрать метки узлов с текущих элементов дерева (путь -> метка)
    QMap<QString, QString> harvestObjectTags() const;

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

    /// Дебаг-вид (промт п.38): стрелки осей ПС и бокс габарита 1Т
    QCheckBox* axesCheck = nullptr;
    QCheckBox* gabaritCheck = nullptr;

    /// Путь к исполняемому файлу Blender (для импорта .blend)
    QLineEdit* blenderPathEdit = nullptr;

    /// Конвертация .blend -> GLB внешним Blender
    BlendImporter* blendImporter = nullptr;

    /// Блокировка обработки сигналов при программной перестройке дерева
    bool objectTreeUpdating = false;

    // --- Вкладка «Анимации» ---
    /// Список анимаций: имя + ссылка на vsg::Animation модели (если есть)
    struct AnimEntry
    {
        QString name;                            ///< Имя анимации
        vsg::ref_ptr<vsg::Animation> animation;  ///< Анимация glTF (может быть null)
    };

    QListWidget* animationsList = nullptr;
    QComboBox* animEventCombo = nullptr;
    QDoubleSpinBox* animSpeedSpin = nullptr;
    QCheckBox* animLoopCheck = nullptr;
    QLineEdit* animSoundEdit = nullptr;
    std::vector<AnimEntry> animEntries;

    /// Блокировка сигналов при программном заполнении полей анимации
    bool animUpdating = false;

    // --- Тяговая характеристика ---
    QTableWidget* tractiveTable = nullptr;
    TractionChart* tractiveChart = nullptr;

    /// Блокировка сигналов при программной перестройке таблицы
    bool tractiveUpdating = false;

    // --- Центр масс ---
    QDoubleSpinBox* comXSpin = nullptr;
    QDoubleSpinBox* comYSpin = nullptr;
    QDoubleSpinBox* comZSpin = nullptr;

    /// Маркер ЦМ показан в 3D (переустанавливается при запуске вьюпорта)
    bool comMarkerShown = false;

    // --- Вкладка «Физические точки» ---
    QTableWidget* pointsTable = nullptr;

    /// Блокировка обработки сигналов при программной перестройке таблицы
    bool pointsTableUpdating = false;

    // --- Кабины (CAB EDITER, промт п.17) ---
    /// Список кабин (по Camera-точкам); в itemData хранится строка таблицы
    QComboBox* cabCombo = nullptr;
    QDoubleSpinBox* cabXSpin = nullptr;
    QDoubleSpinBox* cabYSpin = nullptr;
    QDoubleSpinBox* cabZSpin = nullptr;
    QDoubleSpinBox* cabDirSpin = nullptr;
    QPushButton* cabApplyButton = nullptr;

    /// Блокировка сигналов при программном заполнении полей кабины
    bool cabUpdating = false;

    // --- Спец-инструменты (промт п.10-12) ---
    /// Тормозной рукав: <HoseParam Length Radius Mass/>
    QDoubleSpinBox* hoseLengthSpin = nullptr;
    QDoubleSpinBox* hoseRadiusSpin = nullptr;
    QDoubleSpinBox* hoseMassSpin = nullptr;

    /// Концевой кран: <AngleCock Type SwitchTime/>
    QComboBox* angleCockTypeCombo = nullptr;
    QDoubleSpinBox* angleCockTimeSpin = nullptr;

    /// Автосцепка СА-3: <CouplingSA3 Device Stiffness MaxForce CouplingSpeed/>
    QComboBox* sa3DeviceCombo = nullptr;
    QDoubleSpinBox* sa3StiffnessSpin = nullptr;
    QDoubleSpinBox* sa3MaxForceSpin = nullptr;
    QDoubleSpinBox* sa3SpeedSpin = nullptr;

    /// Токоприёмник (инструмент пакета, НЕ [Pantograph] движка):
    /// <PantographTool MinHeight MaxHeight StaticForce/>
    QDoubleSpinBox* pantMinSpin = nullptr;
    QDoubleSpinBox* pantMaxSpin = nullptr;
    QDoubleSpinBox* pantForceSpin = nullptr;

    // --- Вкладка «Звук» ---
    QTableWidget* soundTable = nullptr;

    /// Папка поиска звуковых файлов (QSettings "soundDir")
    QLineEdit* soundDirEdit = nullptr;

    /// Путь к ffmpeg для конвертации в OGG (QSettings "ffmpegPath")
    QLineEdit* ffmpegPathEdit = nullptr;

    /// Процесс конвертации ffmpeg (одна конвертация одновременно)
    QProcess* ffmpegProcess = nullptr;

    /// Блокировка сигналов при программной перестройке таблицы звуков
    bool soundTableUpdating = false;

    // --- Осевые нагрузки (промт п.15) ---
    QTableWidget* axleTable = nullptr;

    /// Ускорение для динамической нагрузки, м/с^2
    QDoubleSpinBox* axleAccelSpin = nullptr;

    // --- Дерево модели: поиск и теги (промт п.32-34) ---
    QLineEdit* objectFilterEdit = nullptr;
    QLineEdit* objectTagEdit = nullptr;

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
