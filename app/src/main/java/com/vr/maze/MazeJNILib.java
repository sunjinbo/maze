package com.vr.maze;

// Wrapper for native library
public class MazeJNILib {

     static {
          System.loadLibrary("gles3jni");
     }

     public static native void init();
     public static native void resize(int width, int height);
     public static native void step();
}
