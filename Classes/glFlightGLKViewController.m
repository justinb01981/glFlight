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
#import "gameGraphics.h"
#import "world.h"
#import "PurchaseManager.h"
#include "textures.h"

GLfloat const vertices[] =
{
    -1.0, 1.0, 1,
    1.0, 1.0, 1,
    1.0, -1.0, 1,
    -1.0, -1.0, 1
};

GLfloat const txUv[] =
{
    -1.0,-1.0,
    1.0,-1.0,
    1.0,1.0,
    -1.0,1.0
};

GLushort elements[] = {
   0,1,2,
   2,3,0
};

// TODO: -- relocate to an extension
extern int model_my_ship;
static void fulfillShipPurchase(void) {

    [PurchaseManager.shared purchase: ^(UIViewController* vc) {
        // cool
        printf("purchase attempting: %d", PurchaseManager.shared.purchased);
    }];

}

#pragma mark: globals
const char *vertexShaderSource =    ""
"#version 300 es                            \n"
"precision highp float;                     \n"
"precision highp int;                       \n"
"layout(std140, column_major) uniform;      \n"
""
"uniform transform                          \n"
"{                          \n"
"    mat4 MVP;                              \n"
"    mat4 persp;                            \n"
"} Transform;                               \n"
"\n"
"in vec3 Position;                          \n"
"in vec2 Texcoord;                          \n"
"\n"
"out vec2 Fragtexco;                        \n"
"void main()                                \n"
"{                                          \n"
"    Fragtexco = Texcoord;                  \n"
"    gl_Position = Transform.persp * Transform.MVP * vec4(Position, 1.0);  \n"
"}                                          \n";

const char* fragmentShaderSrc = ""
"#version 300 es                                    \n"
"precision highp float;                              \n"
"precision highp int;                                \n"
"layout(std140, column_major) uniform;               \n"
""
"uniform sampler2D Diffuse;                          \n"
""
"in vec2 Fragtexco;                                  \n"
""
"layout (location = 0) out vec4 Color;               \n"
""
"void main()                                        \n"
"{                                                  \n"
"    Color = texture(Diffuse, Fragtexco);           \n"
"}                                                  \n";


const unsigned tx_dim = 8;

unsigned short indices[] = {0,1,2, 2,3,0};
extern float viewWidth, viewHeight;
float camDist = 3.0;
int tex_id = 2;
float Zee = 1.5;
GLfloat X = 0.0, Y = 0.0, Z = 0.0, gTheta = 0.0, gRad = 3;

const mat4 Ident = MAT4IDENT();
mat4  *MxProjection, *MxModel, *Pointer;
mat4 cur = MAT4IDENT(),
    scale = {
        1.00, 0.0, 0.0, 0.0,
        0.0, 1.00, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    };

#pragma mark: render

typedef struct ShaderPrep {
    const GLfloat *vert, *texUVco, *stor, *write;
    size_t lVert, lTexUv;
    
} ShaderPrep;

ShaderPrep gShaderPrep = {.vert = 0, .texUVco = 0, .stor = 0 , .lVert = 0, .lTexUv = 0};

void glPrepareShaderAttributesVertices(const GLfloat* src, const size_t len) {
    gShaderPrep.vert = src;
    gShaderPrep.lVert = len;
}

void glPrepareShaderAttributesTexUV(const GLfloat* src, const size_t len) {
    gShaderPrep.texUVco = src;
    gShaderPrep.lTexUv = len;
}

int texK = 1, texKmax = 6400;
void glPrepareShaderAttributesDraw(const GLushort* src, const size_t len) {
    int i;
    
    assert(gShaderPrep.stor);
    
    gShaderPrep.write = gShaderPrep.stor;
    
    GLfloat* vTmp = gShaderPrep.vert;
    GLfloat* tTmp = gShaderPrep.texUVco;
    
    glBindTexture(GL_TEXTURE_2D, texture_list[texK / 100]);
    texK += 2;
    if(texK > texKmax) texK = 1;
    
    // copy over to bound shader attributes
    for(i=0; i< len; i++) {
        // fill position attr
        memcpy(gShaderPrep.write, vTmp, sizeof(vec3));
        vTmp += 3; gShaderPrep.write += 3;
        
        // fill texture U,V position attr
        memcpy(gShaderPrep.write, tTmp, sizeof(vec2));
        tTmp += 2; gShaderPrep.write += 2;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, BufferName[BUFFERNAME_VTX]);
    glBufferData(GL_ARRAY_BUFFER, (gShaderPrep.lTexUv + gShaderPrep.lVert) * sizeof(GLfloat), gShaderPrep.stor, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, len, src, GL_STATIC_DRAW);
    
    assert(gShaderPrep.vert && gShaderPrep.texUVco);
    
    glDrawElements(GL_TRIANGLES, (GLsizei) len, GL_UNSIGNED_SHORT, 0);  // passing NULL indicates to use bound..um...buffers
}

