//
//  textures.m
//  gl_flight
//
//  Created by Justin Brady on 1/17/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>

#include "gameIncludes.h"
#include "gameDebug.h"
#include "assert.h"

#include "textures.h"
#include "framebuffer.h"

// remove this hack but requires .h dependency redo
#ifndef GL_NONE
#define GL_NONE 0
#endif

unsigned int texture_list[MAX_TEXTURES] = {0};
unsigned int texture_list_loaded[MAX_TEXTURES] = {0};
GLubyte* texture_data_table[MAX_TEXTURES];
int texture_preload_count = 12;
const static unsigned char alpha_black = 0x00;
const static unsigned char alpha_semitrans = 0xD0;
char initTexturesPrefix[255];

static u_int32_t
convert_to_le_u32(u_int32_t v)
{
    unsigned int test = 1;
    unsigned char* ptr = (unsigned char*) &test;

    assert(sizeof(unsigned int) == 4);
    
    if(*ptr == 0)
    {
        //big-endian hardware
        unsigned char t;
        
        ptr = (unsigned char*) &v;
        
        t = ptr[0];
        ptr[0] = ptr[3];
        ptr[3] = t;
        t = ptr[1];
        ptr[1] = ptr[2];
        ptr[2] = t;
        
        return v;
    }
    else
    {
        //little-endian hardware
        return v;
    }
}

static int
read_bitmap_to_gltexture_with_replace(char replace_rgb_pixel_from[3], char replace_rgb_pixel_to[3], int tex_id)
{
    const int max_bitmap_dim = 4096;
    const int pixel_size = sizeof(char[4]);
    char file_name[255];
    
    FILE* fp = NULL;
    int err = -1;
    unsigned long file_size;
    unsigned char* data;
    int width = max_bitmap_dim;
    int height = max_bitmap_dim;
    
    sprintf(file_name, "%s""texture%d.bmp", initTexturesPrefix, tex_id);

    /*
     * bitmaps are 24-bit BMP files (usually 512x512 but some power of 2)
     * from 2048 - 2
     * exported with "do not write colorspace information checked
     * created with GIMP
     */

    assert(texture_data_table[tex_id] == NULL);
    
    fp = fopen(file_name, "rb");
    if(fp)
    {
        // seek to end
        fseek(fp, 0, SEEK_END);
        
        file_size = ftell(fp);
        if(file_size > 0)
        {
            unsigned long rgba_size = (file_size * 4) / 3;
            data = malloc(rgba_size);
            
            // find largest square bitmap this image could be
            while(width >= 2)
            {
                if(pixel_size * width * height <= rgba_size) break;
                
                width /= 2;
                height /= 2;
            }
            
            u_int32_t pixel_offset = 0;
            fseek(fp, 10, SEEK_SET);
            fread(&pixel_offset, 1, sizeof(pixel_offset), fp);
            // offset is little-endian
            pixel_offset = convert_to_le_u32(pixel_offset);

            fseek(fp, pixel_offset, SEEK_SET);
            
            DBPRINTF(("texture %s: size:%lu pixel_offset:%d\n", file_name, file_size, pixel_offset));
            
            if(data)
            {
                int offset = 0;
                while(offset < rgba_size)
                {
                    unsigned char pixel[4];

                    memset(pixel, 0x00, sizeof(pixel));
                    
                    int read_len = fread(pixel, 1, 3, fp);
                    
                    if (read_len < 3) {
                        DBPRINTF(("\tbail: read_len=%u, offset=%u\n", read_len, offset));
                        break;
                    }
                    
                    // little-endian-RGB -> RGBA
                    data[offset+0] = pixel[2];
                    data[offset+1] = pixel[1];
                    data[offset+2] = pixel[0];
                    
                    // set alpha channel, (0, 0, 0 color is transparent)
                    if(pixel[2] == 0 && pixel[1] == 0 && pixel[0] == 0)
                    {
                        data[offset+3] = alpha_black;
                    }
                    else if(pixel[2] == 1 && pixel[1] == 1 && pixel[0] == 1)
                    {
                        data[offset+3] = alpha_semitrans;
                    }
                    else
                    {
                        data[offset+3] = 0xff;
                    }
                    
                    // check for replace pixels
                    if(pixel[2] == replace_rgb_pixel_from[0] &&
                       pixel[1] == replace_rgb_pixel_from[1] &&
                       pixel[0] == replace_rgb_pixel_from[2])
                    {
                        memcpy(&(data[offset]), replace_rgb_pixel_to, 3);
                    }
                    
                    offset += sizeof(pixel);
                }
                
                texture_data_table[tex_id] = data;
                
                if(offset >= (pixel_size * width * height))
                {
                    glGenTextures(1, &texture_list[tex_id]);
                    glBindTexture(GL_TEXTURE_2D, texture_list[tex_id]);
                    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); 
                    
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                                 /*GL_BGRA*/ GL_RGBA, GL_UNSIGNED_BYTE, data);
                    
                    texture_list_loaded[tex_id] = 1;
                    err = 0;
                }
                
                // this is OK says openGL docs
                free(data);
                texture_data_table[tex_id] = NULL;
            }
        }
        
        fclose(fp);
    }
    else
    {
        DBPRINTF(("texture %s failed to load\n", file_name));
    }
    
    return err;
}

static int
read_bitmap_to_gltexture(int tex_id)
{
    char key_pixel[] = {0xFF, 0xFF, 0xFF};
    
    return read_bitmap_to_gltexture_with_replace(key_pixel, key_pixel, tex_id);
}

void initTextures(const char *prefix)
{

    uninitTextures();


    int i;
    strcpy(initTexturesPrefix, prefix);
    
    // for(i = 0; i < MAX_TEXTURES; i++)
    // {
    //     texture_list_loaded[i] = 0;
    //
    //     bindTextureRequestCore(i);
    // }

    
}

int bindTextureRequestCore(int tex_id)
{

    if(tex_id <= TEXTURE_ID_NONE)
    {
        return 0;
    }
    
    if(!texture_list_loaded[tex_id])
    {
        extern void console_clear(void);
        extern void console_write(char* fmt, ...);
        
        //console_clear();
        //console_write("loading texture: %d", tex_id);

        if(read_bitmap_to_gltexture(tex_id) != 0)
        {
            printf("failed to load texture: %d\n", tex_id);
            return 0;
        }
    }
    
    return texture_list_loaded[tex_id];
}

void textures_hack_framebuffer(GLuint newtx) {
//    // accept new texture rendered from framebuf
//    if(!texture_list_loaded[TEXTURE_ID_BOUNDING]) {
//        texture_list_loaded[TEXTURE_ID_BOUNDING] = 1;
//        texture_list[TEXTURE_ID_BOUNDING] = newtx;
//    }
}

void textures_hack_framebuffer_cleanup(void) {
    // accept new texture rendered from framebuf
//    texture_list_loaded[TEXTURE_ID_BOUNDING] = 0;
//    texture_list[TEXTURE_ID_BOUNDING] = -1;
}


void uninitTextures() {
    int i;
    for(i = 0; i < MAX_TEXTURES; i++)
    {
        if(texture_list_loaded[i]) {
            glDeleteTextures(1, &texture_list[i]);
            texture_list[i] = GL_NONE;
            texture_list_loaded[i] = 0;
        }
    }
}
