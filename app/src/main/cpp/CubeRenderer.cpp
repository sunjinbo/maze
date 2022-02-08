//
// Created by sunjinbo on 2022/1/12.
//

#include "gles3jni.h"
#include "cardboard.h"
#include "CubeRenderer.h"

#include <string.h>

#define STR(s) #s
#define STRV(s) STR(s)

#define POS_ATTRIB 0
#define COLOR_ATTRIB 1

// The objects are about 1 meter in radius, so the min/max target distance are
// set so that the objects are always within the room (which is about 5 meters
// across) and the reticle is always closer than any objects.
constexpr float kMinTargetDistance = 2.5f;
constexpr float kMaxTargetDistance = 3.5f;
constexpr float kMinTargetHeight = 0.5f;
constexpr float kMaxTargetHeight = kMinTargetHeight + 3.0f;

constexpr float kDefaultFloorHeight = -1.7f;

constexpr uint64_t kPredictionTimeWithoutVsyncNanos = 50000000;

// Angle threshold for determining whether the controller is pointing at the
// object.
constexpr float kAngleLimit = 0.2f;

// Number of different possible targets
constexpr int kTargetMeshCount = 3;

static const char VERTEX_SHADER[] =
        "#version 300 es\n"
        "layout(location = " STRV(POS_ATTRIB) ") in vec4 pos;\n"
        "layout(location=" STRV(COLOR_ATTRIB) ") in vec4 color;\n"
        "out vec4 vColor;\n"
        "uniform mat4 uMatrix;\n"
        "void main() {\n"
        "    gl_Position  = uMatrix * pos;\n"
        "    vColor = color;\n"
        "}\n";

