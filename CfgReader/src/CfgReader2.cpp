#include "CfgReader2.h"

#include "convert.h"

#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QString>
#include <QXmlStreamReader>

CfgReader2::CfgReader2()
    : current_section_index(-1)
{
}

CfgReader2::~CfgReader2() = default;

bool CfgReader2::load(const QString& path)
{
    return parse_xml(path);
}

bool CfgReader2::get_first_section(const QString& section)
{
    if (sections.contains(section))
    {
        current_section_index = section_order.indexOf(section);
        return true;
    }

    current_section_index = -1;
    return false;
}

bool CfgReader2::get_next_section()
{
    if (current_section_index < 0 || current_section_index >= section_order.size() - 1)
    {
        return false;
    }

    const QString next_section = section_order.at(current_section_index + 1);
    if (sections.contains(next_section))
    {
        ++current_section_index;
        return true;
    }

    return false;
}

bool CfgReader2::get_bool(const QString& field, bool& value)
{
    QString tmp;
    if (!get_string(field, tmp))
    {
        return false;
    }

    tmp = normalize_value(tmp).toLower();

    if (tmp == "true" || tmp == "1")
    {
        value = true;
        return true;
    }

    if (tmp == "false" || tmp == "0")
    {
        value = false;
        return true;
    }

    return false;
}

bool CfgReader2::get_int(const QString& field, int& value)
{
    QString tmp;
    if (!get_string(field, tmp))
    {
        return false;
    }

    return TextToInt(tmp, value);
}

bool CfgReader2::get_float(const QString& field, float& value)
{
    QString tmp;
    if (!get_string(field, tmp))
    {
        return false;
    }

    return TextToFloat(tmp, value);
}

bool CfgReader2::get_double(const QString& field, double& value)
{
    QString tmp;
    if (!get_string(field, tmp))
    {
        return false;
    }

    return TextToDouble(tmp, value);
}

bool CfgReader2::get_string(const QString& field, QString& value)
{
    if (current_section_index < 0 || current_section_index >= section_order.size())
    {
        return false;
    }

    const QString section_name = section_order.at(current_section_index);
    const SectionData& data = sections[section_name];

    if (data.fields.contains(field))
    {
        value = data.fields[field];
        return true;
    }

    return false;
}

bool CfgReader2::get_bool(const QString& section, const QString& field, bool& value)
{
    if (!get_first_section(section))
    {
        return false;
    }

    return get_bool(field, value);
}

bool CfgReader2::get_int(const QString& section, const QString& field, int& value)
{
    QString tmp;
    if (!get_string(section, field, tmp))
    {
        return false;
    }

    return TextToInt(tmp, value);
}

bool CfgReader2::get_float(const QString& section, const QString& field, float& value)
{
    QString tmp;
    if (!get_string(section, field, tmp))
    {
        return false;
    }

    return TextToFloat(tmp, value);
}

bool CfgReader2::get_double(const QString& section, const QString& field, double& value)
{
    QString tmp;
    if (!get_string(section, field, tmp))
    {
        return false;
    }

    return TextToDouble(tmp, value);
}

bool CfgReader2::get_string(const QString& section, const QString& field, QString& value)
{
    if (!get_first_section(section))
    {
        return false;
    }

    return get_string(field, value);
}

bool CfgReader2::parse_xml(const QString& path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly))
    {
        qDebug() << "Failed to open config file:" << path;
        return false;
    }

    QXmlStreamReader reader(&file);

    // Очищаем предыдущие данные
    sections.clear();
    section_order.clear();
    current_section_index = -1;

    QString current_section;
    SectionData current_data;
    bool in_config = false;

    while (!reader.atEnd() && !reader.hasError())
    {
        const QXmlStreamReader::TokenType token = reader.readNext();
        if (token == QXmlStreamReader::StartElement)
        {
            const QString element_name = reader.name().toString();

            if (!in_config)
            {
                // Ищем корневой элемент Config
                if (element_name == "Config")
                {
                    in_config = true;
                }
                else
                {
                    // Неправильный формат файла
                    qDebug() << "Invalid config file: root element is not 'Config'";
                    return false;
                }
            }
            else
            {
                // Внутри Config: либо секция, либо поле верхнего уровня
                // Проверяем, есть ли у элемента дочерние элементы (это секция)
                bool has_children = false;

                const QXmlStreamReader::TokenType next_token = reader.readNext();
                if (next_token ==  QXmlStreamReader::StartElement)
                {
                    has_children = true;
                }
                else if (next_token == QXmlStreamReader::Characters)
                {
                    // Проверяем текстовое содержимое
                    const QString text = reader.text().toString().trimmed();
                    if (!text.isEmpty())
                    {
                        // Это поле верхнего уровня (без секции)
                        current_data.fields[element_name] = text;
                    }

                    // Пропускаем до конца элемента
                    while (!(reader.tokenType() == QXmlStreamReader::EndElement
                             && reader.name() == element_name))
                    {
                        reader.readNext();
                    }
                }

                reader.readNext(); // возвращаемся назад или продолжаем

                if (has_children)
                {
                    // Это секция с дочерними элементами
                    if (!current_section.isEmpty())
                    {
                        // Сохраняем предыдущую секцию
                        sections[current_section] = current_data;
                        section_order.append(current_section);
                    }

                    current_section = element_name;
                    current_data.fields.clear();

                    // Читаем все поля внутри секции
                    while (!(reader.tokenType() == QXmlStreamReader::EndElement
                             && reader.name() == current_section)
                           && !reader.atEnd() && !reader.hasError())
                    {
                        reader.readNext();

                        if (reader.tokenType() == QXmlStreamReader::StartElement)
                        {
                            const QString field_name = reader.name().toString();
                            QString field_value;

                            // Читаем значение поля
                            reader.readNext();
                            if (reader.tokenType() == QXmlStreamReader::Characters)
                            {
                                field_value = reader.text().toString();
                            }
                            current_data.fields[field_name] = field_value;

                            // Пропускаем до конца поля
                            while (!(reader.tokenType() == QXmlStreamReader::EndElement
                                     && reader.name() == field_name)
                                   && !reader.atEnd())
                            {
                                reader.readNext();
                            }
                        }
                    }
                }
            }
        }
    }

    // Сохраняем последнюю секцию
    if (!current_section.isEmpty())
    {
        sections[current_section] = current_data;
        section_order.append(current_section);
    }

    if (reader.hasError())
    {
        qDebug() << "XML parsing error:" << reader.errorString() << "in file:" << path;
        return false;
    }

    return in_config && !sections.isEmpty();
}

QString CfgReader2::normalize_value(const QString& value)
{
    QString result = value;
    result = result.trimmed();
    // Убираем лишние пробелы внутри (но сохраняем одиночные)
    result.replace(QRegularExpression("\\s+"), " ");
    return result;
}

