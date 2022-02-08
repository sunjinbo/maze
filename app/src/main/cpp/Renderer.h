//
// Created by sunjjinbo on 2022/1/26.
//

#ifndef MAZE_RENDERER_H
#define MAZE_RENDERER_H

// ----------------------------------------------------------------------------
// Interface to the renderers, used by JNI code.

class Renderer {
public:
    virtual bool create() = 0;
    virtual void resume() = 0;
    virtual void pause() = 0;
    virtual void destroy() = 0;
    virtual void resize(int w, int h) = 0;
    virtual void step() = 0;
    virtual void switchViewer() = 0;
};

#endif //MAZE_RENDERER_H
