#include "NewVehicleWizard.h"
#include "train-project.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QVBoxLayout>

//------------------------------------------------------------------------------
//
//  Мастер нового ПС (промт п.41): QDialog с QStackedWidget на 8 шагов.
//  Шаблоны и базовые точки берутся из train-project (projectTemplates/
//  templatePoints), конфиг строит MainWindow по итоговым данным.
//
//------------------------------------------------------------------------------
NewVehicleWizard::NewVehicleWizard(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Мастер нового ПС"));
    resize(520, 420);

    auto* layout = new QVBoxLayout(this);

    stepLabel = new QLabel(this);
    layout->addWidget(stepLabel);

    stack = new QStackedWidget(this);

    for (int step = 0; step < TotalSteps; ++step)
    {
        stack->addWidget(createStepWidget(step));
    }

    layout->addWidget(stack, 1);

    // Кнопки навигации
    auto* buttonsRow = new QHBoxLayout();

    backButton = new QPushButton(tr("< Назад"), this);
    connect(backButton, &QPushButton::clicked, this, &NewVehicleWizard::slotBack);
    buttonsRow->addWidget(backButton);

    buttonsRow->addStretch();

    nextButton = new QPushButton(tr("Далее >"), this);
    nextButton->setDefault(true);
    connect(nextButton, &QPushButton::clicked, this, &NewVehicleWizard::slotNext);
    buttonsRow->addWidget(nextButton);

    layout->addLayout(buttonsRow);

    setStep(0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const WizardData& NewVehicleWizard::data() const
{
    return data_;
}

//------------------------------------------------------------------------------
//
//  Страницы мастера
//
//------------------------------------------------------------------------------
QWidget* NewVehicleWizard::createStepWidget(int step)
{
    auto* page = new QWidget(this);
    auto* layout = new QVBoxLayout(page);
    layout->addStretch();

    switch (step)
    {
        case 0:
        {
            // Шаг 1: тип ПС по шаблонам train-project
            layout->insertWidget(0, new QLabel(
                tr("Выберите тип создаваемого подвижного состава. "
                   "Шаблон задаёт начальные массы, тормозное "
                   "оборудование и базовые физические точки."), page));

            auto* form = new QFormLayout();
            templateCombo = new QComboBox(page);

            for (const TemplateSpec& spec : projectTemplates())
            {
                templateCombo->addItem(spec.title, spec.key);
            }

            form->addRow(tr("Тип ПС:"), templateCombo);
            layout->insertLayout(1, form);

            templateDescLabel = new QLabel(page);
            templateDescLabel->setWordWrap(true);
            templateDescLabel->setStyleSheet("color: gray;");
            layout->insertWidget(2, templateDescLabel);

            connect(templateCombo, &QComboBox::currentIndexChanged,
                    this, [this](int index)
            {
                if (index >= 0 && templateDescLabel != nullptr)
                {
                    templateDescLabel->setText(
                                projectTemplates().at(index).description);
                }
            });

            // Описание шаблона, выбранного по умолчанию
            if (!projectTemplates().isEmpty())
            {
                templateDescLabel->setText(
                            projectTemplates().front().description);
            }
            break;
        }

        case 1:
        {
            // Шаг 2: имя ПС и файл конфигурации
            layout->insertWidget(0, new QLabel(
                tr("Имя подвижного состава используется как имя пакета "
                   "(addons/<имя>). Файл конфигурации можно не выбирать — "
                   "тогда его предложит сохранить диалог сохранения."),
                page));

            auto* form = new QFormLayout();

            nameEdit = new QLineEdit(page);
            nameEdit->setPlaceholderText(tr("например, pass_car_1"));
            form->addRow(tr("Имя ПС:"), nameEdit);

            auto* config_row = new QHBoxLayout();
            configFileEdit = new QLineEdit(page);
            config_row->addWidget(configFileEdit, 1);

            auto* browse_button = new QPushButton(tr("..."), page);
            connect(browse_button, &QPushButton::clicked, this,
                    &NewVehicleWizard::slotBrowseConfigFile);
            config_row->addWidget(browse_button);

            form->addRow(tr("Файл конфигурации:"), config_row);
            layout->insertLayout(1, form);
            break;
        }

        case 2:
        {
            // Шаг 3: модель (.blend/.glb/.gltf, BlendImporter вызывается
            // после создания проекта самим MainWindow)
            layout->insertWidget(0, new QLabel(
                tr("Укажите модель ПС. Файлы .blend конвертируются "
                   "во внешний Blender в GLB (путь к Blender — на вкладке "
                   "«Модель»), glb/gltf загружаются сразу. Можно пропустить "
                   "и добавить модель позже."), page));

            auto* form = new QFormLayout();

            auto* model_row = new QHBoxLayout();
            modelFileEdit = new QLineEdit(page);
            model_row->addWidget(modelFileEdit, 1);

            auto* browse_button = new QPushButton(tr("..."), page);
            connect(browse_button, &QPushButton::clicked, this,
                    &NewVehicleWizard::slotBrowseModelFile);
            model_row->addWidget(browse_button);

            form->addRow(tr("Файл модели:"), model_row);
            layout->insertLayout(1, form);

            modelHintLabel = new QLabel(page);
            modelHintLabel->setWordWrap(true);
            modelHintLabel->setStyleSheet("color: gray;");
            layout->insertWidget(2, modelHintLabel);

            connect(modelFileEdit, &QLineEdit::textChanged, this, [this]()
            {
                if (modelHintLabel == nullptr)
                {
                    return;
                }

                const QString suffix = QFileInfo(modelFileEdit->text())
                        .suffix().toLower();

                modelHintLabel->setText(
                            suffix == QStringLiteral("blend")
                                ? tr(".blend будет сконвертирован в GLB "
                                     "после нажатия «Создать»")
                                : QString());
            });
            break;
        }

        case 3:
        {
            // Шаг 4: массы
            layout->insertWidget(0, new QLabel(
                tr("Массы записываются в [Vehicle]: EmptyMass, PayloadMass, "
                   "NumAxis (значения шаблона можно изменить позже)."),
                page));

            auto* form = new QFormLayout();

            emptyMassSpin = new QDoubleSpinBox(page);
            emptyMassSpin->setRange(1000.0, 10000000.0);
            emptyMassSpin->setDecimals(0);
            emptyMassSpin->setSingleStep(1000.0);
            emptyMassSpin->setValue(40000.0);
            form->addRow(tr("Масса тары, кг:"), emptyMassSpin);

            payloadMassSpin = new QDoubleSpinBox(page);
            payloadMassSpin->setRange(0.0, 10000000.0);
            payloadMassSpin->setDecimals(0);
            payloadMassSpin->setSingleStep(1000.0);
            payloadMassSpin->setValue(60000.0);
            form->addRow(tr("Масса груза, кг:"), payloadMassSpin);

            numAxisSpin = new QSpinBox(page);
            numAxisSpin->setRange(2, 16);
            numAxisSpin->setValue(4);
            form->addRow(tr("Число осей:"), numAxisSpin);

            layout->insertLayout(1, form);
            break;
        }

        case 4:
        {
            // Шаг 5: габариты
            layout->insertWidget(0, new QLabel(
                tr("Основные размеры: длина по осям автосцепок и диаметр "
                   "колеса (ключи Length и WheelDiameter секции [Vehicle])."),
                page));

            auto* form = new QFormLayout();

            lengthSpin = new QDoubleSpinBox(page);
            lengthSpin->setRange(1.0, 100.0);
            lengthSpin->setDecimals(2);
            lengthSpin->setSingleStep(0.5);
            lengthSpin->setValue(15.0);
            form->addRow(tr("Длина, м:"), lengthSpin);

            wheelDiameterSpin = new QDoubleSpinBox(page);
            wheelDiameterSpin->setRange(0.3, 2.5);
            wheelDiameterSpin->setDecimals(3);
            wheelDiameterSpin->setSingleStep(0.05);
            wheelDiameterSpin->setValue(0.95);
            form->addRow(tr("Диаметр колеса, м:"), wheelDiameterSpin);

            layout->insertLayout(1, form);
            break;
        }

        case 5:
        {
            // Шаг 6: физические точки — шаг пропускается с подсказкой
            layout->insertWidget(0, new QLabel(
                tr("Физические точки настраиваются после создания на вкладке "
                   "«Физические точки» (14 типов: автосцепки, колёсные пары, "
                   "рукава, краны, двери, камеры и другие)."), page));

            templatePointsCheck = new QCheckBox(
                        tr("Создать базовые точки по шаблону "
                           "(сцепки, рукава, краны...)"), page);
            templatePointsCheck->setChecked(true);
            layout->insertWidget(1, templatePointsCheck);
            break;
        }

        case 6:
        {
            // Шаг 7: кабина (Camera-точка)
            layout->insertWidget(0, new QLabel(
                tr("Позиция машиниста: добавляется точка типа «Камера» "
                   "(позже записывается в [Cabine] как DriverPos/DriverDir). "
                   "Для вагонов кабину можно не задавать."), page));

            auto* form = new QFormLayout();

            cameraCheck = new QCheckBox(
                        tr("Добавить Camera-точку (кабину)"), page);
            cameraCheck->setChecked(true);
            form->addRow(QString(), cameraCheck);

            auto make_spin = [page](double min, double max,
                                    double value) -> QDoubleSpinBox*
            {
                auto* spin = new QDoubleSpinBox(page);
                spin->setRange(min, max);
                spin->setDecimals(2);
                spin->setSingleStep(0.1);
                spin->setValue(value);
                return spin;
            };

            camXSpin = make_spin(-20.0, 20.0, 0.0);
            form->addRow(tr("DriverPos X, м:"), camXSpin);

            camYSpin = make_spin(-20.0, 20.0, 5.0);
            form->addRow(tr("DriverPos Y, м:"), camYSpin);

            camZSpin = make_spin(-10.0, 10.0, 2.5);
            form->addRow(tr("DriverPos Z, м:"), camZSpin);

            camDirSpin = make_spin(-360.0, 360.0, 0.0);
            camDirSpin->setDecimals(1);
            form->addRow(tr("DriverDir, град:"), camDirSpin);

            layout->insertLayout(1, form);
            break;
        }

        default:
        {
            // Шаг 8: итог
            layout->insertWidget(0, new QLabel(
                tr("Проверьте итоговые параметры и нажмите «Создать». "
                   "Будет построен конфиг по шаблону и открыт в редакторе."),
                page));

            summaryEdit = new QPlainTextEdit(page);
            summaryEdit->setReadOnly(true);
            layout->insertWidget(1, summaryEdit);
            break;
        }
    }

    return page;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void NewVehicleWizard::setStep(int step)
{
    if (step < 0 || step >= TotalSteps)
    {
        return;
    }

    currentStep = step;
    stack->setCurrentIndex(step);

    stepLabel->setText(tr("Шаг %1 из %2")
                           .arg(step + 1)
                           .arg(TotalSteps));
    backButton->setEnabled(step > 0);
    nextButton->setText(step == TotalSteps - 1
                            ? tr("Создать")
                            : tr("Далее >"));

    if (step == TotalSteps - 1)
    {
        slotUpdateSummary();
    }
}

//------------------------------------------------------------------------------
//
//  Сбор данных из полей мастера
//
//------------------------------------------------------------------------------
void NewVehicleWizard::collectData()
{
    if (templateCombo != nullptr)
    {
        data_.templateKey = templateCombo->currentData().toString();
    }

    data_.vehicleName = nameEdit != nullptr ? nameEdit->text().trimmed()
                                            : QString();
    data_.configPath = configFileEdit != nullptr
        ? configFileEdit->text().trimmed() : QString();
    data_.modelPath = modelFileEdit != nullptr
        ? modelFileEdit->text().trimmed() : QString();

    if (emptyMassSpin != nullptr)
    {
        data_.emptyMass = emptyMassSpin->value();
    }

    if (payloadMassSpin != nullptr)
    {
        data_.payloadMass = payloadMassSpin->value();
    }

    if (numAxisSpin != nullptr)
    {
        data_.numAxis = numAxisSpin->value();
    }

    if (lengthSpin != nullptr)
    {
        data_.length = lengthSpin->value();
    }

    if (wheelDiameterSpin != nullptr)
    {
        data_.wheelDiameter = wheelDiameterSpin->value();
    }

    data_.addTemplatePoints = templatePointsCheck != nullptr
        && templatePointsCheck->isChecked();

    data_.addCamera = cameraCheck != nullptr && cameraCheck->isChecked();

    if (camXSpin != nullptr)
    {
        data_.camX = camXSpin->value();
        data_.camY = camYSpin->value();
        data_.camZ = camZSpin->value();
        data_.camDir = camDirSpin->value();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void NewVehicleWizard::slotNext()
{
    // Имя обязательно
    if (currentStep == 1 && nameEdit != nullptr &&
        nameEdit->text().trimmed().isEmpty())
    {
        QMessageBox::information(this, tr("Мастер нового ПС"),
                                 tr("Задайте имя подвижного состава"));
        return;
    }

    if (currentStep < TotalSteps - 1)
    {
        setStep(currentStep + 1);
        return;
    }

    // Последний шаг — «Создать»
    collectData();
    accept();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void NewVehicleWizard::slotBack()
{
    if (currentStep > 0)
    {
        setStep(currentStep - 1);
    }
}

//------------------------------------------------------------------------------
//
//  Итоговый отчёт последнего шага
//
//------------------------------------------------------------------------------
void NewVehicleWizard::slotUpdateSummary()
{
    if (summaryEdit == nullptr)
    {
        return;
    }

    collectData();

    QString template_title = data_.templateKey;

    for (const TemplateSpec& spec : projectTemplates())
    {
        if (spec.key == data_.templateKey)
        {
            template_title = spec.title;
            break;
        }
    }

    QString text;
    text += tr("Тип ПС:            %1\n").arg(template_title);
    text += tr("Имя ПС:            %1\n").arg(data_.vehicleName);

    text += tr("Файл конфигурации: %1\n")
                .arg(data_.configPath.isEmpty()
                     ? tr("(будет предложен при сохранении)")
                     : data_.configPath);

    text += tr("Модель:            %1\n")
                .arg(data_.modelPath.isEmpty()
                     ? tr("(не задана)") : data_.modelPath);

    text += tr("Тара/груз:         %1 / %2 кг\n")
                .arg(data_.emptyMass, 0, 'f', 0)
                .arg(data_.payloadMass, 0, 'f', 0);
    text += tr("Осей:              %1\n").arg(data_.numAxis);
    text += tr("Длина:             %1 м\n").arg(data_.length, 0, 'f', 2);
    text += tr("Диаметр колеса:    %1 м\n")
                .arg(data_.wheelDiameter, 0, 'f', 3);
    text += tr("Базовые точки:     %1\n")
                .arg(data_.addTemplatePoints
                     ? tr("да (по шаблону)") : tr("нет"));

    if (data_.addCamera)
    {
        text += tr("Кабина (Camera):   X=%1 Y=%2 Z=%3 м, "
                   "направление %4 град\n")
                    .arg(data_.camX, 0, 'f', 2)
                    .arg(data_.camY, 0, 'f', 2)
                    .arg(data_.camZ, 0, 'f', 2)
                    .arg(data_.camDir, 0, 'f', 1);
    }
    else
    {
        text += tr("Кабина (Camera):   не добавлять\n");
    }

    summaryEdit->setPlainText(text);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void NewVehicleWizard::slotBrowseConfigFile()
{
    if (configFileEdit == nullptr)
    {
        return;
    }

    const QString path = QFileDialog::getSaveFileName(this,
        tr("Файл конфигурации ПС"),
        configFileEdit->text(),
        tr("XML-конфигурации (*.xml);;Все файлы (*.*)"));

    if (!path.isEmpty())
    {
        configFileEdit->setText(path);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void NewVehicleWizard::slotBrowseModelFile()
{
    if (modelFileEdit == nullptr)
    {
        return;
    }

    const QString path = QFileDialog::getOpenFileName(this,
        tr("Модель ПС"), QString(),
        tr("Модели (*.blend *.glb *.gltf);;Все файлы (*.*)"));

    if (!path.isEmpty())
    {
        modelFileEdit->setText(path);
    }
}
