//
// Created by sunjinbo on 2022/1/12.
//

#include "gles3jni.h"
#include <EGL/egl.h>

#define STR(s) #s
#define STRV(s) STR(s)

#define POS_ATTRIB 0
#define COLOR_ATTRIB 1

static const char VERTEX_SHADER[] =
        "#version 300 es\n"
        "layout(location = " STRV(POS_ATTRIB) ") in vec2 pos;\n"
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

//    glGenBuffers(VB_COUNT, mVB);
//    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_INSTANCE]);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD), &QUAD[0], GL_STATIC_DRAW);
//    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_SCALEROT]);
//    glBufferData(GL_ARRAY_BUFFER, MAX_INSTANCES * 4*sizeof(float), NULL, GL_DYNAMIC_DRAW);
//    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_OFFSET]);
//    glBufferData(GL_ARRAY_BUFFER, MAX_INSTANCES * 2*sizeof(float), NULL, GL_STATIC_DRAW);
//
//    glGenVertexArrays(1, &mVBState);
//    glBindVertexArray(mVBState);
//
//    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_INSTANCE]);
//    glVertexAttribPointer(POS_ATTRIB, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)offsetof(Vertex, pos));
//    glVertexAttribPointer(COLOR_ATTRIB, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (const GLvoid*)offsetof(Vertex, rgba));
//    glEnableVertexAttribArray(POS_ATTRIB);
//    glEnableVertexAttribArray(COLOR_ATTRIB);
//
//    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_SCALEROT]);
//    glVertexAttribPointer(SCALEROT_ATTRIB, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
//    glEnableVertexAttribArray(SCALEROT_ATTRIB);
//    glVertexAttribDivisor(SCALEROT_ATTRIB, 1);
//
//    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_OFFSET]);
//    glVertexAttribPointer(OFFSET_ATTRIB, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), 0);
//    glEnableVertexAttribArray(OFFSET_ATTRIB);
//    glVertexAttribDivisor(OFFSET_ATTRIB, 1);

    ALOGV("Using OpenGL ES 3.0 renderer");
    return true;
}

void CubeRenderer::resize(int w, int h) {

}

void CubeRenderer::render() {
//    glUseProgram(mProgram);
//    glBindVertexArray(mVBState);
//    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, numInstances);
}
