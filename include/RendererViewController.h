#ifndef MINEOLA_RENDERVIEWCONTROLLER_H
#define MINEOLA_RENDERVIEWCONTROLLER_H

#import <GLKit/GLKit.h>

@protocol RenderDelegate;

@interface RendererViewController : GLKViewController

@property (nonatomic) id<RenderDelegate> renderDelegate;

@end

@protocol RenderDelegate <NSObject>
@optional
- (void) initScene;

@end

#endif
