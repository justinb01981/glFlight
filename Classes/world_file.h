//
//  world_file.h
//  gl_flight
//
//  Created by jbrady on 11/8/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef gl_flight_world_file_h
#define gl_flight_world_file_h

#define WORLD_MAP_COMMAND_PARAMS_MAX 16

typedef enum
{
    MAP_SET_SIZE,
    MAP_ADD_BOUNDING_VEC,
    MAP_ADD_OBJECT,
    MAP_SET_BLOCK_SCALE,
    MAP_SET_OBJECT_INFO,
    MAP_SET_OBJECT_SUB_INFO,
    MAP_ADD_OBJECTS_PACKED,
    MAP_ADD_MESH,
    MAP_MANIP_MESH_ADD,
    MAP_MANIP_MESH_INFO,
    MAP_MANIP_MESH_PULL,
    MAP_MANIP_MESH_SET_ROUND,
    MAP_MANIP_MESH_COMPLETE,
    MAP_ADD_SPAWN_POINT,
    MAP_ADD_ENEMY_JET,
    MAP_ADD_ENEMY_TURRET,
    MAP_ADD_TELEPORTER,
    MAP_REGISTER_PARAMS,
    MAP_REGISTER_UPDATE_PARAMS,
    MAP_REGISTER_CLEAR_PARAMS,
    MAP_REGISTER_UPDATE_MUL_PARAMS,
    MAP_DEFINE_PLANE,
    MAP_SET_BG_INFO,
    MAP_ADD_VECTOR,
    MAP_SET_OBJECT_VELOCITY,
    MAP_IDENT_OBJECT
} world_map_command_type;

typedef struct
{
    world_map_command_type type;
    float params[WORLD_MAP_COMMAND_PARAMS_MAX];
    char cparams[255];
    int n_params;
} world_map_command;

typedef struct
{
    char filedata[1];
} world_map_copy;

void    gameMapFileName(char* name);
void    gameMapFilePrefix(char *prefix);
void    gameMapWrite();
char*   gameMapRead();
void    gameMapReRender();
char*   gameMapReadRendered();
void    gameMapSetMap(const char *data);

#endif
