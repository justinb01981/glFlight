//
//  textures.h
//  gl_flight
//
//  Created by Justin Brady on 1/17/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#ifndef __TEXTURES_H__
#define __TEXTURES_H__

#define MAX_TEXTURES 256

enum
{
    TEXTURE_ID_POOPED_CUBE = 33,
    TEXTURE_ID_POOPED_CUBE_BOOST = 2,
    TEXTURE_ID_POOPED_CUBE_ENEMY = 74,
    TEXTURE_ID_POOPED_CUBE_SHIELDLOW = 66,
    TEXTURE_ID_POOPED_CUBE_SHIELDLOW_ENEMY = 74,
    TEXTURE_ID_BULLET = /*2*/ /*66*/ 87,
    TEXTURE_ID_MISSLE = 53,
    TEXTURE_ID_SHIP1 = /*1*/ /*73*/81,
    TEXTURE_ID_SHIP2 = /*1*/ 13,
    TEXTURE_ID_SHIP3 = /*1*/ 23,
    TEXTURE_ID_SHIP4 = /*1*/ 24,
    TEXTURE_ID_SHIP5 = /*1*/ 54,
    TEXTURE_ID_SHIP6 = /*1*/ 55,
    TEXTURE_ID_SHIP7 = /*1*/ 56,
    TEXTURE_ID_SHIP8 = /*1*/ 28,
    TEXTURE_ID_BRICKWALL = 17,
    TEXTURE_ID_BLOCK = /*19*//*28*/31,
    TEXTURE_ID_BUILDING = 16,
    TEXTURE_ID_EXPLOSION = 34,
    TEXTURE_ID_FILELOST = 104,
    TEXTURE_ID_WRECKAGE = 22,
    TEXTURE_ID_SPAWNPOINT = /*18*/ /*42*/ 91,
    TEXTURE_ID_ENEMY_SPAWNPOINT = /*18*/ /*58*/89,
    TEXTURE_INTERFACE_FIRE = 6,
    TEXTURE_ID_TURRET = 35,
    TEXTURE_ID_ENEMYSHIP = 24,
    TEXTURE_ID_ENEMYSHIP_ACE = 71,
    TEXTURE_ID_ALLYSHIP = 13,
    TEXTURE_ID_BACKGROUND = /*27*/96,
    TEXTURE_ID_BACKGROUND2 = 61,
    TEXTURE_ID_CAPTUREPOINT = 29,
    TEXTURE_ID_POWERUP = 30,
    TEXTURE_ID_ASTEROID = 14,
    TEXTURE_ID_DATA = 40,
    TEXTURE_ID_RADAR_BLUE = 2,
    TEXTURE_ID_RADAR_RED = 9,
    TEXTURE_ID_RADAR_BEHIND = 46,
    TEXTURE_ID_OBJECTIVE_FIREWALL = 94,
    TEXTURE_ID_FONTMAP = /*12*/ 44,
    TEXTURE_ID_FONTMAP_SEMITRANS = /*12*/ /*44*/ 70,
    TEXTURE_ID_FONTMAP2 = /*12*/ 68,
    TEXTURE_ID_PICKUP_EXPLOSION = 18,
    TEXTURE_ID_POWERUP_LIFE = 30,
    TEXTURE_ID_POWERUP_SHIP = 43,
    TEXTURE_ID_MAINMENU = 43,
    TEXTURE_ID_POWERUP_TURRET = 30,
    TEXTURE_ID_POWERUP_MISSLE = 75,
    TEXTURE_ID_POWERUP_POINTS = 91,
    TEXTURE_ID_LOCKEDON = /*50*/ 65,
    TEXTURE_ID_CONTROLS_TEXTMENU = /*69*/ 48,
    TEXTURE_ID_CONTROLS_FIRE = 6,
    TEXTURE_ID_CONTROLS_MISSLE = /*50*/ 65,
    TEXTURE_ID_CONTROLS_HELP = 51,
    TEXTURE_ID_BASE = 17,
    TEXTURE_ID_CONTROLS_TOW = 59,
    TEXTURE_ID_CONTROLS_BOOST = 67,
    TEXTURE_ID_CONTROLS_ROLL = 93,
    TEXTURE_ID_RADAR_OBJECTIVE = 62,
    TEXTURE_ID_DATA_GRABBED = 86,
    TEXTURE_ID_DATA_GRABBED2 = 102,
    TEXTURE_ID_CONTROLS_DIALOG_BOX = 63,
    TEXTURE_ID_CONTROLS_RADAR = 20,
    TEXTURE_ID_STATIC = 82,
    TEXTURE_ID_UNKNOWN = 0,
    TEXTURE_ID_ARROW_OBJ = 100,
    TEXTURE_ID_ARROW_ENEMY = 101,
    TEXTURE_ID_ARROW_PLAYER = 103,
    TEXTURE_ID_LAST
};

const static int texture_id_table[] =
{
    TEXTURE_ID_POOPED_CUBE,
    TEXTURE_ID_BLOCK,
    TEXTURE_ID_SPAWNPOINT,
    TEXTURE_ID_SHIP1
};

enum {
    TEXTURE_ID_TABLE_IDX_POOPEDCUBE = 0,
    TEXTURE_ID_TABLE_IDX_BLOCK,
    TEXTURE_ID_TABLE_IDX_SPAWNPOINT,
    TEXTURE_ID_TABLE_IDX_SHIP,
    TEXTURE_ID_TABLE_IDX_LAST
};

enum { TEXTURE_ANIMATION_LEN = 10 };

enum {
    TEXTURE_ANIMATION_SPAWNPOINT = 1,
    TEXTURE_ANIMATION_LAST
};

const static int TEXTURE_ANIMATION_TABLE[][TEXTURE_ANIMATION_LEN] =
{
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {TEXTURE_ID_SPAWNPOINT, TEXTURE_ID_STATIC, TEXTURE_ID_STATIC, TEXTURE_ID_STATIC, TEXTURE_ID_SPAWNPOINT,
        TEXTURE_ID_SPAWNPOINT, TEXTURE_ID_SPAWNPOINT, TEXTURE_ID_SPAWNPOINT, TEXTURE_ID_SPAWNPOINT, TEXTURE_ID_SPAWNPOINT},
    {TEXTURE_ID_DATA_GRABBED, TEXTURE_ID_DATA_GRABBED2, TEXTURE_ID_DATA_GRABBED2, TEXTURE_ID_DATA_GRABBED2, TEXTURE_ID_DATA_GRABBED,
        TEXTURE_ID_DATA_GRABBED, TEXTURE_ID_DATA_GRABBED, TEXTURE_ID_DATA_GRABBED, TEXTURE_ID_DATA_GRABBED, TEXTURE_ID_DATA_GRABBED}
};

static inline int texture_animated(int texture_id, int idx)
{
    if(texture_id == TEXTURE_ID_SPAWNPOINT)
    {
        return TEXTURE_ANIMATION_TABLE[1][idx];
    }
    else if(texture_id == TEXTURE_ID_DATA_GRABBED)
    {
        return TEXTURE_ANIMATION_TABLE[2][idx];
    }
    
    return texture_id;
}

typedef unsigned char pixelRGBA[4];

extern unsigned int texture_list[];
extern int n_textures;

extern int texture_id_playership;

extern int texture_id_background;

void initTextures(const char *prefix);

int bindTextureRequest(int tex_id);

#endif
