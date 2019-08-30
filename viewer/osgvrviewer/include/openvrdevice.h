/*
 * openvrdevice.h
 *
 *  Created on: Dec 18, 2015
 *      Author: Chris Denham
 */

#ifndef     OSG_OPENVRDEVICE_H
#define     OSG_OPENVRDEVICE_H

// Include the OpenVR SDK
#include    <openvr/openvr_mingw.hpp>

#include    <osg/Geode>
#include    <osg/Texture2D>
#include    <osg/Version>
#include    <osg/FrameBufferObject>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#if(OSG_VERSION_GREATER_OR_EQUAL(3, 4, 0))
	typedef osg::GLExtensions OSG_GLExtensions;
	typedef osg::GLExtensions OSG_Texture_Extensions;
#else
	typedef osg::FBOExtensions OSG_GLExtensions;
	typedef osg::Texture::Extensions OSG_Texture_Extensions;
#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class OpenVRTextureBuffer : public osg::Referenced
{
public:

    OpenVRTextureBuffer(osg::ref_ptr<osg::State> state,
                        int width,
                        int height,
                        int msaaSamples);

    void destroy(osg::GraphicsContext* gc);

    GLuint getTexture() { return m_Resolve_ColorTex; }

    int textureWidth() const { return m_width; }

    int textureHeight() const { return m_height; }

    int samples() const { return m_samples; }

    void onPreRender(osg::RenderInfo& renderInfo);

    void onPostRender(osg::RenderInfo& renderInfo);

protected:

	~OpenVRTextureBuffer() {}

	friend class OpenVRMirrorTexture;

    // MSAA FBO is copied to this FBO after render.
    GLuint m_Resolve_FBO;

    // color texture for above FBO.
    GLuint m_Resolve_ColorTex;

    // framebuffer for MSAA RTT
    GLuint m_MSAA_FBO;

    // color texture for MSAA RTT
    GLuint m_MSAA_ColorTex;

    // depth texture for MSAA RTT
    GLuint m_MSAA_DepthTex;

    // width of texture in pixels
    GLint m_width;

    // height of texture in pixels
    GLint m_height;

    // sample width for MSAA
    int m_samples;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class OpenVRMirrorTexture : public osg::Referenced
{
public:

    OpenVRMirrorTexture(osg::ref_ptr<osg::State> state,
                        GLint width,
                        GLint height);

	void destroy(osg::GraphicsContext* gc);

    void blitTexture(osg::GraphicsContext* gc,
                     OpenVRTextureBuffer* leftEye,
                     OpenVRTextureBuffer* rightEye);

protected:

	~OpenVRMirrorTexture() {}

	GLuint m_mirrorFBO;

	GLuint m_mirrorTex;

	GLint m_width;

	GLint m_height;
};


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class OpenVRPreDrawCallback : public osg::Camera::DrawCallback
{
public:

	OpenVRPreDrawCallback(osg::Camera* camera, OpenVRTextureBuffer* textureBuffer)
		: m_camera(camera)
		, m_textureBuffer(textureBuffer)
	{

	}

	virtual void operator()(osg::RenderInfo& renderInfo) const;

protected:

	osg::Camera* m_camera;

	OpenVRTextureBuffer* m_textureBuffer;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class OpenVRPostDrawCallback : public osg::Camera::DrawCallback
{
public:

	OpenVRPostDrawCallback(osg::Camera* camera, OpenVRTextureBuffer* textureBuffer)
		: m_camera(camera)
		, m_textureBuffer(textureBuffer)
	{

	}

	virtual void operator()(osg::RenderInfo& renderInfo) const;

protected:

	osg::Camera* m_camera;

	OpenVRTextureBuffer* m_textureBuffer;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class OpenVRDevice : public osg::Referenced
{

public:

	typedef enum Eye_
	{
		LEFT = 0,
		RIGHT = 1,
		COUNT = 2
	} Eye;

    OpenVRDevice(float nearClip,
                 float farClip,
                 const float worldUnitsPerMetre = 1.0f,
                 const int samples = 0);

    void createRenderBuffers(osg::ref_ptr<osg::State> state);

    void init();

    void shutdown(osg::GraphicsContext* gc);

	static bool hmdPresent();

    bool hmdInitialized() const;

	osg::Matrix projectionMatrixCenter() const;

	osg::Matrix projectionMatrixLeft() const;

	osg::Matrix projectionMatrixRight() const;

	osg::Matrix projectionOffsetMatrixLeft() const;

	osg::Matrix projectionOffsetMatrixRight() const;

	osg::Matrix viewMatrixLeft() const;

	osg::Matrix viewMatrixRight() const;

    float nearClip() const
    {
        return m_nearClip;
    }

    float farClip() const
    {
        return m_farClip;
    }

	void resetSensorOrientation() const;

	void updatePose();

    osg::Vec3 position() const
    {
        return m_position;
    }

    osg::Quat orientation() const
    {
        return m_orientation;
    }

    osg::Camera* createRTTCamera(OpenVRDevice::Eye eye,
                                 osg::Transform::ReferenceFrame referenceFrame,
                                 const osg::Vec4& clearColor,
                                 osg::GraphicsContext* gc = 0) const;

	bool submitFrame();

	void blitMirrorTexture(osg::GraphicsContext* gc);

    osg::GraphicsContext::Traits* graphicsContextTraits() const;

    vr::IVRSystem *getVrSystem();

protected:

    // Since we inherit from osg::Referenced we must make destructor protected
    ~OpenVRDevice();

	void calculateEyeAdjustment();

	void calculateProjectionMatrices();

	void trySetProcessAsHighPriority() const;

	vr::IVRSystem* m_vrSystem;
	vr::IVRRenderModels* m_vrRenderModels;
	const float m_worldUnitsPerMetre;

	osg::ref_ptr<OpenVRTextureBuffer> m_textureBuffer[2];
	osg::ref_ptr<OpenVRMirrorTexture> m_mirrorTexture;

	osg::Matrixf m_leftEyeProjectionMatrix;
	osg::Matrixf m_rightEyeProjectionMatrix;
	osg::Vec3f m_leftEyeAdjust;
	osg::Vec3f m_rightEyeAdjust;

	osg::Vec3 m_position;
	osg::Quat m_orientation;

	float m_nearClip;
	float m_farClip;
	int m_samples;

private:

    std::string GetDeviceProperty(vr::TrackedDeviceProperty prop);

    // Do not allow copy
    OpenVRDevice(const OpenVRDevice&);

    // Do not allow assignment operator
    OpenVRDevice& operator=(const OpenVRDevice&);
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class OpenVRRealizeOperation : public osg::GraphicsOperation
{
public:

    explicit OpenVRRealizeOperation(osg::ref_ptr<OpenVRDevice> device)
        : osg::GraphicsOperation("OpenVRRealizeOperation", false)
        , m_device(device)
        , m_realized(false)
    {

    }

    virtual void operator () (osg::GraphicsContext* gc);

    bool realized() const { return m_realized; }

protected:

    OpenThreads::Mutex  _mutex;

    osg::observer_ptr<OpenVRDevice> m_device;

    bool m_realized;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class OpenVRSwapCallback : public osg::GraphicsContext::SwapCallback
{
public:

    explicit OpenVRSwapCallback(osg::ref_ptr<OpenVRDevice> device)
        : m_device(device)
        , m_frameIndex(0)
    {

    }

    void swapBuffersImplementation(osg::GraphicsContext* gc);

    int frameIndex() const { return m_frameIndex; }

private:

    osg::observer_ptr<OpenVRDevice> m_device;

    int m_frameIndex;
};


#endif // OSG_OPENVRDEVICE_H
