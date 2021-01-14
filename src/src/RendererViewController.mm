#import <mineola/RendererViewController.h>
#include <mineola/AppHelper.h>
#include <mineola/Engine.h>

enum {kVCStateStart = 0, kVCStateInit, kVCStateFBOSet, kVCStateFBOInvalid};

@interface RendererViewController () {
  int _currentFBO;
  int _width, _height;
}

@end

@implementation RendererViewController

- (id)init {
  self = [super init];
  if (self) {
    self->_currentFBO = -1;
    self->_width = 0;
    self->_height = 0;
  }
  return self;
}

- (void)viewDidLoad {
  [super viewDidLoad];
  // Do any additional setup after loading the view.

  self.preferredFramesPerSecond = 30;

  // Create an OpenGL ES context and assign it to the view loaded from storyboard
  GLKView *view = (GLKView *)self.view;;
  view.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
  [EAGLContext setCurrentContext:view.context];

  // Configure renderbuffers created by the view
  view.drawableColorFormat = GLKViewDrawableColorFormatRGBA8888;
  view.drawableDepthFormat = GLKViewDrawableDepthFormat24;

  // Init mineola
  auto &en = mineola::Engine::Instance();
  mineola::InitEngine();
  if (self.renderDelegate && [self.renderDelegate respondsToSelector:@selector(initScene)]) {
    [self.renderDelegate initScene];
  }
  mineola::StartEngine();
  if (self.renderDelegate && [self.renderDelegate respondsToSelector:@selector(started)]) {
    [self.renderDelegate started];
  }

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

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
  // Process screen fbo change and resize events.
  // These events are not explicitly fired by iOS,
  // so we have to query the fbo and size before every rendering.
  auto &en = mineola::Engine::Instance();

  // Resize, should be checked before screen fbo change,
  // because otherwise a fbo of size (0, 0) will be set.
  if (view.drawableWidth != _width || view.drawableHeight != _height) {
    _width = (int)view.drawableWidth;
    _height = (int)view.drawableHeight;
    mineola::ResizeScreen(_width, _height);
  }

  // Screen fbo change
  GLint currentFBO = -1;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
  if (currentFBO != _currentFBO) {
    _currentFBO = currentFBO;
    mineola::SetScreenFramebuffer(_currentFBO, uint32_t(_width), uint32_t(_height));
  }

  en.FrameMove();
  en.Render();
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
