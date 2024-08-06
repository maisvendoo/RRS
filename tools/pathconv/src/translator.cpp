#include    "translator.h"

#include    <QDir>
#include    <QDirIterator>
#include    <QStringConverter>
#include    <QTextStream>

#include    <QDebug>

#include    <path-funcs.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Translator::Translator(const QString &routeDir)
{
    alphabet.insert(QChar(0x410), "A");
    alphabet.insert(QChar(0x411), "B");
    alphabet.insert(QChar(0x412), "V");
    alphabet.insert(QChar(0x413), "G");
    alphabet.insert(QChar(0x414), "D");
    alphabet.insert(QChar(0x415), "E");
    alphabet.insert(QChar(0x416), "Zh");
    alphabet.insert(QChar(0x417), "Z");
    alphabet.insert(QChar(0x418), "I");
    alphabet.insert(QChar(0x419), "J");
    alphabet.insert(QChar(0x41A), "K");
    alphabet.insert(QChar(0x41B), "L");
    alphabet.insert(QChar(0x41C), "M");
    alphabet.insert(QChar(0x41D), "N");
    alphabet.insert(QChar(0x41E), "O");
    alphabet.insert(QChar(0x41F), "P");
    alphabet.insert(QChar(0x420), "R");
    alphabet.insert(QChar(0x421), "S");
    alphabet.insert(QChar(0x422), "T");
    alphabet.insert(QChar(0x423), "U");
    alphabet.insert(QChar(0x424), "F");
    alphabet.insert(QChar(0x425), "H");
    alphabet.insert(QChar(0x426), "C");
    alphabet.insert(QChar(0x427), "Ch");
    alphabet.insert(QChar(0x428), "Sh");
    alphabet.insert(QChar(0x429), "Shj");
    alphabet.insert(QChar(0x42A), "");
    alphabet.insert(QChar(0x42B), "Y");
    alphabet.insert(QChar(0x42C), "");
    alphabet.insert(QChar(0x42D), "E");
    alphabet.insert(QChar(0x42E), "Yu");
    alphabet.insert(QChar(0x42F), "Ya");

    alphabet.insert(QChar(0x430), "a");
    alphabet.insert(QChar(0x431), "b");
    alphabet.insert(QChar(0x432), "v");
    alphabet.insert(QChar(0x433), "g");
    alphabet.insert(QChar(0x434), "d");
    alphabet.insert(QChar(0x435), "e");
    alphabet.insert(QChar(0x436), "zh");
    alphabet.insert(QChar(0x437), "z");
    alphabet.insert(QChar(0x438), "i");
    alphabet.insert(QChar(0x439), "j");
    alphabet.insert(QChar(0x43A), "k");
    alphabet.insert(QChar(0x43B), "l");
    alphabet.insert(QChar(0x43C), "m");
    alphabet.insert(QChar(0x43D), "n");
    alphabet.insert(QChar(0x43E), "o");
    alphabet.insert(QChar(0x43F), "p");
    alphabet.insert(QChar(0x440), "r");
    alphabet.insert(QChar(0x441), "s");
    alphabet.insert(QChar(0x442), "t");
    alphabet.insert(QChar(0x443), "u");
    alphabet.insert(QChar(0x444), "f");
    alphabet.insert(QChar(0x445), "x");
    alphabet.insert(QChar(0x446), "c");
    alphabet.insert(QChar(0x447), "ch");
    alphabet.insert(QChar(0x448), "sh");
    alphabet.insert(QChar(0x449), "shj");
    alphabet.insert(QChar(0x44A), "");
    alphabet.insert(QChar(0x44B), "y");
    alphabet.insert(QChar(0x44C), "");
    alphabet.insert(QChar(0x44D), "e");
    alphabet.insert(QChar(0x44E), "yu");
    alphabet.insert(QChar(0x44F), "ya");

    process(routeDir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Translator::~Translator()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Translator::process(const QString &routeDir)
{
    routeDirectory = routeDir;

    if (!QDir(routeDirectory).exists())
    {
        qDebug() << "Route directory is't found\n";
        return;
    }

    if (*(routeDir.end() - 1) != QDir::separator())
        routeDirectory += QDir::separator();

    translateObjectsRef(routeDirectory + "objects.ref");
    translateFiles(routeDirectory);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Translator::translateObjectsRef(const QString &path)
{
    QFile file(path);

    QString new_data = "";

    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray data = file.readAll();
        auto toUtf8 = QStringDecoder(QStringConverter::Utf8);
        QString tmp = toUtf8(data);

        new_data = latin(tmp);

        file.close();
    }

    QFile objRef(routeDirectory + "objects.ref");

    if (!objRef.open(QIODevice::WriteOnly))
        return false;

    QTextStream stream(&objRef);

    stream << new_data;

    objRef.close();

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Translator::translateFiles(const QString &routeDir)
{
    QString modelsDir = routeDir + "models";
    QString texturesDir = routeDir + "textures";

    QDir models(modelsDir);
    QDirIterator model_files(models.path(), QStringList() << "*.dmd",
                             QDir::NoDotAndDotDot | QDir::Files,
                             QDirIterator::Subdirectories);

    while (model_files.hasNext())
    {
        QString old_path = model_files.next();
        QFile file(old_path);

        QString new_path = latin(old_path);

        if (file.rename(new_path))
        {
            qDebug() << "Renamed " << old_path << " to " << new_path << "\n";
        }
    }

    QDir textures(texturesDir);
    QDirIterator texture_files(textures.path(), QStringList() << "*.bmp" << "*.tga",
                               QDir::NoDotAndDotDot | QDir::Files,
                               QDirIterator::Subdirectories);

    while (texture_files.hasNext())
    {
        QString old_path = texture_files.next();
        QFile file(old_path);

        QString new_path = latin(old_path);

        if (file.rename(new_path))
        {
            qDebug() << "Renamed " << old_path << " to " << new_path << "\n";
        }
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Translator::latin(QString str)
{
    QString result;

    foreach (QChar ch, str)
    {
        if (alphabet.contains(ch))
            result += alphabet[ch];
        else
            result += ch;
    }

    return result;
}
