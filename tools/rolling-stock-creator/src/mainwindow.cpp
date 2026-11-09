#include "mainwindow.h"

#include <QStatusBar>
#include <QXmlStreamWriter>

#include "BlendImporter.h"
#include "export-pipeline.h"
#include "filesystem.h"
#include "NewVehicleWizard.h"
#include "TractionChart.h"

#include <QAction>
#include <QApplication>
#include <QColor>
#include <QComboBox>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDialog>
#include <QDomDocument>
#include <QDoubleSpinBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSpinBox>
#include <QSplitter>
#include <QTableWidget>
#include <QTextStream>
#include <QToolBar>
#include <QTreeWidget>
#include <QUrl>
#include <QVBoxLayout>

#include <QStringConverter>

#include <QXmlStreamWriter>

#include <QDir>
#include <QTreeWidgetItemIterator>

#include <vsg/animation/Animation.h>
#include <vsg/nodes/Node.h>

#include <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
namespace
{

constexpr int RoleSectionName = Qt::UserRole + 1;
constexpr int RoleInstanceIndex = Qt::UserRole + 2;
constexpr int RoleKnownSchema = Qt::UserRole + 3;
constexpr int RoleNodePath = Qt::UserRole + 4;
constexpr int RoleNodeTag = Qt::UserRole + 5;

/// Предел статической нагрузки на ось, тс (промт п.15)
constexpr double AxleLoadLimitTs = 22.5;

/// Узел дерева соответствует фильтру: имя содержит текст
/// или соответствует хотя бы один потомок (промт п.32)
bool itemMatchesFilter(QTreeWidgetItem* item, const QString& filter)
{
    if (item->text(0).contains(filter, Qt::CaseInsensitive))
    {
        return true;
    }

    for (int i = 0; i < item->childCount(); ++i)
    {
        if (itemMatchesFilter(item->child(i), filter))
        {
            return true;
        }
    }

    return false;
}

/// Скрыть/показать узел вместе со всеми потомками
void setItemHiddenRecursive(QTreeWidgetItem* item, bool hidden)
{
    item->setHidden(hidden);

    for (int i = 0; i < item->childCount(); ++i)
    {
        setItemHiddenRecursive(item->child(i), hidden);
    }
}

/// Первый дочерний элемент корня с заданным именем тега
QDomElement firstSectionElement(const QDomDocument& doc, const QString& name)
{
    const QDomElement root = doc.documentElement();

    if (root.isNull())
    {
        return QDomElement();
    }

    for (QDomNode node = root.firstChild(); !node.isNull();
         node = node.nextSibling())
    {
        if (node.isElement() && node.toElement().tagName() == name)
        {
            return node.toElement();
        }
    }

    return QDomElement();
}

/// Текст простого тега внутри секции (без схемы)
QString tagText(const QDomElement& section, const char* tag, bool& found)
{
    found = false;

    if (section.isNull())
    {
        return QString();
    }

    const QDomElement element =
            section.firstChildElement(QString::fromLatin1(tag));

    if (!element.isNull())
    {
        found = true;
        return element.text();
    }

    return QString();
}

/// N-й по счёту дочерний элемент корня с заданным именем тега
QDomElement sectionElementByIndex(const QDomDocument& doc,
                                  const QString& name, int index)
{
    const QDomElement root = doc.documentElement();

    if (root.isNull() || index < 0)
    {
        return QDomElement();
    }

    int current = 0;

    for (QDomNode node = root.firstChild(); !node.isNull();
         node = node.nextSibling())
    {
        if (node.isElement() && node.toElement().tagName() == name)
        {
            if (current == index)
            {
                return node.toElement();
            }

            ++current;
        }
    }

    return QDomElement();
}

/// Добавить в конец корня новый элемент секции
QDomElement appendSectionElement(QDomDocument& doc, const QString& name)
{
    QDomElement root = doc.documentElement();

    if (root.isNull())
    {
        root = doc.createElement("Config");
        doc.appendChild(root);
    }

    QDomElement element = doc.createElement(name);
    root.appendChild(element);

    return element;
}

/// Удалить все секции с заданным именем тега
void removeSectionElements(QDomDocument& doc, const QString& name)
{
    QDomElement root = doc.documentElement();

    if (root.isNull())
    {
        return;
    }

    QDomNode node = root.firstChild();

    while (!node.isNull())
    {
        QDomNode next = node.nextSibling();

        if (node.isElement() && node.toElement().tagName() == name)
        {
            root.removeChild(node);
        }

        node = next;
    }
}

/// Записать текстовое значение дочернего тега секции (создаёт тег)
void setSectionChildValue(QDomDocument& doc, QDomElement& section,
                          const QString& key, const QString& value)
{
    for (QDomNode node = section.firstChild(); !node.isNull();
         node = node.nextSibling())
    {
        if (node.isElement() && node.toElement().tagName() == key)
        {
            QDomElement element = node.toElement();
            QDomNode text_node = element.firstChild();

            if (!text_node.isNull() && text_node.isText())
            {
                text_node.toText().setData(value);
            }
            else
            {
                element.appendChild(doc.createTextNode(value));
            }

            return;
        }
    }

    QDomElement element = doc.createElement(key);
    element.appendChild(doc.createTextNode(value));
    section.appendChild(element);
}

int countSectionElements(const QDomDocument& doc, const QString& name)
{
    const QDomElement root = doc.documentElement();

    if (root.isNull())
    {
        return 0;
    }

    int count = 0;

    for (QDomNode node = root.firstChild(); !node.isNull();
         node = node.nextSibling())
    {
        if (node.isElement() && node.toElement().tagName() == name)
        {
            ++count;
        }
    }

    return count;
}

QString boolToText(bool value)
{
    return value ? QStringLiteral("true") : QStringLiteral("false");
}

/// Число для записи в конфиг: компактный вид без лишних нулей
QString numberToText(double value)
{
    return QString::number(value, 'g', 6);
}

/// Каталог addons по умолчанию: <корень игры>/addons
QString defaultAddonsDir()
{
    const FileSystem& fs = FileSystem::getInstance();
    const QString config_dir = QString::fromStdString(fs.getConfigDir());

    if (!config_dir.isEmpty())
    {
        const QDir root = QFileInfo(config_dir).absoluteDir();

        if (!root.absolutePath().isEmpty() && root.absolutePath() != ".")
        {
            return root.absoluteFilePath(QStringLiteral("addons"));
        }
    }

    return QDir::homePath();
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    createInterface();
    rebuildTree();
    rebuildPointsTable();
    rebuildTractiveTable();
    refreshMassCenterFields();
    refreshAnimationsList();
    rebuildSoundsTable();
    refreshAxleLoadTable();
    updateStatusBar();
    updateRecentMenu();
    updateWindowTitle();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MainWindow::~MainWindow() = default;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::createInterface()
{
    resize(1200, 760);

    // --- Панель инструментов конфигурации (существующие операции) ---
    auto* toolbar = addToolBar(tr("Файл"));
    toolbar->setMovable(false);

    QAction* openAct = toolbar->addAction(tr("Открыть конфигурацию..."));
    openAct->setShortcut(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, &MainWindow::slotOpenConfig);

    QAction* saveAct = toolbar->addAction(tr("Сохранить"));
    saveAct->setShortcut(QKeySequence::Save);
    connect(saveAct, &QAction::triggered, this, &MainWindow::slotSaveConfig);

    QAction* saveAsAct = toolbar->addAction(tr("Сохранить как..."));
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::slotSaveAsConfig);

    toolbar->addSeparator();

    QAction* openExternalAct = toolbar->addAction(tr("Открыть конфигурацию"));
    openExternalAct->setToolTip(
        tr("Открыть текущий файл во внешнем (системном) редакторе"));
    connect(openExternalAct, &QAction::triggered,
            this, &MainWindow::slotOpenInSystemEditor);

    // --- Меню «Файл»: мастер нового ПС (промт п.41) ---
    QMenu* file_menu = menuBar()->addMenu(tr("Файл"));

    QAction* wizard_act = file_menu->addAction(tr("Мастер нового ПС..."));
    wizard_act->setToolTip(
        tr("Компактный мастер создания ПС в 8 шагов: шаблон, модель, "
           "массы, габариты, кабина"));
    connect(wizard_act, &QAction::triggered, this, &MainWindow::slotRunWizard);

    // --- Меню «Проект» (.trainproject, ТЗ п.30-31) ---
    QMenu* project_menu = menuBar()->addMenu(tr("Проект"));

    // «Файл» должен стоять левее «Проекта»
    menuBar()->insertMenu(project_menu->menuAction(), file_menu);

    QAction* new_act = project_menu->addAction(tr("Новый из шаблона..."));
    connect(new_act, &QAction::triggered, this, &MainWindow::slotNewProject);

    QAction* open_act = project_menu->addAction(tr("Открыть проект..."));
    connect(open_act, &QAction::triggered, this, &MainWindow::slotOpenProject);

    project_menu->addSeparator();

    QAction* save_act = project_menu->addAction(tr("Сохранить проект"));
    connect(save_act, &QAction::triggered, this, &MainWindow::slotSaveProject);

    QAction* save_as_act = project_menu->addAction(tr("Сохранить проект как..."));
    connect(save_as_act, &QAction::triggered, this,
            &MainWindow::slotSaveProjectAs);

    project_menu->addSeparator();

    recentMenu = project_menu->addMenu(tr("Недавние проекты"));

    // --- Вкладки ---
    tabWidget = new QTabWidget(this);
    tabWidget->addTab(createCharacteristicsTab(), tr("Характеристики"));
    tabWidget->addTab(createModelTab(), tr("Модель"));
    tabWidget->addTab(createAnimationsTab(), tr("Анимации"));
    tabWidget->addTab(createSoundTab(), tr("Звук"));
    tabWidget->addTab(createPointsTab(), tr("Физические точки"));
    tabWidget->addTab(createExportTab(), tr("Экспорт"));

    setCentralWidget(tabWidget);

    // Drag&Drop моделей (.blend/.glb/.gltf) на окно
    setAcceptDrops(true);

    // Конвертация .blend внешним Blender
    blendImporter = new BlendImporter(this);
    connect(blendImporter, &BlendImporter::finished,
            this, &MainWindow::slotBlendFinished);

    // Конвертация звуков в OGG внешним ffmpeg (промт п.8-9)
    ffmpegProcess = new QProcess(this);
    connect(ffmpegProcess, &QProcess::finished,
            this, [this](int exit_code, QProcess::ExitStatus status)
    {
        if (ffmpegPathEdit == nullptr)
        {
            return;
        }

        if (status == QProcess::NormalExit && exit_code == 0)
        {
            statusBar()->showMessage(
                        tr("Конвертация в OGG завершена"), 8000);
            slotSoundCheckFiles();
        }
        else
        {
            statusBar()->showMessage(
                        tr("Ошибка ffmpeg (код %1). Проверьте путь: %2")
                            .arg(exit_code)
                            .arg(ffmpegPathEdit->text()), 10000);
        }
    });

    connect(ffmpegProcess, &QProcess::errorOccurred,
            this, [this](QProcess::ProcessError)
    {
        if (ffmpegPathEdit != nullptr)
        {
            statusBar()->showMessage(
                        tr("Не удалось запустить ffmpeg: %1")
                            .arg(ffmpegPathEdit->text()), 10000);
        }
    });

    // Настройки экспорта по умолчанию
    exportNameEdit->setText(QStringLiteral("new_vehicle"));
    exportDirEdit->setText(defaultAddonsDir());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QWidget* MainWindow::createCharacteristicsTab()
{
    // Дерево секций слева
    sectionsTree = new QTreeWidget(this);
    sectionsTree->setHeaderLabel(tr("Секции конфигурации"));
    sectionsTree->setRootIsDecorated(true);
    connect(sectionsTree, &QTreeWidget::itemClicked,
            this, &MainWindow::slotSectionSelected);

    // Форма справа
    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);

    auto* content = new QWidget(scrollArea);
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(8, 8, 8, 8);

    formTitle = new QLabel(tr("Откройте XML-конфиг ПС (\"Открыть конфигурацию...\") "
                              "или создайте проект из шаблона"),
                           content);
    formTitle->setWordWrap(true);
    contentLayout->addWidget(formTitle);

    auto* buttonsRow = new QHBoxLayout();
    addInstanceButton = new QPushButton(tr("Добавить"), content);
    addInstanceButton->setVisible(false);
    connect(addInstanceButton, &QPushButton::clicked,
            this, &MainWindow::slotAddCabElement);
    buttonsRow->addWidget(addInstanceButton);

    removeInstanceButton = new QPushButton(tr("Удалить экземпляр"), content);
    removeInstanceButton->setVisible(false);
    connect(removeInstanceButton, &QPushButton::clicked,
            this, &MainWindow::slotRemoveCurrentSectionInstance);
    buttonsRow->addWidget(removeInstanceButton);

    // Экспорт характеристик в CSV (промт п.20, вторая часть —
    // подготовка к предпросмотру динамики)
    auto* exportCsvButton = new QPushButton(tr("Экспорт CSV..."), content);
    exportCsvButton->setToolTip(
        tr("Все секции схемы характеристик в CSV вида "
           "Секция;Ключ;Значение (для Excel/предпросмотра динамики)"));
    connect(exportCsvButton, &QPushButton::clicked,
            this, &MainWindow::slotExportCsv);
    buttonsRow->addWidget(exportCsvButton);

    // Проверка характеристик (валидатор, промт п.18)
    auto* validateButton = new QPushButton(tr("Проверить характеристики"),
                                            content);
    validateButton->setToolTip(
        tr("Диапазоны схемы (min/max) и логические проверки: массы, "
           "оси, колёса, центр масс, тип привода, песок"));
    connect(validateButton, &QPushButton::clicked,
            this, &MainWindow::slotValidateCharacteristics);
    buttonsRow->addWidget(validateButton);

    buttonsRow->addStretch();

    contentLayout->addLayout(buttonsRow);

    formWidget = new QWidget(content);
    auto* formLayout = new QFormLayout(formWidget);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    contentLayout->addWidget(formWidget);

    // Тяговая характеристика (ТЗ п.6): таблица точек F(v)
    // + превью-график; точки хранятся в секции <TractiveCurve>
    auto* tractiveGroup = new QGroupBox(
                tr("Тяговая характеристика (сила тяги по скорости)"),
                content);
    auto* tractiveLayout = new QHBoxLayout(tractiveGroup);

    auto* tractiveLeft = new QVBoxLayout();

    auto* tractiveButtons = new QHBoxLayout();
    auto* tractiveAddButton = new QPushButton(tr("Добавить точку"),
                                              tractiveGroup);
    connect(tractiveAddButton, &QPushButton::clicked,
            this, &MainWindow::slotTractiveAddRow);
    tractiveButtons->addWidget(tractiveAddButton);

    auto* tractiveRemoveButton = new QPushButton(tr("Удалить точку"),
                                                 tractiveGroup);
    connect(tractiveRemoveButton, &QPushButton::clicked,
            this, &MainWindow::slotTractiveRemoveRow);
    tractiveButtons->addWidget(tractiveRemoveButton);
    tractiveButtons->addStretch();
    tractiveLeft->addLayout(tractiveButtons);

    tractiveTable = new QTableWidget(tractiveGroup);
    tractiveTable->setColumnCount(2);
    
    tractiveTable->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Скорость, км/ч")));
    tractiveTable->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Сила, кН")));

    tractiveTable->horizontalHeader()->setSectionResizeMode(
                QHeaderView::ResizeToContents);
    tractiveTable->verticalHeader()->setVisible(false);
    tractiveTable->setMaximumHeight(180);
    connect(tractiveTable, &QTableWidget::cellChanged, this,
            &MainWindow::slotTractiveCellChanged);
    tractiveLeft->addWidget(tractiveTable);

    tractiveLayout->addLayout(tractiveLeft, 1);

    tractiveChart = new TractionChart(tractiveGroup);
    tractiveLayout->addWidget(tractiveChart, 1);

    contentLayout->addWidget(tractiveGroup);

    // Нагрузки по осям (промт п.15, вторая часть): таблица по числу осей
    // [Vehicle] NumAxis; статическая — из полной массы, динамическая —
    // с учётом вертикального ускорения; подсветка >22.5 тс красным
    auto* axleGroup = new QGroupBox(
                tr("Нагрузки по осям (брутто/тара/статическая/динамическая)"),
                content);
    auto* axleLayout = new QVBoxLayout(axleGroup);

    auto* axleControls = new QHBoxLayout();
    axleControls->addWidget(new QLabel(tr("Ускорение, м/с²:"), axleGroup));

    axleAccelSpin = new QDoubleSpinBox(axleGroup);
    axleAccelSpin->setRange(0.0, 5.0);
    axleAccelSpin->setDecimals(2);
    axleAccelSpin->setSingleStep(0.1);
    axleAccelSpin->setValue(1.0);
    axleAccelSpin->setToolTip(
        tr("Вертикальное ускорение для динамической нагрузки: "
           "динамическая = статическая × (1 + ускорение / 9.81)"));
    connect(axleAccelSpin, &QDoubleSpinBox::valueChanged,
            this, &MainWindow::slotAxleAccelChanged);
    axleControls->addWidget(axleAccelSpin);

    auto* axleHint = new QLabel(
        tr("Красным — нагрузка свыше %1 тс.")
            .arg(AxleLoadLimitTs, 0, 'f', 1), axleGroup);
    axleControls->addWidget(axleHint);
    axleControls->addStretch();
    axleLayout->addLayout(axleControls);

    axleTable = new QTableWidget(axleGroup);
    axleTable->setColumnCount(5);
    
    axleTable->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Ось")));
    axleTable->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Брутто, т/ось")));
    axleTable->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Тара, т/ось")));
    axleTable->setHorizontalHeaderItem(3, new QTableWidgetItem(tr("Статическая, тс")));
    axleTable->setHorizontalHeaderItem(4, new QTableWidgetItem(tr("Динамическая, тс")));

    axleTable->horizontalHeader()->setSectionResizeMode(
                QHeaderView::ResizeToContents);
    axleTable->verticalHeader()->setVisible(false);
    axleTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    axleTable->setMaximumHeight(160);
    axleTable->setToolTip(tr(
        "Заполняется по [Vehicle]: EmptyMass, PayloadMass, NumAxis"));
    axleLayout->addWidget(axleTable);

    contentLayout->addWidget(axleGroup);
    contentLayout->addStretch();

    scrollArea->setWidget(content);

    auto* splitter = new QSplitter(this);
    splitter->addWidget(sectionsTree);
    splitter->addWidget(scrollArea);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({320, 780});

    return splitter;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QWidget* MainWindow::createModelTab()
{
    auto* widget = new QWidget(this);
    auto* layout = new QVBoxLayout(widget);

    // Панель управления вкладки «Модель»
    auto* controls = new QHBoxLayout();

    auto* loadButton = new QPushButton(tr("Загрузить модель..."), widget);
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::slotLoadModel);
    controls->addWidget(loadButton);

    // Импорт .blend: конвертация внешним Blender в GLB (ТЗ п.1)
    auto* blendButton = new QPushButton(tr("Импорт .blend..."), widget);
    blendButton->setToolTip(
        tr("Конвертация .blend в GLB внешним Blender "
           "(путь задаётся в поле «Blender» ниже)"));
    connect(blendButton, &QPushButton::clicked,
            this, &MainWindow::slotImportBlend);
    controls->addWidget(blendButton);

    auto* autoButton = new QPushButton(tr("Распознать роли"), widget);
    autoButton->setToolTip(tr("Автоматическое определение ролей по именам "
                              "объектов (ТЗ п.3)"));
    connect(autoButton, &QPushButton::clicked, this,
            &MainWindow::slotAutoAssignRoles);
    controls->addWidget(autoButton);

    toggle3DButton = new QPushButton(tr("Показать 3D"), widget);
    connect(toggle3DButton, &QPushButton::clicked, this, &MainWindow::slotToggle3D);
    controls->addWidget(toggle3DButton);

    auto* focusButton = new QPushButton(tr("Фокус"), widget);
    focusButton->setToolTip(tr("Навести камеру на выбранный объект"));
    connect(focusButton, &QPushButton::clicked, this,
            &MainWindow::slotFocusSelected);
    controls->addWidget(focusButton);

    auto* renameButton = new QPushButton(tr("Переименовать"), widget);
    renameButton->setToolTip(tr("Изменить display-имя выбранного объекта"));
    connect(renameButton, &QPushButton::clicked, this,
            &MainWindow::slotRenameSelected);
    controls->addWidget(renameButton);

    controls->addStretch();

    controls->addWidget(new QLabel(tr("LOD:"), widget));

    lodCombo = new QComboBox(widget);
    lodCombo->setToolTip(tr("Варианты модели LOD рядом с файлом "
                            "(ТЗ п.20, только превью)"));
    lodCombo->setEnabled(false);
    connect(lodCombo, &QComboBox::currentIndexChanged, this,
            &MainWindow::slotLodChanged);
    controls->addWidget(lodCombo);

    projectionCombo = new QComboBox(widget);
    projectionCombo->addItem(tr("Перспектива"));
    projectionCombo->addItem(tr("Ортография"));
    connect(projectionCombo, &QComboBox::currentIndexChanged, this,
            &MainWindow::slotProjectionChanged);
    controls->addWidget(projectionCombo);

    gridCheck = new QCheckBox(tr("Сетка"), widget);
    gridCheck->setChecked(true);
    connect(gridCheck, &QCheckBox::toggled, this, &MainWindow::slotGridToggled);
    controls->addWidget(gridCheck);

    collisionCheck = new QCheckBox(tr("Коллизии"), widget);
    collisionCheck->setToolTip(tr("Полупрозрачные примитивы коллизий "
                                  "поверх модели (ТЗ п.19)"));
    collisionCheck->setChecked(true);
    connect(collisionCheck, &QCheckBox::toggled, this,
            &MainWindow::slotCollisionToggled);
    controls->addWidget(collisionCheck);

    // Дебаг-вид (промт п.38): стрелки осей XYZ в центре модели
    axesCheck = new QCheckBox(tr("Оси ПС"), widget);
    axesCheck->setToolTip(tr("Трёхцветные стрелки осей в центре модели: "
                             "X — красная (вправо), Y — зелёная (вдоль "
                             "пути), Z — синяя (вверх)"));
    connect(axesCheck, &QCheckBox::toggled, this,
            &MainWindow::slotAxesToggled);
    controls->addWidget(axesCheck);

    // Дебаг-вид (промт п.38): полупрозрачный бокс габарита 1Т
    gabaritCheck = new QCheckBox(tr("Габарит"), widget);
    gabaritCheck->setToolTip(tr("Полупрозрачный бокс габарита 1Т: "
                                "3.7 м x 5.3 м, длина — по bbox модели"));
    connect(gabaritCheck, &QCheckBox::toggled, this,
            &MainWindow::slotGabaritToggled);
    controls->addWidget(gabaritCheck);

    layout->addLayout(controls);

    // Путь к Blender для импорта .blend (QSettings, по умолчанию "blender")
    auto* blenderRow = new QHBoxLayout();
    blenderRow->addWidget(new QLabel(tr("Blender:"), widget));

    blenderPathEdit = new QLineEdit(widget);
    blenderPathEdit->setToolTip(tr("Исполняемый файл Blender "
                                   "(blender или blender.exe); "
                                   "поиск в PATH, если задано имя"));
    blenderPathEdit->setMinimumWidth(280);

    QSettings settings;
    blenderPathEdit->setText(
                settings.value(QStringLiteral("blenderPath"),
                               QStringLiteral("blender")).toString());

    connect(blenderPathEdit, &QLineEdit::editingFinished, this, [this]()
    {
        QSettings save_settings;
        save_settings.setValue(QStringLiteral("blenderPath"),
                               blenderPathEdit->text().trimmed());
    });

    blenderRow->addWidget(blenderPathEdit, 1);

    auto* blenderBrowseButton = new QPushButton(tr("Обзор..."), widget);
    connect(blenderBrowseButton, &QPushButton::clicked,
            this, &MainWindow::slotBrowseBlender);
    blenderRow->addWidget(blenderBrowseButton);

    layout->addLayout(blenderRow);

    // Статус модели
    modelStatusLabel = new QLabel(tr("Модель не загружена"), widget);
    modelStatusLabel->setWordWrap(true);
    layout->addWidget(modelStatusLabel);

    // Панель поиска и тегов дерева объектов (промт п.32-34)
    auto* treeTools = new QHBoxLayout();

    objectFilterEdit = new QLineEdit(widget);
    objectFilterEdit->setPlaceholderText(tr("Поиск объектов по имени..."));
    objectFilterEdit->setClearButtonEnabled(true);
    objectFilterEdit->setMinimumWidth(180);
    connect(objectFilterEdit, &QLineEdit::textChanged,
            this, &MainWindow::slotFilterObjectTree);
    treeTools->addWidget(objectFilterEdit, 1);

    objectTagEdit = new QLineEdit(widget);
    objectTagEdit->setPlaceholderText(tr("Метка узла"));
    objectTagEdit->setToolTip(tr("Пользовательская метка узла; "
                                 "сохраняется в проекте .trainproject"));
    treeTools->addWidget(objectTagEdit);

    auto* assignTagButton = new QPushButton(tr("Назначить тег"), widget);
    assignTagButton->setToolTip(
        tr("Присвоить метку из поля слева выбранному узлу дерева"));
    connect(assignTagButton, &QPushButton::clicked,
            this, &MainWindow::slotAssignNodeTag);
    treeTools->addWidget(assignTagButton);

    auto* selectTagButton = new QPushButton(tr("Выделить по тегу"), widget);
    selectTagButton->setToolTip(
        tr("Выделить все узлы с меткой из поля тега"));
    connect(selectTagButton, &QPushButton::clicked,
            this, &MainWindow::slotSelectByTag);
    treeTools->addWidget(selectTagButton);

    auto* massRoleButton = new QPushButton(tr("Массово назначить роль"), widget);
    massRoleButton->setToolTip(
        tr("Применить роль выбранного узла ко всем видимым объектам "
           "(после фильтра поиска)"));
    connect(massRoleButton, &QPushButton::clicked,
            this, &MainWindow::slotMassAssignRole);
    treeTools->addWidget(massRoleButton);

    layout->addLayout(treeTools);

    // Дерево объектов модели (assimp/glTF scene graph)
    objectTree = new QTreeWidget(widget);
    objectTree->setHeaderLabels({tr("Объект"), tr("Игровая роль"),
                                 tr("Метка")});
    objectTree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    objectTree->setColumnWidth(1, 180);
    connect(objectTree, &QTreeWidget::itemChanged, this,
            &MainWindow::slotObjectItemChanged);
    layout->addWidget(objectTree, 1);

    return widget;
}

//------------------------------------------------------------------------------
//
//  Вкладка «Анимации»: список анимаций glTF-модели и привязка
//  событий/скорости/зацикливания/звука (ТЗ п.5-6). Данные пишутся
//  в секции <Animation Name="..." Event="..." Speed="1.0" Loop="0"
//  Sound=""/> конфига; проигрывание — в 3D-вьюпорте.
//
//------------------------------------------------------------------------------
QWidget* MainWindow::createAnimationsTab()
{
    auto* widget = new QWidget(this);
    auto* layout = new QHBoxLayout(widget);

    // Слева: список анимаций
    auto* leftLayout = new QVBoxLayout();

    auto* buttonsRow = new QHBoxLayout();
    auto* addButton = new QPushButton(tr("Добавить..."), widget);
    addButton->setToolTip(tr("Добавить анимацию по имени вручную "
                             "(если её нет в модели)"));
    connect(addButton, &QPushButton::clicked, this,
            &MainWindow::slotAnimAdd);
    buttonsRow->addWidget(addButton);

    auto* removeButton = new QPushButton(tr("Удалить"), widget);
    removeButton->setToolTip(tr("Удалить выбранную анимацию из списка "
                                "и из конфига"));
    connect(removeButton, &QPushButton::clicked, this,
            &MainWindow::slotAnimRemove);
    buttonsRow->addWidget(removeButton);
    buttonsRow->addStretch();
    leftLayout->addLayout(buttonsRow);

    animationsList = new QListWidget(widget);
    animationsList->setMinimumWidth(240);
    connect(animationsList, &QListWidget::itemSelectionChanged, this,
            &MainWindow::slotAnimSelected);
    leftLayout->addWidget(animationsList, 1);

    auto* listHint = new QLabel(
        tr("Список заполняется из анимаций glTF-модели и секций "
           "<Animation> конфига. Отсутствующие в модели анимации "
           "можно добавить вручную."), widget);
    listHint->setWordWrap(true);
    leftLayout->addWidget(listHint);

    layout->addLayout(leftLayout, 1);

    // Справа: параметры выбранной анимации
    auto* rightPanel = new QWidget(widget);
    auto* form = new QFormLayout(rightPanel);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // Событие: типовые ключи + произвольное значение
    animEventCombo = new QComboBox(rightPanel);
    animEventCombo->setEditable(true);

    struct EventItem
    {
        const char* key;
        const char* tooltip;
    };

    static const EventItem events[] =
    {
        {"door_open",      "Открытие двери"},
        {"door_close",     "Закрытие двери"},
        {"pantograph_up",  "Подъём токоприёмника"},
        {"pantograph_down","Опускание токоприёмника"},
        {"sand",           "Подача песка"},
        {"compressor",     "Компрессор"},
        {"whistle",        "Свисток"}
    };

    for (const EventItem& item : events)
    {
        animEventCombo->addItem(QString::fromLatin1(item.key));
        animEventCombo->setItemData(animEventCombo->count() - 1,
                                    tr(item.tooltip),
                                    Qt::ToolTipRole);
    }

    form->addRow(tr("Событие:"), animEventCombo);

    animSpeedSpin = new QDoubleSpinBox(rightPanel);
    animSpeedSpin->setRange(0.1, 5.0);
    animSpeedSpin->setSingleStep(0.1);
    animSpeedSpin->setValue(1.0);
    form->addRow(tr("Скорость (множитель):"), animSpeedSpin);

    animLoopCheck = new QCheckBox(tr("Зацикливание"), rightPanel);
    form->addRow(QString(), animLoopCheck);

    auto* soundRow = new QHBoxLayout();
    animSoundEdit = new QLineEdit(rightPanel);
    animSoundEdit->setPlaceholderText(tr("путь к звуковому файлу"));
    soundRow->addWidget(animSoundEdit, 1);

    auto* soundBrowseButton = new QPushButton(tr("..."), rightPanel);
    connect(soundBrowseButton, &QPushButton::clicked, this, [this]()
    {
        const QString path = QFileDialog::getOpenFileName(this,
            tr("Звук анимации"), QString(),
            tr("Звуковые файлы (*.ogg *.wav *.mp3 *.flac);;Все файлы (*.*)"));

        if (!path.isEmpty())
        {
            animSoundEdit->setText(path);
        }
    });
    soundRow->addWidget(soundBrowseButton);
    form->addRow(tr("Звук:"), soundRow);

    // Изменения полей пишутся в конфиг сразу
    connect(animEventCombo, &QComboBox::currentTextChanged, this,
            &MainWindow::slotAnimDataChanged);
    connect(animEventCombo, &QComboBox::editTextChanged, this,
            &MainWindow::slotAnimDataChanged);
    connect(animSpeedSpin, &QDoubleSpinBox::valueChanged, this,
            &MainWindow::slotAnimDataChanged);
    connect(animLoopCheck, &QCheckBox::toggled, this,
            &MainWindow::slotAnimDataChanged);
    connect(animSoundEdit, &QLineEdit::editingFinished, this,
            &MainWindow::slotAnimDataChanged);

    // Проигрывание в 3D-вьюпорте
    auto* playRow = new QHBoxLayout();
    auto* playButton = new QPushButton(tr("Play"), rightPanel);
    playButton->setToolTip(tr("Проигрывать выбранную анимацию "
                              "в 3D-вьюпорте (запускает вьюпорт "
                              "при необходимости)"));
    connect(playButton, &QPushButton::clicked, this,
            &MainWindow::slotAnimPlay);
    playRow->addWidget(playButton);

    auto* stopButton = new QPushButton(tr("Stop"), rightPanel);
    connect(stopButton, &QPushButton::clicked, this,
            &MainWindow::slotAnimStop);
    playRow->addWidget(stopButton);
    playRow->addStretch();
    form->addRow(tr("Проигрывание:"), playRow);

    layout->addWidget(rightPanel, 2);

    return widget;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QWidget* MainWindow::createPointsTab()
{
    auto* widget = new QWidget(this);
    auto* layout = new QVBoxLayout(widget);

    auto* hint = new QLabel(tr("Физические точки ПС (ТЗ п.14). Точки типа "
                               "«Камера (машинист)» экспортируются в секции "
                               "[Cabine] конфигурации (DriverPos/DriverDir), "
                               "остальные хранятся в проекте .trainproject. "
                               "Координаты: X — вправо, Y — вдоль, Z — вверх, м."),
                            widget);
    hint->setWordWrap(true);
    layout->addWidget(hint);

    auto* controls = new QHBoxLayout();

    auto* addButton = new QPushButton(tr("Добавить точку"), widget);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::slotAddPoint);
    controls->addWidget(addButton);

    auto* removeButton = new QPushButton(tr("Удалить точку"), widget);
    connect(removeButton, &QPushButton::clicked, this,
            &MainWindow::slotRemovePoint);
    controls->addWidget(removeButton);

    controls->addStretch();
    layout->addLayout(controls);

    pointsTable = new QTableWidget(widget);
    pointsTable->setColumnCount(6);
    
    pointsTable->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Имя")));
    pointsTable->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Тип")));
    pointsTable->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("X, м")));
    pointsTable->setHorizontalHeaderItem(3, new QTableWidgetItem(tr("Y, м")));
    pointsTable->setHorizontalHeaderItem(4, new QTableWidgetItem(tr("Z, м")));
    pointsTable->setHorizontalHeaderItem(5, new QTableWidgetItem(tr("Направление, град")));

    pointsTable->horizontalHeader()->setSectionResizeMode(
                QHeaderView::ResizeToContents);
    pointsTable->verticalHeader()->setVisible(false);
    connect(pointsTable, &QTableWidget::cellChanged, this,
            &MainWindow::slotPointChanged);
    layout->addWidget(pointsTable, 1);

    // Центр масс (ТЗ п.14): координаты от центра ПЕ, пишутся
    // в секцию [MassCenter] (Height/Longitudinal/Lateral)
    auto* comGroup = new QGroupBox(
                tr("Центр масс (от центра ПЕ, в метрах)"), widget);
    auto* comLayout = new QHBoxLayout(comGroup);

    auto* comForm = new QFormLayout();

    auto make_com_spin = [widget](double min, double max) -> QDoubleSpinBox*
    {
        auto* spin = new QDoubleSpinBox(widget);
        spin->setRange(min, max);
        spin->setDecimals(3);
        spin->setSingleStep(0.05);
        spin->setValue(0.0);
        spin->setMinimumWidth(110);
        return spin;
    };

    comXSpin = make_com_spin(-2.0, 2.0);
    comForm->addRow(tr("X, м (вправо — Lateral):"), comXSpin);

    comYSpin = make_com_spin(-15.0, 15.0);
    comForm->addRow(tr("Y, м (вдоль — Longitudinal):"), comYSpin);

    comZSpin = make_com_spin(0.0, 5.0);
    comForm->addRow(tr("Z, м (вверх — Height):"), comZSpin);

    comLayout->addLayout(comForm);
    comLayout->addStretch();

    auto* comButtons = new QVBoxLayout();
    auto* comSaveButton = new QPushButton(tr("Сохранить ЦМ"), comGroup);
    comSaveButton->setToolTip(
        tr("Записать значения в секцию [MassCenter] конфигурации"));
    connect(comSaveButton, &QPushButton::clicked, this,
            &MainWindow::slotSaveMassCenter);
    comButtons->addWidget(comSaveButton);

    auto* comShowButton = new QPushButton(tr("Показать в 3D"), comGroup);
    comShowButton->setToolTip(
        tr("Жёлтый маркер-сфера в позиции центра масс "
           "в 3D-вьюпорте"));
    connect(comShowButton, &QPushButton::clicked, this,
            &MainWindow::slotShowMassCenter);
    comButtons->addWidget(comShowButton);
    comButtons->addStretch();

    comLayout->addLayout(comButtons);

    // Нижняя прокручиваемая панель: центр масс, кабины и спец-инструменты
    auto* bottomScroll = new QScrollArea(widget);
    bottomScroll->setWidgetResizable(true);

    auto* bottom = new QWidget(bottomScroll);
    auto* bottomLayout = new QVBoxLayout(bottom);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->addWidget(comGroup);

    // --- CAB EDITER (промт п.17): список кабин по Camera-точкам,
    // редактирование DriverPos/DriverDir выбранной кабины
    auto* cabGroup = new QGroupBox(
                tr("Кабины (позиции машиниста, Camera-точки)"), bottom);
    auto* cabForm = new QFormLayout(cabGroup);
    cabForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    cabCombo = new QComboBox(cabGroup);
    cabCombo->setToolTip(tr("Список кабин: по точкам типа «Камера (машинист)» "
                            "из таблицы выше"));
    connect(cabCombo, &QComboBox::currentIndexChanged,
            this, &MainWindow::slotCabSelected);
    cabForm->addRow(tr("Кабина:"), cabCombo);

    auto make_cab_spin = [cabGroup](double min, double max,
                                    const QString& suffix) -> QDoubleSpinBox*
    {
        auto* spin = new QDoubleSpinBox(cabGroup);
        spin->setRange(min, max);
        spin->setDecimals(3);
        spin->setSingleStep(0.05);
        spin->setValue(0.0);
        spin->setSuffix(suffix);
        spin->setMinimumWidth(110);
        return spin;
    };

    cabXSpin = make_cab_spin(-20.0, 20.0, QString());
    cabYSpin = make_cab_spin(-20.0, 20.0, QString());
    cabZSpin = make_cab_spin(-10.0, 10.0, QString());
    cabDirSpin = make_cab_spin(-360.0, 360.0, QStringLiteral("°"));
    cabDirSpin->setDecimals(1);
    cabDirSpin->setSingleStep(5.0);

    auto* cabPosRow = new QHBoxLayout();
    cabPosRow->addWidget(new QLabel(tr("X, м:"), cabGroup));
    cabPosRow->addWidget(cabXSpin);
    cabPosRow->addWidget(new QLabel(tr("Y, м:"), cabGroup));
    cabPosRow->addWidget(cabYSpin);
    cabPosRow->addWidget(new QLabel(tr("Z, м:"), cabGroup));
    cabPosRow->addWidget(cabZSpin);
    cabPosRow->addStretch();
    cabForm->addRow(tr("DriverPos:"), cabPosRow);

    auto* cabDirRow = new QHBoxLayout();
    cabDirRow->addWidget(cabDirSpin);
    cabDirRow->addStretch();
    cabForm->addRow(tr("DriverDir:"), cabDirRow);

    // Изменения полей пишутся в Camera-точку таблицы сразу
    connect(cabXSpin, &QDoubleSpinBox::valueChanged,
            this, &MainWindow::slotCabDataChanged);
    connect(cabYSpin, &QDoubleSpinBox::valueChanged,
            this, &MainWindow::slotCabDataChanged);
    connect(cabZSpin, &QDoubleSpinBox::valueChanged,
            this, &MainWindow::slotCabDataChanged);
    connect(cabDirSpin, &QDoubleSpinBox::valueChanged,
            this, &MainWindow::slotCabDataChanged);

    cabApplyButton = new QPushButton(tr("Применить к конфигу"), cabGroup);
    cabApplyButton->setToolTip(
        tr("Записать все Camera-точки в секции [Cabine] конфигурации "
           "(DriverPos/DriverDir)"));
    connect(cabApplyButton, &QPushButton::clicked,
            this, &MainWindow::slotCabApplyToConfig);
    cabForm->addRow(QString(), cabApplyButton);

    bottomLayout->addWidget(cabGroup);

    // --- Спец-инструменты (промт п.10-12): параметры, записываемые
    // в отдельные секции конфига (атрибуты, как у <Animation>)
    auto* specGroup = new QGroupBox(
                tr("Спец-инструменты (рукав, кран, СА-3, токоприёмник)"),
                bottom);
    auto* specGrid = new QGridLayout(specGroup);

    // Общая строка кнопок «Сохранить»/«Загрузить из конфига» для группы
    auto add_save_load_buttons = [this](QWidget* parent, QFormLayout* form,
                                        void (MainWindow::*save_slot)(),
                                        void (MainWindow::*load_slot)())
    {
        auto* row = new QHBoxLayout();

        auto* save_button = new QPushButton(tr("Сохранить"), parent);
        save_button->setToolTip(tr("Записать параметры группы "
                                   "в конфигурацию"));
        connect(save_button, &QPushButton::clicked, this, save_slot);
        row->addWidget(save_button);

        auto* load_button = new QPushButton(tr("Загрузить из конфига"),
                                            parent);
        connect(load_button, &QPushButton::clicked, this, load_slot);
        row->addWidget(load_button);

        row->addStretch();
        form->addRow(QString(), row);
    };

    // Тормозной рукав: <HoseParam Length Radius Mass/>
    auto* hoseBox = new QGroupBox(tr("Тормозной рукав"), specGroup);
    auto* hoseForm = new QFormLayout(hoseBox);
    hoseForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    hoseLengthSpin = new QDoubleSpinBox(hoseBox);
    hoseLengthSpin->setRange(0.1, 10.0);
    hoseLengthSpin->setDecimals(2);
    hoseLengthSpin->setSingleStep(0.1);
    hoseLengthSpin->setValue(1.5);
    hoseForm->addRow(tr("Длина, м:"), hoseLengthSpin);

    hoseRadiusSpin = new QDoubleSpinBox(hoseBox);
    hoseRadiusSpin->setRange(0.01, 1.0);
    hoseRadiusSpin->setDecimals(3);
    hoseRadiusSpin->setSingleStep(0.01);
    hoseRadiusSpin->setValue(0.12);
    hoseRadiusSpin->setToolTip(tr("Радиус провиса рукава"));
    hoseForm->addRow(tr("Радиус провиса, м:"), hoseRadiusSpin);

    hoseMassSpin = new QDoubleSpinBox(hoseBox);
    hoseMassSpin->setRange(0.1, 50.0);
    hoseMassSpin->setDecimals(2);
    hoseMassSpin->setSingleStep(0.1);
    hoseMassSpin->setValue(2.0);
    hoseForm->addRow(tr("Масса, кг:"), hoseMassSpin);

    add_save_load_buttons(hoseBox, hoseForm,
                          &MainWindow::slotSaveHoseParam,
                          &MainWindow::slotLoadHoseParam);
    specGrid->addWidget(hoseBox, 0, 0);

    // Концевой кран: <AngleCock Type SwitchTime/>
    auto* cockBox = new QGroupBox(tr("Концевой кран"), specGroup);
    auto* cockForm = new QFormLayout(cockBox);
    cockForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    angleCockTypeCombo = new QComboBox(cockBox);
    angleCockTypeCombo->addItem(tr("шаровой"), QStringLiteral("ball"));
    angleCockTypeCombo->addItem(tr("пробковый"), QStringLiteral("plug"));
    cockForm->addRow(tr("Тип:"), angleCockTypeCombo);

    angleCockTimeSpin = new QDoubleSpinBox(cockBox);
    angleCockTimeSpin->setRange(0.1, 30.0);
    angleCockTimeSpin->setDecimals(2);
    angleCockTimeSpin->setSingleStep(0.5);
    angleCockTimeSpin->setValue(2.0);
    cockForm->addRow(tr("Время переключения, с:"), angleCockTimeSpin);

    add_save_load_buttons(cockBox, cockForm,
                          &MainWindow::slotSaveAngleCock,
                          &MainWindow::slotLoadAngleCock);
    specGrid->addWidget(cockBox, 0, 1);

    // Автосцепка СА-3: <CouplingSA3 Device Stiffness MaxForce CouplingSpeed/>
    auto* sa3Box = new QGroupBox(tr("Автосцепка СА-3"), specGroup);
    auto* sa3Form = new QFormLayout(sa3Box);
    sa3Form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    sa3DeviceCombo = new QComboBox(sa3Box);
    sa3DeviceCombo->addItem(tr("ПМ-467"), QStringLiteral("pm467"));
    sa3DeviceCombo->addItem(tr("СА-3"), QStringLiteral("sa3"));
    sa3DeviceCombo->addItem(tr("переходной"), QStringLiteral("adapter"));
    sa3Form->addRow(tr("Тип аппарата:"), sa3DeviceCombo);

    sa3StiffnessSpin = new QDoubleSpinBox(sa3Box);
    sa3StiffnessSpin->setRange(100.0, 1.0e7);
    sa3StiffnessSpin->setDecimals(1);
    sa3StiffnessSpin->setSingleStep(1000.0);
    sa3StiffnessSpin->setValue(60000.0);
    sa3Form->addRow(tr("Жёсткость аппарата, кН/м:"), sa3StiffnessSpin);

    sa3MaxForceSpin = new QDoubleSpinBox(sa3Box);
    sa3MaxForceSpin->setRange(10.0, 10000.0);
    sa3MaxForceSpin->setDecimals(1);
    sa3MaxForceSpin->setSingleStep(50.0);
    sa3MaxForceSpin->setValue(1500.0);
    sa3Form->addRow(tr("Максимальная сила, кН:"), sa3MaxForceSpin);

    sa3SpeedSpin = new QDoubleSpinBox(sa3Box);
    sa3SpeedSpin->setRange(0.05, 5.0);
    sa3SpeedSpin->setDecimals(2);
    sa3SpeedSpin->setSingleStep(0.05);
    sa3SpeedSpin->setValue(1.0);
    sa3Form->addRow(tr("Скорость сцепления, м/с:"), sa3SpeedSpin);

    add_save_load_buttons(sa3Box, sa3Form,
                          &MainWindow::slotSaveCouplingSA3,
                          &MainWindow::slotLoadCouplingSA3);
    specGrid->addWidget(sa3Box, 1, 0);

    // Токоприёмник — ИНСТРУМЕНТ ПАКЕТА (промт п.10-12): секция
    // <PantographTool>; секцию [Pantograph] движка не трогаем
    auto* pantBox = new QGroupBox(
                tr("Токоприёмник (инструмент пакета, НЕ [Pantograph])"),
                specGroup);
    auto* pantForm = new QFormLayout(pantBox);
    pantForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    pantMinSpin = new QDoubleSpinBox(pantBox);
    pantMinSpin->setRange(0.5, 3.0);
    pantMinSpin->setDecimals(2);
    pantMinSpin->setSingleStep(0.05);
    pantMinSpin->setValue(1.1);
    pantForm->addRow(tr("Минимальная высота, м:"), pantMinSpin);

    pantMaxSpin = new QDoubleSpinBox(pantBox);
    pantMaxSpin->setRange(1.0, 6.0);
    pantMaxSpin->setDecimals(2);
    pantMaxSpin->setSingleStep(0.05);
    pantMaxSpin->setValue(2.9);
    pantForm->addRow(tr("Максимальная высота, м:"), pantMaxSpin);

    pantForceSpin = new QDoubleSpinBox(pantBox);
    pantForceSpin->setRange(20.0, 500.0);
    pantForceSpin->setDecimals(1);
    pantForceSpin->setSingleStep(5.0);
    pantForceSpin->setValue(120.0);
    pantForm->addRow(tr("Статический прижим, Н:"), pantForceSpin);

    add_save_load_buttons(pantBox, pantForm,
                          &MainWindow::slotSavePantographTool,
                          &MainWindow::slotLoadPantographTool);
    specGrid->addWidget(pantBox, 1, 1);

    specGrid->setRowStretch(2, 1);
    bottomLayout->addWidget(specGroup);
    bottomLayout->addStretch();

    bottomScroll->setWidget(bottom);
    layout->addWidget(bottomScroll);

    return widget;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QWidget* MainWindow::createExportTab()
{
    auto* widget = new QWidget(this);
    auto* layout = new QVBoxLayout(widget);

    auto* form = new QFormLayout();

    exportNameEdit = new QLineEdit(widget);
    exportNameEdit->setToolTip(tr("Имя ПС и пакета (по умолчанию "
                                  "addons/<имя>/)"));
    form->addRow(tr("Имя подвижного состава:"), exportNameEdit);

    auto* dir_row = new QHBoxLayout();
    exportDirEdit = new QLineEdit(widget);
    dir_row->addWidget(exportDirEdit);

    auto* browseButton = new QPushButton(tr("Обзор..."), widget);
    connect(browseButton, &QPushButton::clicked, this,
            &MainWindow::slotBrowseExportDir);
    dir_row->addWidget(browseButton);

    form->addRow(tr("Папка назначения:"), dir_row);

    copyModelCheck = new QCheckBox(tr("Копировать glTF-модель в пакет"), widget);
    copyModelCheck->setChecked(true);
    form->addRow(QString(), copyModelCheck);

    layout->addLayout(form);

    // Проверка пакета (Asset Validator, промт п.22)
    auto* validateButton = new QPushButton(tr("Проверка пакета..."), widget);
    validateButton->setToolTip(
        tr("Полная проверка: [Vehicle], диапазоны схемы, коллизии, "
           "LOD, кабины, файлы звуков, имена анимаций"));
    connect(validateButton, &QPushButton::clicked,
            this, &MainWindow::slotValidatePackage);
    layout->addWidget(validateButton);

    auto* exportButton = new QPushButton(tr("ПЕРЕНЕСТИ В ИГРУ"), widget);
    exportButton->setMinimumHeight(44);
    exportButton->setToolTip(tr("Собрать пакет ПС: конфигурация XML, "
                                "модель, инструкция (ТЗ п.28-29)"));
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::slotExport);
    layout->addWidget(exportButton);

    layout->addWidget(new QLabel(tr("Журнал экспорта:"), widget));

    exportLog = new QPlainTextEdit(widget);
    exportLog->setReadOnly(true);
    layout->addWidget(exportLog, 1);

    return widget;
}

