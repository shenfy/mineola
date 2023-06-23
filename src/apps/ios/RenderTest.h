//
//  TestRenderApp.h
//  mineola
//
//  Created by Fangyang Shen on 2018/9/11.
//  Copyright © 2018 Fangyang Shen. All rights reserved.
//

#import <Foundation/Foundation.h>
#include <mineola/RendererViewController.h>

@interface RenderTest : NSObject <RenderDelegate>

- (void) initScene;
- (void) started;

@end
