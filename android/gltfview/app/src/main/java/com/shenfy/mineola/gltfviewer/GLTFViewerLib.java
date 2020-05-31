package com.shenfy.mineola.gltfviewer;

public class GLTFViewerLib {
  static {
    System.loadLibrary("gltfviewer_lib");
  }

  public static native void initScene();
}
