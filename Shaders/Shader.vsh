//
//  Shader.vsh
//  gl_flight
//
//  Created by Justin Brady on 12/14/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

//uniform mat4 mvp_matrix;

attribute vec4 position;
//attribute vec4 color;
out vec4 vertexColor; // specify a color output to the fragment shader

//varying vec4 colorVarying;

//uniform float translate;

void main()
{
    //gl_Position = position * mvp_matrix;
    gl_Position = position;
    gl_ = vec4(0.5, 0.0, 0.0, 1.0);
    //gl_Position.y += sin(translate) / 2.0;
    //gl_Position.x += sin(translate) / 2.0;

    //colorVarying = color;
}
