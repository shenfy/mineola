#import "../include/RendererViewController.h"
#include "../include/AppHelper.h"
#include "../include/Engine.h"

enum {kVCStateStart = 0, kVCStateInit, kVCStateFBOSet, kVCStateFBOInvalid};

@interface RendererViewController () {
  int _defaultFBO;
  int _width, _height;
  int _frameCount;
  int _vcState;
}

@end

@implementation RendererViewController

- (id)init {
  self = [super init];
  if (self) {
    self->_defaultFBO = -1;
    self->_width = 0;
    self->_height = 0;
    self->_frameCount = 0;
    self->_vcState = kVCStateStart;
  }
  return self;
}

- (void)viewDidLoad {
  [super viewDidLoad];
  // Do any additional setup after loading the view.

  self.preferredFramesPerSecond = 30;

  GLKView *view = [[GLKView alloc] init];
  view.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
  view.drawableColorFormat = GLKViewDrawableColorFormatRGBA8888;
  view.drawableDepthFormat = GLKViewDrawableDepthFormat24;

  self.view = view;
  
  // set up UI event handlers
  UIPanGestureRecognizer *panRecognizer = [[UIPanGestureRecognizer alloc]
                                           initWithTarget:self action:@selector(handlePan:)];
  [view addGestureRecognizer:panRecognizer];

  UIPinchGestureRecognizer *pinchRecognizer = [[UIPinchGestureRecognizer alloc]
                                            initWithTarget:self action:@selector(handlePinch:)];
  [view addGestureRecognizer:pinchRecognizer];
}

- (void) handlePinch: (UIPinchGestureRecognizer *)recognizer {
  if (recognizer.state == UIGestureRecognizerStateBegan ||
    recognizer.state == UIGestureRecognizerStateChanged) {
    mineola::Engine::Instance().OnPinch(recognizer.scale);
  }
  recognizer.scale = 1.0;
}

- (void) handlePan: (UIPanGestureRecognizer *)recognizer {
  if (recognizer.view != self.view)
    return;
  
  using namespace mineola;
  auto &en = Engine::Instance();
  
  CGPoint pt = [recognizer locationInView:self.view];
  pt.x *= self.view.contentScaleFactor;
  pt.y *= self.view.contentScaleFactor;
  if (recognizer.state == UIGestureRecognizerStateBegan) {
    en.OnMouseButton(Engine::MOUSE_LBUTTON, Engine::BUTTON_DOWN, (int)pt.x, (int)pt.y);
  } else if (recognizer.state == UIGestureRecognizerStateChanged) {
    en.OnMouseMove((int)pt.x, (int)pt.y);
  } else if (recognizer.state == UIGestureRecognizerStateEnded) {
    en.OnMouseButton(Engine::MOUSE_LBUTTON, Engine::BUTTON_UP, (int)pt.x, (int)pt.y);
  }
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)update {
  if (_vcState == kVCStateStart) {
    mineola::InitEngine();
    if (self.renderDelegate && [self.renderDelegate respondsToSelector:@selector(initScene)]) {
      [self.renderDelegate initScene];
    }
    mineola::StartEngine();
    _vcState = kVCStateInit;
  } else {
    // handle fbo name/number change
    bool fboChanged = false, sizeChanged = false;
    
    GLint defaultFBO = -1;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);
    if (defaultFBO != _defaultFBO) {  // default FBO changed!
      _defaultFBO = defaultFBO;
      NSLog(@"default fbo changed to: %d", _defaultFBO);
      fboChanged = true;
    }
    // handle resize
    GLKView *view = (GLKView *)self.view;
    if (view.drawableWidth != self->_width || view.drawableHeight != self->_height) {
      _width = (int)view.drawableWidth;
      _height = (int)view.drawableHeight;
      NSLog(@"size changed to: %d x %d", _width, _height);
      sizeChanged = true;
    }

    if (fboChanged) {
      if (defaultFBO == 0) {
        _vcState = kVCStateFBOInvalid;
      } else {
        mineola::SetScreenFramebuffer(_defaultFBO, _width, _height);
        mineola::ResizeScreen(_width, _height);
        _vcState = kVCStateFBOSet;
      }
    } else if (sizeChanged) {
      mineola::ResizeScreen(_width, _height);
    }
  }
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
  if (_vcState == kVCStateFBOSet) {
    auto &en = mineola::Engine::Instance();
    en.FrameMove();
    
    en.Render();
    
    if (_frameCount == 0) {
      _frameCount = 1;
    }
  }

}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
