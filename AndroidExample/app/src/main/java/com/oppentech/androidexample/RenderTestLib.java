package com.oppentech.androidexample;

public class RenderTestLib {
  static {
    System.loadLibrary("rendertest");
  }

  public static native void initScene();
}
