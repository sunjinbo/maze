package com.vr.maze;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

class MazeView extends GLSurfaceView {
    private static final String TAG = "maze";
    private static final boolean DEBUG = true;

    public MazeView(Context context) {
        super(context);
        init();
    }

    public MazeView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public void resume() {
        super.onResume();
        MazeJNILib.resume();
    }

    public void pause() {
        super.onPause();
        MazeJNILib.pause();
    }

    public void destroy() {
        MazeJNILib.destroy();
    }

    public void switchViewer() {
        MazeJNILib.switchViewer();
    }

    public void init() {
        // Pick an EGLConfig with RGB8 color, 16-bit depth, no stencil,
        // supporting OpenGL ES 2.0 or later backwards-compatible versions.
        setEGLConfigChooser(8, 8, 8, 0, 16, 0);
        setEGLContextClientVersion(3);
        setRenderer(new Renderer());
        setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        MazeJNILib.init(getContext());
    }

    private static class Renderer implements GLSurfaceView.Renderer {
        public void onDrawFrame(GL10 gl) {
            MazeJNILib.step();
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            MazeJNILib.resize(width, height);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            MazeJNILib.create();
        }
    }
}
