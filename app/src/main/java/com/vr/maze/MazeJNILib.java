package com.vr.maze;

// Wrapper for native library
public class MazeJNILib {

     static {
          System.loadLibrary("gles3jni");
     }

     public static native void init();
     public static native void create();
     public static native void resume();
     public static native void pause();
     public static native void destroy();
     public static native void resize(int width, int height);
     public static native void step();
     public static native void switchViewer();
}
