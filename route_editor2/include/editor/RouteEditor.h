#ifndef ROUTE_EDITOR_H
#define ROUTE_EDITOR_H

#include <vsg/core/ref_ptr.h>

namespace vsg
{

class Options;

}

class RouteEditor
{
public:
    RouteEditor();
    ~RouteEditor();

private:
    vsg::ref_ptr<vsg::Options> vsg_options;

private:
    void initialize_journal() const;
};

#endif // ROUTE_EDITOR_H
