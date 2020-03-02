package com.oppentech.mineola;

public class MineolaJNILib {
  static {
    System.loadLibrary("mineola");
  }

  public static native void initEngine();
  public static native void initScene();
  public static native void startEngine();
  public static native void resizeScreen(int width, int height);
  public static native void setScreenFramebuffer(int fbo, int width, int height);
  public static native void frameMove();
  public static native void render();
  public static native void addSearchPath(String path);
  public static native void onMouseButton(int button, boolean action, int x, int y);
  public static native void onMouseMove(int x, int y);
  public static native void onPinch(float scale);

}
