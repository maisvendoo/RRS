/*
 * openvrupdateslavecallback.h
 *
 *  Created on: Dec 18, 2015
 *      Author: Chris Denham
 */

#ifndef _OSG_OPENVRUPDATESLAVECALLBACK_H_
#define _OSG_OPENVRUPDATESLAVECALLBACK_H_

#include <osgViewer/View>

#include "openvrdevice.h"


struct OpenVRUpdateSlaveCallback : public osg::View::Slave::UpdateSlaveCallback
{
	enum CameraType
	{
		LEFT_CAMERA,
		RIGHT_CAMERA
	};

	OpenVRUpdateSlaveCallback(CameraType cameraType, OpenVRDevice* device, OpenVRSwapCallback* swapCallback) :
		m_cameraType(cameraType),
		m_device(device),
		m_swapCallback(swapCallback) {}

	virtual void updateSlave(osg::View& view, osg::View::Slave& slave);

	CameraType m_cameraType;
	osg::ref_ptr<OpenVRDevice> m_device;
	osg::ref_ptr<OpenVRSwapCallback> m_swapCallback;
};

#endif // _OSG_OPENVRUPDATESLAVECALLBACK_H_