static const char FRAGMENT_SHADER[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4 vColor;\n"
        "out vec4 outColor;\n"
        "void main() {\n"
        "    outColor = vColor;\n"
        "}\n";

static const float COORDS[72] = {
        0.5f,  0.5f, 0.5f, // A
        0.5f, 0.5f, -0.5f, // B

        0.5f, 0.5f, -0.5f, // B
        0.5f, -0.5f, -0.5f, // C

        0.5f, -0.5f, -0.5f, // C
        0.5f, -0.5f, 0.5f,  // D

        0.5f, -0.5f, 0.5f,  // D
        0.5f,  0.5f, 0.5f, // A

        -0.5f, 0.5f, 0.5f,  // F
        0.5f,  0.5f, 0.5f, // A

        -0.5f, 0.5f, 0.5f,  // F
        -0.5f, -0.5f, 0.5f,  // E

        -0.5f, -0.5f, 0.5f,  // E
        0.5f, -0.5f, 0.5f,  // D

        -0.5f, 0.5f, -0.5f,  // G
        0.5f, 0.5f, -0.5f, // B

        -0.5f, 0.5f, -0.5f,  // G
        -0.5f, 0.5f, 0.5f,  // F

        -0.5f, 0.5f, -0.5f,  // G
        -0.5f, -0.5f, -0.5f,  // H

        -0.5f, -0.5f, -0.5f,  // H
        -0.5f, -0.5f, 0.5f,  // E

        -0.5f, -0.5f, -0.5f,  // H
        0.5f, -0.5f, -0.5f  // C
};

static const float COLORS[96] = {
        0.49f, 0.73f, 0.91f, 1.0f, // A
        0.01f, 0.28f, 1.0f, 1.0f, // B
        0.01f, 0.28f, 1.0f, 1.0f, // B
        0.77f, 0.12f, 0.23f, 1.0f, // C
        0.77f, 0.12f, 0.23f, 1.0f, // C
        0.59f, 0.44f, 0.09f, 1.0f, // D
        0.59f, 0.44f, 0.09f, 1.0f, // D
        0.49f, 0.73f, 0.91f, 1.0f, // A
        1.0f, 0.0f, 0.0f, 1.0f, // F
        0.49f, 0.73f, 0.91f, 1.0f, // A
        1.0f, 0.0f, 0.0f, 1.0f, // F
        0.0f, 1.0f, 0.25f, 1.0f, // E
        0.0f, 1.0f, 0.25f, 1.0f, // E
        0.59f, 0.44f, 0.09f, 1.0f, // D
        0.0f, 0.1f, 0.0f, 1.0f, // G
        0.01f, 0.28f, 1.0f, 1.0f, // B
        0.0f, 0.1f, 0.0f, 1.0f, // G
        1.0f, 0.0f, 0.0f, 1.0f, // F
        0.0f, 0.1f, 0.0f, 1.0f, // G
        0.64f, 0.0f, 0.43f, 1.0f, // H
        0.64f, 0.0f, 0.43f, 1.0f, // H
        0.0f, 1.0f, 0.25f, 1.0f, // E
        0.64f, 0.0f, 0.43f, 1.0f, // H
        0.77f, 0.12f, 0.23f, 1.0f // C
};

CubeRenderer::CubeRenderer(JavaVM* vm, jobject obj)
        :   mEglContext(eglGetCurrentContext()),
            mProgram(0),
            mPositionVBO(0),
            mColorVBO(0),
            mVAO(0),
            mHeadTracker(nullptr),
            mLensDistortion(nullptr),
            mDistortionRenderer(nullptr),
            mScreenParamsChanged(false),
            mDeviceParamsChanged(false),
            mScreenWidth(0),
            mScreenHeight(0),
            mDepthRenderBuffer(0),
            mFramebuffer(0),
            mDistortionTextureId(0)
{
    Cardboard_initializeAndroid(vm, obj);
    mHeadTracker = CardboardHeadTracker_create();
}

CubeRenderer::~CubeRenderer() {
    /* The destructor may be called after the context has already been
     * destroyed, in which case our objects have already been destroyed.
     *
     * If the context exists, it must be current. This only happens when we're
     * cleaning up after a failed init().
     */
    if (eglGetCurrentContext() != mEglContext)
        return;
//    glDeleteVertexArrays(1, &mVBState);
//    glDeleteBuffers(VB_COUNT, mVB);
//    glDeleteProgram(mProgram);
}

bool CubeRenderer::create() {
    mProgram = createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    if (!mProgram)
        return false;

    GLuint posBuffers[1];
    glGenBuffers(1, posBuffers);
    mPositionVBO = posBuffers[0];

    glBindBuffer(GL_ARRAY_BUFFER, mPositionVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(COORDS), &COORDS[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint colorBuffers[1];
    glGenBuffers(1, colorBuffers);
    mColorVBO = colorBuffers[0];

    glBindBuffer(GL_ARRAY_BUFFER, mColorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(COLORS), &COLORS[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint vaoBuffers[1];
    glGenVertexArrays(1, vaoBuffers);
    mVAO = vaoBuffers[0];

    glBindVertexArray(mVAO);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, mPositionVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, mColorVBO);
    glVertexAttribPointer(1, 4, GL_FLOAT, false, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    ALOGV("Using OpenGL ES 3.0 renderer");

    return true;
}

void CubeRenderer::resume() {
    CardboardHeadTracker_resume(mHeadTracker);

    // Parameters may have changed.
    mDeviceParamsChanged = true;

    // Check for device parameters existence in external storage. If they're
    // missing, we must scan a Cardboard QR code and save the obtained parameters.
    uint8_t* buffer;
    int size = 0;
    CardboardQrCode_getSavedDeviceParams(&buffer, &size);
    if (size == 0) {
        switchViewer();
    }

    CardboardQrCode_destroy(buffer);
}

void CubeRenderer::pause() {
    CardboardHeadTracker_pause(mHeadTracker);
}

void CubeRenderer::destroy() {
    CardboardHeadTracker_destroy(mHeadTracker);
    CardboardLensDistortion_destroy(mLensDistortion);
    CardboardDistortionRenderer_destroy(mDistortionRenderer);
}

void CubeRenderer::resize(int w, int h) {
    mScreenWidth = w;
    mScreenHeight = h;
    mScreenParamsChanged = true;

    glViewport(0, 0, w, h);
    float ratio = (float) w / (float) h;

    mProjectionMatrix = glm::frustum(-ratio, ratio, -1.0f, 1.0f, 3.0f, 7.0f);
    glm::vec3 eye(0.0f,0.0f,5.0f);
    glm::vec3 look(0.0f,0.0f,0.0f);
    glm::vec3 up(0.0f,1.0f,0.0f);
    mViewMatrix = glm::lookAt(eye, look, up);

    mMVPMatrix = mProjectionMatrix * mViewMatrix;
}

void CubeRenderer::step() {
    if (!updateDeviceParams()) {
        return;
    }

    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(mProgram);

    glm::vec3 axis(0.5f,1.0f,0.5f);
    mMVPMatrix = glm::rotate(mMVPMatrix, 0.01f, axis);

    int uMatrixLocation = glGetUniformLocation(mProgram, "uMatrix");
    glUniformMatrix4fv(uMatrixLocation, 1, false, glm::value_ptr(mMVPMatrix));

    glBindVertexArray(mVAO);
    glDrawArrays(GL_LINES, 0, 24);
    glBindVertexArray(0);
}

void CubeRenderer::switchViewer() {
    CardboardQrCode_scanQrCodeAndSaveDeviceParams();
}

bool CubeRenderer::updateDeviceParams() {
    if (!mScreenParamsChanged && !mDeviceParamsChanged) {
        return true;
    }

    // Get saved device parameters
    uint8_t* buffer;
    int size;
    CardboardQrCode_getSavedDeviceParams(&buffer, &size);

    // If there are no parameters saved yet, returns false.
    if (size == 0) {
        return false;
    }

    CardboardLensDistortion_destroy(mLensDistortion);
    mLensDistortion = CardboardLensDistortion_create(
            buffer, size, mScreenWidth, mScreenHeight);

    CardboardQrCode_destroy(buffer);

    setupGL();

    CardboardDistortionRenderer_destroy(mDistortionRenderer);
    mDistortionRenderer = CardboardOpenGlEs2DistortionRenderer_create();

    CardboardMesh left_mesh;
    CardboardMesh right_mesh;
    CardboardLensDistortion_getDistortionMesh(mLensDistortion, kLeft,
                                              &left_mesh);
    CardboardLensDistortion_getDistortionMesh(mLensDistortion, kRight,
                                              &right_mesh);

    CardboardDistortionRenderer_setMesh(mDistortionRenderer, &left_mesh, kLeft);
    CardboardDistortionRenderer_setMesh(mDistortionRenderer, &right_mesh,
                                        kRight);

    // Get eye matrices
    CardboardLensDistortion_getEyeFromHeadMatrix(mLensDistortion, kLeft,
                                                 eye_matrices_[0]);
    CardboardLensDistortion_getEyeFromHeadMatrix(mLensDistortion, kRight,
                                                 eye_matrices_[1]);
    CardboardLensDistortion_getProjectionMatrix(mLensDistortion, kLeft, kZNear,
                                                kZFar, projection_matrices_[0]);
    CardboardLensDistortion_getProjectionMatrix(mLensDistortion, kRight, kZNear,
                                                kZFar, projection_matrices_[1]);

    mScreenParamsChanged = false;
    mDeviceParamsChanged = false;

    return true;
}

void CubeRenderer::setupGL() {
    if (mFramebuffer != 0) {
        teardownGL();
    }

    // Create render texture.
    glGenTextures(1, &mDistortionTextureId);
    glBindTexture(GL_TEXTURE_2D, mDistortionTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mScreenWidth, mScreenHeight, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, 0);

    mLeftEyeTextureDescription.texture = mDistortionTextureId;
    mLeftEyeTextureDescription.left_u = 0;
    mLeftEyeTextureDescription.right_u = 0.5;
    mLeftEyeTextureDescription.top_v = 1;
    mLeftEyeTextureDescription.bottom_v = 0;

    mRightEyeTextureDescription.texture = mDistortionTextureId;
    mRightEyeTextureDescription.left_u = 0.5;
    mRightEyeTextureDescription.right_u = 1;
    mRightEyeTextureDescription.top_v = 1;
    mRightEyeTextureDescription.bottom_v = 0;

    // Generate depth buffer to perform depth test.
    glGenRenderbuffers(1, &mDepthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, mDepthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, mScreenWidth,
                          mScreenHeight);

    // Create render target.
    glGenFramebuffers(1, &mFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           mDistortionTextureId, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, mDepthRenderBuffer);
}

void CubeRenderer::teardownGL() {
    if (mFramebuffer == 0) {
        return;
    }
    glDeleteRenderbuffers(1, &mDepthRenderBuffer);
    mDepthRenderBuffer = 0;
    glDeleteFramebuffers(1, &mFramebuffer);
    mFramebuffer = 0;
    glDeleteTextures(1, &mDistortionTextureId);
    mDistortionTextureId = 0;
}
