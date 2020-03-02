package com.oppentech.androidexample;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.opengl.GLES30;
import android.opengl.GLSurfaceView;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import static android.content.ContentValues.TAG;

import com.oppentech.mineola.MineolaJNILib;

public class GLES3JNIView extends GLSurfaceView {

  // Renderer
  private static class GLES3Renderer implements GLSurfaceView.Renderer {
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
      if (!inited) {
        MineolaJNILib.initEngine();

        RenderTestLib.initScene();

        MineolaJNILib.startEngine();
        inited = true;
        Log.d(TAG, "onSurfaceCreated");
      }
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
      if (!inited)  // do nothing if not initialized
        return;

      gl.glGetIntegerv(GLES30.GL_DRAW_FRAMEBUFFER_BINDING, params, 0);
      Log.d(TAG, "onSurfaceChanged: " + params[0] + " " + width + "x" + height);

      int current_fbo = params[0];

      if (current_fbo != this.fbo) {
        MineolaJNILib.setScreenFramebuffer(current_fbo, width, height);
        MineolaJNILib.resizeScreen(width, height);
        this.fbo = current_fbo;
        this.width = width;
        this.height = height;
        fboSet = true;
      } else if (width != this.width || height != this.height) {
        MineolaJNILib.resizeScreen(width, height);
        this.width = width;
        this.height = height;
      }
    }

    @Override
    public void onDrawFrame(GL10 gl) {
      if (inited && fboSet) {

        gl.glGetIntegerv(GLES30.GL_DRAW_FRAMEBUFFER_BINDING, params, 0);

        MineolaJNILib.frameMove();
        MineolaJNILib.render();
      }
    }

    protected int fbo = -1;
    protected int width = 0, height = 0;

    private int[] params = new int[2];
    private boolean inited = false;
    private boolean fboSet = false;
  }

  private class PinchGestureListener extends
          ScaleGestureDetector.SimpleOnScaleGestureListener {
    @Override
    public boolean onScale(ScaleGestureDetector detector) {
      MineolaJNILib.onPinch(detector.getScaleFactor());
      return true;
    }

    @Override
    public boolean onScaleBegin(ScaleGestureDetector detector) {
      pinching = true;
      return true;
    }

    @Override
    public void onScaleEnd(ScaleGestureDetector detector) {
      pinching = false;
    }

    public boolean isPinching() {
      return pinching;
    }

    private boolean pinching = false;
  }

  // View

  public GLES3JNIView(Context context, String appPath) {
    super(context);

    // calc resource search dirs
    File internalStorageDir = context.getFilesDir();
    File resrcDir = new File(internalStorageDir, appPath);
    if (!resrcDir.exists()) {
      resrcDir.mkdirs();
    }
    String resrcPath = resrcDir.getAbsolutePath();

    // check if need to copy assets
    SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
    if (!prefs.getBoolean("assets_copied", false)) {
      // copy assets
      Log.d(TAG, "GLES3JNIView: copying assets");
      copyAssets(context, "bundled", resrcPath);

//            // set assets copied flag
//            SharedPreferences.Editor editor = prefs.edit();
//            editor.putBoolean("assets_copied", true);
//            editor.commit();
    }

    MineolaJNILib.addSearchPath(resrcPath);

    setEGLConfigChooser(8, 8, 8, 8, 16, 0);
    setEGLContextClientVersion(3);

    setRenderer(new GLES3Renderer());

    this.pinchListener = new PinchGestureListener();
    this.detector = new ScaleGestureDetector(context, pinchListener);
  }

  private ScaleGestureDetector detector;
  private PinchGestureListener pinchListener;


  @Override
  public boolean onTouchEvent(MotionEvent e) {

    // handle gestures first
    detector.onTouchEvent(e);

    if (pinchListener.isPinching()) {
      dragging = false;
      return true;
    }

    float x = e.getX();
    float y = e.getY();
    int button = e.getActionButton();

    switch (e.getAction()) {
      case MotionEvent.ACTION_DOWN:
        MineolaJNILib.onMouseButton(button, true, (int)x, (int)y);
        dragging = true;
        return true;
      case MotionEvent.ACTION_UP:
        MineolaJNILib.onMouseButton(button, false, (int)x, (int)y);
        dragging = false;
        return true;
      case MotionEvent.ACTION_MOVE:
        if (dragging) {
          MineolaJNILib.onMouseMove((int) x, (int) y);
          return true;
        }
        return false;
      default:
        return false;
    }
  }

  private boolean dragging = false;

  private boolean copyAssets(Context context, String srcPath, String dstPath) {
    AssetManager assetManager = getResources().getAssets();
    String[] files = null;
    try {
      files = assetManager.list(srcPath);
    } catch (IOException e) {
      Log.e("Java IO", "Failed to list asset files in " + srcPath, e);
      return false;
    }

    for (String filename : files) {
      Log.v("Java", "file " + filename);
      InputStream in = null;
      OutputStream out = null;
      try {
        in = assetManager.open(srcPath + "/" + filename);
        File outFile = new File(dstPath, filename);

        out = new FileOutputStream(outFile);
        copyFile(in, out);
        in.close();
        in = null;
        out.flush();
        out.close();
        out = null;
      } catch (IOException e) {
        Log.e("Java IO", "Failed to copy file " + filename, e);
      }
    }

    return true;
  }

  private void copyFile(InputStream in, OutputStream out) throws IOException {
    byte[] buffer = new byte[1024 * 32];
    int read;
    while ((read = in.read(buffer)) != -1) {
      out.write(buffer, 0, read);
    }
  }
}
