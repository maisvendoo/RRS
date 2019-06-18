/*
 * openvreventhandler.h
 *
 *  Created on: Dec 18, 2015
 *      Author: Chris Denham
 */

#ifndef     OSG_OPENVREVENTHANDLER_H
#define     OSG_OPENVREVENTHANDLER_H

#include    <osgViewer/ViewerEventHandlers>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class OpenVRDevice;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class OpenVREventHandler : public osgGA::GUIEventHandler
{
public:

    explicit OpenVREventHandler(osg::ref_ptr<OpenVRDevice> device)
        : m_openvrDevice(device)
        , m_usePositionalTracking(true)
    {

    }

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter &);

protected:

	osg::ref_ptr<OpenVRDevice> m_openvrDevice;

	bool m_usePositionalTracking;
};

#endif /* _OSG_OPENVREVENTHANDLER_H_ */