void glDrawSecondly(void) {
    
}

void glDrawFirstly(void) {
    
    int r;
    
    while((r = glGetError() && r != 0)) {
        // todo: happens elsewhere external during drawing
        printf("glGetError ignored: %u\n",  r);
        if(r != 1) assert(0);
    }

    glViewport(0,0, viewWidth, viewHeight);
    
    {
        glBindBuffer(GL_ARRAY_BUFFER, BufferName[BUFFERNAME_VTX]);
        glBindBuffer(GL_UNIFORM_BUFFER, BufferName[BUFFERNAME_UTX]);
        mat4u* Pointer = (mat4u*) glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(mat4u), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        
        assert(Pointer);
        
        Y = sin(gTheta) * gRad;
        Z = cos(gTheta) * gRad;
        
        gTheta += 0.01;
        
        mat4 drawPos = MAT4ROTY(gTheta), trans = MAT4TRANSLATE(0, 0, 1.5);
        
        mat4 Perspective = MAT4PERSPECTIVE(Zee, ZeePfoc);
        //mat4 Perspective = MAT4ORTHO(viewWidth, viewHeight);

        MAT4XLT_inplace(drawPos, trans);
        
        (*Pointer).f = drawPos;
        (*Pointer).p = Perspective;
        
        glUnmapBuffer(GL_UNIFORM_BUFFER);
    }
}

void glDrawLastly(void) {

    vec4 cl = {.f =  0, 0, 0, 1};
    glClearBufferfv(GL_COLOR, 0, &cl.f[0]);

    glUseProgram(shaderProgram);

    // keep bufferData (building) confined to init not render

    glBindBufferBase(GL_UNIFORM_BUFFER, VERTXATTRIB_XFORM, BufferName[BUFFERNAME_UTX]);
    glBindVertexArray(VAONames[VERTXATTRIB_XFORM]);
    
    /*
    glDrawElements(GL_TRIANGLES, sizeof(elements)/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);  // passing NULL indicates to use bound..um...buffers
     */
    glPrepareShaderAttributesDraw(elements, sizeof(elements));
    
    // unbind
    
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    assert( !glGetError() );
    return;
}

#pragma mark: glFlightGLKViewController
@implementation glFlightGLKViewController
{
    GLKView* glView;
}

-(UIInterfaceOrientationMask)supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskLandscapeRight;
}

-(void)initGL {
    
    if(!gShaderPrep.stor) {
        gShaderPrep.stor = malloc(sizeof(GLfloat)*65536); // todo: free
    }
    
    glGenBuffers(BUFFERNAME_MAX, &BufferName[0]);
    
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
        printf("%s\n", infoLog);
        assert(0);
    }
    
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragmentShader, sizeof(infoLog), NULL, infoLog);///glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("%s\n", infoLog);
        assert(0);
    }
    
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    
    glBindAttribLocation(shaderProgram, VERTXATTRIB_XFORM, "Position");
    glBindAttribLocation(shaderProgram, VERTXATTRIB_TXPOS, "Texcoord");
    //glBindFragDataLocation(shaderProgram, VERTXATTRIB_COLOR, "Color");
//    https://stackoverflow.com/questions/19064055/os-x-opengl-3-2-doesnt-include-glbindfragdatalocation

    initBuffer();
    
    initTexture();
    
    initVertexArray();
    
    int blockIdx = glGetUniformBlockIndex(shaderProgram, "transform");
    glUniformBlockBinding(shaderProgram, blockIdx, VERTXATTRIB_XFORM);
    
    glUseProgram(shaderProgram);
    
    UniformEnvironment = glGetUniformLocation(shaderProgram, "Diffuse");
    
