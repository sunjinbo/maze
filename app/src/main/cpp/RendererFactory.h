//
// Created by sunjjinbo on 2022/1/26.
//

#ifndef MAZE_RENDERERFACTORY_H
#define MAZE_RENDERERFACTORY_H

#include <jni.h>
#include "Renderer.h"

class RendererFactory {
public:
    static Renderer* getRenderer(JavaVM* vm, jobject obj);
};

#endif //MAZE_RENDERERFACTORY_H
