//
//  gl_flightViewController.m
//  gl_flight
//
//  Created by Justin Brady on 12/14/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//



THIS FILE IS DEAD -- REMOVE IT

#import <QuartzCore/QuartzCore.h>

#import <sys/time.h>

#import "gl_flightViewController.h"
#import "EAGLView.h" // remove
#import "cocoaGameAudio.h"

#include "object.h"
#include "models.h"
#include "world.h"
#include "textures.h"
#include "cocoaGameInput.h"
#include "gameInterface.h"
#include "gameAI.h"
#include "gameCamera.h"
#include "gameUtils.h"
#include "quaternions.h"
#include "gameAudio.h"
#include "gameGraphics.h"
#include "gameShip.h"
#include "world_file.h"
#include "gameNetwork.h"
#include "maps.h"
#include "action.h"
#include "glFlight.h"
#include "collision.h"
#include "gamePlay.h"


// Uniform index.
enum {
    UNIFORM_TRANSLATE,
    NUM_UNIFORMS
};
GLint uniforms[NUM_UNIFORMS];

// Attribute index.
enum {
    ATTRIB_VERTEX,
    ATTRIB_COLOR,
    NUM_ATTRIBUTES
};

////////////////////////////////////////////////////

@interface gl_flightViewController ()
@property (nonatomic, retain) EAGLContext *context;
@property (nonatomic, assign) CADisplayLink *displayLink;
@property (nonatomic, retain) NSMutableArray *activeTouches;

- (BOOL)loadShaders;
- (BOOL)compileShader:(GLuint *)shader type:(GLenum)type file:(NSString *)file;
- (BOOL)linkProgram:(GLuint)prog;
- (BOOL)validateProgram:(GLuint)prog;
@end

@implementation gl_flightViewController

@synthesize animating, context, displayLink;
@synthesize activeTouches;

-(UIInterfaceOrientationMask)supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskLandscapeRight;
}

-(BOOL)shouldAutorotate
{
    return true;
}

-(BOOL)prefersHomeIndicatorAutoHidden
{
    return true;
}

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    EAGLContext *aContext = [[EAGLContext alloc] initWithAPI: kEAGLRenderingAPIOpenGLES2];
    
    if (!aContext)
    {
        aContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
    }
    
    if (!aContext)
        NSLog(@"Failed to create ES context");
    else if (![EAGLContext setCurrentContext:aContext])
        NSLog(@"Failed to set ES context current");
    
	self.context = aContext;
	[aContext release];
	
    EAGLView* glView = (EAGLView*) self.view;
    //[glView setContentScaleFactor:2.0];
    
    // shrink inside of safe-area insets (iphone x)
    if (@available(iOS 11.0, *))
    {
        UIEdgeInsets insets = [glView safeAreaInsets];
        
        CGRect childRect = [glView frame];
        
        childRect.size.width -= insets.left + insets.right;
        childRect.origin.x += insets.left;
        
        [glView setFrame: childRect];
    
        [[glView superview] setBackgroundColor:[UIColor blackColor]];
    }
    
    [glView setContext:context];
    [glView setFramebuffer];
    [glView setMultipleTouchEnabled:true];
    
    if ([context API] == kEAGLRenderingAPIOpenGLES2)
    {
        [self loadShaders];
        
        [self validateProgram: program];
    }
    
    assert(glGetError() == GL_NO_ERROR);
    
    animating = FALSE;
    animationFrameInterval = 1;
    self.displayLink = nil;
}

- (void)dealloc
{
    if (program)
    {
        glDeleteProgram(program);
        program = 0;
    }
    
    // Tear down context.
    if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
    
    [context release];
    
    [super dealloc];
}

- (void)viewWillAppear:(BOOL)animated
{
    [self startAnimation];
    
    [super viewWillAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
    [self stopAnimation];
    
    [super viewWillDisappear:animated];
}

- (void)viewDidUnload
{
	[super viewDidUnload];
	
    if (program)
    {
        glDeleteProgram(program);
        program = 0;
    }

    // Tear down context.
    if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
	self.context = nil;	
}

- (NSInteger)animationFrameInterval
{
    return animationFrameInterval;
}

- (void)setAnimationFrameInterval:(NSInteger)frameInterval
{
    /*
	 Frame interval defines how many display frames must pass between each time the display link fires.
	 The display link will only fire 30 times a second when the frame internal is two on a display that
     refreshes 60 times a second. The default frame interval setting of one will fire 60 times a second
     when the display refreshes at 60 times a second. A frame interval setting of less than one results
     in undefined behavior.
	 */
    
    if (frameInterval >= 1)
    {
        animationFrameInterval = frameInterval;
        
        if (animating)
        {
            [self stopAnimation];
            [self startAnimation];
        }
    }
}

- (void)startAnimation
{
    if (!animating)
    {
        CADisplayLink *aDisplayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawFrame)];
        [aDisplayLink setFrameInterval:animationFrameInterval];
        [aDisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        self.displayLink = aDisplayLink;
        
        animating = TRUE;
    }
}

- (void)stopAnimation
{
    if (animating)
    {
        [self.displayLink invalidate];
        self.displayLink = nil;
        animating = FALSE;
    }
}