//    uniformS = glGetUniformLocation ( samplerLoc, "Color");
    uniformPos = glGetUniformLocation( samplerLoc, "Position");
    glUniform1i(glGetUniformLocation(samplerLoc, "Diffuse"), 1);
    
    glLinkProgram(shaderProgram);
    success = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("%s", infoLog);
        assert(0);
    }
    
    //assert(uniformS != 0 && uniformPos != 0);
}

static bool initVertexArray(void)
{
    glGenVertexArrays(1, &VAONames[VERTXATTRIB_XFORM]);
    glBindVertexArray(VAONames[VERTXATTRIB_XFORM]);
    
    glBindBuffer(GL_ARRAY_BUFFER, BufferName[BUFFERNAME_VTX]);
    
    glVertexAttribPointer(VERTXATTRIB_XFORM, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_v2fv2f), BUFFER_OFFSET(0));
    glVertexAttribPointer(VERTXATTRIB_TXPOS, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_v2fv2f), BUFFER_OFFSET(sizeof(vec3)));
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glEnableVertexAttribArray(VERTXATTRIB_XFORM);
    glEnableVertexAttribArray(VERTXATTRIB_TXPOS);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[BUFFERNAME_ELX]);
    glBindVertexArray(0);
    
    return true;
}

static bool initBuffer(void) {
    
    UniformBufferOffset = 0;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
    GLint UniformBlockSize = MAX(sizeof(mat4u), UniformBufferOffset);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[BUFFERNAME_ELX]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);  /* "indices" formerly */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    // todo: see drawState2dSet
    glBindBuffer(GL_ARRAY_BUFFER, BufferName[BUFFERNAME_VTX]);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_UNIFORM_BUFFER, BufferName[BUFFERNAME_UTX]);
    glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, NULL, GL_DYNAMIC_DRAW);
    
    glPrepareShaderAttributesVertices(vertices, sizeof(vertices)/sizeof(GLfloat));
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    return true;
}

static bool initTexture(void) {
    int tx_lev = 1;
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    glGenTextures(1, &texture_list[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_list[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, tx_lev-1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, tx_lev-1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tx_lev == 1 ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, -1000.f);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 1000.f);
    //    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f);
    
    int width = tx_dim, height = tx_dim;
    txData = malloc(width * height * sizeof(GLuint) + 64);
    for(int off = 0; off < width*height; off++) {
        txData[off] = rand() % 0xEF<<16 | rand() % 0xEF<<8 | rand() % 0xEF<<0  | 0xff000000;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 /*GL_BGRA*/ GL_RGBA, GL_UNSIGNED_BYTE, txData);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    
    glPrepareShaderAttributesTexUV(txUv, sizeof(txUv)/sizeof(GLfloat));
    
    return true;
}

-(void) viewDidAppear:(BOOL)animated{
    
    glView = (GLKView*)self.view;
    
    if(!glView.context) glView.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    
    [EAGLContext setCurrentContext:glView.context];
    
    [self startAnimation];
    
    // init purchases
    [PurchaseManager.shared uponActivation:^{
        // activation successful
        printf("purchase successful: %d", PurchaseManager.shared.purchased);
        
        gameInterfaceActivateShip();
        
    }];
    glFlightOnPurchase = fulfillShipPurchase;
}

- (void)glkView: (GLKView*)glkView drawInRect: (CGRect)rect {
    [EAGLContext setCurrentContext:glView.context];
    
    if(self.initBlock != nil) {
        self.initBlock(self.view.frame.size);
        self.initBlock = nil;
        
        printf("glErr@initblock: \n", glGetError());
        assert(!glGetError());
    }
    
    gameInput();
    
    glFlightFrameStage1();
    
    glFlightFrameStage2();
}

-(void) viewWillDisppear:(BOOL)animated {
    [self stopAnimation];
}

-(void) viewInitialized: (void (^)(CGSize)) execute
{
    self.initBlock = execute;
}

-(void)startAnimation
{
    [self initGL];
    
    self.paused = NO;
    self.preferredFramesPerSecond = PLATFORM_TICK_RATE;
    
    ((GLKView*)self.view).delegate = self;
}

-(void)stopAnimation
{
    self.paused = YES;
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



