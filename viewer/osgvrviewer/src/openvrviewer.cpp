/*
 * openvrviewer.cpp
 *
 *  Created on: Dec 18, 2015
 *      Author: Chris Denham
 */

#include    "openvrviewer.h"
#include    "openvrupdateslavecallback.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void OpenVRViewer::traverse(osg::NodeVisitor& nv)
{
	// Must be realized before any traversal
	if (!m_configured && m_realizeOperation->realized())
	{
		configure();
	}

	osg::Group::traverse(nv);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void OpenVRViewer::configure()
{
	osg::ref_ptr<osg::GraphicsContext> gc =  m_view->getCamera()->getGraphicsContext();

	// Attach a callback to detect swap
	osg::ref_ptr<OpenVRSwapCallback> swapCallback = new OpenVRSwapCallback(m_device);
	gc->setSwapCallback(swapCallback);

	osg::ref_ptr<osg::Camera> camera = m_view->getCamera();
	camera->setName("Main");
	osg::Vec4 clearColor = camera->getClearColor();

	// master projection matrix
	camera->setProjectionMatrix(m_device->projectionMatrixCenter());

	// Create RTT cameras and attach textures
	m_cameraRTTLeft = m_device->createRTTCamera(OpenVRDevice::LEFT, osg::Camera::RELATIVE_RF, clearColor, gc);
	m_cameraRTTRight = m_device->createRTTCamera(OpenVRDevice::RIGHT, osg::Camera::RELATIVE_RF, clearColor, gc);
	m_cameraRTTLeft->setName("LeftRTT");
	m_cameraRTTRight->setName("RightRTT");

	// Add RTT cameras as slaves, specifying offsets for the projection
	m_view->addSlave(m_cameraRTTLeft.get(),
					 m_device->projectionOffsetMatrixLeft(),
					 m_device->viewMatrixLeft(),
					 true);

    m_view->getSlave(0)._updateSlaveCallback = new OpenVRUpdateSlaveCallback(OpenVRUpdateSlaveCallback::LEFT_CAMERA, m_device.get(), swapCallback.get());

	m_view->addSlave(m_cameraRTTRight.get(),
					 m_device->projectionOffsetMatrixRight(),
					 m_device->viewMatrixRight(),
					 true);

    m_view->getSlave(1)._updateSlaveCallback = new OpenVRUpdateSlaveCallback(OpenVRUpdateSlaveCallback::RIGHT_CAMERA, m_device.get(), swapCallback.get());

	// Use sky light instead of headlight to avoid light changes when head movements
	m_view->setLightingMode(osg::View::SKY_LIGHT);

	// Disable rendering of main camera since its being overwritten by the swap texture anyway
	camera->setGraphicsContext(nullptr);

	m_configured = true;
}
