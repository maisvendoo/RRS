#ifndef     TRANSLATOR_H
#define     TRANSLATOR_H

#include    <QString>
#include    <QMap>

class Translator
{
public:

    Translator(const QString& routeDir);
    ~Translator();

private:

    QString routeDirectory;
    QMap<QChar, QString> alphabet;

    void process(const QString& routeDir);

    bool translateFileContent(const QString& path,
                              std::function<bool(QString&)> check_no_transliterate);

    bool translateFiles(const QString& routeDir);

    QString latin(QString& str);
};

#endif // TRANSLATOR_H
