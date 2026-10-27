#include "BlendImporter.h"

#include <QDir>
#include <QFileInfo>

//------------------------------------------------------------------------------
//
//  Конвертация .blend в GLB внешним Blender в фоновом режиме.
//
//------------------------------------------------------------------------------
namespace
{

/// Python-скрипт экспорта glTF: путь результата передаётся после "--"
/// (только ASCII, чтобы не зависеть от кодировки скрипта)
const char* EXPORT_SCRIPT =
        "import sys\n"
        "import bpy\n"
        "\n"
        "out_path = sys.argv[sys.argv.index('--') + 1]\n"
        "bpy.ops.export.scene.gltf(filepath=out_path, export_format='GLB')\n";

/// Хвост вывода процесса для сообщения об ошибке (без середины)
QString tailOf(const QByteArray& output, int max_chars)
{
    const QString text = QString::fromUtf8(output).trimmed();
    const int length = text.length();

    if (length <= max_chars)
    {
        return text;
    }

    return QStringLiteral("...") + text.right(max_chars);
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BlendImporter::BlendImporter(QObject* parent)
    : QObject(parent)
{
    // Вывод Blender (stdout+stderr) в один канал — показываем при ошибке
    process.setProcessChannelMode(QProcess::MergedChannels);

    connect(&process, &QProcess::finished,
            this, &BlendImporter::slotFinished);

    connect(&process, &QProcess::errorOccurred,
            this, &BlendImporter::slotErrorOccurred);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool BlendImporter::isRunning() const
{
    return process.state() != QProcess::NotRunning;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool BlendImporter::start(const QString& blend_path,
                          const QString& blender_path)
{
    if (isRunning())
    {
        return false;
    }

    if (blend_path.isEmpty() ||
        !QFileInfo::exists(blend_path))
    {
        emit finished(false, QString(),
                      QStringLiteral("Файл не найден: %1").arg(blend_path));

        return true;
    }

    // Результат — GLB рядом с исходным .blend
    const QFileInfo blend_info(blend_path);
    targetGlb = blend_info.absoluteDir().filePath(
                blend_info.completeBaseName() + QStringLiteral(".glb"));
    blendPath = blend_path;

    // Временный Python-скрипт экспорта (удаляется при разрушении объекта)
    if (scriptFile.isOpen())
    {
        scriptFile.close();
    }

    scriptFile.setFileTemplate(
                QDir::tempPath() + QStringLiteral("/rrs_gltf_export_XXXXXX.py"));

    if (!scriptFile.open())
    {
        emit finished(false, QString(),
                      QStringLiteral("Не удалось создать временный "
                                     "Python-скрипт экспорта"));

        return true;
    }

    scriptFile.write(EXPORT_SCRIPT);
    scriptFile.flush();

    resultEmitted = false;

    // blender --background <файл> --python <скрипт> -- <результат.glb>
    process.start(blender_path,
                  {QStringLiteral("--background"),
                   blendPath,
                   QStringLiteral("--python"),
                   scriptFile.fileName(),
                   QStringLiteral("--"),
                   targetGlb});

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BlendImporter::slotFinished(int exit_code,
                                 QProcess::ExitStatus status)
{
    if (resultEmitted)
    {
        return;
    }

    resultEmitted = true;

    // Вывод процесса больше не нужен, скрипт можно закрыть
    if (scriptFile.isOpen())
    {
        scriptFile.close();
    }

    const QString output = tailOf(process.readAll(), 4000);

    const bool ok = (status == QProcess::NormalExit) &&
                    (exit_code == 0) &&
                    QFileInfo::exists(targetGlb);

    if (ok)
    {
        emit finished(true, targetGlb, QString());
        return;
    }

    QString error = tr("Blender завершился с кодом %1.")
                        .arg(exit_code);

    if (status == QProcess::CrashExit)
    {
        error = tr("Процесс Blender аварийно завершился.");
    }

    if (!QFileInfo::exists(targetGlb))
    {
        error += tr(" GLB-файл не создан: %1").arg(targetGlb);
    }

    if (!output.isEmpty())
    {
        error += QStringLiteral("\n\n") + output;
    }

    emit finished(false, QString(), error);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BlendImporter::slotErrorOccurred(QProcess::ProcessError error)
{
    if (resultEmitted || error != QProcess::FailedToStart)
    {
        // Остальные ошибки завершатся сигналом finished
        return;
    }

    resultEmitted = true;

    emit finished(false, QString(),
                  tr("Не удалось запустить Blender. Проверьте путь "
                     "(поле «Blender» на вкладке «Модель» или переменную "
                     "PATH)."));
}
