package com.shenfy.mineola.gltfviewer;

import android.app.Activity;
import android.os.Bundle;

public class MainActivity extends Activity {

  GLES3JNIView renderView;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    renderView = new GLES3JNIView(getApplication(), "/gltfviewer/");
    setContentView(renderView);
  }

  @Override
  protected void onPause() {
    super.onPause();
    renderView.onPause();
  }

  @Override
  protected void onResume() {
    super.onResume();
    renderView.onResume();
  }
}

