//-----------------------------------------------------------------------------
//
//      Класс статической картинки
//      (c) РГУПС, ВЖД 07/03/2017
//      Разработал: Ковшиков С. В.
//
//-----------------------------------------------------------------------------

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H


#include <QLabel>
#include <QPixmap>
#include <QStaticText>
#include <QPainter>
#include <QFile>


class ImageWidget : public QLabel
{
    Q_OBJECT

public:
    ImageWidget(QString resprefix, QString imgname, QSize imgsize,
                QWidget* parent = Q_NULLPTR);
    virtual ~ImageWidget();

    // Установка картинки
    void setImage(QString resprefix, QString imgname);

    // Получение префикса
    QString resourcePrefix();


private:
    // Префикс для загрузки из файла ресурсов
    QString resourcePrefix_;

    // Имя картинки для загрузки из файла ресурсов
    QString imageName_;

    // Метод создания картинки с ошибкой
    void errorImage_(QPixmap *pix);

};

#endif // IMAGEWIDGET_H
