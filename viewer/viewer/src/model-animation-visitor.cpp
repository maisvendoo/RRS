#include    "model-animation-visitor.h"

#include    <QString>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ModelAnimationVisitor::ModelAnimationVisitor(model_parts_list_t *parts,
                                             const std::string &anim_name)
    : osg::NodeVisitor()
    , parts(parts)
    , anim_name(anim_name)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelAnimationVisitor::apply(osg::Transform &transform)
{
    osg::MatrixTransform *matrix_trans = static_cast<osg::MatrixTransform *>(&transform);
    std::string name = matrix_trans->getName();

    QString n = QString(name.c_str());

    size_t pos = name.find(anim_name);

    if (pos == 0)
    {
        parts->push_back(new ModelPartAnimation(matrix_trans));
    }

    traverse(transform);
}
