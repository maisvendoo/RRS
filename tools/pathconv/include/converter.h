#ifndef     CONVERTER_H
#define     CONVERTER_H

#include    <QString>
#include    <QMap>
#include    <QVector>

enum LineType
{
    CommentLine,
    OptionLine,
    EmptyLine,
    RefLine
};

struct objects_ref_line_t
{
    LineType type;
    QString  content;
};

class Converter
{
public:

    Converter(const QString &routeDir);
    ~Converter();

private:

    QString routeDirectory;

    QMap<QString, QString> model_names;
    QMap<QString, QString> texture_names;    

    QVector<objects_ref_line_t> ref_lines;

    void renameModels();

    void process(const QString &routeDir);

    bool readObjectsRef(const QString &path);    

    bool rewriteObjectsRef();
};

#endif // CONVERTER_H
