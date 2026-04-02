#ifndef CFG_READER2_H
#define CFG_READER2_H

#include <QHash>
#include <QString>
#include <QStringList>
#include <Qt>

#if defined(CFG_READER_LIB)
    #define CFG_READER_EXPORT Q_DECL_EXPORT
#else
    #define CFG_READER_EXPORT Q_DECL_IMPORT
#endif

class CFG_READER_EXPORT CfgReader2
{
public:
    CfgReader2();
    ~CfgReader2();

    bool load(const QString& path);

    bool get_first_section(const QString& section);
    bool get_next_section();

    bool get_bool(const QString& field, bool& value);
    bool get_int(const QString& field, int& value);
    bool get_float(const QString& field, float& value);
    bool get_double(const QString& field, double& value);
    bool get_string(const QString& field, QString& value);

    bool get_bool(const QString& section, const QString& field, bool& value);
    bool get_int(const QString& section, const QString& field, int& value);
    bool get_float(const QString& section, const QString& field, float& value);
    bool get_double(const QString& section, const QString& field, double& value);
    bool get_string(const QString& section, const QString& field, QString& value);

private:
    bool parse_xml(const QString& path);
    QString normalize_value(const QString& value);

private:
    struct SectionData
    {
        QHash<QString, QString> fields;
    };

    QHash<QString, SectionData> sections;
    QStringList section_order;
    int current_section_index;
};

#endif // CFG_READER2_H
