#include    "translator.h"

#include    <QDir>
#include    <QDirIterator>
#include    <QTextCodec>
#include    <QTextDecoder>
#include    <QTextStream>

#include    <QDebug>

#include    "path-funcs.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Translator::Translator(const QString &routeDir)
{
    alphabet.insert(0x410, "A");
    alphabet.insert(0x411, "B");
    alphabet.insert(0x412, "V");
    alphabet.insert(0x413, "G");
    alphabet.insert(0x414, "D");
    alphabet.insert(0x415, "E");
    alphabet.insert(0x416, "Zh");
    alphabet.insert(0x417, "Z");
    alphabet.insert(0x418, "I");
    alphabet.insert(0x419, "J");
    alphabet.insert(0x41A, "K");
    alphabet.insert(0x41B, "L");
    alphabet.insert(0x41C, "M");
    alphabet.insert(0x41D, "N");
    alphabet.insert(0x41E, "O");
    alphabet.insert(0x41F, "P");
    alphabet.insert(0x420, "R");
    alphabet.insert(0x421, "S");
    alphabet.insert(0x422, "T");
    alphabet.insert(0x423, "U");
    alphabet.insert(0x424, "F");
    alphabet.insert(0x425, "H");
    alphabet.insert(0x426, "C");
    alphabet.insert(0x427, "Ch");
    alphabet.insert(0x428, "Sh");
    alphabet.insert(0x429, "Shj");
    alphabet.insert(0x42A, "");
    alphabet.insert(0x42B, "Y");
    alphabet.insert(0x42C, "");
    alphabet.insert(0x42D, "E");
    alphabet.insert(0x42E, "Yu");
    alphabet.insert(0x42F, "Ya");

    alphabet.insert(0x430, "a");
    alphabet.insert(0x431, "b");
    alphabet.insert(0x432, "v");
    alphabet.insert(0x433, "g");
    alphabet.insert(0x434, "d");
    alphabet.insert(0x435, "e");
    alphabet.insert(0x436, "zh");
    alphabet.insert(0x437, "z");
    alphabet.insert(0x438, "i");
    alphabet.insert(0x439, "j");
    alphabet.insert(0x43A, "k");
    alphabet.insert(0x43B, "l");
    alphabet.insert(0x43C, "m");
    alphabet.insert(0x43D, "n");
    alphabet.insert(0x43E, "o");
    alphabet.insert(0x43F, "p");
    alphabet.insert(0x440, "r");
    alphabet.insert(0x441, "s");
    alphabet.insert(0x442, "t");
    alphabet.insert(0x443, "u");
    alphabet.insert(0x444, "f");
    alphabet.insert(0x445, "x");
    alphabet.insert(0x446, "c");
    alphabet.insert(0x447, "ch");
    alphabet.insert(0x448, "sh");
    alphabet.insert(0x449, "shj");
    alphabet.insert(0x44A, "");
    alphabet.insert(0x44B, "y");
    alphabet.insert(0x44C, "");
    alphabet.insert(0x44D, "e");
    alphabet.insert(0x44E, "yu");
    alphabet.insert(0x44F, "ya");

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
        QTextCodec *codec_1251 = QTextCodec::codecForName("Windows-1251");
        QString tmp = codec_1251->toUnicode(data);

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
