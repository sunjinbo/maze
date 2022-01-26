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

class CubeRenderer: public Renderer {
public:
    CubeRenderer();
    ~CubeRenderer();

    virtual bool create();
    virtual void resume();
    virtual void pause();
    virtual void destroy();
    virtual void resize(int w, int h);
    virtual void step();

private:
    const EGLContext mEglContext;
    GLuint mProgram;
    GLuint mPositionVBO;
    GLuint mColorVBO;
    GLuint mVAO;

    glm::mat4 mViewMatrix;
    glm::mat4 mProjectionMatrix;
    glm::mat4 mMVPMatrix;
};

Renderer* createCubeRenderer() {
    return new CubeRenderer;
}

#endif //MAZE_CUBERENDERER_H
