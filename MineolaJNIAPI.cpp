#include <jni.h>
#include <string>
#include <iostream>
#include <android/log.h>
#include "include/AppHelper.h"

#include "include/MeshIO.h"
#include "include/SceneLoader.h"
#include "include/CameraController.h"

extern "C" {
  JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_initEngine(
      JNIEnv *env, jobject obj);
  JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_startEngine(
      JNIEnv *env, jobject obj);
  JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_release(
      JNIEnv *env, jobject obj);
  JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_resizeScreen(
    JNIEnv *env, jobject obj, jint width, jint height);
  JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_setScreenFramebuffer(
    JNIEnv *env, jobject obj, jint fbo, jint width, jint height);
  JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_frameMove(
    JNIEnv *env, jobject obj);
  JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_render(
    JNIEnv *env, jobject obj);
  JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_addSearchPath(
    JNIEnv *env, jobject obj, jstring path);
  JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_initScene(
    JNIEnv *env, jobject obj);
  JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_onMouseButton(
    JNIEnv *env, jobject obj, jint button, jboolean is_down, jint x, jint y);
  JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_onMouseMove(
    JNIEnv *env, jobject obj, jint x, jint y);
  JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_onPinch(
    JNIEnv *env, jobject obj, jfloat scale);
}

JNIEXPORT void JNICALL
Java_com_oppentech_mineola_MineolaJNILib_initEngine(JNIEnv *env, jobject obj) {
  mineola::InitEngine();
  return;
}

JNIEXPORT void JNICALL
Java_com_oppentech_mineola_MineolaJNILib_startEngine(JNIEnv *env, jobject obj) {
  mineola::StartEngine();
  return;
}

JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_release(
  JNIEnv *env, jobject obj) {
  mineola::ReleaseEngine();
  return;
}

JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_resizeScreen(
  JNIEnv *env, jobject obj, jint width, jint height) {
  mineola::ResizeScreen((uint32_t)width, (uint32_t)height);
}

JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_setScreenFramebuffer(
  JNIEnv *env, jobject obj, jint fbo, jint width, jint height) {
  mineola::SetScreenFramebuffer((int)fbo, (uint32_t)width, (uint32_t)height);
  return;
}

JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_frameMove(
  JNIEnv *env, jobject obj) {
  mineola::FrameMove();
  return;
}

JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_render(
  JNIEnv *env, jobject obj) {
  mineola::Render();
  return;
}

JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_addSearchPath(
  JNIEnv *env, jobject obj, jstring path) {
  const char *path_str = env->GetStringUTFChars(path, JNI_FALSE);
  mineola::AddSearchPath(path_str);
  env->ReleaseStringUTFChars(path, path_str);
  return;
}

JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_initScene(
  JNIEnv *env, jobject obj) {
  using namespace mineola;

  auto &en = Engine::Instance();

  BuildSceneFromConfigFile("scene.json", {mesh_io::LoadPLY});

  en.RenderPasses().push_back(RenderPass());
  en.ChangeCamera("main", false);

  static auto cam_ctrl = std::make_shared<ArcballController>();
  cam_ctrl->BindToNode("camera");
  cam_ctrl->Activate();
  bd_cast<ArcballController>(cam_ctrl)->SetSpeed(0.02f);

  return;
}

int MapAndroidButtonToMineola(int android_button) {
  using namespace mineola;
  switch (android_button) {
    case 1:  // BUTTON_PRIMARY
      return Engine::MOUSE_LBUTTON;
    case 2:  // BUTTON_SECONDARY
      return Engine::MOUSE_RBUTTON;
    case 4:  // BUTTON_TERTIARY
      return Engine::MOUSE_MBUTTON;
    default:
      return Engine::MOUSE_LBUTTON;
  }
}

JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_onMouseButton(
  JNIEnv *env, jobject obj, jint button, jboolean is_down, jint x, jint y) {
  using namespace mineola;

  Engine::Instance().OnMouseButton(MapAndroidButtonToMineola((int)button),
    (bool)is_down ? Engine::BUTTON_DOWN : Engine::BUTTON_UP,
    (int)x, (int)y);
}

JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_onMouseMove(
  JNIEnv *env, jobject obj, jint x, jint y) {
  mineola::Engine::Instance().OnMouseMove((int)x, (int)y);
}

JNIEXPORT void JNICALL Java_com_oppentech_mineola_MineolaJNILib_onPinch(
  JNIEnv *env, jobject obj, jfloat scale) {
  mineola::Engine::Instance().OnPinch((float)scale);
}
