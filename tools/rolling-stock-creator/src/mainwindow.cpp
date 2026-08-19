#include "mainwindow.h"

#include "export-pipeline.h"
#include "filesystem.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDomDocument>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSpinBox>
#include <QSplitter>
#include <QTextStream>
#include <QToolBar>
#include <QTreeWidget>
#include <QUrl>
#include <QVBoxLayout>

#include <QXmlStreamWriter>

#include <QDir>
#include <QTreeWidgetItemIterator>

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
    updateStatusBar();
    updateRecentMenu();
    updateWindowTitle();
}

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

    // --- Меню «Проект» (.trainproject, ТЗ п.30-31) ---
    QMenu* project_menu = menuBar()->addMenu(tr("Проект"));

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
    tabWidget->addTab(createPointsTab(), tr("Физические точки"));
    tabWidget->addTab(createExportTab(), tr("Экспорт"));

    setCentralWidget(tabWidget);

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
    buttonsRow->addStretch();

    contentLayout->addLayout(buttonsRow);

    formWidget = new QWidget(content);
    auto* formLayout = new QFormLayout(formWidget);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    contentLayout->addWidget(formWidget);
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

    layout->addLayout(controls);

    // Статус модели
    modelStatusLabel = new QLabel(tr("Модель не загружена"), widget);
    modelStatusLabel->setWordWrap(true);
    layout->addWidget(modelStatusLabel);

    // Дерево объектов модели (assimp/glTF scene graph)
    objectTree = new QTreeWidget(widget);
    objectTree->setHeaderLabels({tr("Объект"), tr("Игровая роль")});
    objectTree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    objectTree->setColumnWidth(1, 180);
    connect(objectTree, &QTreeWidget::itemChanged, this,
            &MainWindow::slotObjectItemChanged);
    layout->addWidget(objectTree, 1);

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
    pointsTable->setHorizontalHeaderItems({
        new QTableWidgetItem(tr("Имя")),
        new QTableWidgetItem(tr("Тип")),
        new QTableWidgetItem(tr("X, м")),
        new QTableWidgetItem(tr("Y, м")),
        new QTableWidgetItem(tr("Z, м")),
        new QTableWidgetItem(tr("Направление, град"))
    });
    pointsTable->horizontalHeader()->setSectionResizeMode(
                QHeaderView::ResizeToContents);
    pointsTable->verticalHeader()->setVisible(false);
    connect(pointsTable, &QTableWidget::cellChanged, this,
            &MainWindow::slotPointChanged);
    layout->addWidget(pointsTable, 1);

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

    lodCombo->setCurrentIndex(std::clamp(project.lodIndex, 0,
                                         qMax(0, variants.size() - 1)));
    lodCombo->blockSignals(false);

    modelStatusLabel->setText(tr("Модель: %1 (%2 объектов)")
                                  .arg(path)
                                  .arg(static_cast<int>(scene.nodes().size())));

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
                scene.collisionFromRoles(), pointsFromTable());

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

    for (const SceneNodeInfo& info : scene.nodes())
    {
        ProjectNode node;
        node.path = info.path;
        node.roleKey = meshRoleKey(info.role);
        node.displayName = info.displayName;
        node.visible = info.visible;
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
bool MainWindow::writeConfigFile(const QString& file_path) const
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
    writer.setCodec("UTF-8");

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
//------------------------------------------------------------------------------
void MainWindow::setModified(bool modified)
{
    isModified = modified;
    updateWindowTitle();
}