//------------------------------------------------------------------------------
//
//  Вкладка «Характеристики»: существующие операции над конфигом
//
//------------------------------------------------------------------------------
void MainWindow::slotOpenConfig()
{
    QString open_dir = QDir::homePath();

    const std::string vehicles_dir =
        FileSystem::getInstance().getVehiclesDir();

    if (QDir(QString::fromStdString(vehicles_dir)).exists())
    {
        open_dir = QString::fromStdString(vehicles_dir);
    }

    const QString path = QFileDialog::getOpenFileName(this,
        tr("Открыть конфигурацию ПС"), open_dir,
        tr("XML-конфигурации (*.xml);;Все файлы (*.*)"));

    if (path.isEmpty())
    {
        return;
    }

    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, tr("Ошибка"),
            tr("Не удалось открыть файл:\n%1").arg(path));

        return;
    }

    QDomDocument new_doc;
    QString error_string;
    int error_line = 0;
    int error_column = 0;

    if (!new_doc.setContent(&file, &error_string, &error_line, &error_column))
    {
        file.close();
        QMessageBox::warning(this, tr("Ошибка разбора XML"),
            tr("%1\nСтрока %2, столбец %3")
                .arg(error_string)
                .arg(error_line)
                .arg(error_column));

        return;
    }

    file.close();

    doc = new_doc;
    filePath = path;
    project.configPath = path;
    setModified(false);

    rebuildTree();
    rebuildTractiveTable();
    refreshMassCenterFields();
    refreshAnimationsList();
    rebuildSoundsTable();
    updateStatusBar();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSaveConfig()
{
    if (filePath.isEmpty())
    {
        slotSaveAsConfig();
        return;
    }

    const QStringList errors = validateConfig();

    if (!errors.isEmpty())
    {
        QMessageBox::warning(this, tr("Конфигурация некорректна"),
            tr("Исправьте ошибки перед сохранением:\n\n%1")
                .arg(errors.join('\n')));

        return;
    }

    if (writeConfigFile(filePath))
    {
        project.configPath = filePath;
        setModified(false);
        updateStatusBar();
        statusBar()->showMessage(tr("Сохранено: %1").arg(filePath), 5000);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSaveAsConfig()
{
    QString suggested = filePath;

    if (suggested.isEmpty())
    {
        suggested = QDir(defaultAddonsDir()).absoluteFilePath(
            exportNameEdit->text() + QStringLiteral(".xml"));
    }

    const QString path = QFileDialog::getSaveFileName(this,
        tr("Сохранить конфигурацию ПС"), suggested,
        tr("XML-конфигурации (*.xml);;Все файлы (*.*)"));

    if (path.isEmpty())
    {
        return;
    }

    const QStringList errors = validateConfig();

    if (!errors.isEmpty())
    {
        QMessageBox::warning(this, tr("Конфигурация некорректна"),
            tr("Исправьте ошибки перед сохранением:\n\n%1")
                .arg(errors.join('\n')));

        return;
    }

    if (writeConfigFile(path))
    {
        filePath = path;
        project.configPath = path;
        setModified(false);
        updateStatusBar();
        statusBar()->showMessage(tr("Сохранено: %1").arg(path), 5000);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOpenInSystemEditor()
{
    if (filePath.isEmpty())
    {
        statusBar()->showMessage(
            tr("Сначала откройте конфигурацию"), 5000);

        return;
    }

    if (isModified)
    {
        const QMessageBox::StandardButton answer = QMessageBox::question(this,
            tr("Есть несохранённые изменения"),
            tr("Сохранить изменения перед открытием во внешнем редакторе?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (answer == QMessageBox::Cancel)
        {
            return;
        }

        if (answer == QMessageBox::Save)
        {
            slotSaveConfig();
        }
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSectionSelected(QTreeWidgetItem* item, int column)
{
    (void)column;

    if (!item)
    {
        return;
    }

    const QString section_name =
        item->data(0, RoleSectionName).toString();

    const int instance_index = item->data(0, RoleInstanceIndex).toInt();
    const bool known_schema = item->data(0, RoleKnownSchema).toBool();

    currentSpec = known_schema ? findSectionSpec(section_name) : nullptr;
    currentInstanceIndex = currentSpec && currentSpec->multiple
        ? instance_index : -1;

    if (!currentSpec)
    {
        fieldEditors.clear();
        formTitle->setText(tr("Секция \"%1\" не редактируется этой программой "
                              "(значения сохраняются как есть).")
                               .arg(section_name));
        addInstanceButton->setVisible(false);
        removeInstanceButton->setVisible(false);
        currentSectionElement = QDomElement();

        return;
    }

    if (currentSpec->multiple && instance_index < 0)
    {
        // Выбрана «шапка» списка экземпляров
        fieldEditors.clear();
        formTitle->setText(tr("Секция %1 (список): выберите экземпляр "
                              "в дереве или добавьте новый.")
                               .arg(currentSpec->name));
        addInstanceButton->setText(tr("Добавить %1...").arg(currentSpec->name));
        addInstanceButton->setVisible(true);
        removeInstanceButton->setVisible(false);
        currentSectionElement = QDomElement();

        return;
    }

    currentSectionElement = currentSpec->multiple
        ? sectionElementByIndex(doc, currentSpec->name, instance_index)
        : firstSectionElement(doc, currentSpec->name);

    buildSectionForm(*currentSpec, currentSectionElement);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotAddCabElement()
{
    if (!currentSpec)
    {
        return;
    }

    if (!currentSpec->multiple &&
        !firstSectionElement(doc, currentSpec->name).isNull())
    {
        return;
    }

    const QDomElement element =
        appendSectionElement(doc, currentSpec->name);

    // Заполняем поля значениями по умолчанию
    QDomElement editable = element;

    for (const FieldSpec& field : currentSpec->fields)
    {
        setFieldValue(editable, field, field.default_value);
    }

    setModified(true);

    rebuildTree();

    // Выделяем созданный экземпляр (у одиночных секций индекс -1)
    const int new_index = currentSpec->multiple
        ? countSectionElements(doc, currentSpec->name) - 1 : -1;

    for (QTreeWidgetItemIterator it(sectionsTree); *it; ++it)
    {
        QTreeWidgetItem* item = *it;

        if (item->data(0, RoleSectionName).toString() == currentSpec->name &&
            item->data(0, RoleInstanceIndex).toInt() == new_index)
        {
            sectionsTree->setCurrentItem(item);
            slotSectionSelected(item, 0);
            break;
        }
    }

    updateStatusBar();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotRemoveCurrentSectionInstance()
{
    if (!currentSpec || !currentSpec->multiple || currentInstanceIndex < 0)
    {
        return;
    }

    QDomElement element = sectionElementByIndex(doc, currentSpec->name,
                                                currentInstanceIndex);

    if (element.isNull())
    {
        return;
    }

    element.parentNode().removeChild(element);

    setModified(true);

    rebuildTree();

    currentSectionElement = QDomElement();
    fieldEditors.clear();
    formTitle->setText(tr("Секция %1: выберите экземпляр в дереве.")
                           .arg(currentSpec->name));

    updateStatusBar();
}

//------------------------------------------------------------------------------
//
//  Проект .trainproject (ТЗ п.30-31)
//
//------------------------------------------------------------------------------
void MainWindow::slotNewProject()
{
    QStringList titles;

    for (const TemplateSpec& spec : projectTemplates())
    {
        titles << spec.title;
    }

    bool ok = false;
    const QString chosen = QInputDialog::getItem(this,
        tr("Новый проект"),
        tr("Шаблон подвижного состава (ТЗ п.31):"), titles, 0, false, &ok);

    if (!ok)
    {
        return;
    }

    QString template_key;

    for (const TemplateSpec& spec : projectTemplates())
    {
        if (spec.title == chosen)
        {
            template_key = spec.key;
            break;
        }
    }

    // Предзаполненная конфигурация по шаблону
    QDomDocument new_doc;
    QString error_string;
    int error_line = 0;
    int error_column = 0;

    if (!new_doc.setContent(templateConfigXml(template_key),
                            &error_string, &error_line, &error_column))
    {
        QMessageBox::warning(this, tr("Ошибка"),
            tr("Не удалось создать конфигурацию по шаблону: %1")
                .arg(error_string));

        return;
    }

    // Новый проект
    project = TrainProject();
    project.templateKey = template_key;
    project.points = templatePoints(template_key);
    project.exportSettings.vehicleName = template_key;
    project.exportSettings.targetDir = defaultAddonsDir();
    project.exportSettings.copyModel = true;
    projectFilePath.clear();

    doc = new_doc;
    filePath.clear();
    setModified(false);

    scene.clear();

    applyProjectToUi();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOpenProject()
{
    const QString path = QFileDialog::getOpenFileName(this,
        tr("Открыть проект"),
        QDir::homePath(),
        tr("Проекты rolling-stock-creator (*.trainproject);;Все файлы (*.*)"));

    if (path.isEmpty())
    {
        return;
    }

    QString error;

    if (!project.load(path, &error))
    {
        QMessageBox::warning(this, tr("Ошибка"), error);
        return;
    }

    projectFilePath = path;
    pushRecentProject(path);

    applyProjectToUi();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSaveProject()
{
    if (projectFilePath.isEmpty())
    {
        slotSaveProjectAs();
        return;
    }

    collectProjectFromUi();

    QString error;

    if (!project.save(projectFilePath, &error))
    {
        QMessageBox::warning(this, tr("Ошибка"), error);
        return;
    }

    pushRecentProject(projectFilePath);
    updateWindowTitle();
    statusBar()->showMessage(tr("Проект сохранён: %1").arg(projectFilePath),
                             5000);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSaveProjectAs()
{
    const QString path = QFileDialog::getSaveFileName(this,
        tr("Сохранить проект"),
        QDir::homePath() + QStringLiteral("/vehicle.trainproject"),
        tr("Проекты rolling-stock-creator (*.trainproject);;Все файлы (*.*)"));

    if (path.isEmpty())
    {
        return;
    }

    projectFilePath = path;
    slotSaveProject();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOpenRecentProject()
{
    auto* action = qobject_cast<QAction*>(sender());

    if (action == nullptr)
    {
        return;
    }

    const QString path = action->data().toString();

    if (path.isEmpty() || !QFile::exists(path))
    {
        return;
    }

    QString error;

    if (!project.load(path, &error))
    {
        QMessageBox::warning(this, tr("Ошибка"), error);
        return;
    }

    projectFilePath = path;
    pushRecentProject(path);

    applyProjectToUi();
}

//------------------------------------------------------------------------------
//
//  Вкладка «Модель» (ТЗ п.1-4, 19-20)
//
//------------------------------------------------------------------------------
void MainWindow::slotLoadModel()
{
    QString open_dir = QDir::homePath();

    if (!project.modelPath.isEmpty())
    {
        open_dir = QFileInfo(project.modelPath).absolutePath();
    }

    const QString path = QFileDialog::getOpenFileName(this,
        tr("Загрузить модель ПС (glTF)"), open_dir,
        tr("Модели glTF (*.gltf *.glb);;Все файлы (*.*)"));

    if (path.isEmpty())
    {
        return;
    }

    loadModelFile(path);
}

//------------------------------------------------------------------------------
//
//  Импорт .blend (ТЗ п.1): конвертация внешним Blender в GLB
//  рядом с исходным файлом, затем GLB грузится как обычная модель.
//
//------------------------------------------------------------------------------
void MainWindow::slotImportBlend()
{
    if (blendImporter == nullptr)
    {
        return;
    }

    if (blendImporter->isRunning())
    {
        statusBar()->showMessage(
                    tr("Конвертация .blend уже выполняется..."), 5000);
        return;
    }

    QString open_dir = QDir::homePath();

    if (!project.modelPath.isEmpty())
    {
        open_dir = QFileInfo(project.modelPath).absolutePath();
    }

    const QString path = QFileDialog::getOpenFileName(this,
        tr("Импорт .blend (конвертация Blender в GLB)"), open_dir,
        tr("Файлы Blender (*.blend);;Все файлы (*.*)"));

    if (path.isEmpty())
    {
        return;
    }

    QString blender_path = blenderPathEdit != nullptr
        ? blenderPathEdit->text().trimmed() : QString();

    if (blender_path.isEmpty())
    {
        blender_path = QStringLiteral("blender");
    }

    statusBar()->showMessage(
        tr("Конвертация %1 в GLB (Blender)...").arg(QFileInfo(path).fileName()));

    blendImporter->start(path, blender_path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotBrowseBlender()
{
    if (blenderPathEdit == nullptr)
    {
        return;
    }

    QString filter = tr("Все файлы (*.*)");

#ifdef Q_OS_WIN
    filter = tr("Программы (*.exe);;Все файлы (*.*)");
#endif

    const QString path = QFileDialog::getOpenFileName(this,
        tr("Путь к исполняемому файлу Blender"),
        blenderPathEdit->text(), filter);

    if (path.isEmpty())
    {
        return;
    }

    blenderPathEdit->setText(path);

    QSettings settings;
    settings.setValue(QStringLiteral("blenderPath"), path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotBlendFinished(bool ok, const QString& glb_path,
                                   const QString& error)
{
    if (!ok)
    {
        statusBar()->showMessage(tr("Импорт .blend не удался"), 5000);
        QMessageBox::warning(this, tr("Ошибка импорта .blend"),
            tr("Не удалось сконвертировать .blend:\n\n%1").arg(error));
        return;
    }

    statusBar()->showMessage(
        tr("Конвертация завершена: %1").arg(glb_path), 5000);

    loadModelFile(glb_path);
}

//------------------------------------------------------------------------------
//
//  Drag&Drop моделей (.blend/.glb/.gltf) на окно
//
//------------------------------------------------------------------------------
void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData() == nullptr || !event->mimeData()->hasUrls())
    {
        return;
    }

    for (const QUrl& url : event->mimeData()->urls())
    {
        const QString suffix =
                QFileInfo(url.toLocalFile()).suffix().toLower();

        if (suffix == QStringLiteral("blend") ||
            suffix == QStringLiteral("glb") ||
            suffix == QStringLiteral("gltf"))
        {
            event->acceptProposedAction();
            return;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::dropEvent(QDropEvent* event)
{
    if (event->mimeData() == nullptr || !event->mimeData()->hasUrls())
    {
        return;
    }

    for (const QUrl& url : event->mimeData()->urls())
    {
        const QString path = url.toLocalFile();
        const QString suffix = QFileInfo(path).suffix().toLower();

        if (suffix == QStringLiteral("blend"))
        {
            event->acceptProposedAction();

            if (blendImporter != nullptr && !blendImporter->isRunning())
            {
                QString blender_path = blenderPathEdit != nullptr
                    ? blenderPathEdit->text().trimmed() : QString();

                if (blender_path.isEmpty())
                {
                    blender_path = QStringLiteral("blender");
                }

                statusBar()->showMessage(tr(
                    "Конвертация %1 в GLB (Blender)...")
                    .arg(QFileInfo(path).fileName()));

                blendImporter->start(path, blender_path);
            }

            return;
        }

        if (suffix == QStringLiteral("glb") ||
            suffix == QStringLiteral("gltf"))
        {
            event->acceptProposedAction();
            loadModelFile(path);
            return;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MainWindow::loadModelFile(const QString& path)
{
    const bool was_running = viewer.isRunning();

    if (was_running)
    {
        viewer.stop();
        toggle3DButton->setText(tr("Показать 3D"));
    }

    QString error;

    if (!scene.loadModel(path, &error))
    {
        QMessageBox::warning(this, tr("Ошибка загрузки модели"), error);
        modelStatusLabel->setText(tr("Модель не загружена: %1").arg(error));
        return false;
    }

    project.modelPath = path;

    applySavedNodeProperties();
    rebuildObjectTree();
    refreshCollisionPreview();

    // Варианты LOD рядом с файлом модели (ТЗ п.20)
    const QStringList variants = scene.findLodVariants();

    lodCombo->blockSignals(true);
    lodCombo->clear();
    lodCombo->setEnabled(variants.size() > 1);

    for (int i = 0; i < variants.size(); ++i)
    {
        lodCombo->addItem(QFileInfo(variants.at(i)).fileName(),
                          variants.at(i));
    }

    lodCombo->setCurrentIndex(std::clamp<int>(project.lodIndex, 0,
                                         static_cast<int>(qMax(0, variants.size() - 1))));
    lodCombo->blockSignals(false);

    modelStatusLabel->setText(tr("Модель: %1 (%2 объектов)")
                                  .arg(path)
                                  .arg(static_cast<int>(scene.nodes().size())));

    // Список анимаций glTF (вкладка «Анимации»)
    refreshAnimationsList();

    if (was_running)
    {
        showModelIn3D();
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::applySavedNodeProperties()
{
    for (const ProjectNode& saved : project.nodes)
    {
        SceneNodeInfo* info = scene.nodeByPath(saved.path);

        if (info == nullptr)
        {
            continue;
        }

        info->role = meshRoleFromKey(saved.roleKey);
        info->visible = saved.visible;

        if (!saved.displayName.isEmpty())
        {
            info->displayName = saved.displayName;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::rebuildObjectTree()
{
    // Метки узлов теряются при перестройке — сохраняем их заранее
    const QMap<QString, QString> previous_tags = harvestObjectTags();

    objectTreeUpdating = true;
    objectTree->clear();

    QMap<QString, QTreeWidgetItem*> items_by_path;

    for (const SceneNodeInfo& info : scene.nodes())
    {
        const int separator = info.path.lastIndexOf(QLatin1Char('/'));
        const QString parent_path = (separator >= 0)
            ? info.path.left(separator) : QString();

        QTreeWidgetItem* parent = items_by_path.value(parent_path, nullptr);

        auto* item = new QTreeWidgetItem(QStringList()
            << (info.displayName.isEmpty() ? info.name : info.displayName));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled |
                       Qt::ItemIsUserCheckable);
        item->setCheckState(0, info.visible ? Qt::Checked : Qt::Unchecked);
        item->setData(0, RoleNodePath, info.path);
        item->setToolTip(0, tr("Путь в графе: %1").arg(info.path));

        // Метка узла: с прежних элементов дерева либо из проекта (промт п.32-34)
        QString tag = previous_tags.value(info.path);

        if (tag.isEmpty())
        {
            for (const ProjectNode& saved : project.nodes)
            {
                if (saved.path == info.path)
                {
                    tag = saved.tag;
                    break;
                }
            }
        }

        item->setData(0, RoleNodeTag, tag);
        item->setText(2, tag);

        if (parent != nullptr)
        {
            parent->addChild(item);
        }
        else
        {
            objectTree->addTopLevelItem(item);
        }

        items_by_path.insert(info.path, item);

        // Комбобокс игровой роли (ТЗ п.4)
        auto* role_combo = new QComboBox(objectTree);

        for (const MeshRole role : allMeshRoles())
        {
            role_combo->addItem(meshRoleName(role),
                                static_cast<int>(role));
        }

        const int role_index = static_cast<int>(info.role);

        for (int i = 0; i < role_combo->count(); ++i)
        {
            if (role_combo->itemData(i).toInt() == role_index)
            {
                role_combo->setCurrentIndex(i);
                break;
            }
        }

        const QString node_path = info.path;

        connect(role_combo, &QComboBox::currentIndexChanged, this,
                [this, node_path](int index)
        {
            if (objectTreeUpdating)
            {
                return;
            }

            SceneNodeInfo* node_info = scene.nodeByPath(node_path);

            if (node_info == nullptr || index < 0)
            {
                return;
            }

            auto* combo = qobject_cast<QComboBox*>(sender());

            if (combo == nullptr)
            {
                return;
            }

            node_info->role = static_cast<MeshRole>(
                        combo->itemData(index).toInt());

            // Отражаем роль в тексте подсказки узла
            QList<QTreeWidgetItem*> matches =
                objectTree->findItems(QString(), Qt::MatchContains |
                                                 Qt::MatchRecursive, 0);

            for (QTreeWidgetItem* item : matches)
            {
                if (item->data(0, RoleNodePath).toString() == node_path)
                {
                    item->setToolTip(1, meshRoleName(node_info->role));
                    break;
                }
            }

            refreshCollisionPreview();
        });

        objectTree->setItemWidget(item, 1, role_combo);
    }

    objectTreeUpdating = false;
    objectTree->expandAll();

    // Переустанавливаем фильтр поиска после перестройки дерева
    applyObjectFilter();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotObjectItemChanged(QTreeWidgetItem* item, int column)
{
    if (objectTreeUpdating || item == nullptr || column != 0)
    {
        return;
    }

    const QString path = item->data(0, RoleNodePath).toString();

    SceneNodeInfo* info = scene.nodeByPath(path);

    if (info == nullptr)
    {
        return;
    }

    const bool visible = (item->checkState(0) == Qt::Checked);
    info->visible = visible;

    if (viewer.isRunning())
    {
        viewer.setNodeVisible(info->parent, info->indexInParent,
                              info->node, visible);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotAutoAssignRoles()
{
    if (!scene.isLoaded())
    {
        statusBar()->showMessage(tr("Сначала загрузите модель"), 5000);
        return;
    }

    const int assigned = scene.autoAssignRoles();

    rebuildObjectTree();
    refreshCollisionPreview();

    statusBar()->showMessage(
        tr("Автоматически назначено ролей: %1").arg(assigned), 5000);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotToggle3D()
{
    if (viewer.isRunning())
    {
        viewer.stop();
        toggle3DButton->setText(tr("Показать 3D"));
        return;
    }

    showModelIn3D();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MainWindow::showModelIn3D()
{
    if (!scene.isLoaded())
    {
        statusBar()->showMessage(tr("Сначала загрузите модель"), 5000);
        return false;
    }

    vsg::dvec3 bounds_min;
    vsg::dvec3 bounds_max;
    scene.overallBounds(bounds_min, bounds_max);

    if (!viewer.show(scene.root(), bounds_min, bounds_max))
    {
        QMessageBox::warning(this, tr("Ошибка 3D-вьюпорта"), viewer.error());
        toggle3DButton->setText(tr("Показать 3D"));
        return false;
    }

    toggle3DButton->setText(tr("Остановить 3D"));

    // Применяем текущие настройки отображения
    viewer.setGridVisible(gridCheck->isChecked());
    viewer.setOrthographic(projectionCombo->currentIndex() == 1);
    refreshCollisionPreview();
    refreshPointMarkers();

    // Маркер центра масс, если он был показан (ТЗ п.14)
    if (comMarkerShown)
    {
        refreshComMarker();
    }

    // Дебаг-вид (промт п.38): стрелки осей и бокс габарита 1Т;
    // длина габарита — по bbox модели вдоль пути
    viewer.setAxesMarker(axesCheck != nullptr && axesCheck->isChecked());

    if (gabaritCheck != nullptr)
    {
        vsg::dvec3 bounds_min;
        vsg::dvec3 bounds_max;
        scene.overallBounds(bounds_min, bounds_max);

        viewer.setGabaritMarker(gabaritCheck->isChecked(),
                                std::max(1.0, bounds_max.y - bounds_min.y));
    }

    // Скрываем невидимые узлы (состояние видимости из проекта/дерева)
    for (const SceneNodeInfo& info : scene.nodes())
    {
        if (!info.visible)
        {
            viewer.setNodeVisible(info.parent, info.indexInParent,
                                  info.node, false);
        }
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotLodChanged(int index)
{
    if (index < 0 || lodCombo->itemData(index).isNull())
    {
        return;
    }

    project.lodIndex = index;

    const QString variant = lodCombo->itemData(index).toString();

    if (variant != scene.modelPath())
    {
        loadModelFile(variant);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotGridToggled(bool checked)
{
    if (viewer.isRunning())
    {
        viewer.setGridVisible(checked);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotCollisionToggled(bool checked)
{
    (void)checked;
    refreshCollisionPreview();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotProjectionChanged(int index)
{
    if (viewer.isRunning())
    {
        viewer.setOrthographic(index == 1);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotFocusSelected()
{
    QTreeWidgetItem* item = objectTree->currentItem();

    if (item == nullptr || !scene.isLoaded())
    {
        return;
    }

    const SceneNodeInfo* info =
            scene.nodeByPath(item->data(0, RoleNodePath).toString());

    if (info == nullptr)
    {
        return;
    }

    const vsg::dvec3 center = 0.5 * (info->aabbMin + info->aabbMax);
    const vsg::dvec3 size = info->aabbMax - info->aabbMin;
    const double radius = std::max(0.5 * vsg::length(size), 0.1);

    if (viewer.isRunning())
    {
        viewer.focusOn(center, radius);
    }
    else
    {
        statusBar()->showMessage(
            tr("Камера будет наведена при запуске 3D-вьюпорта"), 5000);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotRenameSelected()
{
    QTreeWidgetItem* item = objectTree->currentItem();

    if (item == nullptr || !scene.isLoaded())
    {
        return;
    }

    const QString path = item->data(0, RoleNodePath).toString();
    SceneNodeInfo* info = scene.nodeByPath(path);

    if (info == nullptr)
    {
        return;
    }

    bool ok = false;
    const QString new_name = QInputDialog::getText(this,
        tr("Переименование"), tr("Display-имя объекта:"),
        QLineEdit::Normal, info->displayName, &ok);

    if (!ok || new_name.isEmpty())
    {
        return;
    }

    info->displayName = new_name;

    objectTreeUpdating = true;
    item->setText(0, new_name);
    objectTreeUpdating = false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::refreshCollisionPreview()
{
    if (!collisionCheck->isChecked() || !viewer.isRunning() ||
        !scene.isLoaded())
    {
        return;
    }

    viewer.setCollisionPreview(scene.collisionFromRoles());
}

//------------------------------------------------------------------------------
//
//  Вкладка «Физические точки» (ТЗ п.14)
//
//------------------------------------------------------------------------------
void MainWindow::rebuildPointsTable()
{
    pointsTableUpdating = true;
    pointsTable->setRowCount(0);

    for (const PhysPoint& point : project.points)
    {
        const int row = pointsTable->rowCount();
        pointsTable->insertRow(row);

        auto* name_item = new QTableWidgetItem(point.name);
        pointsTable->setItem(row, 0, name_item);

        auto* type_combo = new QComboBox(pointsTable);

        for (const PhysPointType type : allPointTypes())
        {
            type_combo->addItem(pointTypeName(type), static_cast<int>(type));
        }

        for (int i = 0; i < type_combo->count(); ++i)
        {
            if (type_combo->itemData(i).toInt() == static_cast<int>(point.type))
            {
                type_combo->setCurrentIndex(i);
                break;
            }
        }

        connect(type_combo, &QComboBox::currentIndexChanged, this,
                [this](int)
        {
            if (!pointsTableUpdating)
            {
                refreshPointMarkers();
            }
        });

        pointsTable->setCellWidget(row, 1, type_combo);

        auto* x_item = new QTableWidgetItem(QString::number(point.x, 'f', 3));
        auto* y_item = new QTableWidgetItem(QString::number(point.y, 'f', 3));
        auto* z_item = new QTableWidgetItem(QString::number(point.z, 'f', 3));
        auto* h_item = new QTableWidgetItem(
                    QString::number(point.heading, 'f', 1));
        pointsTable->setItem(row, 2, x_item);
        pointsTable->setItem(row, 3, y_item);
        pointsTable->setItem(row, 4, z_item);
        pointsTable->setItem(row, 5, h_item);
    }

    pointsTableUpdating = false;

    // Список кабин зависит от Camera-точек (CAB EDITER, промт п.17)
    refreshCabList();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PhysPoint MainWindow::pointFromRow(int row) const
{
    PhysPoint point;

    if (row < 0 || row >= pointsTable->rowCount())
    {
        return point;
    }

    const QTableWidgetItem* name_item = pointsTable->item(row, 0);

    if (name_item != nullptr)
    {
        point.name = name_item->text();
    }

    auto* type_combo = qobject_cast<QComboBox*>(pointsTable->cellWidget(row, 1));

    if (type_combo != nullptr)
    {
        point.type = static_cast<PhysPointType>(
                    type_combo->currentData().toInt());
    }

    auto number_from_cell = [this, row](int column) -> double
    {
        const QTableWidgetItem* cell = pointsTable->item(row, column);

        if (cell == nullptr)
        {
            return 0.0;
        }

        bool ok = false;
        const double value = cell->text().replace(',', '.').toDouble(&ok);

        return ok ? value : 0.0;
    };

    point.x = number_from_cell(2);
    point.y = number_from_cell(3);
    point.z = number_from_cell(4);
    point.heading = number_from_cell(5);

    return point;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<PhysPoint> MainWindow::pointsFromTable() const
{
    std::vector<PhysPoint> points;

    for (int row = 0; row < pointsTable->rowCount(); ++row)
    {
        points.push_back(pointFromRow(row));
    }

    return points;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotAddPoint()
{
    const int row = pointsTable->rowCount();
    pointsTable->insertRow(row);

    PhysPoint point;
    point.name = tr("Точка %1").arg(row + 1);
    point.type = PhysPointType::Coupler;

    pointsTableUpdating = true;

    pointsTable->setItem(row, 0, new QTableWidgetItem(point.name));

    auto* type_combo = new QComboBox(pointsTable);

    for (const PhysPointType type : allPointTypes())
    {
        type_combo->addItem(pointTypeName(type), static_cast<int>(type));
    }

    connect(type_combo, &QComboBox::currentIndexChanged, this, [this](int)
    {
        if (!pointsTableUpdating)
        {
            refreshPointMarkers();
        }
    });

    pointsTable->setCellWidget(row, 1, type_combo);

    pointsTable->setItem(row, 2,
        new QTableWidgetItem(QString::number(point.x, 'f', 3)));
    pointsTable->setItem(row, 3,
        new QTableWidgetItem(QString::number(point.y, 'f', 3)));
    pointsTable->setItem(row, 4,
        new QTableWidgetItem(QString::number(point.z, 'f', 3)));
    pointsTable->setItem(row, 5,
        new QTableWidgetItem(QString::number(point.heading, 'f', 1)));

    pointsTableUpdating = false;

    refreshPointMarkers();

    // Новая точка могла стать Camera-точкой — обновляем список кабин
    refreshCabList();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotRemovePoint()
{
    const int row = pointsTable->currentRow();

    if (row < 0)
    {
        return;
    }

    pointsTable->removeRow(row);

    refreshPointMarkers();

    // Удалённая точка могла быть кабиной
    refreshCabList();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotPointChanged(int row, int column)
{
    if (pointsTableUpdating)
    {
        return;
    }

    // Пустое имя недопустимо — восстанавливаем предыдущее
    if (column == 0)
    {
        QTableWidgetItem* item = pointsTable->item(row, 0);

        if (item != nullptr && item->text().trimmed().isEmpty())
        {
            pointsTableUpdating = true;
            item->setText(tr("Точка %1").arg(row + 1));
            pointsTableUpdating = false;
        }
    }

    refreshPointMarkers();

    // Смена имени/типа точки меняет список кабин
    if (column == 0 || column == 1)
    {
        refreshCabList();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::refreshPointMarkers()
{
    if (!viewer.isRunning())
    {
        return;
    }

    const double radius = scene.isLoaded()
        ? std::max(0.05, scene.boundRadius() * 0.012) : 0.08;

    viewer.setPointMarkers(pointsFromTable(), radius);
}

//------------------------------------------------------------------------------
//
//  Центр масс (ТЗ п.14): чтение [MassCenter] в поля редактора.
//  Соответствие осей: X — Lateral, Y — Longitudinal, Z — Height.
//
//------------------------------------------------------------------------------
void MainWindow::refreshMassCenterFields()
{
    if (comXSpin == nullptr || comYSpin == nullptr || comZSpin == nullptr)
    {
        return;
    }

    const QDomElement section =
            firstSectionElement(doc, QStringLiteral("MassCenter"));

    auto read_value = [&section](const char* key) -> double
    {
        bool found = false;
        const double value = tagText(section, key, found).toDouble();
        return found ? value : 0.0;
    };

    comXSpin->setValue(read_value("Lateral"));
    comYSpin->setValue(read_value("Longitudinal"));
    comZSpin->setValue(read_value("Height"));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::refreshComMarker()
{
    if (!viewer.isRunning() || comXSpin == nullptr ||
        comYSpin == nullptr || comZSpin == nullptr)
    {
        return;
    }

    const double radius = scene.isLoaded()
        ? std::max(0.05, scene.boundRadius() * 0.02) : 0.15;

    viewer.setComMarker(comMarkerShown,
                        vsg::dvec3(comXSpin->value(),
                                   comYSpin->value(),
                                   comZSpin->value()),
                        radius);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSaveMassCenter()
{
    if (comXSpin == nullptr || comYSpin == nullptr || comZSpin == nullptr)
    {
        return;
    }

    QDomElement section =
            firstSectionElement(doc, QStringLiteral("MassCenter"));

    if (section.isNull())
    {
        section = appendSectionElement(doc, QStringLiteral("MassCenter"));
    }

    setSectionChildValue(doc, section, QStringLiteral("Height"),
                         numberToText(comZSpin->value()));
    setSectionChildValue(doc, section, QStringLiteral("Longitudinal"),
                         numberToText(comYSpin->value()));
    setSectionChildValue(doc, section, QStringLiteral("Lateral"),
                         numberToText(comXSpin->value()));

    setModified(true);
    rebuildTree();
    updateStatusBar();

    statusBar()->showMessage(tr("Центр масс записан в [MassCenter]"), 5000);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotShowMassCenter()
{
    if (!viewer.isRunning())
    {
        if (!showModelIn3D())
        {
            return;
        }
    }

    comMarkerShown = true;
    refreshComMarker();

    statusBar()->showMessage(
        tr("Маркер центра масс показан (жёлтая сфера)"), 5000);
}

//------------------------------------------------------------------------------
//
//  Вкладка «Анимации» (ТЗ п.5-6): список из анимаций glTF-модели
//  (scene.animations()) и секций <Animation> конфига; параметры
//  выбранной анимации хранятся атрибутами секции.
//
//------------------------------------------------------------------------------
void MainWindow::refreshAnimationsList()
{
    if (animationsList == nullptr)
    {
        return;
    }

    // Сохраняем выделение
    const QString selected_name = (animationsList->currentItem() != nullptr)
        ? animationsList->currentItem()->text() : QString();

    animEntries.clear();

    // Анимации загруженной glTF-модели
    int index = 1;

    for (const vsg::ref_ptr<vsg::Animation>& animation : scene.animations())
    {
        if (animation == nullptr)
        {
            continue;
        }

        AnimEntry entry;
        entry.animation = animation;

        if (animation->name.empty())
        {
            entry.name = tr("Анимация %1").arg(index);
        }
        else
        {
            entry.name = QString::fromStdString(animation->name);
        }

        animEntries.push_back(entry);
        ++index;
    }

    // Анимации, описанные только в конфиге (модель не загружена
    // или не содержит такой анимации)
    const QDomElement root = doc.documentElement();

    if (!root.isNull())
    {
        for (QDomNode node = root.firstChild(); !node.isNull();
             node = node.nextSibling())
        {
            if (!node.isElement() ||
                node.toElement().tagName() != QStringLiteral("Animation"))
            {
                continue;
            }

            const QString name = node.toElement().attribute(
                        QStringLiteral("Name")).trimmed();

            if (name.isEmpty())
            {
                continue;
            }

            const bool known = [&name, this]()
            {
                for (const AnimEntry& entry : animEntries)
                {
                    if (entry.name == name)
                    {
                        return true;
                    }
                }

                return false;
            }();

            if (!known)
            {
                AnimEntry entry;
                entry.name = name;
                animEntries.push_back(entry);
            }
        }
    }

    // Перестроение списка
    animationsList->blockSignals(true);
    animationsList->clear();

    for (const AnimEntry& entry : animEntries)
    {
        auto* item = new QListWidgetItem(entry.name);
        item->setToolTip(entry.animation != nullptr
            ? tr("Анимация glTF-модели")
            : tr("Описана только в конфиге (нет в модели)"));
        animationsList->addItem(item);
    }

    // Восстановление выделения
    for (int i = 0; i < animationsList->count(); ++i)
    {
        if (animationsList->item(i)->text() == selected_name)
        {
            animationsList->setCurrentRow(i);
            break;
        }
    }

    animationsList->blockSignals(false);

    slotAnimSelected();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QDomElement MainWindow::findAnimationElement(const QString& name) const
{
    const QDomElement root = doc.documentElement();

    if (root.isNull())
    {
        return QDomElement();
    }

    for (QDomNode node = root.firstChild(); !node.isNull();
         node = node.nextSibling())
    {
        if (node.isElement() &&
            node.toElement().tagName() == QStringLiteral("Animation") &&
            node.toElement().attribute(QStringLiteral("Name")) == name)
        {
            return node.toElement();
        }
    }

    return QDomElement();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotAnimAdd()
{
    bool ok = false;
    const QString name = QInputDialog::getText(this,
        tr("Новая анимация"),
        tr("Имя анимации (как в glTF-модели или произвольное):"),
        QLineEdit::Normal, QString(), &ok);

    if (!ok || name.trimmed().isEmpty())
    {
        return;
    }

    // Секция <Animation> со значениями по умолчанию
    if (findAnimationElement(name).isNull())
    {
        QDomElement element =
                appendSectionElement(doc, QStringLiteral("Animation"));
        element.setAttribute(QStringLiteral("Name"), name);
        element.setAttribute(QStringLiteral("Event"),
                             QStringLiteral("door_open"));
        element.setAttribute(QStringLiteral("Speed"),
                             QStringLiteral("1.0"));
        element.setAttribute(QStringLiteral("Loop"),
                             QStringLiteral("0"));
        element.setAttribute(QStringLiteral("Sound"), QString());

        setModified(true);
    }

    refreshAnimationsList();

    // Выделяем добавленную
    for (int i = 0; i < animationsList->count(); ++i)
    {
        if (animationsList->item(i)->text() == name)
        {
            animationsList->setCurrentRow(i);
            break;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotAnimRemove()
{
    const int row = animationsList->currentRow();

    if (row < 0 || row >= static_cast<int>(animEntries.size()))
    {
        return;
    }

    const QString name = animEntries.at(static_cast<size_t>(row)).name;

    QDomElement element = findAnimationElement(name);

    if (!element.isNull())
    {
        element.parentNode().removeChild(element);
        setModified(true);
    }

    refreshAnimationsList();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotAnimSelected()
{
    const int row = animationsList != nullptr
        ? animationsList->currentRow() : -1;

    if (row < 0 || row >= static_cast<int>(animEntries.size()))
    {
        // Ничего не выбрано — поля неактивны
        if (animEventCombo != nullptr)
        {
            animUpdating = true;
            animEventCombo->setCurrentText(QString());
            animSpeedSpin->setValue(1.0);
            animLoopCheck->setChecked(false);
            animSoundEdit->setText(QString());
            animUpdating = false;
        }

        return;
    }

    const QString name = animEntries.at(static_cast<size_t>(row)).name;
    const QDomElement element = findAnimationElement(name);

    animUpdating = true;

    animEventCombo->setCurrentText(
                element.attribute(QStringLiteral("Event"),
                                  QStringLiteral("door_open")));

    animSpeedSpin->setValue(element.attribute(QStringLiteral("Speed"),
                                              QStringLiteral("1.0")).toDouble());

    const QString loop = element.attribute(QStringLiteral("Loop"),
                                           QStringLiteral("0"));
    animLoopCheck->setChecked(loop == QStringLiteral("1") ||
                              loop.compare(QStringLiteral("true"),
                                           Qt::CaseInsensitive) == 0);

    animSoundEdit->setText(element.attribute(QStringLiteral("Sound")));

    animUpdating = false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotAnimDataChanged()
{
    if (animUpdating || animationsList == nullptr)
    {
        return;
    }

    const int row = animationsList->currentRow();

    if (row < 0 || row >= static_cast<int>(animEntries.size()))
    {
        return;
    }

    const QString name = animEntries.at(static_cast<size_t>(row)).name;
    QDomElement element = findAnimationElement(name);

    if (element.isNull())
    {
        element = appendSectionElement(doc, QStringLiteral("Animation"));
        element.setAttribute(QStringLiteral("Name"), name);
    }

    element.setAttribute(QStringLiteral("Event"),
                         animEventCombo->currentText().trimmed());
    element.setAttribute(QStringLiteral("Speed"),
                         numberToText(animSpeedSpin->value()));
    element.setAttribute(QStringLiteral("Loop"),
                         animLoopCheck->isChecked()
                             ? QStringLiteral("1")
                             : QStringLiteral("0"));
    element.setAttribute(QStringLiteral("Sound"), animSoundEdit->text());

    setModified(true);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotAnimPlay()
{
    if (animationsList == nullptr || !scene.isLoaded())
    {
        statusBar()->showMessage(
            tr("Сначала загрузите модель с анимациями"), 5000);
        return;
    }

    const int row = animationsList->currentRow();

    if (row < 0 || row >= static_cast<int>(animEntries.size()))
    {
        statusBar()->showMessage(tr("Выберите анимацию в списке"), 5000);
        return;
    }

    const AnimEntry& entry = animEntries.at(static_cast<size_t>(row));

    if (entry.animation == nullptr)
    {
        statusBar()->showMessage(
            tr("Анимация «%1» есть только в конфиге — "
               "в модели она отсутствует").arg(entry.name), 5000);
        return;
    }

    // Вьюпорт нужен для проигрывания — запускаем при необходимости
    if (!viewer.isRunning())
    {
        if (!showModelIn3D())
        {
            return;
        }
    }

    viewer.stopAllAnimations();
    viewer.playAnimation(entry.animation,
                         animSpeedSpin->value(),
                         animLoopCheck->isChecked());

    statusBar()->showMessage(
        tr("Проигрывание анимации «%1»").arg(entry.name), 5000);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotAnimStop()
{
    if (viewer.isRunning())
    {
        viewer.stopAllAnimations();
        statusBar()->showMessage(tr("Проигрывание остановлено"), 5000);
    }
}

//------------------------------------------------------------------------------
//
//  Конфиг: секции [Collision] и [Cabine] по ролям и точкам
//
//------------------------------------------------------------------------------
bool MainWindow::applyCollisionToConfig()
{
    const SceneModel::CollisionParams params = scene.collisionFromRoles();

    if (!params.valid)
    {
        return false;
    }

    removeSectionElements(doc, QStringLiteral("Collision"));

    QDomElement section = appendSectionElement(
                doc, QStringLiteral("Collision"));

    setSectionChildValue(doc, section, QStringLiteral("Enabled"),
                         boolToText(true));
    setSectionChildValue(doc, section, QStringLiteral("BodyHalfLength"),
                         numberToText(params.bodyHalfLength));
    setSectionChildValue(doc, section, QStringLiteral("BodyHalfWidth"),
                         numberToText(params.bodyHalfWidth));
    setSectionChildValue(doc, section, QStringLiteral("BodyHalfHeight"),
                         numberToText(params.bodyHalfHeight));
    setSectionChildValue(doc, section, QStringLiteral("BodyOffsetZ"),
                         numberToText(params.bodyOffsetZ));

    if (params.numBogies > 0)
    {
        setSectionChildValue(doc, section, QStringLiteral("NumBogies"),
                             QString::number(params.numBogies));
        setSectionChildValue(doc, section, QStringLiteral("BogieHalfLength"),
                             numberToText(params.bogieHalfLength));
        setSectionChildValue(doc, section, QStringLiteral("BogieHalfWidth"),
                             numberToText(params.bogieHalfWidth));
        setSectionChildValue(doc, section, QStringLiteral("BogieHalfHeight"),
                             numberToText(params.bogieHalfHeight));
        setSectionChildValue(doc, section, QStringLiteral("BogieOffset"),
                             numberToText(params.bogieOffset));
        setSectionChildValue(doc, section, QStringLiteral("BogieOffsetZ"),
                             numberToText(params.bogieOffsetZ));
    }

    if (params.numAxis > 0)
    {
        setSectionChildValue(doc, section, QStringLiteral("WheelsetSpacing"),
                             numberToText(params.wheelsetSpacing));
        setSectionChildValue(doc, section, QStringLiteral("WheelsetHalfWidth"),
                             numberToText(params.wheelsetHalfWidth));
        setSectionChildValue(doc, section, QStringLiteral("WheelRadius"),
                             numberToText(params.wheelRadius));
    }

    setModified(true);
    rebuildTree();

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MainWindow::applyCameraPointsToConfig()
{
    const std::vector<PhysPoint> points = pointsFromTable();

    std::vector<PhysPoint> cameras;

    for (const PhysPoint& point : points)
    {
        if (point.type == PhysPointType::Camera)
        {
            cameras.push_back(point);
        }
    }

    if (cameras.empty())
    {
        return false;
    }

    // Формат, который читает Vehicle/VehicleExterior:
    // секции [Cabine] с ключами DriverPos ("x y z") и DriverDir (град)
    removeSectionElements(doc, QStringLiteral("Cabine"));

    for (const PhysPoint& point : cameras)
    {
        QDomElement section = appendSectionElement(
                    doc, QStringLiteral("Cabine"));

        setSectionChildValue(doc, section, QStringLiteral("DriverPos"),
                             QStringLiteral("%1 %2 %3")
                                 .arg(QString::number(point.x, 'f', 3),
                                      QString::number(point.y, 'f', 3),
                                      QString::number(point.z, 'f', 3)));
        setSectionChildValue(doc, section, QStringLiteral("DriverDir"),
                             QString::number(point.heading, 'f', 1));
    }

    setModified(true);
    rebuildTree();

    return true;
}

//------------------------------------------------------------------------------
//
//  Вкладка «Экспорт» (ТЗ п.28-29)
//
//------------------------------------------------------------------------------
void MainWindow::slotBrowseExportDir()
{
    const QString dir = QFileDialog::getExistingDirectory(this,
        tr("Папка назначения пакета"), exportDirEdit->text());

    if (!dir.isEmpty())
    {
        exportDirEdit->setText(dir);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotExport()
{
    // Проверки перед экспортом (ТЗ п.22: критические ошибки — стоп)
    const QStringList errors = validateConfig();

    if (!errors.isEmpty())
    {
        QMessageBox::warning(this, tr("Экспорт невозможен"),
            tr("Конфигурация некорректна:\n\n%1").arg(errors.join('\n')));
        return;
    }

    // Расширенная проверка пакета (Asset Validator, промт п.22):
    // ошибки блокируют экспорт, предупреждения пишутся в журнал
    QStringList package_errors;
    QStringList package_warnings;
    validatePackage(&package_errors, &package_warnings);

    if (!package_errors.isEmpty())
    {
        QMessageBox::warning(this, tr("Экспорт невозможен"),
            tr("Проверка пакета выявила ошибки:\n\n%1")
                .arg(package_errors.join('\n')));
        return;
    }

    collectProjectFromUi();

    const QString vehicle_name = exportNameEdit->text().trimmed();

    if (vehicle_name.isEmpty())
    {
        QMessageBox::warning(this, tr("Экспорт невозможен"),
            tr("Задайте имя подвижного состава на вкладке «Экспорт»"));
        return;
    }

    exportLog->clear();
    exportLog->appendPlainText(tr("=== Сборка пакета %1 ===").arg(vehicle_name));

    for (const QString& warning : package_warnings)
    {
        exportLog->appendPlainText(tr("[ВНИМАНИЕ] %1").arg(warning));
    }

    // 1. Коллизии по ролям (ТЗ п.19)
    if (scene.isLoaded())
    {
        if (applyCollisionToConfig())
        {
            exportLog->appendPlainText(
                tr("[OK] Секция [Collision] сгенерирована по ролям "
                   "Body/Bogie/Wheel"));
        }
        else
        {
            exportLog->appendPlainText(
                tr("[ВНИМАНИЕ] Роли кузова/тележек/колёс не назначены — "
                   "[Collision] не сгенерирована"));
        }
    }

    // 2. Точки камер -> [Cabine] (ТЗ п.14)
    if (applyCameraPointsToConfig())
    {
        exportLog->appendPlainText(
            tr("[OK] Точки камер записаны в секции [Cabine]"));
    }

    // 3. Конфигурация XML
    const QByteArray config_xml = configToByteArray();

    if (config_xml.isEmpty())
    {
        QMessageBox::warning(this, tr("Экспорт невозможен"),
            tr("Не удалось сериализовать конфигурацию"));
        return;
    }

    // 4. Пакет: конфиг + модель + README (ТЗ п.28-29)
    const QString model_path = (copyModelCheck->isChecked() &&
                                scene.isLoaded()) ? scene.modelPath()
                                                  : QString();

    const ExportResult result = ExportPipeline::exportPackage(
                exportDirEdit->text(), vehicle_name,
                QString::fromUtf8(config_xml), model_path,
                scene.collisionFromRoles(), pointsFromTable(),
                soundSearchDir());

    for (const QString& message : result.messages)
    {
        exportLog->appendPlainText(tr("[OK] %1").arg(message));
    }

    for (const QString& warning : result.warnings)
    {
        exportLog->appendPlainText(tr("[ВНИМАНИЕ] %1").arg(warning));
    }

    if (result.success)
    {
        exportLog->appendPlainText(tr("Пакет собран: %1").arg(result.packageDir));

        QMessageBox::information(this, tr("Перенос в игру"),
            tr("Пакет собран:\n%1\n\n"
               "Механизма загрузки data-only ПС в движке нет (нужен "
               "C++-модуль ПС) — инструкция по подключению в README.txt "
               "внутри пакета. «Запустить в игре» не предусмотрено.")
                .arg(result.packageDir));
    }
    else
    {
        QMessageBox::warning(this, tr("Ошибка экспорта"),
            tr("Пакет не собран:\n%1").arg(result.warnings.join('\n')));
    }
}

//------------------------------------------------------------------------------
//
//  Проект: сборка/применение
//
//------------------------------------------------------------------------------
void MainWindow::collectProjectFromUi()
{
    project.modelPath = scene.isLoaded() ? scene.modelPath()
                                         : project.modelPath;
    project.configPath = filePath;

    project.nodes.clear();

    // Метки узлов живут на элементах дерева — собираем их перед записью
    const QMap<QString, QString> tags = harvestObjectTags();

    for (const SceneNodeInfo& info : scene.nodes())
    {
        ProjectNode node;
        node.path = info.path;
        node.roleKey = meshRoleKey(info.role);
        node.displayName = info.displayName;
        node.visible = info.visible;
        node.tag = tags.value(info.path);
        project.nodes.push_back(node);
    }

    project.points = pointsFromTable();

    project.exportSettings.vehicleName = exportNameEdit->text().trimmed();
    project.exportSettings.targetDir = exportDirEdit->text().trimmed();
    project.exportSettings.copyModel = copyModelCheck->isChecked();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::applyProjectToUi()
{
    // Конфигурация ПС
    filePath.clear();

    if (!project.configPath.isEmpty() && QFile::exists(project.configPath))
    {
        QFile file(project.configPath);

        if (file.open(QIODevice::ReadOnly))
        {
            QDomDocument new_doc;

            if (new_doc.setContent(&file))
            {
                doc = new_doc;
                filePath = project.configPath;
            }

            file.close();
        }
    }

    if (filePath.isEmpty())
    {
        doc = QDomDocument();
    }

    setModified(false);
    rebuildTree();

    // Модель
    scene.clear();
    modelStatusLabel->setText(tr("Модель не загружена"));

    if (!project.modelPath.isEmpty() && QFile::exists(project.modelPath))
    {
        loadModelFile(project.modelPath);
    }
    else
    {
        rebuildObjectTree();
        lodCombo->blockSignals(true);
        lodCombo->clear();
        lodCombo->setEnabled(false);
        lodCombo->blockSignals(false);
    }

    // Точки
    rebuildPointsTable();

    // Тяговая характеристика, центр масс и анимации из нового конфига
    rebuildTractiveTable();
    refreshMassCenterFields();
    refreshAnimationsList();
    rebuildSoundsTable();

    // Экспорт
    exportNameEdit->setText(project.exportSettings.vehicleName.isEmpty()
        ? QStringLiteral("new_vehicle")
        : project.exportSettings.vehicleName);
    exportDirEdit->setText(project.exportSettings.targetDir.isEmpty()
        ? defaultAddonsDir() : project.exportSettings.targetDir);
    copyModelCheck->setChecked(project.exportSettings.copyModel);

    if (viewer.isRunning())
    {
        viewer.stop();
        toggle3DButton->setText(tr("Показать 3D"));
    }

    updateWindowTitle();
    updateStatusBar();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::updateWindowTitle()
{
    QString title = tr("RRS Rolling Stock Creator");

    if (!projectFilePath.isEmpty())
    {
        title += QStringLiteral(" — ") + QFileInfo(projectFilePath).fileName();
    }

    // Маркер [*]: Qt подставляет "*" при setWindowModified(true)
    title += QStringLiteral(" [*]");

    setWindowTitle(title);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QStringList MainWindow::recentProjects()
{
    QSettings settings;
    return settings.value(QStringLiteral("recentProjects"))
            .toStringList();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::pushRecentProject(const QString& path)
{
    QStringList recent = recentProjects();

    recent.removeAll(path);
    recent.prepend(path);

    while (recent.size() > 10)
    {
        recent.removeLast();
    }

    QSettings settings;
    settings.setValue(QStringLiteral("recentProjects"), recent);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::updateRecentMenu()
{
    recentMenu->clear();

    const QStringList recent = recentProjects();

    if (recent.isEmpty())
    {
        QAction* empty = recentMenu->addAction(tr("(пусто)"));
        empty->setEnabled(false);
        return;
    }

    for (const QString& path : recent)
    {
        QAction* action = recentMenu->addAction(
                    QFileInfo(path).fileName() + QStringLiteral(" — ") + path);
        action->setData(path);
        connect(action, &QAction::triggered, this,
                &MainWindow::slotOpenRecentProject);
    }
}

//------------------------------------------------------------------------------
//
//  Вкладка «Характеристики»: построение формы секции (существующая логика)
//
//------------------------------------------------------------------------------
void MainWindow::rebuildTree()
{
    sectionsTree->clear();

    for (const SectionSpec& spec : vehicleSchema())
    {
        if (spec.multiple)
        {
            const int count = countSectionElements(doc, spec.name);

            auto* top = new QTreeWidgetItem(QStringList()
                << QString("%1 (%2)").arg(spec.name).arg(count));
            top->setData(0, RoleSectionName, spec.name);
            top->setData(0, RoleInstanceIndex, -1);
            top->setData(0, RoleKnownSchema, true);
            top->setForeground(0, palette().brush(QPalette::Link));

            for (int i = 0; i < count; ++i)
            {
                auto* child = new QTreeWidgetItem(QStringList()
                    << QString("%1 #%2").arg(spec.name).arg(i + 1));
                child->setData(0, RoleSectionName, spec.name);
                child->setData(0, RoleInstanceIndex, i);
                child->setData(0, RoleKnownSchema, true);
                top->addChild(child);
            }

            sectionsTree->addTopLevelItem(top);
        }
        else
        {
            const bool found = !firstSectionElement(doc, spec.name).isNull();

            auto* item = new QTreeWidgetItem(QStringList()
                << (found ? spec.name
                          : QString("%1 — (нет в файле)").arg(spec.name)));
            item->setData(0, RoleSectionName, spec.name);
            item->setData(0, RoleInstanceIndex, -1);
            item->setData(0, RoleKnownSchema, true);

            if (!found)
            {
                item->setForeground(0, palette().brush(QPalette::Disabled,
                                                       QPalette::Text));
            }

            sectionsTree->addTopLevelItem(item);
        }
    }

    // Секции из файла, которых нет в схеме (только просмотр)
    const QDomElement root = doc.documentElement();

    if (!root.isNull())
    {
        for (QDomNode node = root.firstChild(); !node.isNull();
             node = node.nextSibling())
        {
            if (!node.isElement())
            {
                continue;
            }

            const QString name = node.toElement().tagName();

            if (findSectionSpec(name) != nullptr)
            {
                continue;
            }

            auto* item = new QTreeWidgetItem(QStringList()
                << QString("%1 — (вне схемы)").arg(name));
            item->setData(0, RoleSectionName, name);
            item->setData(0, RoleInstanceIndex, -1);
            item->setData(0, RoleKnownSchema, false);
            item->setForeground(0, palette().brush(QPalette::Disabled,
                                                   QPalette::Text));
            sectionsTree->addTopLevelItem(item);
        }
    }

    sectionsTree->expandAll();

    // Таблица нагрузок по осям (промт п.15) читает [Vehicle]
    refreshAxleLoadTable();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::buildSectionForm(const SectionSpec& spec,
                                  QDomElement section)
{
    fieldEditors.clear();

    const bool exists = !section.isNull();

    formTitle->setText(exists
        ? tr("Секция %1").arg(spec.name)
        : tr("Секция %1 отсутствует в файле — значения берутся "
             "по умолчанию, при изменении секция будет создана.")
              .arg(spec.name));

    addInstanceButton->setText(spec.multiple
        ? tr("Добавить %1...").arg(spec.name)
        : tr("Добавить секцию %1 в файл").arg(spec.name));
    addInstanceButton->setVisible(true);

    removeInstanceButton->setVisible(spec.multiple && currentInstanceIndex >= 0);

    auto* formLayout = qobject_cast<QFormLayout*>(formWidget->layout());

    // Убираем старые строки формы
    while (formLayout->count() > 0)
    {
        QLayoutItem* layout_item = formLayout->takeAt(0);
        delete layout_item->widget();
        delete layout_item;
    }

    for (const FieldSpec& field : spec.fields)
    {
        bool found = false;
        const QString value = fieldValue(section, field, found);

        QWidget* editor = nullptr;
        FieldEditor field_editor;

        // Указатель стабилен: схема хранится в статической таблице
        const FieldSpec* field_ptr = &field;
        field_editor.spec = field_ptr;

        switch (field.type)
        {
            case FieldType::Double:
            {
                auto* spin = new QDoubleSpinBox(formWidget);

                // Начальная установка значений не должна считаться правкой
                spin->blockSignals(true);
                spin->setDecimals(4);
                spin->setRange(field.min_value, field.max_value);
                spin->setValue(found ? value.toDouble() : field.default_value.toDouble());
                spin->blockSignals(false);
                spin->setMinimumWidth(180);

                connect(spin, &QDoubleSpinBox::valueChanged, this,
                        [this, field_ptr](double new_value)
                {
                    onFieldEdited(field_ptr,
                        QString::number(new_value, 'g', 12));
                });

                field_editor.double_edit = spin;
                editor = spin;
                break;
            }

            case FieldType::Int:
            {
                auto* spin = new QSpinBox(formWidget);

                // Начальная установка значений не должна считаться правкой
                spin->blockSignals(true);
                spin->setRange(static_cast<int>(field.min_value),
                               static_cast<int>(field.max_value));
                spin->setValue(found ? value.toInt() : field.default_value.toInt());
                spin->blockSignals(false);
                spin->setMinimumWidth(180);

                connect(spin, &QSpinBox::valueChanged, this,
                        [this, field_ptr](int new_value)
                {
                    onFieldEdited(field_ptr, QString::number(new_value));
                });

                field_editor.int_edit = spin;
                editor = spin;
                break;
            }

            case FieldType::String:
            {
                auto* line = new QLineEdit(formWidget);
                line->setText(found ? value : field.default_value);
                line->setMinimumWidth(220);

                connect(line, &QLineEdit::editingFinished, this, [this, line, field_ptr]()
                {
                    onFieldEdited(field_ptr, line->text());
                });

                field_editor.line_edit = line;
                editor = line;
                break;
            }

            case FieldType::Bool:
            {
                auto* button = new QPushButton(formWidget);
                const bool bool_value = found
                    ? (value.compare("true", Qt::CaseInsensitive) == 0)
                    : (field.default_value.compare("true", Qt::CaseInsensitive) == 0);

                // Начальная установка не должна считаться правкой
                button->blockSignals(true);
                button->setText(bool_value ? tr("да") : tr("нет"));
                button->setCheckable(true);
                button->setChecked(bool_value);
                button->blockSignals(false);

                connect(button, &QPushButton::toggled, this,
                        [this, button, field_ptr](bool checked)
                {
                    button->setText(checked ? tr("да") : tr("нет"));
                    onFieldEdited(field_ptr, boolToText(checked));
                });

                field_editor.bool_button = button;
                field_editor.bool_value = bool_value;
                editor = button;
                break;
            }
        }

        fieldEditors.insert(field.key, field_editor);

        const QString label = found
            ? QString("%1 (%2)").arg(field.description, field.key)
            : QString("%1 (%2) — %3").arg(field.description, field.key,
                                          tr("нет"));

        auto* label_widget = new QLabel(label, formWidget);
        label_widget->setToolTip(field.key);

        formLayout->addRow(label_widget, editor);

        // Подсветка выхода значения в файле за пределы min/max
        if (found && (field.type == FieldType::Double ||
                      field.type == FieldType::Int))
        {
            const double number = value.toDouble();
            const bool out_of_range =
                (number < field.min_value - 1e-9) ||
                (number > field.max_value + 1e-9);

            if (out_of_range)
            {
                auto* warning = new QLabel(
                    tr("ВНИМАНИЕ: значение в файле (%1) вне диапазона [%2; %3]")
                        .arg(value)
                        .arg(field.min_value)
                        .arg(field.max_value),
                    formWidget);
                warning->setStyleSheet("color: #d32f2f; font-weight: bold;");

                formLayout->addRow(new QLabel("", formWidget), warning);
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::onFieldEdited(const FieldSpec* field, const QString& value)
{
    if (!field || !currentSpec)
    {
        return;
    }

    QDomElement section = currentSpec->multiple
        ? sectionElementByIndex(doc, currentSpec->name, currentInstanceIndex)
        : firstSectionElement(doc, currentSpec->name);

    if (section.isNull())
    {
        // Секции ещё нет — создаём при первом изменении значения
        section = appendSectionElement(doc, currentSpec->name);
    }

    setFieldValue(section, *field, value);

    setModified(true);
    updateStatusBar();

    // Правка [Vehicle] меняет осевые нагрузки
    if (currentSpec != nullptr &&
        currentSpec->name == QLatin1String("Vehicle"))
    {
        refreshAxleLoadTable();
    }
}

//------------------------------------------------------------------------------
//
//  Тяговая характеристика (ТЗ п.6): точки F(v) хранятся в секции
//  <TractiveCurve> тегами <Point Speed="..." Force="..."/>.
//
//------------------------------------------------------------------------------
void MainWindow::rebuildTractiveTable()
{
    if (tractiveTable == nullptr)
    {
        return;
    }

    tractiveUpdating = true;
    tractiveTable->setRowCount(0);

    const QDomElement section =
            firstSectionElement(doc, QStringLiteral("TractiveCurve"));

    if (!section.isNull())
    {
        for (QDomNode node = section.firstChild(); !node.isNull();
             node = node.nextSibling())
        {
            if (!node.isElement() ||
                node.toElement().tagName() != QStringLiteral("Point"))
            {
                continue;
            }

            const QDomElement point = node.toElement();

            const int row = tractiveTable->rowCount();
            tractiveTable->insertRow(row);
            tractiveTable->setItem(row, 0, new QTableWidgetItem(
                point.attribute(QStringLiteral("Speed"))));
            tractiveTable->setItem(row, 1, new QTableWidgetItem(
                point.attribute(QStringLiteral("Force"))));
        }
    }

    tractiveUpdating = false;

    refreshTractiveChart();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::refreshTractiveChart()
{
    if (tractiveChart == nullptr)
    {
        return;
    }

    std::vector<TractionChart::Point> points;

    for (int row = 0; row < tractiveTable->rowCount(); ++row)
    {
        auto cell_value = [this, row](int column) -> double
        {
            const QTableWidgetItem* cell = tractiveTable->item(row, column);

            if (cell == nullptr)
            {
                return 0.0;
            }

            bool ok = false;
            const double value =
                    cell->text().replace(',', '.').toDouble(&ok);

            return ok ? value : 0.0;
        };

        points.emplace_back(cell_value(0), cell_value(1));
    }

    tractiveChart->setPoints(points);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::applyTractiveToConfig()
{
    // Пересоздаём секцию целиком по содержимому таблицы
    removeSectionElements(doc, QStringLiteral("TractiveCurve"));

    if (tractiveTable->rowCount() == 0)
    {
        setModified(true);
        return;
    }

    QDomElement section =
            appendSectionElement(doc, QStringLiteral("TractiveCurve"));

    for (int row = 0; row < tractiveTable->rowCount(); ++row)
    {
        auto cell_value = [this, row](int column) -> double
        {
            const QTableWidgetItem* cell = tractiveTable->item(row, column);

            if (cell == nullptr)
            {
                return 0.0;
            }

            bool ok = false;
            const double value =
                    cell->text().replace(',', '.').toDouble(&ok);

            return ok ? value : 0.0;
        };

        QDomElement point = doc.createElement(QStringLiteral("Point"));
        point.setAttribute(QStringLiteral("Speed"),
                           numberToText(cell_value(0)));
        point.setAttribute(QStringLiteral("Force"),
                           numberToText(cell_value(1)));
        section.appendChild(point);
    }

    setModified(true);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotTractiveAddRow()
{
    if (tractiveTable == nullptr)
    {
        return;
    }

    const int row = tractiveTable->rowCount();

    // Новая точка — продолжение последней (или 0 км/ч)
    QString speed = QStringLiteral("0");
    QString force = QStringLiteral("0");

    if (row > 0)
    {
        const QTableWidgetItem* last_speed = tractiveTable->item(row - 1, 0);
        const QTableWidgetItem* last_force = tractiveTable->item(row - 1, 1);

        if (last_speed != nullptr)
        {
            speed = last_speed->text();
        }

        if (last_force != nullptr)
        {
            force = last_force->text();
        }
    }

    tractiveUpdating = true;
    tractiveTable->insertRow(row);
    tractiveTable->setItem(row, 0, new QTableWidgetItem(speed));
    tractiveTable->setItem(row, 1, new QTableWidgetItem(force));
    tractiveUpdating = false;

    applyTractiveToConfig();
    refreshTractiveChart();
    tractiveTable->setCurrentCell(row, 0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotTractiveRemoveRow()
{
    if (tractiveTable == nullptr)
    {
        return;
    }

    const int row = tractiveTable->currentRow();

    if (row < 0)
    {
        return;
    }

    tractiveTable->removeRow(row);

    applyTractiveToConfig();
    refreshTractiveChart();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotTractiveCellChanged(int row, int column)
{
    (void)row;
    (void)column;

    if (tractiveUpdating)
    {
        return;
    }

    applyTractiveToConfig();
    refreshTractiveChart();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString MainWindow::fieldValue(const QDomElement& section,
                               const FieldSpec& field,
                               bool& found) const
{
    found = false;

    if (section.isNull())
    {
        return QString();
    }

    for (QDomNode node = section.firstChild(); !node.isNull();
         node = node.nextSibling())
    {
        if (node.isElement() && node.toElement().tagName() == field.key)
        {
            found = true;
            return node.toElement().text();
        }
    }

    return QString();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::setFieldValue(QDomElement& section, const FieldSpec& field,
                               const QString& value)
{
    if (section.isNull())
    {
        return;
    }

    for (QDomNode node = section.firstChild(); !node.isNull();
         node = node.nextSibling())
    {
        if (node.isElement() && node.toElement().tagName() == field.key)
        {
            QDomElement element = node.toElement();

            QDomNode text_node = element.firstChild();

            if (!text_node.isNull() && text_node.isText())
            {
                text_node.toText().setData(value);
            }
            else
            {
                element.appendChild(section.ownerDocument().createTextNode(value));
            }

            return;
        }
    }

    // Тега нет — добавляем новый (CfgEditor::editFile этого не умеет,
    // поэтому файл пишем напрямую через QXmlStreamWriter)
    QDomElement element = section.ownerDocument().createElement(field.key);
    element.appendChild(section.ownerDocument().createTextNode(value));
    section.appendChild(element);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QStringList MainWindow::validateConfig() const
{
    QStringList errors;

    const QDomElement vehicle =
        firstSectionElement(doc, "Vehicle");

    if (vehicle.isNull())
    {
        errors << tr("Отсутствует обязательная секция [Vehicle]");
    }
    else
    {
        const SectionSpec* vehicle_spec = findSectionSpec("Vehicle");

        if (vehicle_spec)
        {
            for (const FieldSpec& field : vehicle_spec->fields)
            {
                if (!field.required)
                {
                    continue;
                }

                bool found = false;
                const QString value = fieldValue(vehicle, field, found);

                if (!found || value.trimmed().isEmpty())
                {
                    errors << tr("[Vehicle]: отсутствует обязательный ключ <%1> (%2)")
                                  .arg(field.key, field.description);
                }
            }
        }
    }

    return errors;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::highlightRanges() const
{
    // Диапазоны уже обеспечиваются QSpinBox/QDoubleSpinBox;
    // значения из файла, нарушающие диапазон, помечаются при построении формы
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MainWindow::writeConfigFile(const QString& file_path)
{
    QFile file(file_path);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate |
                   QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Ошибка"),
            tr("Не удалось открыть файл для записи:\n%1").arg(file_path));

        return false;
    }

    file.write(configToByteArray());
    file.close();

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QByteArray MainWindow::configToByteArray() const
{
    QByteArray output;
    QXmlStreamWriter writer(&output);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(4);

    writer.writeStartDocument();

    const QDomNode root = doc.documentElement();

    if (!root.isNull())
    {
        for (QDomNode node = doc.firstChild(); !node.isNull();
             node = node.nextSibling())
        {
            // Начальный XML-декларации QXmlStreamWriter пишет сам
            if (node.isProcessingInstruction())
            {
                continue;
            }

            writeDomNode(writer, node);
        }
    }
    else
    {
        writer.writeStartElement("Config");
        writer.writeEndElement();
    }

    writer.writeEndDocument();

    return output;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::writeDomNode(QXmlStreamWriter& writer,
                              const QDomNode& node) const
{
    if (node.isElement())
    {
        const QDomElement element = node.toElement();

        writer.writeStartElement(element.tagName());

        const QDomNamedNodeMap attributes = element.attributes();

        for (int i = 0; i < attributes.count(); ++i)
        {
            const QDomNode attribute = attributes.item(i);
            writer.writeAttribute(attribute.nodeName(), attribute.nodeValue());
        }

        for (QDomNode child = node.firstChild(); !child.isNull();
             child = child.nextSibling())
        {
            writeDomNode(writer, child);
        }

        writer.writeEndElement();
    }
    else if (node.isText())
    {
        const QString text = node.toText().data();

        if (!text.trimmed().isEmpty())
        {
            writer.writeCharacters(text.trimmed());
        }
    }
    else if (node.isComment())
    {
        writer.writeComment(node.toComment().data());
    }
    else if (node.isCDATASection())
    {
        writer.writeCDATA(node.toCDATASection().data());
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::updateStatusBar()
{
    QString text = tr("Файл: %1").arg(filePath.isEmpty()
        ? tr("не открыт") : filePath);

    if (isModified)
    {
        text += tr("  |  есть несохранённые изменения");
    }

    statusBar()->showMessage(text);
}

//------------------------------------------------------------------------------
//
//  Вкладка «Звук» (промт п.8-9, Sound Manager): таблица звуков
//  с громкостью/pitch/циклом/3D/вариациями; хранение — секции
//  <Sound Name="..." File="..." Volume="1.0" Pitch="1.0" Loop="0"
//  Is3D="1" Variants="a.ogg;b.ogg"/> (паттерн секций <Animation>).
//
//------------------------------------------------------------------------------
QWidget* MainWindow::createSoundTab()
{
    auto* widget = new QWidget(this);
    auto* layout = new QVBoxLayout(widget);

    auto* hint = new QLabel(tr(
        "Звуки подвижного состава. Каждый звук — секция <Sound> конфигурации. "
        "Вариации — до 3 файлов через «;» (выбираются движком случайно). "
        "Прослушивание в редакторе недоступно — вместо него проверка "
        "существования файлов."), widget);
    hint->setWordWrap(true);
    layout->addWidget(hint);

    // Папка звуков: относительно неё проверяются относительные пути
    auto* dirRow = new QHBoxLayout();
    dirRow->addWidget(new QLabel(tr("Папка звуков:"), widget));

    soundDirEdit = new QLineEdit(widget);
    soundDirEdit->setToolTip(tr("Папка, относительно которой ищутся файлы "
                                "звуков (по умолчанию — папка конфига)"));
    soundDirEdit->setMinimumWidth(240);

    QSettings settings;
    soundDirEdit->setText(
                settings.value(QStringLiteral("soundDir")).toString());

    connect(soundDirEdit, &QLineEdit::editingFinished, this, [this]()
    {
        QSettings save_settings;
        save_settings.setValue(QStringLiteral("soundDir"),
                               soundDirEdit->text().trimmed());
    });

    dirRow->addWidget(soundDirEdit, 1);

    auto* dirBrowse = new QPushButton(tr("Обзор..."), widget);
    connect(dirBrowse, &QPushButton::clicked,
            this, &MainWindow::slotBrowseSoundDir);
    dirRow->addWidget(dirBrowse);
    layout->addLayout(dirRow);

    // Путь к ffmpeg для конвертации в OGG (QSettings, дефолт "ffmpeg")
    auto* ffmpegRow = new QHBoxLayout();
    ffmpegRow->addWidget(new QLabel(tr("ffmpeg:"), widget));

    ffmpegPathEdit = new QLineEdit(widget);
    ffmpegPathEdit->setToolTip(tr("Исполняемый файл ffmpeg "
                                  "(ffmpeg или ffmpeg.exe); "
                                  "поиск в PATH, если задано имя"));
    ffmpegPathEdit->setMinimumWidth(240);
    ffmpegPathEdit->setText(
                settings.value(QStringLiteral("ffmpegPath"),
                               QStringLiteral("ffmpeg")).toString());

    connect(ffmpegPathEdit, &QLineEdit::editingFinished, this, [this]()
    {
        QSettings save_settings;
        save_settings.setValue(QStringLiteral("ffmpegPath"),
                               ffmpegPathEdit->text().trimmed());
    });

    ffmpegRow->addWidget(ffmpegPathEdit, 1);

    auto* ffmpegBrowse = new QPushButton(tr("Обзор..."), widget);
    connect(ffmpegBrowse, &QPushButton::clicked,
            this, &MainWindow::slotBrowseFfmpeg);
    ffmpegRow->addWidget(ffmpegBrowse);
    layout->addLayout(ffmpegRow);

    // Кнопки работы с таблицей
    auto* buttonsRow = new QHBoxLayout();

    auto* addButton = new QPushButton(tr("Добавить звук"), widget);
    connect(addButton, &QPushButton::clicked,
            this, &MainWindow::slotSoundAdd);
    buttonsRow->addWidget(addButton);

    auto* removeButton = new QPushButton(tr("Удалить звук"), widget);
    connect(removeButton, &QPushButton::clicked,
            this, &MainWindow::slotSoundRemove);
    buttonsRow->addWidget(removeButton);

    auto* browseButton = new QPushButton(tr("Файл..."), widget);
    browseButton->setToolTip(tr("Выбрать файл звука для выбранной строки"));
    connect(browseButton, &QPushButton::clicked,
            this, &MainWindow::slotSoundBrowseFile);
    buttonsRow->addWidget(browseButton);

    buttonsRow->addStretch();

    auto* checkButton = new QPushButton(tr("Проверить файлы"), widget);
    checkButton->setToolTip(tr("Проверить существование файлов звуков "
                               "по папке звуков; отсутствующие "
                               "подсвечиваются красным"));
    connect(checkButton, &QPushButton::clicked,
            this, &MainWindow::slotSoundCheckFiles);
    buttonsRow->addWidget(checkButton);

    auto* convertButton = new QPushButton(tr("Конвертировать в OGG"), widget);
    convertButton->setToolTip(tr("ffmpeg -i <файл> -c:a libvorbis <файл>.ogg "
                                 "для выбранной строки"));
    connect(convertButton, &QPushButton::clicked,
            this, &MainWindow::slotSoundConvertToOgg);
    buttonsRow->addWidget(convertButton);

    layout->addLayout(buttonsRow);

    // Таблица звуков
    soundTable = new QTableWidget(widget);
    soundTable->setColumnCount(7);
    
    soundTable->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Имя")));
    soundTable->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Файл")));
    soundTable->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Громкость (0-2)")));
    soundTable->setHorizontalHeaderItem(3, new QTableWidgetItem(tr("Pitch (0.5-2)")));
    soundTable->setHorizontalHeaderItem(4, new QTableWidgetItem(tr("Цикл")));
    soundTable->setHorizontalHeaderItem(5, new QTableWidgetItem(tr("3D")));
    soundTable->setHorizontalHeaderItem(6, new QTableWidgetItem(tr("Вариации (до 3, через «;»)")));

    soundTable->horizontalHeader()->setSectionResizeMode(
                QHeaderView::ResizeToContents);
    soundTable->verticalHeader()->setVisible(false);
    connect(soundTable, &QTableWidget::cellChanged, this,
            &MainWindow::slotSoundCellChanged);
    layout->addWidget(soundTable, 1);

    return widget;
}

namespace
{

/// Нормализовать список вариаций: до 3 непустых файлов через ';'
QString normalizeVariants(const QString& text)
{
    QStringList parts;

    for (const QString& part : text.split(';', Qt::SkipEmptyParts))
    {
        const QString trimmed = part.trimmed();

        if (!trimmed.isEmpty())
        {
            parts << trimmed;
        }

        if (parts.size() >= 3)
        {
            break;
        }
    }

    return parts.join(QLatin1Char(';'));
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::fillSoundRow(int row, const QString& name, const QString& file,
                              double volume, double pitch, bool loop,
                              bool is3d, const QString& variants)
{
    soundTableUpdating = true;

    soundTable->setItem(row, 0, new QTableWidgetItem(name));
    soundTable->setItem(row, 1, new QTableWidgetItem(file));

    auto make_spin = [this, row](int column, double min, double max,
                                 double value) -> QDoubleSpinBox*
    {
        auto* spin = new QDoubleSpinBox(soundTable);
        spin->setRange(min, max);
        spin->setDecimals(2);
        spin->setSingleStep(0.05);
        spin->setValue(value);

        connect(spin, &QDoubleSpinBox::valueChanged, this, [this](double)
        {
            if (!soundTableUpdating)
            {
                applySoundsToConfig();
            }
        });

        soundTable->setCellWidget(row, column, spin);
        return spin;
    };

    make_spin(2, 0.0, 2.0, volume);
    make_spin(3, 0.5, 2.0, pitch);

    auto make_check = [this, row](int column, bool checked) -> QCheckBox*
    {
        auto* check = new QCheckBox(soundTable);
        check->setChecked(checked);

        // Чекбокс по центру ячейки
        auto* container = new QWidget(soundTable);
        auto* container_layout = new QHBoxLayout(container);
        container_layout->setContentsMargins(0, 0, 0, 0);
        container_layout->setAlignment(Qt::AlignCenter);
        container_layout->addWidget(check);

        connect(check, &QCheckBox::toggled, this, [this](bool)
        {
            if (!soundTableUpdating)
            {
                applySoundsToConfig();
            }
        });

        soundTable->setCellWidget(row, column, container);
        return check;
    };

    make_check(4, loop);
    make_check(5, is3d);

    soundTable->setItem(row, 6, new QTableWidgetItem(variants));

    soundTableUpdating = false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::rebuildSoundsTable()
{
    if (soundTable == nullptr)
    {
        return;
    }

    soundTableUpdating = true;
    soundTable->setRowCount(0);

    const QDomElement root = doc.documentElement();

    if (!root.isNull())
    {
        for (QDomNode node = root.firstChild(); !node.isNull();
             node = node.nextSibling())
        {
            if (!node.isElement() ||
                node.toElement().tagName() != QStringLiteral("Sound"))
            {
                continue;
            }

            const QDomElement element = node.toElement();

            const int row = soundTable->rowCount();
            soundTable->insertRow(row);

            soundTableUpdating = false;
            fillSoundRow(row,
                         element.attribute(QStringLiteral("Name")),
                         element.attribute(QStringLiteral("File")),
                         element.attribute(QStringLiteral("Volume"),
                                           QStringLiteral("1.0")).toDouble(),
                         element.attribute(QStringLiteral("Pitch"),
                                           QStringLiteral("1.0")).toDouble(),
                         element.attribute(QStringLiteral("Loop")) ==
                             QStringLiteral("1"),
                         element.attribute(QStringLiteral("Is3D"),
                                           QStringLiteral("1")) !=
                             QStringLiteral("0"),
                         element.attribute(QStringLiteral("Variants")));
            soundTableUpdating = true;
        }
    }

    soundTableUpdating = false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::applySoundsToConfig()
{
    if (soundTable == nullptr)
    {
        return;
    }

    removeSectionElements(doc, QStringLiteral("Sound"));

    auto cell_spin = [this](int row, int column) -> QDoubleSpinBox*
    {
        return qobject_cast<QDoubleSpinBox*>(
                    soundTable->cellWidget(row, column));
    };

    auto cell_checkbox = [this](int row, int column) -> QCheckBox*
    {
        QWidget* container = soundTable->cellWidget(row, column);
        return container != nullptr
            ? container->findChild<QCheckBox*>() : nullptr;
    };

    for (int row = 0; row < soundTable->rowCount(); ++row)
    {
        const QTableWidgetItem* name_item = soundTable->item(row, 0);
        const QTableWidgetItem* file_item = soundTable->item(row, 1);
        const QTableWidgetItem* variants_item = soundTable->item(row, 6);

        QDoubleSpinBox* volume_spin = cell_spin(row, 2);
        QDoubleSpinBox* pitch_spin = cell_spin(row, 3);
        QCheckBox* loop_check = cell_checkbox(row, 4);
        QCheckBox* is3d_check = cell_checkbox(row, 5);

        QDomElement element =
                appendSectionElement(doc, QStringLiteral("Sound"));
        element.setAttribute(QStringLiteral("Name"),
                             name_item != nullptr ? name_item->text()
                                                  : QString());
        element.setAttribute(QStringLiteral("File"),
                             file_item != nullptr ? file_item->text()
                                                  : QString());
        element.setAttribute(QStringLiteral("Volume"),
                             numberToText(volume_spin != nullptr
                                              ? volume_spin->value() : 1.0));
        element.setAttribute(QStringLiteral("Pitch"),
                             numberToText(pitch_spin != nullptr
                                              ? pitch_spin->value() : 1.0));
        element.setAttribute(QStringLiteral("Loop"),
                             loop_check != nullptr && loop_check->isChecked()
                                 ? QStringLiteral("1")
                                 : QStringLiteral("0"));
        element.setAttribute(QStringLiteral("Is3D"),
                             is3d_check != nullptr && is3d_check->isChecked()
                                 ? QStringLiteral("1")
                                 : QStringLiteral("0"));
        element.setAttribute(QStringLiteral("Variants"),
                             normalizeVariants(variants_item != nullptr
                                 ? variants_item->text() : QString()));
    }

    setModified(true);
    updateStatusBar();
}

//------------------------------------------------------------------------------
//
//  Папка поиска звуков: поле вкладки либо папка открытого конфига
//
//------------------------------------------------------------------------------
QString MainWindow::soundSearchDir() const
{
    if (soundDirEdit != nullptr && !soundDirEdit->text().trimmed().isEmpty())
    {
        return soundDirEdit->text().trimmed();
    }

    if (!filePath.isEmpty())
    {
        return QFileInfo(filePath).absolutePath();
    }

    return QDir::homePath();
}

//------------------------------------------------------------------------------
//
//  Существует ли звуковой файл: абсолютный путь либо относительный
//  к папке звуков / папке конфига / SoundDir из [Vehicle]
//
//------------------------------------------------------------------------------
bool MainWindow::soundFileExists(const QString& file_path,
                                 QString* found_path) const
{
    const QString trimmed = file_path.trimmed();

    if (trimmed.isEmpty())
    {
        return false;
    }

    if (found_path != nullptr)
    {
        found_path->clear();
    }

    const QFileInfo info(trimmed);

    if (info.isAbsolute())
    {
        if (info.exists())
        {
            if (found_path != nullptr)
            {
                *found_path = info.absoluteFilePath();
            }

            return true;
        }

        return false;
    }

    QStringList candidates;
    candidates << QDir(soundSearchDir()).filePath(trimmed);

    if (!filePath.isEmpty())
    {
        const QString config_dir = QFileInfo(filePath).absolutePath();
        candidates << QDir(config_dir).filePath(trimmed);

        const QDomElement vehicle = firstSectionElement(doc, "Vehicle");

        if (!vehicle.isNull())
        {
            bool found = false;
            const QString sound_dir =
                    tagText(vehicle, "SoundDir", found).trimmed();

            if (found && !sound_dir.isEmpty())
            {
                candidates << QDir(config_dir).filePath(
                                QDir(sound_dir).filePath(trimmed));
            }
        }
    }

    for (const QString& candidate : candidates)
    {
        if (QFile::exists(candidate))
        {
            if (found_path != nullptr)
            {
                *found_path = candidate;
            }

            return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSoundAdd()
{
    if (soundTable == nullptr)
    {
        return;
    }

    const int row = soundTable->rowCount();
    soundTable->insertRow(row);
    fillSoundRow(row, tr("Новый звук"), QString(), 1.0, 1.0, false, true,
                 QString());

    applySoundsToConfig();
    soundTable->setCurrentCell(row, 0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSoundRemove()
{
    if (soundTable == nullptr)
    {
        return;
    }

    const int row = soundTable->currentRow();

    if (row < 0)
    {
        return;
    }

    soundTable->removeRow(row);

    applySoundsToConfig();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSoundBrowseFile()
{
    if (soundTable == nullptr)
    {
        return;
    }

    const int row = soundTable->currentRow();

    if (row < 0)
    {
        statusBar()->showMessage(tr("Выберите строку звука в таблице"), 5000);
        return;
    }

    const QString path = QFileDialog::getOpenFileName(this,
        tr("Файл звука"), soundSearchDir(),
        tr("Звуковые файлы (*.ogg *.wav *.mp3 *.flac);;Все файлы (*.*)"));

    if (path.isEmpty())
    {
        return;
    }

    // Относительный путь — по возможности от папки звуков
    const QString base_dir = soundSearchDir();
    QString stored = path;

    if (path.startsWith(base_dir, Qt::CaseInsensitive))
    {
        stored = QDir(base_dir).relativeFilePath(path);
    }

    QTableWidgetItem* file_item = soundTable->item(row, 1);

    if (file_item != nullptr)
    {
        file_item->setText(stored);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSoundCellChanged(int row, int column)
{
    (void)row;

    // Вариации приводим к формату «до 3 через ;»
    if (column == 6)
    {
        QTableWidgetItem* variants_item = soundTable->item(row, 6);

        if (variants_item != nullptr)
        {
            const QString normalized = normalizeVariants(variants_item->text());

            if (normalized != variants_item->text())
            {
                soundTableUpdating = true;
                variants_item->setText(normalized);
                soundTableUpdating = false;
            }
        }
    }

    if (soundTableUpdating)
    {
        return;
    }

    applySoundsToConfig();
}

//------------------------------------------------------------------------------
//
//  «Проверить файлы»: QFile::exists по папке звуков, подсветка
//  отсутствующих красным (промт п.8-9)
//
//------------------------------------------------------------------------------
void MainWindow::slotSoundCheckFiles()
{
    if (soundTable == nullptr)
    {
        return;
    }

    int total_files = 0;
    int missing_files = 0;

    auto mark_item = [this](QTableWidgetItem* item, bool missing,
                            const QString& tooltip)
    {
        if (item == nullptr)
        {
            return;
        }

        item->setData(Qt::ForegroundRole, missing
                          ? QVariant(QBrush(QColor(0xd3, 0x2f, 0x2f)))
                          : QVariant());
        item->setToolTip(tooltip);
    };

    for (int row = 0; row < soundTable->rowCount(); ++row)
    {
        QTableWidgetItem* file_item = soundTable->item(row, 1);
        QTableWidgetItem* variants_item = soundTable->item(row, 6);

        if (file_item != nullptr && !file_item->text().trimmed().isEmpty())
        {
            ++total_files;

            QString resolved;
            const bool exists = soundFileExists(file_item->text(), &resolved);

            if (!exists)
            {
                ++missing_files;
            }

            mark_item(file_item, !exists,
                      exists ? tr("Найден: %1").arg(resolved)
                             : tr("Файл не найден"));
        }

        if (variants_item != nullptr)
        {
            bool any_missing = false;
            QStringList resolved_list;

            for (const QString& variant :
                 normalizeVariants(variants_item->text()).split(';',
                                                                Qt::SkipEmptyParts))
            {
                QString resolved;
                const bool exists = soundFileExists(variant, &resolved);
                any_missing = any_missing || !exists;
                resolved_list << (exists
                    ? tr("%1 — найден").arg(variant)
                    : tr("%1 — НЕ найден").arg(variant));
            }

            mark_item(variants_item, any_missing,
                      resolved_list.join(QLatin1Char('\n')));
        }
    }

    if (missing_files > 0)
    {
        statusBar()->showMessage(
                    tr("Проверка звуков: отсутствуют файлы: %1 из %2")
                        .arg(missing_files).arg(total_files), 8000);
    }
    else
    {
        statusBar()->showMessage(
                    tr("Проверка звуков: все файлы найдены (%1)")
                        .arg(total_files), 8000);
    }
}

//------------------------------------------------------------------------------
//
//  «Конвертировать в OGG»: ffmpeg -i <in> -c:a libvorbis <out.ogg>
//  для файла выбранной строки; ошибки — в строку состояния
//
//------------------------------------------------------------------------------
void MainWindow::slotSoundConvertToOgg()
{
    if (soundTable == nullptr || ffmpegProcess == nullptr ||
        ffmpegPathEdit == nullptr)
    {
        return;
    }

    if (ffmpegProcess->state() != QProcess::NotRunning)
    {
        statusBar()->showMessage(
                    tr("Конвертация уже выполняется..."), 5000);
        return;
    }

    const int row = soundTable->currentRow();

    if (row < 0)
    {
        statusBar()->showMessage(tr("Выберите строку звука в таблице"), 5000);
        return;
    }

    const QTableWidgetItem* file_item = soundTable->item(row, 1);

    if (file_item == nullptr || file_item->text().trimmed().isEmpty())
    {
        statusBar()->showMessage(
                    tr("У выбранного звука не задан файл"), 5000);
        return;
    }

    QString source;

    if (!soundFileExists(file_item->text(), &source))
    {
        statusBar()->showMessage(
                    tr("Исходный файл не найден: %1")
                        .arg(file_item->text()), 8000);
        return;
    }

    if (QFileInfo(source).suffix().compare(QStringLiteral("ogg"),
                                           Qt::CaseInsensitive) == 0)
    {
        statusBar()->showMessage(
                    tr("Файл уже в формате OGG: %1").arg(source), 5000);
        return;
    }

    QString ffmpeg_path = ffmpegPathEdit->text().trimmed();

    if (ffmpeg_path.isEmpty())
    {
        ffmpeg_path = QStringLiteral("ffmpeg");
    }

    const QFileInfo source_info(source);
    const QString target = QDir(source_info.absolutePath())
            .filePath(source_info.completeBaseName() +
                      QStringLiteral(".ogg"));

    ffmpegProcess->setProgram(ffmpeg_path);
    ffmpegProcess->setArguments({QStringLiteral("-i"), source,
                                 QStringLiteral("-c:a"),
                                 QStringLiteral("libvorbis"), target});
    ffmpegProcess->start();

    statusBar()->showMessage(
                tr("Конвертация %1 в OGG (ffmpeg)...")
                    .arg(source_info.fileName()));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotBrowseFfmpeg()
{
    if (ffmpegPathEdit == nullptr)
    {
        return;
    }

    QString filter = tr("Все файлы (*.*)");

#ifdef Q_OS_WIN
    filter = tr("Программы (*.exe);;Все файлы (*.*)");
#endif

    const QString path = QFileDialog::getOpenFileName(this,
        tr("Путь к исполняемому файлу ffmpeg"),
        ffmpegPathEdit->text(), filter);

    if (path.isEmpty())
    {
        return;
    }

    ffmpegPathEdit->setText(path);

    QSettings settings;
    settings.setValue(QStringLiteral("ffmpegPath"), path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotBrowseSoundDir()
{
    if (soundDirEdit == nullptr)
    {
        return;
    }

    const QString dir = QFileDialog::getExistingDirectory(this,
        tr("Папка звуков"), soundSearchDir());

    if (dir.isEmpty())
    {
        return;
    }

    soundDirEdit->setText(dir);

    QSettings settings;
    settings.setValue(QStringLiteral("soundDir"), dir);
}

//------------------------------------------------------------------------------
//
//  Спец-инструменты (промт п.10-12): запись/чтение секций с атрибутами
//
//------------------------------------------------------------------------------
void MainWindow::slotSaveHoseParam()
{
    if (hoseLengthSpin == nullptr || hoseRadiusSpin == nullptr ||
        hoseMassSpin == nullptr)
    {
        return;
    }

    removeSectionElements(doc, QStringLiteral("HoseParam"));

    QDomElement element =
            appendSectionElement(doc, QStringLiteral("HoseParam"));
    element.setAttribute(QStringLiteral("Length"),
                         numberToText(hoseLengthSpin->value()));
    element.setAttribute(QStringLiteral("Radius"),
                         numberToText(hoseRadiusSpin->value()));
    element.setAttribute(QStringLiteral("Mass"),
                         numberToText(hoseMassSpin->value()));

    setModified(true);
    updateStatusBar();
    statusBar()->showMessage(
                tr("Тормозной рукав записан в <HoseParam>"), 5000);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotLoadHoseParam()
{
    if (hoseLengthSpin == nullptr || hoseRadiusSpin == nullptr ||
        hoseMassSpin == nullptr)
    {
        return;
    }

    const QDomElement section =
            firstSectionElement(doc, QStringLiteral("HoseParam"));

    if (section.isNull())
    {
        statusBar()->showMessage(
                    tr("Секция <HoseParam> в конфиге не найдена — "
                       "значения по умолчанию"), 5000);
    }

    hoseLengthSpin->setValue(section.attribute(
        QStringLiteral("Length"), QStringLiteral("1.5")).toDouble());
    hoseRadiusSpin->setValue(section.attribute(
        QStringLiteral("Radius"), QStringLiteral("0.12")).toDouble());
    hoseMassSpin->setValue(section.attribute(
        QStringLiteral("Mass"), QStringLiteral("2.0")).toDouble());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSaveAngleCock()
{
    if (angleCockTypeCombo == nullptr || angleCockTimeSpin == nullptr)
    {
        return;
    }

    removeSectionElements(doc, QStringLiteral("AngleCock"));

    QDomElement element =
            appendSectionElement(doc, QStringLiteral("AngleCock"));
    element.setAttribute(
                QStringLiteral("Type"),
                angleCockTypeCombo->currentData().toString());
    element.setAttribute(QStringLiteral("SwitchTime"),
                         numberToText(angleCockTimeSpin->value()));

    setModified(true);
    updateStatusBar();
    statusBar()->showMessage(
                tr("Концевой кран записан в <AngleCock>"), 5000);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotLoadAngleCock()
{
    if (angleCockTypeCombo == nullptr || angleCockTimeSpin == nullptr)
    {
        return;
    }

    const QDomElement section =
            firstSectionElement(doc, QStringLiteral("AngleCock"));

    if (section.isNull())
    {
        statusBar()->showMessage(
                    tr("Секция <AngleCock> в конфиге не найдена — "
                       "значения по умолчанию"), 5000);
    }

    const QString type = section.attribute(QStringLiteral("Type"),
                                           QStringLiteral("ball"));

    for (int i = 0; i < angleCockTypeCombo->count(); ++i)
    {
        if (angleCockTypeCombo->itemData(i).toString() == type)
        {
            angleCockTypeCombo->setCurrentIndex(i);
            break;
        }
    }

    angleCockTimeSpin->setValue(section.attribute(
        QStringLiteral("SwitchTime"), QStringLiteral("2.0")).toDouble());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSaveCouplingSA3()
{
    if (sa3DeviceCombo == nullptr || sa3StiffnessSpin == nullptr ||
        sa3MaxForceSpin == nullptr || sa3SpeedSpin == nullptr)
    {
        return;
    }

    removeSectionElements(doc, QStringLiteral("CouplingSA3"));

    QDomElement element =
            appendSectionElement(doc, QStringLiteral("CouplingSA3"));
    element.setAttribute(
                QStringLiteral("Device"),
                sa3DeviceCombo->currentData().toString());
    element.setAttribute(QStringLiteral("Stiffness"),
                         numberToText(sa3StiffnessSpin->value()));
    element.setAttribute(QStringLiteral("MaxForce"),
                         numberToText(sa3MaxForceSpin->value()));
    element.setAttribute(QStringLiteral("CouplingSpeed"),
                         numberToText(sa3SpeedSpin->value()));

    setModified(true);
    updateStatusBar();
    statusBar()->showMessage(
                tr("Автосцепка СА-3 записана в <CouplingSA3>"), 5000);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotLoadCouplingSA3()
{
    if (sa3DeviceCombo == nullptr || sa3StiffnessSpin == nullptr ||
        sa3MaxForceSpin == nullptr || sa3SpeedSpin == nullptr)
    {
        return;
    }

    const QDomElement section =
            firstSectionElement(doc, QStringLiteral("CouplingSA3"));

    if (section.isNull())
    {
        statusBar()->showMessage(
                    tr("Секция <CouplingSA3> в конфиге не найдена — "
                       "значения по умолчанию"), 5000);
    }

    const QString device = section.attribute(QStringLiteral("Device"),
                                             QStringLiteral("sa3"));

    for (int i = 0; i < sa3DeviceCombo->count(); ++i)
    {
        if (sa3DeviceCombo->itemData(i).toString() == device)
        {
            sa3DeviceCombo->setCurrentIndex(i);
            break;
        }
    }

    sa3StiffnessSpin->setValue(section.attribute(
        QStringLiteral("Stiffness"), QStringLiteral("60000.0")).toDouble());
    sa3MaxForceSpin->setValue(section.attribute(
        QStringLiteral("MaxForce"), QStringLiteral("1500.0")).toDouble());
    sa3SpeedSpin->setValue(section.attribute(
        QStringLiteral("CouplingSpeed"), QStringLiteral("1.0")).toDouble());
}

//------------------------------------------------------------------------------
//
//  Токоприёмник — инструмент пакета: секция <PantographTool>;
//  секцию [Pantograph] движка не трогаем (промт п.10-12)
//
//------------------------------------------------------------------------------
void MainWindow::slotSavePantographTool()
{
    if (pantMinSpin == nullptr || pantMaxSpin == nullptr ||
        pantForceSpin == nullptr)
    {
        return;
    }

    removeSectionElements(doc, QStringLiteral("PantographTool"));

    QDomElement element =
            appendSectionElement(doc, QStringLiteral("PantographTool"));
    element.setAttribute(QStringLiteral("MinHeight"),
                         numberToText(pantMinSpin->value()));
    element.setAttribute(QStringLiteral("MaxHeight"),
                         numberToText(pantMaxSpin->value()));
    element.setAttribute(QStringLiteral("StaticForce"),
                         numberToText(pantForceSpin->value()));

    setModified(true);
    updateStatusBar();
    statusBar()->showMessage(
                tr("Токоприёмник записан в <PantographTool>"), 5000);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotLoadPantographTool()
{
    if (pantMinSpin == nullptr || pantMaxSpin == nullptr ||
        pantForceSpin == nullptr)
    {
        return;
    }

    const QDomElement section =
            firstSectionElement(doc, QStringLiteral("PantographTool"));

    if (section.isNull())
    {
        statusBar()->showMessage(
                    tr("Секция <PantographTool> в конфиге не найдена — "
                       "значения по умолчанию"), 5000);
    }

    pantMinSpin->setValue(section.attribute(
        QStringLiteral("MinHeight"), QStringLiteral("1.1")).toDouble());
    pantMaxSpin->setValue(section.attribute(
        QStringLiteral("MaxHeight"), QStringLiteral("2.9")).toDouble());
    pantForceSpin->setValue(section.attribute(
        QStringLiteral("StaticForce"), QStringLiteral("120.0")).toDouble());
}

//------------------------------------------------------------------------------
//
//  CAB EDITER (промт п.17): список кабин по Camera-точкам таблицы
//
//------------------------------------------------------------------------------
void MainWindow::refreshCabList()
{
    if (cabCombo == nullptr || pointsTable == nullptr)
    {
        return;
    }

    cabUpdating = true;

    // Запоминаем выбранную кабину (по строке таблицы)
    const int keep_row = cabCombo->currentIndex() >= 0
        ? cabCombo->currentData().toInt() : -1;

    cabCombo->clear();

    const std::vector<PhysPoint> points = pointsFromTable();
    int cab_index = 0;

    for (int row = 0; row < static_cast<int>(points.size()); ++row)
    {
        if (points.at(static_cast<size_t>(row)).type !=
                PhysPointType::Camera)
        {
            continue;
        }

        ++cab_index;
        cabCombo->addItem(
                    tr("Кабина %1 (%2)")
                        .arg(cab_index)
                        .arg(points.at(static_cast<size_t>(row)).name),
                    row);
    }

    const bool has_cabs = cabCombo->count() > 0;
    cabCombo->setEnabled(has_cabs);

    if (cabXSpin != nullptr)
    {
        cabXSpin->setEnabled(has_cabs);
        cabYSpin->setEnabled(has_cabs);
        cabZSpin->setEnabled(has_cabs);
        cabDirSpin->setEnabled(has_cabs);
    }

    if (cabApplyButton != nullptr)
    {
        cabApplyButton->setEnabled(has_cabs);
    }

    // Восстанавливаем выбор
    for (int i = 0; i < cabCombo->count(); ++i)
    {
        if (cabCombo->itemData(i).toInt() == keep_row)
        {
            cabCombo->setCurrentIndex(i);
            break;
        }
    }

    cabUpdating = false;

    // Заполняем поля выбранной кабины
    slotCabSelected();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotCabSelected()
{
    if (cabUpdating || cabCombo == nullptr || cabXSpin == nullptr)
    {
        return;
    }

    const int row = cabCombo->currentIndex() >= 0
        ? cabCombo->currentData().toInt() : -1;

    if (row < 0 || row >= pointsTable->rowCount())
    {
        return;
    }

    const PhysPoint point = pointFromRow(row);

    cabUpdating = true;
    cabXSpin->setValue(point.x);
    cabYSpin->setValue(point.y);
    cabZSpin->setValue(point.z);
    cabDirSpin->setValue(point.heading);
    cabUpdating = false;
}

//------------------------------------------------------------------------------
//
//  Изменение полей кабины пишется в Camera-точку выбранной строки
//  таблицы физических точек (Camera-точка и есть редактируемая кабина)
//
//------------------------------------------------------------------------------
void MainWindow::slotCabDataChanged()
{
    if (cabUpdating || cabCombo == nullptr || pointsTable == nullptr)
    {
        return;
    }

    const int row = cabCombo->currentIndex() >= 0
        ? cabCombo->currentData().toInt() : -1;

    if (row < 0 || row >= pointsTable->rowCount())
    {
        return;
    }

    auto set_number = [this, row](int column, double value, char format,
                                  int precision)
    {
        QTableWidgetItem* item = pointsTable->item(row, column);

        if (item == nullptr)
        {
            item = new QTableWidgetItem();
            pointsTable->setItem(row, column, item);
        }

        item->setText(QString::number(value, format, precision));
    };

    set_number(2, cabXSpin->value(), 'f', 3);
    set_number(3, cabYSpin->value(), 'f', 3);
    set_number(4, cabZSpin->value(), 'f', 3);
    set_number(5, cabDirSpin->value(), 'f', 1);

    // Обновляем маркеры во вьюпорте (cellChanged отработает сам)
    refreshPointMarkers();
}

//------------------------------------------------------------------------------
//
//  «Применить к конфигу»: существующий applyCameraPointsToConfig
//  записывает все Camera-точки в секции [Cabine] (промт п.17)
//
//------------------------------------------------------------------------------
void MainWindow::slotCabApplyToConfig()
{
    if (applyCameraPointsToConfig())
    {
        statusBar()->showMessage(
                    tr("Кабины записаны в секции [Cabine] конфигурации"),
                    5000);
    }
    else
    {
        statusBar()->showMessage(
                    tr("Camera-точки не найдены — добавьте точку типа "
                       "«Камера (машинист)» в таблицу"), 8000);
    }
}

//------------------------------------------------------------------------------
//
//  Таблица нагрузок по осям (промт п.15): осей = NumAxis из [Vehicle];
//  статическая = полная масса / оси; динамическая = статическая ×
//  (1 + ускорение / 9.81); подсветка свыше 22.5 тс красным
//
//------------------------------------------------------------------------------
void MainWindow::refreshAxleLoadTable()
{
    if (axleTable == nullptr)
    {
        return;
    }

    axleTable->setRowCount(0);

    const QDomElement vehicle =
            firstSectionElement(doc, QStringLiteral("Vehicle"));

    double empty_mass = 0.0;
    double payload_mass = 0.0;
    int num_axis = 0;

    if (!vehicle.isNull())
    {
        auto read_double = [&vehicle](const char* tag) -> double
        {
            bool found = false;
            const double value = tagText(vehicle, tag, found).toDouble();
            return found ? value : 0.0;
        };

        empty_mass = read_double("EmptyMass");
        payload_mass = read_double("PayloadMass");

        bool found = false;
        num_axis = tagText(vehicle, "NumAxis", found).toInt();
    }

    if (num_axis <= 0)
    {
        return;
    }

    const double accel = axleAccelSpin != nullptr
        ? axleAccelSpin->value() : 0.0;

    const double gross = (empty_mass + payload_mass) / 1000.0 / num_axis;
    const double tare = empty_mass / 1000.0 / num_axis;
    const double stat = gross;
    const double dyn = stat * (1.0 + accel / 9.81);

    for (int axis = 1; axis <= num_axis; ++axis)
    {
        const int row = axleTable->rowCount();
        axleTable->insertRow(row);

        auto* number_item = new QTableWidgetItem(QString::number(axis));
        number_item->setTextAlignment(Qt::AlignCenter);
        axleTable->setItem(row, 0, number_item);

        const double values[4] = {gross, tare, stat, dyn};

        for (int column = 0; column < 4; ++column)
        {
            auto* value_item = new QTableWidgetItem(
                        QString::number(values[column], 'f', 2));
            value_item->setTextAlignment(Qt::AlignCenter);

            if (values[column] > AxleLoadLimitTs)
            {
                value_item->setForeground(
                            QBrush(QColor(0xd3, 0x2f, 0x2f)));
            }

            axleTable->setItem(row, column + 1, value_item);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotAxleAccelChanged()
{
    refreshAxleLoadTable();
}

//------------------------------------------------------------------------------
//
//  Asset Validator (промт п.22, расширенный): ошибки блокируют
//  экспорт, предупреждения — только информация
//
//------------------------------------------------------------------------------
void MainWindow::validatePackage(QStringList* errors,
                                 QStringList* warnings) const
{
    QStringList error_list;
    QStringList warning_list;

    // 1. Обязательная [Vehicle] и её обязательные ключи (validateConfig)
    error_list += validateConfig();

    // 2. Диапазоны схемы vehicle-schema: числовые ключи вне [min; max]
    for (const SectionSpec& spec : vehicleSchema())
    {
        auto check_section = [&spec, &error_list, &warning_list](
                const QDomElement& section)
        {
            if (section.isNull())
            {
                return;
            }

            for (const FieldSpec& field : spec.fields)
            {
                if (field.type != FieldType::Double &&
                    field.type != FieldType::Int)
                {
                    continue;
                }

                bool found = false;
                const QString value = tagText(section,
                                              field.key.toUtf8().constData(),
                                              found);

                if (!found)
                {
                    continue;
                }

                bool ok = false;
                const double number = value.trimmed().toDouble(&ok);

                if (!ok)
                {
                    warning_list << tr("[%1]: ключ <%2> содержит "
                                       "нечисловое значение «%3»")
                                      .arg(spec.name, field.key, value.trimmed());
                    continue;
                }

                if (number < field.min_value - 1e-9 ||
                    number > field.max_value + 1e-9)
                {
                    error_list << tr("[%1]: ключ <%2> = %3 вне диапазона "
                                     "[%4; %5]")
                                  .arg(spec.name, field.key)
                                  .arg(numberToText(number))
                                  .arg(numberToText(field.min_value))
                                  .arg(numberToText(field.max_value));
                }
            }
        };

        if (spec.multiple)
        {
            const int count = countSectionElements(doc, spec.name);

            for (int i = 0; i < count; ++i)
            {
                check_section(sectionElementByIndex(doc, spec.name, i));
            }
        }
        else
        {
            check_section(firstSectionElement(doc, spec.name));
        }
    }

    // 3. Отсутствие [Collision] — предупреждение (генерируется по ролям)
    if (firstSectionElement(doc, "Collision").isNull())
    {
        warning_list << tr("Отсутствует секция [Collision] — при экспорте "
                           "она будет сгенерирована по ролям "
                           "Body/Bogie/Wheel, если роли назначены");
    }

    // 4. LOD-варианты — только предупреждение (промт п.20/22)
    if (!scene.isLoaded())
    {
        warning_list << tr("Модель не загружена — проверить LOD "
                           "и визуально невозможно");
    }
    else if (scene.findLodVariants().size() < 2)
    {
        warning_list << tr("LOD-варианты не найдены (рядом с моделью нет "
                           "файлов lod1/lod2) — будет один уровень "
                           "детализации");
    }

    // 5. Camera-точки без [Cabine] — предупреждение
    {
        int camera_count = 0;

        for (const PhysPoint& point : pointsFromTable())
        {
            if (point.type == PhysPointType::Camera)
            {
                ++camera_count;
            }
        }

        const int cabine_count = countSectionElements(doc, "Cabine");

        if (camera_count > 0 && cabine_count == 0)
        {
            warning_list << tr("Есть Camera-точки (%1), но секций [Cabine] "
                               "в конфиге нет — нажмите «Применить к "
                               "конфигу» в группе «Кабины»").arg(camera_count);
        }

        if (camera_count == 0 && cabine_count > 0)
        {
            warning_list << tr("В конфиге %1 секций [Cabine], но Camera-точек "
                               "в проекте нет").arg(cabine_count);
        }
    }

    // 6. Звуки: файлы из <Sound> должны существовать; пустые имена — ошибка
    {
        const QDomElement root = doc.documentElement();

        if (!root.isNull())
        {
            for (QDomNode node = root.firstChild(); !node.isNull();
                 node = node.nextSibling())
            {
                const QDomElement element = node.toElement();

                if (node.isElement() &&
                    element.tagName() == QStringLiteral("Sound"))
                {
                    const QString name = element.attribute(
                                QStringLiteral("Name")).trimmed();
                    const QString file = element.attribute(
                                QStringLiteral("File")).trimmed();

                    if (name.isEmpty())
                    {
                        error_list << tr("Секция <Sound>: пустое имя (Name)");
                    }

                    if (file.isEmpty())
                    {
                        error_list << tr("Звук «%1»: не задан файл (File)")
                                      .arg(name.isEmpty()
                                           ? tr("(без имени)") : name);
                    }
                    else if (!soundFileExists(file))
                    {
                        error_list << tr("Звук «%1»: файл не найден: %2")
                                      .arg(name.isEmpty()
                                           ? tr("(без имени)") : name)
                                      .arg(file);
                    }

                    for (const QString& variant : normalizeVariants(
                             element.attribute(
                                 QStringLiteral("Variants")))
                             .split(';', Qt::SkipEmptyParts))
                    {
                        if (!soundFileExists(variant))
                        {
                            error_list << tr("Звук «%1»: вариация не найдена: "
                                             "%2")
                                          .arg(name.isEmpty()
                                               ? tr("(без имени)") : name)
                                          .arg(variant);
                        }
                    }
                }
                else if (node.isElement() &&
                         element.tagName() == QStringLiteral("Animation"))
                {
                    // 7. Пустые имена анимаций — предупреждение
                    if (element.attribute(
                            QStringLiteral("Name")).trimmed().isEmpty())
                    {
                        warning_list << tr("Секция <Animation> с пустым "
                                           "именем (Name)");
                    }
                }
            }
        }
    }

    if (errors != nullptr)
    {
        *errors = error_list;
    }

    if (warnings != nullptr)
    {
        *warnings = warning_list;
    }
}

//------------------------------------------------------------------------------
//
//  Диалог «Проверка пакета» (Asset Validator, промт п.22)
//
//------------------------------------------------------------------------------
void MainWindow::slotValidatePackage()
{
    QStringList errors;
    QStringList warnings;
    validatePackage(&errors, &warnings);

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Проверка пакета"));
    dialog.resize(620, 420);

    auto* layout = new QVBoxLayout(&dialog);

    auto* summary = new QLabel(
                tr("Ошибок: %1, предупреждений: %2")
                    .arg(errors.size()).arg(warnings.size()), &dialog);
    summary->setStyleSheet(errors.isEmpty()
        ? "color: #2e7d32; font-weight: bold;"
        : "color: #d32f2f; font-weight: bold;");
    layout->addWidget(summary);

    auto* report = new QPlainTextEdit(&dialog);
    report->setReadOnly(true);

    QString text;

    if (errors.isEmpty() && warnings.isEmpty())
    {
        text = tr("Замечаний нет — пакет готов к экспорту.");
    }

    for (const QString& error : errors)
    {
        text += tr("[ОШИБКА]      %1\n").arg(error);
    }

    for (const QString& warning : warnings)
    {
        text += tr("[ПРЕДУПРЕЖДЕНИЕ] %1\n").arg(warning);
    }

    report->setPlainText(text);
    layout->addWidget(report, 1);

    auto* close_button = new QPushButton(tr("Закрыть"), &dialog);
    connect(close_button, &QPushButton::clicked,
            &dialog, &QDialog::accept);
    layout->addWidget(close_button, 0, Qt::AlignRight);

    dialog.exec();
}

//------------------------------------------------------------------------------
//
//  Дерево модели: фильтр по имени (промт п.32)
//
//------------------------------------------------------------------------------
void MainWindow::slotFilterObjectTree(const QString& text)
{
    (void)text;
    applyObjectFilter();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::applyObjectFilter()
{
    if (objectTree == nullptr || objectFilterEdit == nullptr)
    {
        return;
    }

    const QString filter = objectFilterEdit->text().trimmed();

    for (int i = 0; i < objectTree->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* top = objectTree->topLevelItem(i);
        setItemHiddenRecursive(
                    top, !filter.isEmpty() && !itemMatchesFilter(top, filter));
    }
}

//------------------------------------------------------------------------------
//
//  Метки узлов с элементов дерева (путь -> метка), чтобы не терять
//  их при перестройках дерева
//
//------------------------------------------------------------------------------
QMap<QString, QString> MainWindow::harvestObjectTags() const
{
    QMap<QString, QString> tags;

    if (objectTree == nullptr)
    {
        return tags;
    }

    for (QTreeWidgetItemIterator it(objectTree); *it; ++it)
    {
        QTreeWidgetItem* item = *it;
        const QString path = item->data(0, RoleNodePath).toString();
        const QString tag = item->data(0, RoleNodeTag).toString();

        if (!path.isEmpty() && !tag.isEmpty())
        {
            tags.insert(path, tag);
        }
    }

    return tags;
}

//------------------------------------------------------------------------------
//
//  Присвоить метку из поля выбранному узлу (промт п.32)
//
//------------------------------------------------------------------------------
void MainWindow::slotAssignNodeTag()
{
    if (objectTree == nullptr || objectTagEdit == nullptr)
    {
        return;
    }

    QTreeWidgetItem* item = objectTree->currentItem();

    if (item == nullptr)
    {
        statusBar()->showMessage(
                    tr("Выберите узел в дереве объектов"), 5000);
        return;
    }

    const QString tag = objectTagEdit->text().trimmed();

    item->setData(0, RoleNodeTag, tag);
    item->setText(2, tag);

    statusBar()->showMessage(
                tag.isEmpty()
                    ? tr("Метка снята с узла «%1»").arg(item->text(0))
                    : tr("Метка «%1» назначена узлу «%2»")
                          .arg(tag, item->text(0)),
                5000);
}

//------------------------------------------------------------------------------
//
//  Выделить все узлы с меткой из поля тега (промт п.33)
//
//------------------------------------------------------------------------------
void MainWindow::slotSelectByTag()
{
    if (objectTree == nullptr || objectTagEdit == nullptr)
    {
        return;
    }

    const QString tag = objectTagEdit->text().trimmed();

    if (tag.isEmpty())
    {
        statusBar()->showMessage(
                    tr("Введите метку в поле «Метка узла»"), 5000);
        return;
    }

    objectTree->clearSelection();

    int count = 0;

    for (QTreeWidgetItemIterator it(objectTree); *it; ++it)
    {
        QTreeWidgetItem* item = *it;

        if (item->data(0, RoleNodeTag).toString() == tag)
        {
            item->setSelected(true);
            ++count;
        }
    }

    statusBar()->showMessage(
                tr("Выделено узлов с меткой «%1»: %2").arg(tag).arg(count),
                5000);
}

//------------------------------------------------------------------------------
//
//  Массово назначить роль: роль выбранного узла применяется ко всем
//  видимым после фильтра объектам (промт п.34)
//
//------------------------------------------------------------------------------
void MainWindow::slotMassAssignRole()
{
    if (objectTree == nullptr || !scene.isLoaded())
    {
        statusBar()->showMessage(tr("Сначала загрузите модель"), 5000);
        return;
    }

    QTreeWidgetItem* current = objectTree->currentItem();

    if (current == nullptr)
    {
        statusBar()->showMessage(
                    tr("Выберите узел-образец, роль которого назначить"),
                    5000);
        return;
    }

    const SceneNodeInfo* sample =
            scene.nodeByPath(current->data(0, RoleNodePath).toString());

    if (sample == nullptr)
    {
        return;
    }

    const MeshRole role = sample->role;
    int count = 0;

    for (QTreeWidgetItemIterator it(objectTree); *it; ++it)
    {
        QTreeWidgetItem* item = *it;

        // Пропускаем скрытые фильтром узлы
        if (item->isHidden())
        {
            continue;
        }

        SceneNodeInfo* info =
                scene.nodeByPath(item->data(0, RoleNodePath).toString());

        if (info == nullptr)
        {
            continue;
        }

        info->role = role;
        ++count;
    }

    // Перестройка дерева обновляет комбобоксы ролей; фильтр переустановится
    rebuildObjectTree();
    refreshCollisionPreview();

    statusBar()->showMessage(
                tr("Роль «%1» назначена %2 объектам")
                    .arg(meshRoleName(role)).arg(count), 8000);
}

//------------------------------------------------------------------------------
//
//  Мастер нового ПС (промт п.41): 8 шагов, результат применяется
//  к новому проекту и конфигу в редакторе
//
//------------------------------------------------------------------------------
void MainWindow::slotRunWizard()
{
    NewVehicleWizard wizard(this);

    if (wizard.exec() != QDialog::Accepted)
    {
        return;
    }

    const WizardData data = wizard.data();

    // Конфиг по шаблону (train-project templates)
    QDomDocument new_doc;
    QString error_string;
    int error_line = 0;
    int error_column = 0;

    if (!new_doc.setContent(templateConfigXml(data.templateKey),
                            &error_string, &error_line, &error_column))
    {
        QMessageBox::warning(this, tr("Ошибка"),
            tr("Не удалось создать конфигурацию по шаблону: %1")
                .arg(error_string));
        return;
    }

    // Заполняем [Vehicle] данными мастера
    QDomElement vehicle = firstSectionElement(new_doc, "Vehicle");

    if (!vehicle.isNull())
    {
        setSectionChildValue(new_doc, vehicle, "EmptyMass",
                             numberToText(data.emptyMass));
        setSectionChildValue(new_doc, vehicle, "PayloadMass",
                             numberToText(data.payloadMass));
        setSectionChildValue(new_doc, vehicle, "NumAxis",
                             QString::number(data.numAxis));
        setSectionChildValue(new_doc, vehicle, "Length",
                             numberToText(data.length));
        setSectionChildValue(new_doc, vehicle, "WheelDiameter",
                             numberToText(data.wheelDiameter));
    }

    // Новый проект
    project = TrainProject();
    project.templateKey = data.templateKey;
    project.points = data.addTemplatePoints
        ? templatePoints(data.templateKey) : std::vector<PhysPoint>();

    if (data.addCamera)
    {
        PhysPoint camera;
        camera.name = tr("Cab1");
        camera.type = PhysPointType::Camera;
        camera.x = data.camX;
        camera.y = data.camY;
        camera.z = data.camZ;
        camera.heading = data.camDir;
        project.points.push_back(camera);
    }

    project.exportSettings.vehicleName =
            data.vehicleName.isEmpty()
                ? QStringLiteral("new_vehicle") : data.vehicleName;
    project.exportSettings.targetDir = defaultAddonsDir();
    project.exportSettings.copyModel = true;
    project.configPath = data.configPath;
    projectFilePath.clear();

    // Конфиг в редакторе (файл ещё не сохранён — изменения помечены)
    doc = new_doc;
    filePath.clear();
    setModified(true);

    // Модель сбрасывается; при наличии — грузится
    scene.clear();
    modelStatusLabel->setText(tr("Модель не загружена"));
    rebuildObjectTree();
    lodCombo->blockSignals(true);
    lodCombo->clear();
    lodCombo->setEnabled(false);
    lodCombo->blockSignals(false);

    // Интерфейс под новый проект
    rebuildPointsTable();
    rebuildTree();
    rebuildTractiveTable();
    refreshMassCenterFields();
    refreshAnimationsList();
    rebuildSoundsTable();
    exportNameEdit->setText(project.exportSettings.vehicleName);
    exportDirEdit->setText(defaultAddonsDir());

    if (viewer.isRunning())
    {
        viewer.stop();
        toggle3DButton->setText(tr("Показать 3D"));
    }

    // Модель: glTF грузится сразу, .blend конвертируется Blender-ом
    // (BlendImporter); готовый GLB подхватится в slotBlendFinished
    if (!data.modelPath.isEmpty())
    {
        const QString suffix =
                QFileInfo(data.modelPath).suffix().toLower();

        if (suffix == QStringLiteral("blend"))
        {
            if (blendImporter != nullptr && !blendImporter->isRunning())
            {
                QString blender_path = blenderPathEdit != nullptr
                    ? blenderPathEdit->text().trimmed() : QString();

                if (blender_path.isEmpty())
                {
                    blender_path = QStringLiteral("blender");
                }

                statusBar()->showMessage(
                            tr("Мастер: конвертация %1 в GLB (Blender)...")
                                .arg(QFileInfo(data.modelPath).fileName()));

                blendImporter->start(data.modelPath, blender_path);
            }
        }
        else if (QFile::exists(data.modelPath))
        {
            loadModelFile(data.modelPath);
        }
    }

    updateWindowTitle();
    updateStatusBar();

    statusBar()->showMessage(
                tr("Создан новый ПС «%1» (шаблон: %2). "
                   "Не забудьте сохранить конфигурацию.")
                    .arg(project.exportSettings.vehicleName,
                         data.templateKey), 10000);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::setModified(bool modified)
{
    isModified = modified;
    setWindowModified(modified);
    updateWindowTitle();
}

//------------------------------------------------------------------------------
//
//  Экспорт характеристик в CSV (промт п.20, вторая часть — подготовка
//  к предпросмотру динамики): все секции схемы, формат строки
//  «Секция;Ключ;Значение», разделитель ';', UTF-8 c BOM для Excel;
//  числа нормализуются к точке вместо запятой.
//
//------------------------------------------------------------------------------
void MainWindow::slotExportCsv()
{
    if (doc.isNull() || doc.documentElement().isNull())
    {
        QMessageBox::warning(this, tr("Экспорт CSV"),
            tr("Нет данных: откройте XML-конфигурацию ПС"));
        return;
    }

    QString suggested;

    if (filePath.isEmpty())
    {
        suggested = QDir(defaultAddonsDir()).absoluteFilePath(
                    exportNameEdit->text() + QStringLiteral(".csv"));
    }
    else
    {
        const QFileInfo info(filePath);
        suggested = QDir(info.absolutePath()).absoluteFilePath(
                    info.completeBaseName() + QStringLiteral(".csv"));
    }

    const QString path = QFileDialog::getSaveFileName(this,
        tr("Экспорт характеристик в CSV"), suggested,
        tr("CSV-таблицы (*.csv);;Все файлы (*.*)"));

    if (path.isEmpty())
    {
        return;
    }

    QFile file(path);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate |
                   QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Экспорт CSV"),
            tr("Не удалось создать файл:\n%1").arg(path));
        return;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    // BOM UTF-8 (\uFEFF), чтобы Excel открывал файл без выбора кодировки
    stream << QChar(0xFEFF);
    stream << tr("Секция;Ключ;Значение") << '\n';

    int row_count = 0;

    for (const SectionSpec& spec : vehicleSchema())
    {
        // Секции схемы, отсутствующие в файле, в CSV не попадают
        if (!spec.multiple &&
            firstSectionElement(doc, spec.name).isNull())
        {
            continue;
        }

        const int instance_count = spec.multiple
            ? countSectionElements(doc, spec.name) : 1;

        for (int instance = 0; instance < instance_count; ++instance)
        {
            const QDomElement section = spec.multiple
                ? sectionElementByIndex(doc, spec.name, instance)
                : firstSectionElement(doc, spec.name);

            if (section.isNull())
            {
                continue;
            }

            const QString section_title = spec.multiple
                ? QString("%1 #%2").arg(spec.name).arg(instance + 1)
                : spec.name;

            for (const FieldSpec& field : spec.fields)
            {
                bool found = false;
                QString value = tagText(section,
                                        field.key.toUtf8().constData(),
                                        found);

                if (!found || value.trimmed().isEmpty())
                {
                    // Отсутствующий ключ — значение по умолчанию из схемы
                    value = field.default_value;
                }
                else if (field.type == FieldType::Double ||
                         field.type == FieldType::Int)
                {
                    // Числа: запятая -> точка, если значение парсится
                    bool ok = false;
                    const double number = value.trimmed()
                            .replace(',', '.').toDouble(&ok);

                    if (ok)
                    {
                        value = numberToText(number);
                    }
                }

                stream << section_title << ';' << field.key << ';'
                       << value << '\n';
                ++row_count;
            }
        }
    }

    file.close();

    statusBar()->showMessage(
                tr("CSV: записано строк: %1 (%2)").arg(row_count).arg(path),
                8000);
    QMessageBox::information(this, tr("Экспорт CSV"),
        tr("Записано строк: %1\n\n%2").arg(row_count).arg(path));
}

//------------------------------------------------------------------------------
//
//  Дебаг-вид (промт п.38): стрелки осей ПС в центре модели
//
//------------------------------------------------------------------------------
void MainWindow::slotAxesToggled(bool checked)
{
    if (viewer.isRunning())
    {
        viewer.setAxesMarker(checked);
    }
}

//------------------------------------------------------------------------------
//
//  Дебаг-вид (промт п.38): бокс габарита 1Т длиной по bbox модели
//
//------------------------------------------------------------------------------
void MainWindow::slotGabaritToggled(bool checked)
{
    if (!viewer.isRunning())
    {
        return;
    }

    double length = 15.0;

    if (scene.isLoaded())
    {
        vsg::dvec3 bounds_min;
        vsg::dvec3 bounds_max;
        scene.overallBounds(bounds_min, bounds_max);
        length = std::max(1.0, bounds_max.y - bounds_min.y);
    }

    viewer.setGabaritMarker(checked, length);
}

//------------------------------------------------------------------------------
//
//  Валидатор характеристик (промт п.18): диапазоны схемы min/max +
//  логические проверки; ошибки и предупреждения — раздельно
//
//------------------------------------------------------------------------------
void MainWindow::validateCharacteristics(QStringList* errors,
                                         QStringList* warnings) const
{
    QStringList error_list;
    QStringList warning_list;

    // 1. Обязательные ключи [Vehicle] (существующая проверка)
    error_list += validateConfig();

    // 2. Диапазоны схемы vehicle-schema: числовые ключи вне [min; max]
    for (const SectionSpec& spec : vehicleSchema())
    {
        auto check_section = [&spec, &error_list, &warning_list](
                const QDomElement& section)
        {
            if (section.isNull())
            {
                return;
            }

            for (const FieldSpec& field : spec.fields)
            {
                if (field.type != FieldType::Double &&
                    field.type != FieldType::Int)
                {
                    continue;
                }

                bool found = false;
                const QString value = tagText(section,
                                              field.key.toUtf8().constData(),
                                              found);

                if (!found)
                {
                    continue;
                }

                bool ok = false;
                const double number = value.trimmed().toDouble(&ok);

                if (!ok)
                {
                    warning_list << tr("[%1]: ключ <%2> содержит "
                                       "нечисловое значение «%3»")
                                      .arg(spec.name, field.key,
                                           value.trimmed());
                    continue;
                }

                if (number < field.min_value - 1e-9 ||
                    number > field.max_value + 1e-9)
                {
                    error_list << tr("[%1]: ключ <%2> = %3 вне диапазона "
                                     "[%4; %5]")
                                  .arg(spec.name, field.key)
                                  .arg(numberToText(number))
                                  .arg(numberToText(field.min_value))
                                  .arg(numberToText(field.max_value));
                }
            }
        };

        if (spec.multiple)
        {
            const int count = countSectionElements(doc, spec.name);

            for (int i = 0; i < count; ++i)
            {
                check_section(sectionElementByIndex(doc, spec.name, i));
            }
        }
        else
        {
            check_section(firstSectionElement(doc, spec.name));
        }
    }

    // 3. Логические проверки (промт п.18)
    const QDomElement vehicle =
            firstSectionElement(doc, QStringLiteral("Vehicle"));

    if (!vehicle.isNull())
    {
        bool found = false;

        const double empty_mass =
                tagText(vehicle, "EmptyMass", found).toDouble();

        if (found && empty_mass <= 0.0)
        {
            error_list << tr("[Vehicle]: EmptyMass должна быть больше нуля");
        }

        const double payload_mass =
                tagText(vehicle, "PayloadMass", found).toDouble();

        if (found && payload_mass < 0.0)
        {
            error_list << tr("[Vehicle]: PayloadMass не может быть "
                             "отрицательной");
        }

        const int num_axis =
                tagText(vehicle, "NumAxis", found).toInt();

        if (found)
        {
            if (num_axis <= 0)
            {
                error_list << tr("[Vehicle]: NumAxis должна быть "
                                 "больше нуля");
            }
            else if (num_axis % 2 != 0)
            {
                warning_list << tr("[Vehicle]: нечётное число осей (%1) — "
                                   "у большинства ПС чётное число осей "
                                   "(4/6/8)").arg(num_axis);
            }
        }

        const double wheel_diameter =
                tagText(vehicle, "WheelDiameter", found).toDouble();

        if (found && (wheel_diameter < 0.5 || wheel_diameter > 1.5))
        {
            warning_list << tr("[Vehicle]: WheelDiameter = %1 вне "
                               "типового диапазона [0.5; 1.5] м")
                                .arg(numberToText(wheel_diameter));
        }
    }

    // Центр масс: типовая высота 0.5–3.0 м
    const QDomElement mass_center =
            firstSectionElement(doc, QStringLiteral("MassCenter"));

    if (!mass_center.isNull())
    {
        bool found = false;
        const double height =
                tagText(mass_center, "Height", found).toDouble();

        if (found && (height < 0.5 || height > 3.0))
        {
            warning_list << tr("[MassCenter]: Height = %1 вне типового "
                               "диапазона [0.5; 3.0] м")
                                .arg(numberToText(height));
        }
    }

    // Тип привода: дизельная мощность против электрического DriveType
    {
        double diesel_power = 0.0;
        bool diesel_found = false;

        const QDomElement diesel =
                firstSectionElement(doc, QStringLiteral("Diesel"));

        if (!diesel.isNull())
        {
            diesel_power = tagText(diesel, "NominalPower",
                                   diesel_found).toDouble();
        }

        const QDomElement energy =
                firstSectionElement(doc, QStringLiteral("Energy"));

        bool drive_found = false;
        const QString drive_type = energy.isNull()
            ? QString()
            : tagText(energy, "DriveType", drive_found)
                  .trimmed().toLower();

        if (diesel_found && diesel_power > 0.0 && drive_found &&
            drive_type == QLatin1String("electric"))
        {
            warning_list << tr("Гибридная схема: [Diesel] NominalPower = "
                               "%1 кВт при [Energy] DriveType = electric — "
                               "проверьте тип привода (hybrid)")
                                .arg(numberToText(diesel_power));
        }

        if (drive_found && drive_type == QLatin1String("diesel") &&
            (!diesel_found || diesel_power <= 0.0))
        {
            warning_list << tr("[Energy] DriveType = diesel, но секция "
                               "[Diesel] отсутствует или NominalPower "
                               "не задана");
        }
    }

    // Песочная система: ёмкость задана, но секции нет или отключена
    {
        const QDomElement sand =
                firstSectionElement(doc, QStringLiteral("Sand"));

        if (sand.isNull())
        {
            warning_list << tr("Секция [Sand] отсутствует — параметры "
                               "песочной системы (Capacity) не будут "
                               "применены");
        }
        else
        {
            bool found = false;
            const double capacity =
                    tagText(sand, "Capacity", found).toDouble();

            bool enabled_found = false;
            const QString enabled = tagText(sand, "Enabled", enabled_found)
                    .trimmed().toLower();

            if (found && capacity > 0.0 && enabled_found &&
                enabled != QLatin1String("true") &&
                enabled != QLatin1String("1"))
            {
                warning_list << tr("[Sand]: Capacity = %1 кг, но "
                                   "Enabled = false — подача песка "
                                   "отключена").arg(numberToText(capacity));
            }
        }
    }

    if (errors != nullptr)
    {
        *errors = error_list;
    }

    if (warnings != nullptr)
    {
        *warnings = warning_list;
    }
}

//------------------------------------------------------------------------------
//
//  Диалог «Проверка характеристик» (валидатор п.18, по образцу
//  диалога «Проверка пакета»)
//
//------------------------------------------------------------------------------
void MainWindow::slotValidateCharacteristics()
{
    QStringList errors;
    QStringList warnings;
    validateCharacteristics(&errors, &warnings);

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Проверка характеристик"));
    dialog.resize(620, 420);

    auto* layout = new QVBoxLayout(&dialog);

    auto* summary = new QLabel(
                tr("Ошибок: %1, предупреждений: %2")
                    .arg(errors.size()).arg(warnings.size()), &dialog);
    summary->setStyleSheet(errors.isEmpty()
        ? "color: #2e7d32; font-weight: bold;"
        : "color: #d32f2f; font-weight: bold;");
    layout->addWidget(summary);

    auto* report = new QPlainTextEdit(&dialog);
    report->setReadOnly(true);

    QString text;

    if (errors.isEmpty() && warnings.isEmpty())
    {
        text = tr("Замечаний нет — характеристики в порядке.");
    }

    for (const QString& error : errors)
    {
        text += tr("[ОШИБКА]      %1\n").arg(error);
    }

    for (const QString& warning : warnings)
    {
        text += tr("[ПРЕДУПРЕЖДЕНИЕ] %1\n").arg(warning);
    }

    report->setPlainText(text);
    layout->addWidget(report, 1);

    auto* close_button = new QPushButton(tr("Закрыть"), &dialog);
    connect(close_button, &QPushButton::clicked,
            &dialog, &QDialog::accept);
    layout->addWidget(close_button, 0, Qt::AlignRight);

    dialog.exec();
}
