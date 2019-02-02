//
//  glFlightGLKViewController.h
//  gl_flight
//
//  Created by Justin Brady on 1/26/19.
//

#import <GLKit/GLKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface glFlightGLKViewController: GLKViewController <GLKViewControllerDelegate, GLKViewDelegate>

@property (nonatomic, retain) GLKView* view;

@property (nonatomic, nullable) void (^initBlock)(CGSize);
@property (nonatomic, retain) NSMutableArray *activeTouches;

-(void) viewInitialized: (void (^)(CGSize)) execute;

-(void) startAnimation;
-(void) stopAnimation;

@end

NS_ASSUME_NONNULL_END
