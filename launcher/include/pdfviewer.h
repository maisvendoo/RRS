#ifndef     PDF_VIEWER_H
#define     PDF_VIEWER_H

#include    <QScrollArea>
#include    <QPdfDocument>
#include    <QCache>

class       QLabel;
class       QVBoxLayout;
class       QWheelEvent;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class PdfViewer : public QScrollArea
{
    Q_OBJECT

public:

    explicit PdfViewer(QWidget *parent = nullptr);

    bool load(const QString &filePath);

    void setScale(qreal scale);

    int pageCount() const;

    void adjustWindowToPageWidth(int pageIndex);

protected:

    void wheelEvent(QWheelEvent *event) override;

private:

    void renderPage(int index);

    void rebuildLayout();


    QPdfDocument m_doc;
    QWidget *m_content;
    QVBoxLayout *m_layout;
    QCache<int, QImage> m_cache; // Qt6 QCache хранит указатели
    QList<QLabel*> m_labels;
    qreal m_scale = 2.0; // ~144 DPI
};

#endif // PDF_VIEWER_H
