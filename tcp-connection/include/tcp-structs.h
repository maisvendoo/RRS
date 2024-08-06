//-----------------------------------------------------------------------------
//
//      Структуры и функции необходимые как в клиенте так и в сервере
//      (c) РГУПС, ВЖД 29/11/2017
//      Разработал: Ковшиков С. В.
//
//-----------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Структуры и функции необходимые как в клиенте так и в сервере
 *  \copyright РГУПС, ВЖД
 *  \author Ковшиков С. В
 *  \date 29/11/2017
 */

#ifndef TCPSTRUCTS_H
#define TCPSTRUCTS_H

#include "a-tcp-namespace.h"

#include <type_traits>
#include <stddef.h>
#include <stdint.h>
#include <QByteArray>
#include <QTcpSocket>

typedef qint64 buf_size_t;


/*!
 * \struct tcp_cmd_t
 * \brief Структура запроса серверу с динамическим размером буффера
 */
struct tcp_cmd_t
{
    /*!
     * \struct info_t
     * \brief Подструктура храниния информации
     */
#pragma pack(push, 1)
    struct info_t
    {
        // Тип запроса
        ATcp::TcpCommand command;
        // Размер буффера данных
        buf_size_t bufferSize;

        /// Конструктор
        info_t()
            : command(ATcp::tcZERO)
            , bufferSize(0u)
        {

        }
    };
#pragma pack(pop)

    // Информационная структура
    info_t      info;
    // Буффер данных для отправки серверу
    QByteArray  buffer;

    // Размер инфо-части
    static const buf_size_t INFO_SIZE = sizeof(info_t);

    /// Конструктор
    tcp_cmd_t()
    {

    }

    /// Установить буффер из строки с заверш. нулём
    void setData(const char* _null_term_str)
    {
        setData(QByteArray(_null_term_str));
    }

    /// Установить буффер из char* заданной длины
    void setData(const char* _dat, buf_size_t _len)
    {
        setData(QByteArray(_dat, static_cast<int>(_len)));
    }

    /// Установить данные из строки
    void setData(QString _dat)
    {
        setData(_dat.toLocal8Bit());
    }

    /// Установить буффер из массива байт
    void setData(QByteArray _dat)
    {
        buffer = _dat;
        info.bufferSize = buffer.size();
    }

    /// Установить буффер из структуры. (Структура должна быть копируема)
    template<typename T>
    bool setData(T _dat)
    {
        if (std::is_trivially_copyable<T>::value)
        {
            buffer.resize(sizeof(T));
            memcpy(buffer.data(), &_dat, sizeof(T));
            info.bufferSize = sizeof(T);
            return true;
        }
        return false;
    }

    /// Сериализовать данные
    QByteArray toByteArray()
    {
        QByteArray buf(static_cast<int>(INFO_SIZE + info.bufferSize),
                       Qt::Uninitialized);

        memcpy(buf.data(), &info, sizeof(info_t));
        memcpy(buf.data() + INFO_SIZE, buffer.data(), info.bufferSize);

        return buf;
    }

    /// Проверить сокет на количество данных для чтения инфо-структуры
    static bool validInfoSize(QTcpSocket* _sock)
    {
        return static_cast<buf_size_t>(_sock->size()) >= INFO_SIZE;
    }

    /// Получить инфо-структуру
    static info_t extractInfo(QTcpSocket *_sock)
    {
        info_t tmp;
        _sock->read(reinterpret_cast<char*>(&tmp), sizeof(info_t));
        return tmp;
    }

    /// Проверить сокет на количество данных для чтения буффера
    static bool validBufferSize(buf_size_t _bufSz, QTcpSocket* _sock)
    {
        return _bufSz == static_cast<buf_size_t>(_sock->size());
    }
};

#endif // TCPSTRUCTS_H
