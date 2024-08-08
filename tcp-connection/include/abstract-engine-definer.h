//-----------------------------------------------------------------------------
//
//      Класс-фабрика для определения механизма подготовки данных
//      (c) РГУПС, ВЖД 29/11/2017
//      Разработал: Ковшиков С. В.
//
//-----------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Класс-фабрика для определения механизма подготовки данных
 *  \copyright РГУПС, ВЖД
 *  \author Ковшиков С. В
 *  \date 29/11/2017
 */

#ifndef ABSTRACT_ENGINE_DEFINER_H
#define ABSTRACT_ENGINE_DEFINER_H

#include <qcompilerdetection.h>
#include <QString>

class AbstractDataEngine;
class AbstractClientDelegate;

#if defined(TCPCONNECTION_LIB)
    #define ENGINE_DEFINER_EX Q_DECL_EXPORT
#else
    #define ENGINE_DEFINER_EX Q_DECL_IMPORT
#endif


/*!
 * \class AbstractEngineDefiner
 * \brief Класс-фабрика для определения механизма подготовки данных
 */
class ENGINE_DEFINER_EX AbstractEngineDefiner
{
public:
    /// Конструктор
    AbstractEngineDefiner();
    /// Деструктор
    virtual ~AbstractEngineDefiner();

    /// Установить механизм подготовки данных для данного клиента
    void setDataEngine(AbstractClientDelegate* client);


protected:
    /// Вернуть необходимый механизм подготовки данных
    virtual AbstractDataEngine* getDataEngine_(QString name) = 0;
    /*
        Данный метод необходимо переопределить в наследниках класса.
        Классы механизмов данных должны быть унаследованы от AbstractDataEngine
        Содержание метода должно иметь схожий вид со следующим:

        AbstractDataEngine* MyEngineDefiner::getDataEngine_(QString name)
        {
            if (name == "client1")
                return new FirstDataEngine();
            if (name == "client2")
                return new SecondDataEngine();
            return new NullDataEngine();
        }
     */
};


/*!
 * \class NullDataEngineDefiner
 * \brief Класс-заглушка всегда дающий NullDataEngine
 */
class NullDataEngineDefiner Q_DECL_FINAL : public AbstractEngineDefiner
{
public:
    /// Конструктор
    NullDataEngineDefiner();


protected:
    /// Вернуть нулевой механизм подготовки данных
    AbstractDataEngine* getDataEngine_(QString clientName) Q_DECL_OVERRIDE;
};

#endif // ABSTRACT_ENGINE_DEFINER_H
