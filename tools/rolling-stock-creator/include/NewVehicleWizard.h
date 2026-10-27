#ifndef NEW_VEHICLE_WIZARD_H
#define NEW_VEHICLE_WIZARD_H

#include <QDialog>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;
class QStackedWidget;

//------------------------------------------------------------------------------
//
//  Данные, собранные мастером нового ПС (промт п.41).
//  На их основе MainWindow строит конфиг по шаблону train-project,
//  создаёт проект и загружает модель.
//
//------------------------------------------------------------------------------
struct WizardData
{
    QString templateKey;          ///< Ключ шаблона (train-project.h)
    QString vehicleName;          ///< Имя ПС (имя пакета)
    QString configPath;           ///< Путь к файлу конфигурации (может быть пуст)
    QString modelPath;            ///< .blend/.glb/.gltf (может быть пуст)

    double emptyMass = 40000.0;   ///< Масса тары, кг
    double payloadMass = 60000.0; ///< Масса груза, кг
    int numAxis = 4;              ///< Число осей

    double length = 15.0;         ///< Длина по осям автосцепок, м
    double wheelDiameter = 0.95;  ///< Диаметр колеса, м

    bool addTemplatePoints = true; ///< Создать базовые точки шаблона

    bool addCamera = true;        ///< Добавить Camera-точку (кабину)
    double camX = 0.0;            ///< DriverPos, м
    double camY = 5.0;
    double camZ = 2.5;
    double camDir = 0.0;          ///< DriverDir, град
};

//------------------------------------------------------------------------------
//
//  Компактный мастер нового ПС в 8 шагов (промт п.41):
//  1) тип ПС, 2) имя/файл, 3) модель, 4) массы, 5) габариты,
//  6) физические точки (пропуск с подсказкой), 7) кабина, 8) итог.
//
//  AUTOMOC включён в CMakeLists (GLOB include/*.h), поэтому
//  Q_OBJECT и слоты обрабатываются moc-ом штатно.
//
//------------------------------------------------------------------------------
class NewVehicleWizard : public QDialog
{
    Q_OBJECT

public:
    explicit NewVehicleWizard(QWidget* parent = nullptr);

    /// Данные, собранные мастером (валидно после Accepted)
    const WizardData& data() const;

private slots:
    void slotNext();
    void slotBack();
    void slotUpdateSummary();
    void slotBrowseConfigFile();
    void slotBrowseModelFile();

private:
    /// Страница шага (0..7)
    QWidget* createStepWidget(int step);

    /// Показать шаг: подпись, доступность кнопок
    void setStep(int step);

    /// Собрать данные из полей в data_
    void collectData();

    QStackedWidget* stack = nullptr;
    QLabel* stepLabel = nullptr;
    QPushButton* backButton = nullptr;
    QPushButton* nextButton = nullptr;

    // Шаг 1: тип ПС
    QComboBox* templateCombo = nullptr;
    QLabel* templateDescLabel = nullptr;

    // Шаг 2: имя и файл конфигурации
    QLineEdit* nameEdit = nullptr;
    QLineEdit* configFileEdit = nullptr;

    // Шаг 3: модель
    QLineEdit* modelFileEdit = nullptr;
    QLabel* modelHintLabel = nullptr;

    // Шаг 4: массы
    QDoubleSpinBox* emptyMassSpin = nullptr;
    QDoubleSpinBox* payloadMassSpin = nullptr;
    QSpinBox* numAxisSpin = nullptr;

    // Шаг 5: габариты
    QDoubleSpinBox* lengthSpin = nullptr;
    QDoubleSpinBox* wheelDiameterSpin = nullptr;

    // Шаг 6: физические точки (пропуск)
    QCheckBox* templatePointsCheck = nullptr;

    // Шаг 7: кабина
    QCheckBox* cameraCheck = nullptr;
    QDoubleSpinBox* camXSpin = nullptr;
    QDoubleSpinBox* camYSpin = nullptr;
    QDoubleSpinBox* camZSpin = nullptr;
    QDoubleSpinBox* camDirSpin = nullptr;

    // Шаг 8: итог
    QPlainTextEdit* summaryEdit = nullptr;

    WizardData data_;
    int currentStep = 0;

    static const int TotalSteps = 8;
};

#endif // NEW_VEHICLE_WIZARD_H
