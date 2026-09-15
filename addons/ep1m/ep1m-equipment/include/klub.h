#ifndef     KLUB_H
#define     KLUB_H

#include    "device.h"
#include    "klub-stations.h"
#include    "ALSN-struct.h"
#include    "ALSN-coil.h"
#include    "ALSN-decoder.h"
#include    "speedmap.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    NUM_LAMPS = 8,
    WHITE_LAMP = 0,
    RED_LAMP = 1,
    RED_YELLOW_LAMP = 2,
    YELLOW_LAMP = 3,
    GREEN_LAMP1 = 4,
    GREEN_LAMP2 = 5,
    GREEN_LAMP3 = 6,
    GREEN_LAMP4 = 7,
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class KLUB : public Device
{
public:

    KLUB(QObject *parent = Q_NULLPTR);

    ~KLUB();

    void step(double t, double dt) override;

    /// Прием кода АЛСН
    void setAlsnCode(ALSN code_alsn)
    {
        if (!is_shunting_mode)
        {
            old_code_alsn = this->code_alsn;
            this->code_alsn = code_alsn;
        }
        else
        {
            this->code_alsn = ALSN::NO_CODE;
        }
    };

    /// Модуль приёма сигналов АЛС с путевой топологии
    void setCoilALSNModule(CoilALSN *device)
    {
        coilALSN = device;
    }

    /// Модуль работы с ограничениями скорости на путевой топологии
    void setSpeedMapModule(SpeedMap *device)
    {
        speedmap = device;
        speedmap->setCurrentSearchDistance(train_length);
    }

   /// Прием состояния РБ
   void setRBstate(bool state) { state_RB = state; };

   /// Прием состояния РБС
   void setRBSstate(bool state) { state_RBS = state; };

   /// Прием скорости от ДПС
   void setVelocity(double v)
   {
       this->v = qAbs(v);
       v_kmh = this->v * Physics::kmh;
   }

   void setKeyEPK(bool key_epk) { this->key_epk = key_epk; }

   /// Выдача состояния цепи удерживающей катушки ЭПК
   bool getEPKstate() { return epk_state.getState(); };

   /// Получить состояние лампы локомотивного светофора
   float getLampState(size_t lamp_idx)
   {
       if (lamp_idx < lamps.size())
           return lamps[lamp_idx];

        return 0.0f;
   }

   /// Получить активную лампу локомотивного светофора
   float getLampNum()
   {
       for (int i = 0; i <= GREEN_LAMP1; ++i)
       {
           if (lamps[i] == 1.0f)
               return static_cast<float>(i);
       }

       return 0.0f;
   }

   double getCurrentSpeedLimit() const { return current_limit; }

   double getNextSpeedLimit() const { return next_limit; }

   bool isDisplayON() const { return is_dislplay_ON; }

   void setVoltage(double U_pow) { this->U_pow = U_pow; }

   double getVelocityKmh() const { return v_kmh; }

   /// Сигнал "Проверка бдительности"
   bool isCheckVigilanse() const { return check_vigilance; }

   /// Вернуть ускорение поезда
   double getAcceleration() const { return acceleration; }

   /// Задать координату центра локомотива в пространстве
   void setCoord(dvec3 coord) { this->coord = coord; }

   /// Задать координату по железнодорожному пикетажу
   void setRailCoord(double rail_coord) { this->rail_coord = rail_coord; }

    /// Задать длину поезда
    void setTrainLength(double train_length)
    {
        this->train_length = train_length;
        if (speedmap)
            speedmap->setCurrentSearchDistance(train_length);
    }

   /// Задать конструкционную скорость
   void setMaxVelocity(double v_max) { this->v_max = v_max; }

   /// Загрузка станций из ЭК
   void loadStationsMap(QString path);

   double getTargetDistance() const { return target_dist; }

   double getRailCoord() const { return rail_coord / 1000.0; }

   int getStationIndex() const { return station_idx; }

   bool isTractionAllowed() const { return is_trac_allowed; }

    /// Текст в табло "станция"
    QString getStationText() const;

    /// Текст в табло информационной строки
    QString getInfoText() const;

   enum
   {
       NUM_SOUNDS = 2,
       ON_SOUND = 0,
       BUTTON_SOUND = 1
   };

   float getSoundSignal(size_t idx = ON_SOUND) const override
   {
       return sound_states[idx].createSoundSignal();
   }

   void setShuntingMode(int is_shnt_mode)
   {
       if (is_shunting_mode != is_shnt_mode)
       {
           is_shunting_mode = is_shnt_mode;
           epk_state.reset();
           safety_timer->start();
       }       
   }

   bool isShuntingMode() const
   {
       return is_shunting_mode;
   }

private:

   double U_pow = 0.0;

   double U_nom = 50.0;

   ALSN code_alsn = ALSN::NO_CODE;

   ALSN old_code_alsn = ALSN::NO_CODE;

   bool state_RB = false;

   bool state_RB_old = false;

   bool state_RBS = false;

   bool state_RBS_old = false;

   bool state_EPK = false;

   double v_kmh = 0.0;

   double v = 0.0;

   /// Шаг дифференцирования скорости
   double delta_t = 0.1;

   size_t v_count = 0;

   double t_diff = 0.0;

   double acceleration = 0.0;

   bool key_epk = false;

   bool key_epk_old = false;

   bool is_dislplay_ON = false;

   bool check_vigilance = false;

   double beep_interval = 0.5;

   Timer *beepTimer = new Timer(beep_interval, false);

   Timer *safety_timer = new Timer(45.0, false);

   double train_length = 22.532;

   /// Конструкционная скорость
   double v_max = 160.0;

   /// Текущее ограничение скорости
   double current_limit = 300.0;

   /// Следующее ограничение скорости
   double next_limit = 300.0;

   /// Текущее ограничение скорости при отсутствии кода АЛСН
   double no_code_limit = 41.0;

   /// Ограничение скорости при отсутствии кода АЛСН
   double no_code_limit_ref = 41.0;

   /// Счётчик пройденной дистанции
   double passed_distance = 0.0;

   /// Дистанция до следующей цели
   double target_dist = 0.0;

   /// Индекс станции из ЭК
   int station_idx = -1;

   /// Признак разрешения тяги
   bool is_trac_allowed = false;

    /// Модуль приёма сигналов АЛС с путевой топологии
    CoilALSN *coilALSN = nullptr;

    /// Модуль работы с ограничениями скорости на путевой топологии
    SpeedMap *speedmap = nullptr;

    /// Координата локомотива по железнодорожному пикетажу
    double rail_coord = 0.0;

    /// Положение центра локомотива в пространстве
    dvec3 coord = {0.0, 0.0, 0.0};

    /// Радиус поиска ближайшей станции
    double station_search_radius = 5000.0;

    /// База станций
    QVector<klub_station_t> stations;

    std::array<float, NUM_LAMPS> lamps = {0.0f, 0.0f, 0.0f, 0.0f,
                                          0.0f, 0.0f, 0.0f, 0.0f};

    enum
    {
        STATION_MAX_SYMBOLS = 8,
        INFO_MAX_SYMBOLS = 24,
    };

    /// Текст в табло "станция"
    QString station_text = QString("");

    /// Текст в табло информационной строки
    QString info_text = QString("");

   Trigger epk_state;

   Trigger is_red;

   enum
   {
       DIFF_NUM = 3
   };

   /// Мвссив значений скоростей для численного дифференцирования
   std::array<double, DIFF_NUM> v_i = {0.0, 0.0, 0.0};

   std::array<sound_state_t, NUM_SOUNDS> sound_states;

   void preStep(state_vector_t &Y, double t) override;

   void train_mode_process();

   void shunting_mode_process();

   void ode_system(const state_vector_t &Y,
                   state_vector_t &dYdt,
                   double t) override;

   void load_config(CfgReader &cfg) override;

   void alsn_process(ALSN code_alsn);

   /// Озвучка
   void sounds_process();

   /// Вычисление ускорения
   void calc_acceleration(double t, double dt);

   /// Расчет ограничений скорости путевой инфраструктуры
   void calc_speed_limits_by_speedmap();

   /// Расчет ограничений скорости от сигнала следующего светофора
   void calc_speed_limits_by_next_signal();

   /// Работа с ограничениями скорости
   void speed_control();

   /// Определение текущей станции
   void stations_process();

   /// Флаг маневрового режима
   bool is_shunting_mode = false;

private slots:

   void onSafetyTimer();

   void onBeepTimer();
};

#endif // KLUB_H
