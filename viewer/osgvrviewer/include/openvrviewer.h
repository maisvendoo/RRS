/*
 * openvrviewer.h
 *
 *  Created on: Dec 18, 2015
 *      Author: Chris Denham
 */

#ifndef     OSG_OPENVRVIEWER_H
#define     OSG_OPENVRVIEWER_H

#include    <osg/Group>

#include    "openvrdevice.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
namespace osgViewer
{
	class View;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class OpenVRViewer : public osg::Group
{
public:

    OpenVRViewer(osgViewer::View* view,
                 osg::ref_ptr<OpenVRDevice> dev,
                 osg::ref_ptr<OpenVRRealizeOperation> realizeOperation)

        : osg::Group()
        , m_configured(false)
        , m_view(view)
        , m_cameraRTTLeft(nullptr)
        , m_cameraRTTRight(nullptr)
        , m_device(dev)
        , m_realizeOperation(realizeOperation)
    {

    }

	virtual void traverse(osg::NodeVisitor& nv);

protected:

    ~OpenVRViewer()
    {

    }

	virtual void configure();

	bool m_configured;

	osg::observer_ptr<osgViewer::View> m_view;

	osg::observer_ptr<osg::Camera> m_cameraRTTLeft, m_cameraRTTRight;

	osg::observer_ptr<OpenVRDevice> m_device;

	osg::observer_ptr<OpenVRRealizeOperation> m_realizeOperation;
};

#endif /* _OSG_OPENVRVIEWER_H_ */