- (void)drawFrame
{
    /*
     * NOTES: doing expensive operations here not necessary to render the next frame
     * should be avoided so that the asynchronous OpenGL graphics rendering done by the
     * card doesn't skip frames
     */
    
    if (animating)
    {
        [(EAGLView *)self.view setFramebuffer];
        
        if([context API] == kEAGLRenderingAPIOpenGLES2)
        {
            glUseProgram(program);
        }
        
        gameInput();
        
        glFlightFrameStage1();

        [(EAGLView *)self.view presentFramebuffer];
        
        glFlightFrameStage2();
    }
    
    return;
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc. that aren't in use.
}

- (BOOL)compileShader:(GLuint *)shader type:(GLenum)type file:(NSString *)file
{
    GLint status;
    const GLchar *source;
    
    source = (GLchar *)[[NSString stringWithContentsOfFile:file encoding:NSUTF8StringEncoding error:nil] UTF8String];
    if (!source)
    {
        NSLog(@"Failed to load vertex shader");
        return FALSE;
    }
    
    *shader = glCreateShader(type);
    glShaderSource(*shader, 1, &source, NULL);
    glCompileShader(*shader);
    
#if defined(DEBUG)
    GLint logLength;
    glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetShaderInfoLog(*shader, logLength, &logLength, log);
        NSLog(@"Shader compile log:\n%s", log);
        free(log);
    }
#endif
    
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
    if (status == 0)
    {
        glDeleteShader(*shader);
        return FALSE;
    }
    
    return TRUE;
}

- (BOOL)linkProgram:(GLuint)prog
{
    GLint status;
    
    glLinkProgram(prog);
    
#if defined(DEBUG)
    GLint logLength;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        NSLog(@"Program link log:\n%s", log);
        free(log);
    }
#endif
    
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status == 0)
        return FALSE;
    
    return TRUE;
}

- (BOOL)validateProgram:(GLuint)prog
{
    GLint logLength, status;
    
    glValidateProgram(prog);
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        NSLog(@"Program validate log:\n%s", log);
        free(log);
    }
    
    glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
    if (status == 0)
        return FALSE;
    
    return TRUE;
}

- (BOOL)loadShaders
{
    GLuint vertShader, fragShader;
    NSString *vertShaderPathname, *fragShaderPathname;
    
    // Create shader program.
    program = glCreateProgram();
    
    // Create and compile vertex shader.
    vertShaderPathname = [[NSBundle mainBundle] pathForResource:@"Shader" ofType:@"vsh"];
    if (![self compileShader:&vertShader type:GL_VERTEX_SHADER file:vertShaderPathname])
    {
        NSLog(@"Failed to compile vertex shader");
        return FALSE;
    }
    
    // Create and compile fragment shader.
    fragShaderPathname = [[NSBundle mainBundle] pathForResource:@"Shader" ofType:@"fsh"];
    if (![self compileShader:&fragShader type:GL_FRAGMENT_SHADER file:fragShaderPathname])
    {
        NSLog(@"Failed to compile fragment shader");
        return FALSE;
    }
    
    // Attach vertex shader to program.
    glAttachShader(program, vertShader);
    
    // Attach fragment shader to program.
    glAttachShader(program, fragShader);
    
    // Bind attribute locations.
    // This needs to be done prior to linking.
    glBindAttribLocation(program, ATTRIB_VERTEX, "position");
    //glBindAttribLocation(program, ATTRIB_COLOR, "color");
    
    // Link program.
    if (![self linkProgram:program])
    {
        NSLog(@"Failed to link program: %d", program);
        
        if (vertShader)
        {
            glDeleteShader(vertShader);
            vertShader = 0;
        }
        if (fragShader)
        {
            glDeleteShader(fragShader);
            fragShader = 0;
        }
        if (program)
        {
            glDeleteProgram(program);
            program = 0;
        }
        
        return FALSE;
    }
    
    // Get uniform locations.
    uniforms[UNIFORM_TRANSLATE] = glGetUniformLocation(program, "translate");
    
    // Release vertex and fragment shaders.
    if (vertShader)
        glDeleteShader(vertShader);
    if (fragShader)
        glDeleteShader(fragShader);
    
    return TRUE;
}

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
            [activeTouches addObject:touch];
            
            CGPoint pt = touchPointInView(touch, self.view);
            gameInterfaceControls.touchId = [activeTouches indexOfObject:touch]+1;
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
            gameInterfaceControls.touchId = [activeTouches indexOfObject:touch]+1;
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
            [activeTouches removeObject:touch];
            
            CGPoint pt = touchPointInView(touch, self.view);
            gameInterfaceControls.touchId = [activeTouches indexOfObject:touch]+1;
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
            [activeTouches removeObject:touch];
            
            CGPoint pt = [touch locationInView: self.view];
            int x = pt.x;
            int y = pt.y;
            gameInterfaceControls.touchId = [activeTouches indexOfObject:touch]+1;
            gameInterfaceHandleTouchEnd(x, y);
        }
    }
    
    gameInterfaceControls.touchId = 0;
}

@end
