#include    "converter.h"

#include    <QFile>
#include    <QVariant>

#include    "path-utils.h"
#include    "CfgEditor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeModelsConfig()
{
    std::string path = compinePath(topologyDir, FILE_DEFAULT_OBJ);

    QFile file_old(QString(path.c_str()));
    if (file_old.exists())
    {
        std::string backup = FILE_BACKUP_PREFIX + FILE_DEFAULT_OBJ + FILE_BACKUP_EXTENTION;
        file_old.rename( QString(compinePath(topologyDir, backup).c_str()) );
    }

    CfgEditor editor;
    editor.openFileForWrite(QString(path.c_str()));
    editor.setIndentationFormat(-1);

    QString node_models = "Models";

    QString node2_sig_model = "SignalModelsDir";
    QString node2_sig_model_value = "default-objects";

    QString node2_sig_anim = "SignalAnimationsDir";
    QString node2_sig_anim_value = "default-objects";

    FieldsDataList fdl;
    fdl.append(QPair<QString, QString>(node2_sig_model, node2_sig_model_value));
    fdl.append(QPair<QString, QString>(node2_sig_anim, node2_sig_anim_value));
    editor.writeFile(node_models, fdl);

    editor.closeFileAfterWrite();
}
