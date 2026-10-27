#ifndef BLEND_IMPORTER_H
#define BLEND_IMPORTER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QTemporaryFile>

//------------------------------------------------------------------------------
//
//  Импорт .blend: headless-конвертация во внешний Blender
//  (blender --background <файл> --python <скрипт> -- <результат.glb>).
//  Python-скрипт экспорта создаётся во временном файле QTemporaryFile
//  и делает bpy.ops.export.scene.gltf(filepath=..., export_format='GLB').
//  Готовый GLB кладётся рядом с исходным .blend (<имя>.glb), чтобы
//  модель жила дольше сеанса и попадала в пакет при экспорте.
//
//------------------------------------------------------------------------------
class BlendImporter : public QObject
{
    Q_OBJECT

public:
    explicit BlendImporter(QObject* parent = nullptr);

    /// Запустить конвертацию .blend -> GLB; false, если импорт уже идёт
    bool start(const QString& blend_path, const QString& blender_path);

    /// Конвертация выполняется
    bool isRunning() const;

signals:
    /// Конвертация завершена: ok, путь к GLB, текст ошибки (вывод Blender)
    void finished(bool ok, const QString& glb_path, const QString& error);

private slots:
    /// Процесс Blender завершился
    void slotFinished(int exit_code, QProcess::ExitStatus status);

    /// Ошибка процесса (в том числе FailedToStart — Blender не найден)
    void slotErrorOccurred(QProcess::ProcessError error);

private:
    QProcess process;          ///< Процесс Blender
    QTemporaryFile scriptFile; ///< Временный Python-скрипт экспорта
    QString blendPath;         ///< Исходный .blend
    QString targetGlb;         ///< Целевой GLB рядом с .blend
    bool resultEmitted = false; ///< Защита от повторного сигнала finished
};

#endif // BLEND_IMPORTER_H
