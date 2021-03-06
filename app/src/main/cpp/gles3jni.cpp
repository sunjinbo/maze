/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "gles3jni.h"
#include "RendererFactory.h"

bool checkGlError(const char* funcName) {
    GLint err = glGetError();
    if (err != GL_NO_ERROR) {
        ALOGE("GL error after %s(): 0x%08x\n", funcName, err);
        return true;
    }
    return false;
}

GLuint createShader(GLenum shaderType, const char* src) {
    GLuint shader = glCreateShader(shaderType);
    if (!shader) {
        checkGlError("glCreateShader");
        return 0;
    }
    glShaderSource(shader, 1, &src, NULL);

    GLint compiled = GL_FALSE;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLogLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);
        if (infoLogLen > 0) {
            GLchar* infoLog = (GLchar*)malloc(infoLogLen);
            if (infoLog) {
                glGetShaderInfoLog(shader, infoLogLen, NULL, infoLog);
                ALOGE("Could not compile %s shader:\n%s\n",
                        shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment",
                        infoLog);
                free(infoLog);
            }
        }
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint createProgram(const char* vtxSrc, const char* fragSrc) {
    GLuint vtxShader = 0;
    GLuint fragShader = 0;
    GLuint program = 0;
    GLint linked = GL_FALSE;

    vtxShader = createShader(GL_VERTEX_SHADER, vtxSrc);
    if (!vtxShader)
        goto exit;

    fragShader = createShader(GL_FRAGMENT_SHADER, fragSrc);
    if (!fragShader)
        goto exit;

    program = glCreateProgram();
    if (!program) {
        checkGlError("glCreateProgram");
        goto exit;
    }
    glAttachShader(program, vtxShader);
    glAttachShader(program, fragShader);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        ALOGE("Could not link program");
        GLint infoLogLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLen);
        if (infoLogLen) {
            GLchar* infoLog = (GLchar*)malloc(infoLogLen);
            if (infoLog) {
                glGetProgramInfoLog(program, infoLogLen, NULL, infoLog);
                ALOGE("Could not link program:\n%s\n", infoLog);
                free(infoLog);
            }
        }
        glDeleteProgram(program);
        program = 0;
    }

exit:
    glDeleteShader(vtxShader);
    glDeleteShader(fragShader);
    return program;
}

static void printGlString(const char* name, GLenum s) {
    const char* v = (const char*)glGetString(s);
    ALOGV("GL %s: %s\n", name, v);
}

static Renderer* g_renderer = NULL;

extern "C" {
    JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved);
    JNIEXPORT void JNICALL Java_com_vr_maze_MazeJNILib_init(JNIEnv* env, jobject obj, jobject ctx);
    JNIEXPORT void JNICALL Java_com_vr_maze_MazeJNILib_create(JNIEnv* env, jobject obj);
    JNIEXPORT void JNICALL Java_com_vr_maze_MazeJNILib_resume(JNIEnv* env, jobject obj);
    JNIEXPORT void JNICALL Java_com_vr_maze_MazeJNILib_pause(JNIEnv* env, jobject obj);
    JNIEXPORT void JNICALL Java_com_vr_maze_MazeJNILib_destroy(JNIEnv* env, jobject obj);
    JNIEXPORT void JNICALL Java_com_vr_maze_MazeJNILib_resize(JNIEnv* env, jobject obj, jint width, jint height);
    JNIEXPORT void JNICALL Java_com_vr_maze_MazeJNILib_step(JNIEnv* env, jobject obj);
    JNIEXPORT void JNICALL Java_com_vr_maze_MazeJNILib_switchViewer(JNIEnv* env, jobject obj);
}

#if !defined(DYNAMIC_ES3)
static GLboolean gl3stubInit() {
    return GL_TRUE;
}
#endif

static JavaVM* javaVm = nullptr;

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* /*reserved*/) {
    javaVm = vm;
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL
Java_com_vr_maze_MazeJNILib_init(JNIEnv* env, jobject obj, jobject ctx) {
    if (g_renderer) {
        delete g_renderer;
        g_renderer = NULL;
    }

    printGlString("Version", GL_VERSION);
    printGlString("Vendor", GL_VENDOR);
    printGlString("Renderer", GL_RENDERER);
    printGlString("Extensions", GL_EXTENSIONS);

    g_renderer = RendererFactory::getRenderer(javaVm, ctx);
}

JNIEXPORT void JNICALL
Java_com_vr_maze_MazeJNILib_create(JNIEnv* env, jobject obj) {
    if (g_renderer) {
        if (!g_renderer->create()) {
            delete g_renderer;
            g_renderer = NULL;
        }
    }
}

JNIEXPORT void JNICALL
Java_com_vr_maze_MazeJNILib_resume(JNIEnv* env, jobject obj) {
    if (g_renderer) {
        g_renderer->resume();
    }
}

JNIEXPORT void JNICALL
Java_com_vr_maze_MazeJNILib_pause(JNIEnv* env, jobject obj) {
    if (g_renderer) {
        g_renderer->pause();
    }
}

JNIEXPORT void JNICALL
Java_com_vr_maze_MazeJNILib_destroy(JNIEnv* env, jobject obj) {
    if (g_renderer) {
        g_renderer->destroy();
    }
}

JNIEXPORT void JNICALL
Java_com_vr_maze_MazeJNILib_resize(JNIEnv* env, jobject obj, jint width, jint height) {
    if (g_renderer) {
        g_renderer->resize(width, height);
    }
}

JNIEXPORT void JNICALL
Java_com_vr_maze_MazeJNILib_step(JNIEnv* env, jobject obj) {
    if (g_renderer) {
        g_renderer->step();
    }
}

JNIEXPORT void JNICALL
Java_com_vr_maze_MazeJNILib_switchViewer(JNIEnv* env, jobject obj) {
    if (g_renderer) {
        g_renderer->switchViewer();
    }
}
