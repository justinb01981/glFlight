//
//  glFlightGLKViewController.m
//  gl_flight
//
//  Created by Justin Brady on 1/26/19.
//

#import "glFlightGLKViewController.h"
#import "gameInput.h"
#import "gameInterface.h"
#import "glFlight.h"
#import "gameGlobals.h"
#import "world.h"
#import "PurchaseManager.h"


// TODO: -- relocate to an extension
extern int model_my_ship;
static void fulfillShipPurchase(void) {

    [PurchaseManager.shared purchase: ^(UIViewController* vc) {
        // cool
        printf("purchase attempting: %d", PurchaseManager.shared.purchased);
    }];

}

// --

@implementation glFlightGLKViewController
{
//    GLKBaseEffect* effect;
}

-(UIInterfaceOrientationMask)supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskLandscapeRight;
}

-(void) awakeFromNib {
    GLKView* glView;
    
    [super awakeFromNib];
    
    self.initBlock = nil;
    
    self.view.frame = UIScreen.mainScreen.bounds;
    
    self.delegate = self;
    
    glView = (GLKView*)self.view;
    
    glView.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
    [glView.context setMultiThreaded:FALSE];

    glView.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    
//    effect = nil;
}

-(void) viewDidAppear:(BOOL)animated {
    
    [EAGLContext setCurrentContext:((GLKView*)self.view).context];

    //shrink inside of safe-area insets (iphone x)
    if (@available(iOS 11.0, *))
    {
        UIEdgeInsets insets = [self.view safeAreaInsets];

        CGRect childRect = [self.view frame];

        childRect.size.width -= insets.left + insets.right;
        childRect.origin.x += insets.left;

        [self.view setFrame: childRect];

        [[self.view superview] setBackgroundColor:[UIColor blackColor]];
    }
    
    if(self.initBlock != nil) {
        self.initBlock(self.view.frame.size);
        self.initBlock = nil;
    }


    // init purchases
    [PurchaseManager.shared uponActivation:^{
        // activation successful
        printf("purchase successful: %d", PurchaseManager.shared.purchased);

        gameInterfaceActivateShip();

    }];
    glFlightOnPurchase = fulfillShipPurchase;
}



- (void)glkView: (GLKView*)glkView drawInRect: (CGRect)rect {
    
    if(self.initBlock != nil) {
        printf("glkView drawInRect called before init done");
        return;
    }
    
    [EAGLContext setCurrentContext:((GLKView*)self.view).context];

    gameInput();
    
    glFlightFrameStage1();
    
    glFlightFrameStage2();
}

-(void) viewInitialized: (void (^)(CGSize)) execute
{
    self.initBlock = execute;
}

-(void)startAnimation
{
    self.paused = NO;
    self.preferredFramesPerSecond = PLATFORM_TICK_RATE;
    
    ((GLKView*)self.view).delegate = self;
    
    [(GLKView*)self.view display];
}

-(void)stopAnimation
{
    self.paused = YES;
}

- (void)glkViewControllerUpdate:(nonnull GLKViewController *)controller {
    
}

- (void)encodeWithCoder:(nonnull NSCoder *)aCoder {
    
}

- (void)traitCollectionDidChange:(nullable UITraitCollection *)previousTraitCollection {
    
}

- (void)preferredContentSizeDidChangeForChildContentContainer:(nonnull id<UIContentContainer>)container {
    
}

- (CGSize)sizeForChildContentContainer:(nonnull id<UIContentContainer>)container withParentContainerSize:(CGSize)parentSize {
    return parentSize;
}

- (void)systemLayoutFittingSizeDidChangeForChildContentContainer:(nonnull id<UIContentContainer>)container {
    
}

- (void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(nonnull id<UIViewControllerTransitionCoordinator>)coordinator {
    
}

- (void)willTransitionToTraitCollection:(nonnull UITraitCollection *)newCollection withTransitionCoordinator:(nonnull id<UIViewControllerTransitionCoordinator>)coordinator {
    
}

- (void)didUpdateFocusInContext:(nonnull UIFocusUpdateContext *)context withAnimationCoordinator:(nonnull UIFocusAnimationCoordinator *)coordinator {
    
}

- (void)setNeedsFocusUpdate {
    
}

- (BOOL)shouldUpdateFocusInContext:(nonnull UIFocusUpdateContext *)context {
    return true;
}

- (void)updateFocusIfNeeded {
    
}

// mark: -- touch delegate

int touchStateTouchCount = 0;

CGPoint touchPointInView(UITouch* t, UIView* v)
{
    CGPoint p;
    
    float vw = [v bounds].size.width;
    float vh = [v bounds].size.height;
    
    p.x = ((vh - [t locationInView:v].y) / vh) * vw;
    p.y = ([t locationInView:v].x / vw) * vh;
    
    return p;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    int i;
    
    touchStateTouchCount += [touches count];
    
    for (i = 0; i < [touches count]; i++)
    {
        // coords ignore device orientation.
        // 0,0 origin is at upper-left corner
        // if phone is held straight up/down
        UITouch *touch = [[touches allObjects] objectAtIndex:i];
        if(touch)
        {
            [self.activeTouches addObject:touch];
            
            CGPoint pt = touchPointInView(touch, self.view);
            gameInterfaceControls.touchId = [self.activeTouches indexOfObject:touch]+1;
            gameInterfaceHandleTouchBegin(pt.x, pt.y);
        }
    }
    
    [super touchesBegan:touches withEvent:event];
    
    gameInterfaceControls.touchId = 0;
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    int i;
    
    for (i = 0; i < [touches count]; i++)
    {
        UITouch *touch = [[touches allObjects] objectAtIndex:i];
        if(touch)
        {
            CGPoint pt = touchPointInView(touch, self.view);
            gameInterfaceControls.touchId = [self.activeTouches indexOfObject:touch]+1;
            gameInterfaceHandleTouchMove(pt.x, pt.y);
        }
    }
    
    [super touchesMoved:touches withEvent:event];
    gameInterfaceControls.touchId = 0;
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    int i;
    
    touchStateTouchCount -= [touches count];
    
    for (i = 0; i < [touches count]; i++)
    {
        UITouch *touch = [[touches allObjects] objectAtIndex:i];
        
        if(touch)
        {
            [self.activeTouches removeObject:touch];
            
            CGPoint pt = touchPointInView(touch, self.view);
            gameInterfaceControls.touchId = [self.activeTouches indexOfObject:touch]+1;
            gameInterfaceHandleTouchEnd(pt.x, pt.y);
        }
    }
    
    if(touchStateTouchCount == 0)
    {
        gameInterfaceHandleAllTouchEnd();
    }
    
    [super touchesEnded:touches withEvent:event];
    gameInterfaceControls.touchId = 0;
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    int i;
    for (i = 0; i < [touches count]; i++)
    {
        UITouch *touch = [[touches allObjects] objectAtIndex:i];
        if(touch)
        {
            [self.activeTouches removeObject:touch];
            
            CGPoint pt = [touch locationInView: self.view];
            int x = pt.x;
            int y = pt.y;
            gameInterfaceControls.touchId = [self.activeTouches indexOfObject:touch]+1;
            gameInterfaceHandleTouchEnd(x, y);
        }
    }
    
    gameInterfaceControls.touchId = 0;
}

-(BOOL)shouldAutorotate
{
    return true;
}

-(BOOL)prefersHomeIndicatorAutoHidden
{
    return true;
}

@end
