//
// Created by sunjjinbo on 2022/1/26.
//

#include "RendererFactory.h"
#include "CubeRenderer.h"

Renderer* RendererFactory::getRenderer(JavaVM* vm, jobject obj) {
    return new CubeRenderer(vm, obj);
}
