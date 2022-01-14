//
// Created by sunjinbo on 2022/1/12.
//

#include "gles3jni.h"
#include "cardboard.h"
#include <EGL/egl.h>
#include <string.h>

#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/closest_point.hpp>

#define STR(s) #s
#define STRV(s) STR(s)

#define POS_ATTRIB 0
#define COLOR_ATTRIB 1

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

class CubeRenderer: public Renderer {
public:
    CubeRenderer();
    ~CubeRenderer();

    virtual bool init();
    virtual void resize(int w, int h);
    virtual void render();

private:
    const EGLContext mEglContext;
    GLuint mProgram;
    GLuint mPositionVBO;
    GLuint mColorVBO;
    GLuint mVAO;

    float mViewMatrix[16];
    float mProjectionMatrix[16];
    float mMVPMatrix[16];

    glm::mat4 Projection;
};

Renderer* createCubeRenderer() {
    CubeRenderer* renderer = new CubeRenderer;
    if (!renderer->init()) {
        delete renderer;
        return NULL;
    }
    return renderer;
}

CubeRenderer::CubeRenderer()
        :   mEglContext(eglGetCurrentContext()),
            mProgram(0),
            mPositionVBO(0),
            mColorVBO(0),
            mVAO(0)
{
    memset(mViewMatrix, 0, sizeof(mViewMatrix));
    memset(mProjectionMatrix, 0, sizeof(mProjectionMatrix));
    memset(mMVPMatrix, 0, sizeof(mMVPMatrix));
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

bool CubeRenderer::init() {
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

void CubeRenderer::resize(int w, int h) {
    glViewport(0, 0, w, h);
    float ratio = (float) w / (float) h;

//    Matrix.frustumM(mProjectionMatrix, 0, -ratio, ratio, -1, 1, 3, 7);
//    Matrix.setLookAtM(mViewMatrix, 0,
//                      0f, 0f, 5f,
//                      0f, 0f, 0f,
//                      0f, 1f, 0f);
//    Matrix.multiplyMM(mMVPMatrix, 0, mProjectionMatrix, 0, mViewMatrix, 0);
}

void CubeRenderer::render() {
    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(mProgram);

//    Matrix.rotateM(mMVPMatrix, 0, 0.5f, 0.5f, 0.5f, 0.0f);

    mMVPMatrix[0] = 5.497117;
    mMVPMatrix[1] = 5.7131045E-5;
    mMVPMatrix[2] = 0.015426481;
    mMVPMatrix[3] = 0.0061705923;

    mMVPMatrix[4] = 1.0468735E-4;
    mMVPMatrix[5] = 2.9999428;
    mMVPMatrix[6] = -0.015426481;
    mMVPMatrix[7] = -0.0061705923;

    mMVPMatrix[8] = 0.033921115;
    mMVPMatrix[9] = -0.018511776;
    mMVPMatrix[10] = -2.4999049;
    mMVPMatrix[11] = -0.9999619;

    mMVPMatrix[12] = 0;
    mMVPMatrix[13] = 0;
    mMVPMatrix[14] = 2.0;
    mMVPMatrix[15] = 5.0;

    int uMatrixLocation = glGetUniformLocation(mProgram, "uMatrix");
    glUniformMatrix4fv(uMatrixLocation, 1, false, mMVPMatrix);

    glBindVertexArray(mVAO);
    glDrawArrays(GL_LINES, 0, 24);
    glBindVertexArray(0);
}
