#include    "pdfviewer.h"
#include    <QPdfDocument>
#include    <QApplication>
#include    <QScrollBar>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PdfViewer::PdfViewer(QWidget *parent) : QScrollArea(parent)
    , m_cache(30)
{
    m_content = new QWidget(this);
    m_layout = new QVBoxLayout(m_content);
    m_layout->setSpacing(12);
    m_layout->setContentsMargins(12, 12, 12, 12);

    setWidget(m_content);
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    setStyleSheet("QScrollArea { border: none; background: transparent; }");    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PdfViewer::load(const QString &filePath)
{
    if (m_doc.status() == QPdfDocument::Status::Loading)
    {
        return false;
    }

    m_cache.clear(); // QCache автоматически удалит объекты
    qDeleteAll(m_labels);
    m_labels.clear();

    if (m_doc.load(filePath) == QPdfDocument::Error::FileNotFound)
    {
        return false;
    }

    rebuildLayout();
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PdfViewer::setScale(qreal scale)
{
    m_scale = std::clamp(scale, 0.5, 5.0);
    m_cache.clear();
    rebuildLayout();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int PdfViewer::pageCount() const
{
    return m_doc.pageCount();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PdfViewer::rebuildLayout()
{
    const int pages = m_doc.pageCount();
    m_labels.reserve(pages);

    for (int i = 0; i < pages; ++i)
    {
        // Правильный API QtPdf: размер в поинтах (1/72 дюйма)
        QSizeF sizePoints = m_doc.pagePointSize(i);
        QSize pxSize = (sizePoints * m_scale).toSize();

        auto* label = new QLabel(m_content);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("background: white; border: 1px solid #e0e0e0; border-radius: 4px;");
        label->setMinimumSize(pxSize);
        label->setFixedSize(pxSize); // Фиксируем размер, чтобы QLabel не растягивался
        m_layout->addWidget(label);
        m_labels.append(label);

        renderPage(i);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PdfViewer::renderPage(int index)
{
    if (index < 0 || index >= m_doc.pageCount())
    {
        return;
    }

    if (m_cache.contains(index))
    {
        m_labels[index]->setPixmap(QPixmap::fromImage(*m_cache.object(index)));
        return;
    }

    // Рендерим напрямую из QPdfDocument
    QSizeF sizePoints = m_doc.pagePointSize(index);
    QSize renderSize = (sizePoints * m_scale).toSize();
    QImage img = m_doc.render(index, renderSize);

    // QCache принимает указатель и берёт владение
    m_cache.insert(index, new QImage(img));
    m_labels[index]->setPixmap(QPixmap::fromImage(img));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PdfViewer::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier)
    {
        qreal delta = event->angleDelta().y() / 1200.0;
        setScale(m_scale + delta);
        event->accept();
    }
    else
    {
        QScrollArea::wheelEvent(event);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PdfViewer::adjustWindowToPageWidth(int pageIndex)
{
    if (m_doc.status() != QPdfDocument::Status::Ready || pageIndex >= m_doc.pageCount())
    {
        return;
    }

    double pageWidthPts = m_doc.pagePointSize(pageIndex).width();
    int pagePx = static_cast<int>(pageWidthPts * m_scale);

    int margins = m_layout->contentsMargins().left() + m_layout->contentsMargins().right();
    int scrollW = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;
    int contentWidth = pagePx + margins + scrollW;

    QWidget* win = this->window();
    if (!win) return;

    int chromeW = win->frameGeometry().width() - win->geometry().width();

    if (chromeW <= 0)
    {
        chromeW = 12;
    }

    win->resize(contentWidth + chromeW, win->height());
}

