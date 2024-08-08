//-----------------------------------------------------------------------------
//
//      Базовый класс подготовки данных для отправки клиентам
//      (c) РГУПС, ВЖД 29/11/2017
//      Разработал: Ковшиков С. В.
//
//-----------------------------------------------------------------------------
/*!
 * \file
 * \brief Базовый класс подготовки данных для отправки клиентам
 * \copyright РГУПС, ВЖД
 * \author Ковшиков С. В.
 * \date 29/11/2017
 */

#ifndef ABSTRACT_DATA_ENGINE_H
#define ABSTRACT_DATA_ENGINE_H

#include <QObject>
#include <QByteArray>
#include <QMutex>


#if defined(TCPCONNECTION_LIB)
    #define DATA_ENGINE_EX Q_DECL_EXPORT
#else
    #define DATA_ENGINE_EX Q_DECL_IMPORT
#endif


/*!
 * \class AbstractDataPrepareEngine
 * \brief Реализация класса подготовки данных для отправки клиентам
 */
class DATA_ENGINE_EX AbstractDataEngine : public QObject
{
    Q_OBJECT
public:
    /// Конструктор
    AbstractDataEngine();
    /// Деструктор
    ~AbstractDataEngine();

    /// Вернуть подготовленные данные
    virtual QByteArray getPreparedData() = 0;

    /// Установить данные, принятые от клиента
    void setInputBuffer(QByteArray inData);

    /// Установить данные, для отправки клиенту
    void setOutputBuffer(QByteArray outData);

    /// Вернуть буффер полученный от клиента
    QByteArray getInputBuffer();

    /// Вернуть буффер, предназначенный для отправки клиенту
    QByteArray getOutputBuffer();


protected:
    //
    QMutex inMutex_;
    //
    QMutex outMutex_;

    // Буффер данных принятых от клиента
    QByteArray inputBuffer_; ///< Буффер данных принятых от клиента
    // Буффер данных для отправки клиенту
    QByteArray outputBuffer_; ///< Буффер данных для отправки клиенту
};



/*!
 * \class NullDataPrepareEngine
 * \brief Реализация класса отсутствия подготовки данных
 */
class DATA_ENGINE_EX NullDataEngine Q_DECL_FINAL : public AbstractDataEngine
{
    Q_OBJECT
public:
    /// Конструктор
    NullDataEngine();

    /// Вернуть пустой массив данных
    QByteArray getPreparedData() Q_DECL_OVERRIDE;

};
#endif // ABSTRACT_DATA_ENGINE_H
