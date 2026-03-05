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
    GLKView* glView;
}

-(UIInterfaceOrientationMask)supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskLandscapeRight;
}

unsigned int VBO[1], VAO[1];
GLfloat vertices[] = {
    // first triangle
    -0.9f, -0.5f, 0.0f,  // left
    -0.0f, -0.5f, 0.0f,  // right
    -0.45f, 0.5f, 0.0f,  // top
    // second triangle
     0.0f, -0.5f, 0.0f,  // left
     0.9f, -0.5f, 0.0f,  // right
     0.45f, 0.5f, 0.0f   // top
};
GLuint vertexShader;
const char *vertexShaderSource =
"#version 300 es                            \n"
"layout(location = 0) in vec4 a_position;   \n"
"void main()                                \n"
"{                                          \n"
"   gl_Position = a_position;               \n"
"}                                          \n";

// todo: texture sampler
const char* fragmentShaderSrc =
"#version 300 es                                     \n"
"precision mediump float;                            \n"
"layout(location = 0) out vec4 outColor;             \n"
"void main()                                         \n"
"{                                                   \n"
"   outColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);         \n"
"}                                                   \n";
unsigned int fragmentShader;
unsigned int shaderProgram;
unsigned int samplerLoc, uniformS, uniformPos;

-(void)initGL {

    
    glGenBuffers(1, &VBO[0]);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glView.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    
    // shader
    
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if(vertexShader == 0) assert(0);
    
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    int  success;
    char infoLog[512] = {0};
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, sizeof(infoLog), NULL, infoLog);///glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        assert(0);
    }
    
    
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) assert(0);
    
    
    shaderProgram = glCreateProgram();
    
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    success = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        assert(0);
    }
    
    glUseProgram(shaderProgram);
    
    glEnableVertexAttribArray(0);
    
    glGenVertexArraysOES(1, &VAO[0]);
    glBindVertexArrayOES(VAO[0]);
    
    uniformS = glGetUniformLocation ( samplerLoc, "s_texture" );
    uniformPos = glGetUniformLocation( samplerLoc, "a_position");
}

-(void) awakeFromNib {
    
    
    [super awakeFromNib];
    
    glView = (GLKView*)self.view;
    
    glView.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    [glView.context setMultiThreaded:FALSE];
    
    self.initBlock = nil;
    
    self.view.frame = UIScreen.mainScreen.bounds;
    
    self.delegate = self;
    
//    effect = nil;
}

-(void) viewDidAppear:(BOOL)animated {
    
    [EAGLContext setCurrentContext:((GLKView*)self.view).context];
    
    //shrink inside of safe-area insets (iphone x)
    /*
    if (@available(iOS 11.0, *))
    {
        UIEdgeInsets insets = [self.view safeAreaInsets];

        CGRect childRect = [self.view frame];

        childRect.size.width -= insets.left + insets.right;
        childRect.origin.x += insets.left;

        [self.view setFrame: childRect];

        [[self.view superview] setBackgroundColor:[UIColor blackColor]];
    }
    */

    if(self.initBlock != nil) {
        self.initBlock(self.view.frame.size);
        self.initBlock = nil;
        
    }
    
    [self initGL];  // view dimensions set in initBlock

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

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(nullable UIEvent *)event
{
    touchStateTouchCount += [touches count];
    
    for (UITouch* touch in touches)
    {
        // coords ignore device orientation.
        // 0,0 origin is at upper-left corner
        // if phone is held straight up/down

        if(touch)
        {
            CGPoint pt = touchPointInView(touch, self.view);
            gameInterfaceTouchIDSet((int)(long)touch);
            gameInterfaceHandleTouchBegin(pt.x, pt.y);
        }
    }
    
    [super touchesBegan:touches withEvent:event];
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(nullable UIEvent *)event
{
    for (UITouch* touch in touches)
    {
        if(touch)
        {
            CGPoint pt = touchPointInView(touch, self.view);
            gameInterfaceTouchIDSet((int)(long)touch);
            gameInterfaceHandleTouchMove(pt.x, pt.y);
        }
    }
    
    [super touchesMoved:touches withEvent:event];
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(nullable UIEvent *)event;
{
    touchStateTouchCount -= [touches count];
    
    for (UITouch* touch in touches)
    {
        if(touch)
        {
            CGPoint pt = touchPointInView(touch, self.view);
            gameInterfaceTouchIDSet((int) (long) touch);
            gameInterfaceHandleTouchEnd(pt.x, pt.y);
        }
    }
    
    if(touchStateTouchCount == 0)
    {
        gameInterfaceHandleAllTouchEnd(); // dead
    }
    
    [super touchesEnded:touches withEvent:event];
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    for (UITouch* touch in touches) {

        if(touch)
        {
            CGPoint pt = [touch locationInView: self.view];
            int x = pt.x;
            int y = pt.y;
            gameInterfaceTouchIDSet((int) (long) touch);
            gameInterfaceHandleTouchEnd(x, y);
        }
    }
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


unsigned short indices[] = {0,1,2};
extern float viewWidth, viewHeight;
    
void glDrawFirstly(void) {
    glViewport(0,0, viewWidth, viewHeight);

}

void glDrawLastly(void) {
    
    glUseProgram(shaderProgram);
    
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnableVertexAttribArray(0);
    
    glBindVertexArrayOES(VAO[0]);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    
    // todo: see drawState2dSet
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glUniform1i ( samplerLoc, 0 );
    glUniform3f(uniformPos, 1, 1, 0);
    
    glDrawArrays(GL_TRIANGLES, 0, 3);
    return;
}
