//
//  TestRenderApp.m
//  mineola
//
//  Created by Fangyang Shen on 2018/9/11.
//  Copyright © 2018 Fangyang Shen. All rights reserved.
//
#import "RenderTest.h"
#include <functional>
#include "../include/PrimitiveHelper.h"
#include "../include/Renderable.h"
#include "../include/SceneNode.h"
#include "../include/Light.h"
#include "../include/PolygonSoupSerialization.h"
#include "../include/PolygonSoupLoader.h"
#include "../include/Engine.h"
#include "../include/CameraController.h"
#include "../include/glutility.h"
#include "../include/SceneLoader.h"
#include "../include/MeshIO.h"
#include "../include/GLTFLoader.h"
#define MINEOLA_USE_STBIMAGE
#include "../include/STBImagePlugin.h"

@interface RenderTest() {
  std::shared_ptr<mineola::CameraController> _camCtrl;
}

@end

@implementation RenderTest

- (void)initScene {
  using namespace mineola;

  auto &en = Engine::Instance();

  std::string resource_path = [[[NSBundle mainBundle] resourcePath] UTF8String];
  en.ResrcMgr().AddSearchPath(resource_path.c_str());
  en.ResrcMgr().AddSearchPath((resource_path + "/FlightHelmet").c_str());

  {
    namespace ph = std::placeholders;
    auto gltf_loader = std::bind(gltf::LoadScene,
      STBLoadImageFromFile, STBLoadImageFromMem,
      ph::_1, ph::_2, ph::_3, ph::_4, ph::_5);
    BuildSceneFromConfigFile("gltfscene.json", {gltf_loader});
  }

//  std::shared_ptr<vertex_type::VertexArray> va1(new vertex_type::VertexArray);
//  primitive_helper::BuildSphere(3, *va1);

//  std::shared_ptr<Renderable> renderable(new Renderable);
//  renderable->AddVertexArray(va1, "mineola:material:fallback");
//  renderable->SetEffect("mineola:effect:falback");

//  // load PLY
//  PolygonSoup soup;
//  if (LoadSoupFromPLY("quad.ply", soup)) {
//    primitive_helper::BuildFromPolygonSoup(soup, "geometry", "mineola:effect:ambienttex", *renderable);
//  }
//
//  std::shared_ptr<SceneNode> geometry_node(new SceneNode);
//  geometry_node->Renderables().push_back(renderable);
//  SceneNode::LinkTo(geometry_node, en.Scene());

  auto pass = RenderPass();
  en.RenderPasses().push_back(pass);

//  // create material
//  auto material = std::make_shared<Material>();
//  std::string material_name = "mat:geometry";
//  en.ResrcMgr().Add(material_name, material);

//  std::shared_ptr<Camera> camera(new Camera(true));
//  camera->SetProjParams(glm::radians(60.0f), 0.01f, 100.0f);
//  en.CameraMgr().Add("camera:main", camera);
  en.ChangeCamera("main", false);

//  std::shared_ptr<SceneNode> camera_node(new SceneNode);
//  camera_node->SetRbt(math::Rbt::LookAt(glm::vec3(0.0f, 0.0f, 4.0f),
//                                        glm::vec3(0.0f, 0.0f, 0.0f),
//                                        glm::vec3(0.0f, 1.0f, 0.0f)));
//  SceneNode::LinkTo(camera_node, en.Scene());
//  camera_node->Cameras().push_back(camera);
//  camera_node->SetName("cam_node");

//  std::shared_ptr<SceneNode> light_node(new SceneNode);
//  light_node->SetPosition(glm::vec3(3.0f, 3.0f, 3.0f));
//  SceneNode::LinkTo(light_node, en.Scene());

//  std::shared_ptr<Light> light0(new Light(0));
//  light0->SetProjParams(glm::radians(90.0f), 1.0f, 0.1f, 20.0f);
//  light_node->Lights().push_back(light0);

  _camCtrl = std::make_shared<ArcballController>();
  _camCtrl->BindToNode("camera");
  _camCtrl->Activate();
  bd_cast<ArcballController>(_camCtrl)->SetSpeed(0.02f);
}

@end
