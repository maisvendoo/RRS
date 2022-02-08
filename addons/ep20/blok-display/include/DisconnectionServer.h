//-----------------------------------------------------------------------------
//
//      Экран потери связи
//      (c) РГУПС, ВЖД 18/07/2017
//      Разработал: Даглдиян Б.Д.
//
//-----------------------------------------------------------------------------
/*!
 * \file
 * \brief Экран потери связи
 * \copyright РГУПС, ВЖД
 * \author Даглдиян Б.Д.
 * \date 18/07/2017
 */

#ifndef DISCONNECTIONSERVER_H
#define DISCONNECTIONSERVER_H

#include <QLabel>
#include <QPainter>

/*!
 * \class DisconnectionServer
 * \brief Класс, вызываемый при потере связи с сервером
 */
class DisconnectionServer : public QLabel
{

public:
    /*!
     * \brief Конструктор.
     * \param size - размер элемента
     */
    DisconnectionServer(QSize size, QWidget* parent = Q_NULLPTR);

    /// Деструктор
    ~DisconnectionServer();


};

#endif // DISCONNECTIONSERVER_H
