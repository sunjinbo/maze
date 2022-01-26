//
// Created by sunjjinbo on 2022/1/26.
//

#ifndef MAZE_CUBERENDERER_H
#define MAZE_CUBERENDERER_H

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/closest_point.hpp>

#include "Renderer.h"
#include "cardboard.h"

class CubeRenderer: public Renderer {
public:
    CubeRenderer(JavaVM* vm, jobject obj);
    ~CubeRenderer();

    virtual bool create();
    virtual void resume();
    virtual void pause();
    virtual void destroy();
    virtual void resize(int w, int h);
    virtual void step();

private:
    bool updateDeviceParams();
    void setupGL();
    void teardownGL();

private:
    /**
     * Default near clip plane z-axis coordinate.
     */
    static constexpr float kZNear = 0.1f;

    /**
     * Default far clip plane z-axis coordinate.
     */
    static constexpr float kZFar = 100.f;

    const EGLContext mEglContext;
    GLuint mProgram;
    GLuint mPositionVBO;
    GLuint mColorVBO;
    GLuint mVAO;

    glm::mat4 mViewMatrix;
    glm::mat4 mProjectionMatrix;
    glm::mat4 mMVPMatrix;

    CardboardHeadTracker* mHeadTracker;
    CardboardLensDistortion* mLensDistortion;
    CardboardDistortionRenderer* mDistortionRenderer;

    CardboardEyeTextureDescription mLeftEyeTextureDescription;
    CardboardEyeTextureDescription mRightEyeTextureDescription;

    bool mScreenParamsChanged;
    bool mDeviceParamsChanged;

    int mScreenWidth;
    int mScreenHeight;

    GLuint mDepthRenderBuffer;      // depth buffer
    GLuint mFramebuffer;            // framebuffer object
    GLuint mDistortionTextureId;    // distortion texture

    float projection_matrices_[2][16];
    float eye_matrices_[2][16];
};

#endif //MAZE_CUBERENDERER_H
