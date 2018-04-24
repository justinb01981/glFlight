//
//  maps.h
//  gl_flight
//
//  Created by jbrady on 11/13/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef gl_flight_maps_h
#define gl_flight_maps_h

#include "textures.h"

/*
 * models
 */
#define MAPMODEL_SHIP1 "0"
#define MAPMODEL_CUBE "1"
#define MAPMODEL_CUBE2 "2"
#define MAPMODEL_PYRAMID "3"
#define MAPMODEL_SQUARE "4"
#define MAPMODEL_BULLET "5"
#define MAPMODEL_SURFACE "6"
#define MAPMODEL_VERTICAL_PILLAR "7"
#define MAPMODEL_MESH "8"
#define MAPMODEL_TELEPORTER "9"
#define MAPMODEL_SPRITE "10"
#define MAPMODEL_TURRET "11"
#define MAPMODEL_SHIP2 "12"
#define MAPMODEL_MISSLE "13"
#define MAPMODEL_SHIP3 "14"
#define MAPMODEL_ICOSAHEDRON "15"
#define MAPMODEL_CUBE_INVERTED "16"
#define MAPMODEL_SPHERE "17"
#define MAPMODEL_ENEMYBASE "18"

/*
 (Model type, float x, float y, float z, float yaw, float pitch, float roll, float scale, int texture_id)
*/
#define WORLD_ADD_OBJECT(model, x, y, z, yaw, pitch, roll, scale, tx_id) \
"add_object "#model" "#x" "#y" "#z" "#yaw" "#pitch" "#roll" "#scale" "#tx_id"\n"

#define MESH_CUBES_1(tex, scale)                                 \
"mesh_manip_add wx0.0 " " 5" " wz0" " 5 0 0" " 0 0 5" " 40 40\n" \
"mesh_manip_pull rnd_40 rnd_40"" 0 rnd_50 0"" 0.90\n"            \
"mesh_manip_pull rnd_40 rnd_40"" 0 rnd_50 0"" 0.90\n"            \
"mesh_manip_pull rnd_40 rnd_40"" 0 rnd_50 0"" 0.90\n"            \
"mesh_manip_pull rnd_40 rnd_40"" 0 rnd_50 0"" 0.90\n"            \
"mesh_manip_pull rnd_40 rnd_40"" 0 rnd_50 0"" 0.90\n"            \
"mesh_manip_pull rnd_40 rnd_40"" 0 rnd_50 0"" 0.90\n"            \
"mesh_manip_complete 1 "#tex" 1 "#scale"\n"

#define WORLD_SCALED_FRAME_MESH_PULL_RANDOM(height, range)       \
"register_params rndx rndz 0 "#height" 0 0 0 0 0 0 0 0 0 0 0 0\n"       \
"register_params_mul 0.2 0.2 1 1 1 1 1 1 1 1 1 1 1 1 1 1\n"      \
"mesh_manip_pull r r r r 0"" "#range"\n" \
"register_params_update 0 0 8 0 0 0 0 0 0 0 0 0 0 0 0 0\n"  \
"add_object 1 r r r 0 0 0 4 16\n"

#define WORLD_CUBEGRID(y, tex, scale) \
"mesh_manip_add 0 " #y" 0" " 4 0 0" " 0 0 4" " wx0.25 wz0.25\n" \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(50, 0.85) \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(50, 0.85) \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(50, 0.85) \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(50, 0.85) \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(50, 0.85) \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(50, 0.85) \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(50, 0.85) \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(50, 0.85) \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(50, 0.85) \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(50, 0.85) \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(50, 0.85) \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(50, 0.85) \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(50, 0.85) \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(50, 0.85) \
"mesh_manip_round 0 1 0\n" \
"mesh_manip_complete 1 "#tex" 1 "#scale"\n"

#define WORLD_SCALED_FRAME(x, tex, scale)                                \
"register_params 400 100 400 0 0 0 0 0 0 0 0 0 0 0 0 0\n"                \
"register_params_mul "#x" "#x" "#x" 1 1 1 1 1 1 1 1 1 1 1 1 1\n"         \
"set_world_size r r r\n" \
"\n"

#define WORLD_SCALED_FRAME_GL_TERRAIN(x, tex, scale)                                \
"register_params 200 100 200 0 0 0 0 0 0 0 0 0 0 0 0 0\n"                \
"register_params_mul "#x" "#x" "#x" 1 1 1 1 1 1 1 1 1 1 1 1 1\n"         \
"set_world_size r r r\n"                                                 \
"mesh_manip_add 0 " "0" " 0" " 4 0 0" " 0 0 4" " wx0.25 wz0.25\n" \
"mesh_manip_gltriangles_complete "#tex"\n"

#define WORLD_SCALED_FRAME_TERRAIN(y, tex, scale) \
WORLD_CUBEGRID(y, tex, scale)

#define RANDOM_FLOATING_BLOCKS_1                                 \
"add_object 1 rndx rndy rndz rndr rndr rndr 2 18\n"              \
"add_object 1 rndx rndy rndz rndr rndr rndr 2 18\n"              \
"add_object 1 rndx rndy rndz rndr rndr rndr 2 18\n"              \
"add_object 1 rndx rndy rndz rndr rndr rndr 2 18\n"              \
"add_object 1 rndx rndy rndz rndr rndr rndr 2 18\n"              \
"add_object 1 rndx rndy rndz rndr rndr rndr 2 18\n"              \
"add_object 1 rndx rndy rndz rndr rndr rndr 2 18\n"              \
"add_object 1 rndx rndy rndz rndr rndr rndr 2 18\n"              \
"add_object 1 rndx rndy rndz rndr rndr rndr 2 18\n"              \
"add_object 1 rndx rndy rndz rndr rndr rndr 2 18\n"              \
"add_object 1 rndx rndy rndz rndr rndr rndr 2 18\n"              \
"add_object 1 rndx rndy rndz rndr rndr rndr 2 18\n"              \
"add_object 1 rndx rndy rndz rndr rndr rndr 2 18\n"              \
"add_object 1 rndx rndy rndz rndr rndr rndr 2 18\n"              \
"add_object 1 rndx rndy rndz rndr rndr rndr 2 18\n"              \
"add_object 1 rndx rndy rndz rndr rndr rndr 2 18\n"              \
"add_object 1 rndx rndy rndz rndr rndr rndr 2 18\n"              \
"add_object 1 rndx rndy rndz rndr rndr rndr 2 18\n"              \
"add_object 1 rndx rndy rndz rndr rndr rndr 2 18\n"              \
"add_object 1 rndx rndy rndz rndr rndr rndr 2 18\n"

#define BACKGROUND_TEX_SPACE                                                      \
"register_params "BACKGROUND_TEX_BEGIN" 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"          \
"register_params_update rnd_""1"" 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n" \
"set_background_info r""\n"

#define COIN_LINE_1_4X                                                     \
"register_params 10 rndx rndz rndy 0 0 0 2 0 0 0 0 0 0 0 0\n"                \
"register_params_update 0 rnd_2 rnd_2 rnd_2 0 0 0 0 0 0 0 0 0 0 0 0\n"     \
"add_object r r r r 0 0 0 r 91\n"                                          \
"object_set_info 11\n" \
"object_set_sub_info 9\n" \
"register_params_update 0 rnd_2 rnd_2 rnd_2 0 0 0 0 0 0 0 0 0 0 0 0\n"     \
"add_object r r r r 0 0 0 r 91\n"                                          \
"object_set_info 11\n" \
"object_set_sub_info 9\n" \
"register_params_update 0 rnd_2 rnd_2 rnd_2 0 0 0 0 0 0 0 0 0 0 0 0\n"     \
"add_object r r r r 0 0 0 r 91\n"                                          \
"object_set_info 11\n" \
"object_set_sub_info 9\n" \
"register_params_update 0 rnd_2 rnd_2 rnd_2 0 0 0 0 0 0 0 0 0 0 0 0\n"     \
"add_object r r r r 0 0 0 r 91\n" \
"object_set_info 11\n" \
"object_set_sub_info 9\n"

#define ADD_OBJ_BEGIN(model, x, y, z) \
"register_params "#model" "#x" "#y" "#z" 0 0 0 0 0 0 0 0 0 0 0 0\n"

#define ADD_OBJ_BEGIN_RANDOM_XZ(model, y) \
"register_params "#model" rndx "#y" rndz 0 0 0 0 0 0 0 0 0 0 0 0\n"

#define ADD_OBJ_MOVE(x, y, z) \
"register_params_update 0 "#x" "#y" "#z" 0 0 0 0 0 0 0 0 0 0 0 0\n"

#define ADD_OBJ_END(scale, tex) \
"add_object r r r r 0 0 0 "#scale" "#tex"\n"

#define ADD_OBJ_SET_INFO(info, subinfo) \
"object_set_info "#info"\n" \
"object_set_sub_info "#subinfo"\n" \

#define COLLECT_POINT_ADD(x, y, z) \
ADD_OBJ_MOVE(x, y, z) \
ADD_OBJ_END(4, 40) \
"object_identify collect\n"

#define GREEN_TENDRILS_1                                                   \
"register_params 2 rndx 20 rndy 0 0 0 10 0 0 0 0 0 0 0 0\n"                \
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"  \
"add_object r r r r 0 0 0 r 31\n"                                          \
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"  \
"add_object r r r r 0 0 0 r 31\n"                                          \
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"  \
"add_object r r r r 0 0 0 r 31\n"                                          \
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"  \
"add_object r r r r 0 0 0 r 31\n"                                          \
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"  \
"add_object r r r r 0 0 0 r 31\n"                                          \
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"  \
"add_object r r r r 0 0 0 r 31\n"                                          \
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"  \
"add_object r r r r 0 0 0 r 31\n"                                          \
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"  \
"add_object r r r r 0 0 0 r 31\n"                                          \
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"  \
"add_object r r r r 0 0 0 r 31\n"                                          \
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"  \
"add_object r r r r 0 0 0 r 31\n"                                          \
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"  \
"add_object r r r r 0 0 0 r 31\n"

#define RANDOM_DECORATION_1                                                \
"register_params 2 rndx 2 rndz 0 0 0 4 29 0 0 0 0 0 0 0\n"                 \
"register_params_update 0 rnd_-4_4 0 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"   \
"add_object r r r r 0 0 0 r r\n"                                           \
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"   \
"add_object r r r r 0 0 0 r r\n"                                           \
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"   \
"add_object r r r r 0 0 0 r r\n"                                           \
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"   \
"add_object r r r r 0 0 0 r r\n"                                           \
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"   \
"add_object r r r r 0 0 0 r r\n"                                           \
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"   \
"add_object r r r r 0 0 0 r r\n"                                           \
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"   \
"add_object r r r r 0 0 0 r r\n"                                           \
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"   \
"add_object r r r r 0 0 0 r r\n"                                           \
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"   \
"add_object r r r r 0 0 0 r r\n"                                           \
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"   \
"add_object r r r r 0 0 0 r r\n"                                           \
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"   \
"add_object r r r r 0 0 0 r r\n"

#define BASE_MODEL_ID MAPMODEL_SPRITE

#define BASE_FRIENDLY_INVISIBLE(x,y,z)                        \
"register_params 10 "/*"rndx"*/#x" "#y" "#z" 0 0 0 8 "/*"42"*/"0"" 0 0 0 0 0 0 0\n"                 \
"add_object r r r r r r r r " "r" "\n" \
"object_set_info 8\n"

#define BASE_ENEMY_1                           \
"add_object "MAPMODEL_ENEMYBASE" rndx 50 rndz 0 0 0 4 "BASE_TEXTURE_ID_ENEMY"\n"       \
"object_set_info 15\n"

#define BASE_TURRET_1                        \
"register_params 10 "/*"rndx"*/"wx0.5"" 50 "/*"rndz"*/"wz0.5"" 0 0 0 1 "/*"42"*/"92"" 0 0 0 0 0 0 0\n"                 \
"add_object r r r r r r r r " "r" "\n" \
"object_set_info 8\n"

#define BASE_GENERIC(x,y,z)                            \
"register_params 17 "/*"rndx"*/#x" "#y" "/*"rndz"*/#z" 0 0 0 8 "/*"42"*//*"88"*/MAP_TEXTURE_ID_ANIMATED_STATIC" 0 0 0 0 0 0 0\n"                 \
"add_object r r r r r r r r " "r" "\n" \
"object_set_info 13\n" \

#define PLANET_SCENERY(x,y,z,tex_id,scale)                            \
"add_object 17 "#x" "#y" "#z" 0 0 0 "#scale" "#tex_id"\n" \
"object_set_info 16\n"

#define ASTEROID_FIELD_BEGIN_1(x, y, z)                      \
"register_params 15 """#x" "#y" "#z""" 0 0 0 4 14 0 0 0 0 0 0 0\n"

#define ASTEROID_FIELD_ADD_AT_Y(sp, scale, ypos1)                                                       \
"register_params_update 0 rnd_-"#sp"_"#sp" "#ypos1" rnd_-"#sp"_"#sp" 0 0 0 0 0 0 0 0 0 0 0 0\n" \
"add_object r r r r 0 0 0 "#scale" r\n"

#define ASTEROID_FIELD_ADD(sp, scale)                                                              \
"register_params_update 0 rnd_-"#sp"_"#sp" rnd_-"#sp"_"#sp" rnd_-"#sp"_"#sp" 0 0 0 0 0 0 0 0 0 0 0 0\n" \
"add_object r r r r 0 0 0 "#scale" r\n"

#define ASTEROID_FIELD_5 \
ASTEROID_FIELD_BEGIN_1(rndx, rndy, rndz) \
ASTEROID_FIELD_ADD(16, rnd_8) \
ASTEROID_FIELD_ADD(16, rnd_8) \
ASTEROID_FIELD_ADD(16, rnd_8) \
ASTEROID_FIELD_ADD(16, rnd_8) \
ASTEROID_FIELD_ADD(16, rnd_8)

#define ASTEROID_FIELD_PLANE(ypos) \
ASTEROID_FIELD_BEGIN_1(rndx, ypos, rndz) \
ASTEROID_FIELD_ADD_AT_Y(16, rnd_8, 0) \
ASTEROID_FIELD_ADD_AT_Y(16, rnd_8, 0) \
ASTEROID_FIELD_ADD_AT_Y(16, rnd_8, 0) \
ASTEROID_FIELD_ADD_AT_Y(16, rnd_8, 0) \
ASTEROID_FIELD_ADD_AT_Y(16, rnd_8, 0)

#define BUILDING_1(x, y, z)                         \
"set_vector "#x" "#y" "#z" 0 1 0\n"                 \
"add_object 1 vecx_0 vecy_0 vecz_0 0 0 0 4 16\n"    \
"add_object 1 vecx_0 vecy_4 vecz_0 0 0 0 4 16\n"    \
"add_object 1 vecx_0 vecy_8 vecz_0 0 0 0 4 16\n"   \
"add_object 1 vecx_0 vecy_12 vecz_0 0 0 0 4 16\n"   \
"add_object 1 vecx_0 vecy_16 vecz_0 0 0 0 4 16\n"   \
"add_object 1 vecx_0 vecy_20 vecz_0 0 0 0 4 16\n"   \
"add_object 1 vecx_0 vecy_24 vecz_0 0 0 0 4 16\n"   \
"add_object 1 vecx_0 vecy_28 vecz_0 0 0 0 4 16\n"

#define BUILDING_PYRAMID(x, y, z)                   \
ADD_OBJ_BEGIN(2, x, y, z) \
ADD_OBJ_END(4, 31)\
ADD_OBJ_MOVE(4, 0, 0)\
ADD_OBJ_END(4, 31)\
ADD_OBJ_MOVE(4, 0, 0)\
ADD_OBJ_END(4, 31)\
ADD_OBJ_MOVE(4, 0, 0)\
ADD_OBJ_END(4, 31)\
ADD_OBJ_MOVE(4, 0, 0)\
ADD_OBJ_END(4, 31)\
ADD_OBJ_MOVE(0, 0, 4)\
ADD_OBJ_END(4, 31)\
ADD_OBJ_MOVE(0, 0, 4)\
ADD_OBJ_END(4, 31)\
ADD_OBJ_MOVE(0, 0, 4)\
ADD_OBJ_END(4, 31)\
ADD_OBJ_MOVE(0, 0, 4)\
ADD_OBJ_END(4, 31)\
ADD_OBJ_MOVE(-4, 0, 0)\
ADD_OBJ_END(4, 31)\
ADD_OBJ_MOVE(-4, 0, 0)\
ADD_OBJ_END(4, 31)\
ADD_OBJ_MOVE(-4, 0, 0)\
ADD_OBJ_END(4, 31)\
ADD_OBJ_MOVE(-4, 0, 0)\
ADD_OBJ_END(4, 31)\
ADD_OBJ_MOVE(0, 0, -4)\
ADD_OBJ_END(4, 31)\
ADD_OBJ_MOVE(0, 0, -4)\
ADD_OBJ_END(4, 31)\
ADD_OBJ_MOVE(0, 0, -4)\
ADD_OBJ_END(4, 31) \
ADD_OBJ_BEGIN(2, x, y, z) \
ADD_OBJ_MOVE(4, 4, 4)\
ADD_OBJ_END(4, 16) \
ADD_OBJ_MOVE(4, 0, 0) \
ADD_OBJ_END(4, 16) \
ADD_OBJ_MOVE(4, 0, 0) \
ADD_OBJ_END(4, 16) \
ADD_OBJ_MOVE(0, 0, 4) \
ADD_OBJ_END(4, 16) \
ADD_OBJ_MOVE(0, 0, 4) \
ADD_OBJ_END(4, 16) \
ADD_OBJ_MOVE(-4, 0, 0) \
ADD_OBJ_END(4, 16) \
ADD_OBJ_MOVE(-4, 0, 0) \
ADD_OBJ_END(4, 16) \
ADD_OBJ_MOVE(0, 0, -4) \
ADD_OBJ_END(4, 16) \
ADD_OBJ_MOVE(4, 4, 0) \
ADD_OBJ_END(4, 31) \

#define WORLD_SCALED_FRAME_TURRET(x, tex, scale)                         \
"register_params 100 50 100 0 0 0 0 0 0 0 0 0 0 0 0 0\n"                \
"register_params_mul "#x" "#x" "#x" 1 1 1 1 1 1 1 1 1 1 1 1 1\n"         \
"set_world_size r r r\n"                                                 \
"mesh_manip_add wx0.2 " "wy0.2" " wz0.1" " 4 0 0" " 0 0 4" " wx0.2 wz0.2\n" \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(25, 0.93) \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(25, 0.93) \
"mesh_manip_round 0 4 0\n" \
"mesh_manip_complete 2 "#tex" 1 "#scale"\n"

///////////////////////////////////////////////////////
const static char initial_map_200x100x200[] = ""

"set_world_size 300 100 300\n" // all must be divisible by MAX_WORLD_REGIONS (50 currently)

"set_world_plane 0 0 10 0 1 0 0 0 0 1\n" // idx, origin[3], v1[3], v2[3]

"set_background_info "BACKGROUND_TEXTURE_STR"\n"

//"add_object 0 100 50 100 0 0 0 1 13\n" /* add a jet */
//"add_enemy_jet 1 10\n" /* make last object a jet of difficulty n, durability x */

/*
"add_spawn 80 50 80 3.14 0 0\n"
"add_spawn 120 50 80 3.14 0 0\n"
"add_spawn 120 50 120 3.14 0 0\n"
"add_spawn 80 50 120 0 0 0\n"
 */
/*
"add_object 1 100 75 100 0 0 0 4 18\n"
"object_set_info 3\n"
"add_object 1 100 25 100 0 0 0 4 18\n"
"object_set_info 3\n"
"add_object 1 75 50 125 0 0 0 4 18\n"
"object_set_info 3\n"
"add_object 1 125 50 75 0 0 0 4 18\n"
"object_set_info 3\n"
 */

//
//"add_object" " 1"       /* object-id */
//             " 50 50 50"/*coordinate*/
//             " 0 0 0"   /*euler: alpha beta gamma*/
//             " 1"       /*scale*/
//             " 1\n"     /*texture-id*/

// ceiling
//"add_mesh" " 200 104 200"   /*start_coord*/
//           " -10 0 0"       /*row-vec*/
//           " 0 0 -10"       /*col-vec*/
//           " 20 20"         /*nrows/cols*/
//           " -1 -1 -1"      /*plane-normal-vector*/
//           " 11"            /*texture_id*/
//           " 1\n"          /*texture-divisor*/

//"mesh_manip_add 200 104 200 ""-10 0 0 ""0 0 -10 ""20 20\n"
//"mesh_manip_object_info 0 1.570 0\n"
//"mesh_manip_complete 5 11 1 1\n"
//"add_object 5 100 150 100 0 1.570 0 200 11\n" // ceiling
//"add_object 5 100 -50 100 0 1.570 0 200 11\n" // floor
//"add_object 5 100 50 0 0 0 0 200 11\n" // wall
//"add_object 5 100 50 200 0 0 0 200 11\n" // wall

// floor
//"add_mesh 200 -10 200" " -10 0 0" " 0 0 -10" " 20 20" " -1 1 -1 4 1\n"

// mesh of cubes
MESH_CUBES_1(19, 5)

/*
"mesh_manip_add rnd_200 5 rnd_200 ""2 0 0 " "0 0 2 " "10 10\n"
"mesh_manip_pull 5 5 0 20 0 0.9\n"
"mesh_manip_complete 2 32 1 1\n"
"mesh_manip_add rnd_200 5 rnd_200 ""2 0 0 " "0 0 2 " "10 10\n"
"mesh_manip_pull 5 5 0 20 0 0.9\n"
"mesh_manip_complete 2 32 1 1\n"
"mesh_manip_add rnd_200 5 rnd_200 ""2 0 0 " "0 0 2 " "10 10\n"
"mesh_manip_pull 5 5 0 20 0 0.9\n"
"mesh_manip_complete 2 32 1 1\n"
"mesh_manip_add rnd_200 5 rnd_200 ""2 0 0 " "0 0 2 " "10 10\n"
"mesh_manip_pull 5 5 0 20 0 0.9\n"
"mesh_manip_complete 2 32 1 1\n"
"mesh_manip_add rnd_200 5 rnd_200 ""2 0 0 " "0 0 2 " "10 10\n"
"mesh_manip_pull 5 5 0 20 0 0.9\n"
"mesh_manip_complete 2 32 1 1\n"
 */

/*
"mesh_manip_add wx0.4 " "wy0.02" " wz0.4" " 4 0 0" " 0 0 4" " 8 8\n"
"mesh_manip_pull mx0.5 my0.5"" 0 wy0.25 0 ""0.9\n"
"mesh_manip_complete 1 16 1 4\n"
 */

/*
"mesh_manip_add 200 95 200" " -5 0 0" " 0 0 -5" " 40 40\n"
"mesh_manip_pull 10 10"" 0 -30 0"" 0.90\n"
"mesh_manip_pull 10 30"" 0 -30 0"" 0.90\n"
"mesh_manip_pull 30 10"" 0 -30 0"" 0.90\n"
"mesh_manip_pull 30 30"" 0 -30 0"" 0.90\n"
// type, texture, tiling_m, scale
"mesh_manip_complete 1 19 1 4\n"
 */

// random blocks

RANDOM_FLOATING_BLOCKS_1

// walls
/*
"add_mesh 0 0 0" " 0 10 0" " 0 0 10" " 10 20" " 1 1 1 4 40\n"
"add_mesh 0 0 0" " 0 10 0" " 10 0 0" " 10 20" " 1 1 1 4 40\n"
"add_mesh 200 0 200" " 0 10 0" " 0 0 -10" " 10 20" " -1 1 -1 4 40\n"
"add_mesh 200 0 200" " 0 10 0" " -10 0 0" " 10 20" " -1 1 -1 4 40\n"
 */

// octagon
/*      6
 *     ----
 *  7/      \
 *  /        \ 5
 *8|          |
 * |          | 4
 *  \        /
 *   \1     / 3
 *     ----
 *     2
 */



//"mesh_manip_add "/*xyz*/"0 10 67" /*row-vec*/" 13.4 0 -13.4" /*col-vec*/" 0 14.28 0" /*rows/cols*/" 5 7\n"
//"mesh_manip_object_info 1.5707 0.785 -1.5707\n"
//"mesh_manip_complete 1 19 1 5\n"

/*
"mesh_manip_add 67 0 0" " 10 0 0" " 0 10 0" " 5 5\n"
"mesh_manip_object_info 0 0 0\n"
"mesh_manip_complete 1 19 1 20\n"

"mesh_manip_add 134 0 0" " 10 0 10" " 0 10 0" " 5 5\n"
"mesh_manip_object_info 1.57 0.78 -1.57\n"
"mesh_manip_complete 1 19 1 20\n"

"mesh_manip_add 200 0 67" " 0 0 10" " 0 10 0" " 5 5\n"
"mesh_manip_object_info 0 0 0\n"
"mesh_manip_complete 1 19 1 20\n"

"mesh_manip_add 200 0 134" " -10 0 10" " 0 10 0" " 5 5\n"
"mesh_manip_object_info 1.57 0.78 -1.57\n"
"mesh_manip_complete 1 19 1 20\n"

"mesh_manip_add 134 0 200" " -10 0 0" " 0 10 0" " 5 5\n"
"mesh_manip_object_info 0 0 0\n"
"mesh_manip_complete 1 19 1 20\n"

"mesh_manip_add 67 0 200" " -10 0 -10" " 0 10 0" " 5 5\n"
"mesh_manip_object_info 1.57 0.78 -1.57\n"
"mesh_manip_complete 1 19 1 20\n"

"mesh_manip_add 0 0 134" " 0 0 -10" " 0 10 0" " 5 5\n"
"mesh_manip_object_info 0 0 0\n"
"mesh_manip_complete 1 19 1 20\n"
 */

//"add_object 8 50 50 50 0 0 0 10 16\n"

/*
"add_objects_packed 1 40 5 40 0 0 0 10 19 0 10 0 3\n"
"add_objects_packed 1 40 5 80 0 0 0 10 16 0 10 0 3\n"
"add_objects_packed 1 40 5 120 0 0 0 10 16 0 10 0 3\n"
"add_objects_packed 1 40 5 160 0 0 0 10 19 0 10 0 3\n"

"add_objects_packed 1 80 5 40 0 0 0 10 19 0 10 0 3\n"
"add_objects_packed 1 80 5 80 0 0 0 10 16 0 10 0 3\n"
"add_objects_packed 1 80 5 120 0 0 0 10 16 0 10 0 3\n"
"add_objects_packed 1 80 5 160 0 0 0 10 19 0 10 0 3\n"

"add_objects_packed 1 120 5 40 0 0 0 10 19 0 10 0 3\n"
"add_objects_packed 1 120 5 80 0 0 0 10 16 0 10 0 3\n"
"add_objects_packed 1 120 5 120 0 0 0 10 16 0 10 0 3\n"
"add_objects_packed 1 120 5 160 0 0 0 10 19 0 10 0 3\n"

"add_objects_packed 1 160 5 40 0 0 0 10 19 0 10 0 3\n"
"add_objects_packed 1 160 5 80 0 0 0 10 16 0 10 0 3\n"
"add_objects_packed 1 160 5 120 0 0 0 10 16 0 10 0 3\n"
"add_objects_packed 1 160 5 160 0 0 0 10 19 0 10 0 3\n"
 */

// TODO: add directive to build a mesh, pull it, then convert it into a sea of primitives


// cubes as walls
/*
"add_object 1 50 0 50 0 0 0 10 16\n"
"add_object 1 50 10 50 0 0 0 10 16\n"
"add_object 1 50 20 50 0 0 0 10 16\n"
"add_object 1 50 30 50 0 0 0 10 16\n"
"add_object 1 50 40 50 0 0 0 10 16\n"
"add_object 1 50 50 50 0 0 0 10 16\n"
"add_object 1 50 60 50 0 0 0 10 16\n"
"add_object 1 50 70 50 0 0 0 10 16\n"
"add_object 1 50 80 50 0 0 0 10 16\n"
"add_object 1 50 90 50 0 0 0 10 16\n"

"add_object 1 150 0 50 0 0 0 16 3\n"
"add_object 1 150 10 50 0 0 0 16 3\n"
"add_object 1 150 20 50 0 0 0 16 3\n"
"add_object 1 150 30 50 0 0 0 16 3\n"
"add_object 1 150 40 50 0 0 0 16 3\n"
"add_object 1 150 50 50 0 0 0 16 3\n"
"add_object 1 150 60 50 0 0 0 16 3\n"
"add_object 1 150 70 50 0 0 0 16 3\n"
"add_object 1 150 80 50 0 0 0 16 3\n"
"add_object 1 150 90 50 0 0 0 16 3\n"

"add_object 1 150 0 150 0 0 0 16 3\n"
"add_object 1 150 10 150 0 0 0 16 3\n"
"add_object 1 150 20 150 0 0 0 16 3\n"
"add_object 1 150 30 150 0 0 0 16 3\n"
"add_object 1 150 40 150 0 0 0 16 3\n"
"add_object 1 150 50 150 0 0 0 16 3\n"
"add_object 1 150 60 150 0 0 0 16 3\n"
"add_object 1 150 70 150 0 0 0 16 3\n"
"add_object 1 150 80 150 0 0 0 16 3\n"
"add_object 1 150 90 150 0 0 0 16 3\n"

"add_object 1 50 0 150 0 0 0 16 3\n"
"add_object 1 50 10 150 0 0 0 16 3\n"
"add_object 1 50 20 150 0 0 0 16 3\n"
"add_object 1 50 30 150 0 0 0 16 3\n"
"add_object 1 50 40 150 0 0 0 16 3\n"
"add_object 1 50 50 150 0 0 0 16 3\n"
"add_object 1 50 60 150 0 0 0 16 3\n"
"add_object 1 50 70 150 0 0 0 16 3\n"
"add_object 1 50 80 150 0 0 0 16 3\n"
"add_object 1 50 90 150 0 0 0 16 3\n"
*/
;

// MARK: -- map - collection
const static char initial_map_collection[] = ""

//"set_world_size 200 100 200\n" // all must be divisible by MAX_WORLD_REGIONS

"set_background_info "BACKGROUND_TEXTURE_STR"\n"

WORLD_SCALED_FRAME(1, /*57*/28, 4)
//WORLD_SCALED_FRAME_TERRAIN(-10, 28, 4)

// else
//GREEN_TENDRILS_1

/////////////

// buildings
BUILDING_1(rndx, 2, rndz)
BUILDING_1(rndx, 2, rndz)
BUILDING_1(rndx, 2, rndz)
BUILDING_1(rndx, 2, rndz)
BUILDING_1(rndx, 2, rndz)
BUILDING_1(rndx, 2, rndz)

BUILDING_PYRAMID(100, 2, 100)
BUILDING_PYRAMID(300, 2, 100)
BUILDING_PYRAMID(100, 2, 300)
BUILDING_PYRAMID(300, 2, 300)

// drifting asteroid example
//"add_object "MAPMODEL_CUBE2" 100 20 100 0 0 0 11 56\n"
//"object_set_info 4\n" // moving block
//"object_set_velocity 0 0.5 0\n"

// enemy base
BASE_ENEMY_1

// "core" that spits out data
BASE_GENERIC(200, 50, 200)

"add_spawn 200 8 200 3.14 0 0\n"

/*
"register_params 1 100 50 100 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object 1 r r r 0 0 0 10 40\n"
"register_params_update 0 0 10 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object 1 r r r 0 0 0 10 40\n"
"register_params_update 0 10 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object 1 r r r 0 0 0 10 40\n"
"register_params_update 0 -20 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object 1 r r r 0 0 0 10 40\n"
"register_params_update 0 10 10 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object 1 r r r 0 0 0 10 40\n"
 */

/*
// turret tower
"register_params 1 rndx 0 rndz 0 0 0 4 40 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object 11 r r r 0 0 0 4 35\n"
"object_set_info 3\n"

// turret tower
"register_params 1 rndx 0 rndz 0 0 0 4 40 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object 11 r r r 0 0 0 4 35\n"
"object_set_info 3\n"

// turret tower
"register_params 1 rndx 0 rndz 0 0 0 2 40 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object 11 r r r 0 0 0 4 35\n"
"object_set_info 3\n"

 */
;

const static char initial_map_turret[] = ""

//"set_world_size 200 100 200\n" // all must be divisible by MAX_WORLD_REGIONS

"set_background_info "BACKGROUND_TEXTURE_STR"\n"

//MESH_CUBES_2(28, 4)
WORLD_SCALED_FRAME_TURRET(1, 57, 4)

// else
GREEN_TENDRILS_1

COIN_LINE_1_4X
COIN_LINE_1_4X
COIN_LINE_1_4X
COIN_LINE_1_4X
COIN_LINE_1_4X
COIN_LINE_1_4X
COIN_LINE_1_4X
COIN_LINE_1_4X

/////////////

// buildings
BUILDING_1(rndx, 8, rndz)
BUILDING_1(rndx, 8, rndz)
BUILDING_1(rndx, 8, rndz)
BUILDING_1(rndx, 8, rndz)
BUILDING_1(rndx, 8, rndz)
BUILDING_1(rndx, 8, rndz)

// drifting asteroid example
/*
 "add_object "MAPMODEL_CUBE2" 100 20 100 0 0 0 11 56\n"
 "object_set_info 4\n" // moving block
 "object_set_velocity 0 0.5 0\n"
 */

// add test tower
// add base
BASE_TURRET_1

// clear model
"register_params_clear 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
// set model
"register_params_update 10 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"

// add "protect" points
COLLECT_POINT_ADD(rnd_-10_10, 0, rnd_-10_10)
COLLECT_POINT_ADD(rnd_-10_10, 0, rnd_-10_10)
COLLECT_POINT_ADD(rnd_-10_10, 0, rnd_-10_10)

// enemy base
BASE_ENEMY_1

// air platform
/*
 "register_params 2 rndx 20 rndy 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "register_params_update 0 10 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 6 31\n"
 "register_params_update 0 -20 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 6 31\n"
 "register_params_update 0 10 0 10 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 6 31\n"
 "register_params_update 0 0 0 -20 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 6 31\n"
 */

/*
RANDOM_DECORATION_1
RANDOM_DECORATION_1
RANDOM_DECORATION_1
RANDOM_DECORATION_1
RANDOM_DECORATION_1
RANDOM_DECORATION_1
RANDOM_DECORATION_1
RANDOM_DECORATION_1
RANDOM_DECORATION_1
RANDOM_DECORATION_1
RANDOM_DECORATION_1
RANDOM_DECORATION_1
RANDOM_DECORATION_1
RANDOM_DECORATION_1
RANDOM_DECORATION_1
RANDOM_DECORATION_1
*/
/*
 "register_params 1 100 50 100 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object 1 r r r 0 0 0 10 40\n"
 "register_params_update 0 0 10 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object 1 r r r 0 0 0 10 40\n"
 "register_params_update 0 10 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object 1 r r r 0 0 0 10 40\n"
 "register_params_update 0 -20 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object 1 r r r 0 0 0 10 40\n"
 "register_params_update 0 10 10 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object 1 r r r 0 0 0 10 40\n"
 */

/*
 // turret tower
 "register_params 1 rndx 0 rndz 0 0 0 4 40 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object 11 r r r 0 0 0 4 35\n"
 "object_set_info 3\n"
 
 // turret tower
 "register_params 1 rndx 0 rndz 0 0 0 4 40 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object 11 r r r 0 0 0 4 35\n"
 "object_set_info 3\n"
 
 // turret tower
 "register_params 1 rndx 0 rndz 0 0 0 2 40 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object 11 r r r 0 0 0 4 35\n"
 "object_set_info 3\n"
 
 */
;

// MARK: -- map - deathmatch
const static char initial_map_deathmatch[] = ""

"set_world_size 200 100 200\n" // all must be divisible by MAX_WORLD_REGIONS
///WORLD_SCALED_FRAME_TERRAIN(-10, 28, 4)
"set_background_info "BACKGROUND_TEXTURE_STR"\n"

// stuff
RANDOM_DECORATION_1
RANDOM_DECORATION_1
RANDOM_DECORATION_1
RANDOM_DECORATION_1
RANDOM_DECORATION_1
RANDOM_DECORATION_1
RANDOM_DECORATION_1
RANDOM_DECORATION_1
RANDOM_DECORATION_1

// asteroid
/*
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
 */
ASTEROID_FIELD_PLANE(30)
ASTEROID_FIELD_PLANE(35)
ASTEROID_FIELD_PLANE(40)
ASTEROID_FIELD_PLANE(45)
ASTEROID_FIELD_PLANE(50)
ASTEROID_FIELD_PLANE(55)
ASTEROID_FIELD_PLANE(60)
ASTEROID_FIELD_PLANE(65)
ASTEROID_FIELD_PLANE(70)
ASTEROID_FIELD_PLANE(80)
;

const static char initial_map_survival[] = ""

WORLD_SCALED_FRAME(1, 28, 4)
"set_background_info "BACKGROUND_TEXTURE_STR"\n"

// else
"register_params 2 rndx 20 rndy 0 0 0 10 0 0 0 0 0 0 0 0\n"
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r 31\n"
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r 31\n"
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r 31\n"
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r 31\n"
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r 31\n"
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r 31\n"
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r 31\n"
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r 31\n"
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r 31\n"
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r 31\n"
"register_params_update 0 rnd_20 rnd_20 rnd_20 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r 31\n"
/////////////

// buildings
/*
 "set_vector rndx 0 rndz 0 1 0\n" // origin_x,y,z vx,y,z
 "add_object 1 vecx_0 vecy_4 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_8 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_12 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_16 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_20 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_24 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_28 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_32 vecz_0 0 0 0 4 16\n"
 
 "set_vector rndx 0 rndz 0 1 0\n" // origin_x,y,z vx,y,z
 "add_object 1 vecx_0 vecy_4 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_8 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_12 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_16 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_20 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_24 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_28 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_32 vecz_0 0 0 0 4 16\n"
 
 "set_vector rndx 0 rndz 0 1 0\n" // origin_x,y,z vx,y,z
 "add_object 1 vecx_0 vecy_4 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_8 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_12 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_16 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_20 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_24 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_28 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_32 vecz_0 0 0 0 4 16\n"
 
 "set_vector rndx 0 rndz 0 1 0\n" // origin_x,y,z vx,y,z
 "add_object 1 vecx_0 vecy_4 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_8 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_12 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_16 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_20 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_24 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_28 vecz_0 0 0 0 4 16\n"
 "add_object 1 vecx_0 vecy_32 vecz_0 0 0 0 4 16\n"
 */

// drifting asteroid example
/*
 "add_object "MAPMODEL_CUBE2" 100 20 100 0 0 0 11 56\n"
 "object_set_info 4\n" // moving block
 "object_set_velocity 0 0.5 0\n"
 */

BASE_ENEMY_1

// air platform
/*
 "register_params 2 rndx 20 rndy 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "register_params_update 0 10 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 6 31\n"
 "register_params_update 0 -20 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 6 31\n"
 "register_params_update 0 10 0 10 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 6 31\n"
 "register_params_update 0 0 0 -20 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 6 31\n"
 */

"register_params 2 rndx 0 rndz 0 0 0 4 29 0 0 0 0 0 0 0\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params 2 rndx 0 rndz 0 0 0 4 29 0 0 0 0 0 0 0\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params 2 rndx 0 rndz 0 0 0 4 29 0 0 0 0 0 0 0\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params 2 rndx 0 rndz 0 0 0 4 29 0 0 0 0 0 0 0\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params 2 rndx 0 rndz 0 0 0 4 29 0 0 0 0 0 0 0\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params 2 rndx 0 rndz 0 0 0 4 29 0 0 0 0 0 0 0\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params 2 rndx 0 rndz 0 0 0 4 29 0 0 0 0 0 0 0\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params 2 rndx 0 rndz 0 0 0 4 29 0 0 0 0 0 0 0\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"


/*
 "register_params 1 100 50 100 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object 1 r r r 0 0 0 10 40\n"
 "register_params_update 0 0 10 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object 1 r r r 0 0 0 10 40\n"
 "register_params_update 0 10 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object 1 r r r 0 0 0 10 40\n"
 "register_params_update 0 -20 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object 1 r r r 0 0 0 10 40\n"
 "register_params_update 0 10 10 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object 1 r r r 0 0 0 10 40\n"
 */

/*
 // turret tower
 "register_params 1 rndx 0 rndz 0 0 0 4 40 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object 11 r r r 0 0 0 4 35\n"
 "object_set_info 3\n"
 
 // turret tower
 "register_params 1 rndx 0 rndz 0 0 0 4 40 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object 11 r r r 0 0 0 4 35\n"
 "object_set_info 3\n"
 
 // turret tower
 "register_params 1 rndx 0 rndz 0 0 0 2 40 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object r r r r 0 0 0 r r\n"
 "register_params_update 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
 "add_object 11 r r r 0 0 0 4 35\n"
 "object_set_info 3\n"
 
 */
;

const static char map_400x100x400[] = ""

"set_world_size 400 100 400\n" // all must be divisible by MAX_WORLD_REGIONS (50 currently)

"set_world_plane 0 0 10 0 1 0 0 0 0 1\n" // idx, origin[3], v1[3], v2[3]
"set_background_info "BACKGROUND_TEXTURE_STR"\n"

"add_bounding_vec wx0.5 wy0 wz0.5 0 1 0\n" // floor
"add_bounding_vec wx0.5 wy1.0 wz0.5 0 -1 0\n" // ceiling

// side walls
"add_bounding_vec wx0 wy0.5 wz0.5 1 0 0\n"
"add_bounding_vec wx1 wy0.5 wz0.5 -1 0 0\n"

// front/back walls
"add_bounding_vec wx0.5 wy0.5 wz0 0 0 1\n"
"add_bounding_vec wx0.5 wy0.5 wz1 0 0 -1\n"

// mesh of cubes
"mesh_manip_add wx1.0 " "wy0.0" " wz1.0" " -8 0 0" " 0 0 -8" " 50 50\n"
"mesh_manip_pull rnd_100 rnd_50"" 0 rnd_50 0"" 0.90\n"
"mesh_manip_pull rnd_100 rnd_50"" 0 rnd_50 0"" 0.90\n"
"mesh_manip_pull rnd_100 rnd_50"" 0 rnd_50 0"" 0.90\n"
"mesh_manip_pull rnd_100 rnd_50"" 0 rnd_50 0"" 0.90\n"
"mesh_manip_pull rnd_100 rnd_50"" 0 rnd_50 0"" 0.90\n"
"mesh_manip_pull rnd_100 rnd_50"" 0 rnd_50 0"" 0.90\n"
"mesh_manip_complete 1 19 1 8\n"

// sky
"mesh_manip_add wx1.0 " "wy0.95" " wz1.0" " -10 0 0" " 0 0 -10" " 40 40\n"
"mesh_manip_complete 1 18 1 10\n"

;

const static char map_400x100x400_flat[] = ""

"set_world_size 400 100 400\n" // all must be divisible by MAX_WORLD_REGIONS (50 currently)

"set_world_plane 0 0 10 0 1 0 0 0 0 1\n" // idx, origin[3], v1[3], v2[3]
"set_background_info "BACKGROUND_TEXTURE_STR"\n"

// mesh of cubes
"mesh_manip_add wx1.0 " "wy0.0" " wz1.0" " -8 0 0" " 0 0 -8" " 50 50\n"
"mesh_manip_complete 1 19 1 8\n"
;

const static char map_200x100x200_pits[] = ""

"set_world_size 200 100 200\n" // all must be divisible by MAX_WORLD_REGIONS (50 currently)

"set_world_plane 0 0 10 0 1 0 0 0 0 1\n" // idx, origin[3], v1[3], v2[3]
"set_background_info "BACKGROUND_TEXTURE_STR"\n"

// mesh of cubes
"mesh_manip_add wx1.0 " "wy0.5" " wz1.0" " -4 0 0" " 0 0 -4" " 50 50\n"
"mesh_manip_pull rnd_50 rnd_50"" 0 -50 0"" 0.999\n"
"mesh_manip_pull rnd_50 rnd_50"" 0 -50 0"" 0.999\n"
"mesh_manip_pull rnd_50 rnd_50"" 0 -50 0"" 0.999\n"
"mesh_manip_pull rnd_50 rnd_50"" 0 -50 0"" 0.999\n"
"mesh_manip_pull rnd_50 rnd_50"" 0 -50 0"" 0.999\n"
"mesh_manip_pull rnd_50 rnd_50"" 0 -50 0"" 0.999\n"
"mesh_manip_pull rnd_50 rnd_50"" 0 -50 0"" 0.999\n"
"mesh_manip_pull rnd_50 rnd_50"" 0 -50 0"" 0.999\n"
"mesh_manip_complete 1 19 1 4\n"

// random blocks
RANDOM_FLOATING_BLOCKS_1
RANDOM_FLOATING_BLOCKS_1
RANDOM_FLOATING_BLOCKS_1
RANDOM_FLOATING_BLOCKS_1
RANDOM_FLOATING_BLOCKS_1
RANDOM_FLOATING_BLOCKS_1

;

const static char map_portal_lobby[] = ""
"set_world_size 50 50 50\n" // all must be divisible by MAX_WORLD_REGIONS (50 currently)
"set_background_info "BACKGROUND_TEXTURE_STR"\n"
"mesh_manip_add wx-0.1 " "4" " wz-0.1" " 4 0 0" " 0 0 4" " 10 10\n"
"mesh_manip_complete 1 19 1 4\n"
;

const static char pokeball_map[] = ""

"set_world_size 200 100 200\n" // all must be divisible by MAX_WORLD_REGIONS (50 currently)

"set_world_plane 0 0 10 0 1 0 0 0 0 1\n" // idx, origin[3], v1[3], v2[3]

"set_background_info "BACKGROUND_TEXTURE_STR"\n"
MESH_CUBES_1(19, 5)

// add test tower
// add base
BASE_FRIENDLY_INVISIBLE(100, 20, 100)

// enemy base
BASE_ENEMY_1

// add ball
WORLD_ADD_OBJECT(15, 0, 0, 0, 0, 0, 0, 4, 105)
"object_set_info 17\n"
;

const static char map_speedrun[] = "" \
"register_params 2000 100 100 0 0 0 0 0 0 0 0 0 0 0 0 0\n"               \
"set_world_size r r r\n"                                                 \

// starting position
BASE_FRIENDLY_INVISIBLE(50, 50, 50)

// enemy turret
WORLD_ADD_OBJECT(11, rndx, rndy, rndz, 0, 0, 0, 1, 35) \
"object_set_info 3\n" \
WORLD_ADD_OBJECT(11, rndx, rndy, rndz, 0, 0, 0, 1, 35) \
"object_set_info 3\n" \
WORLD_ADD_OBJECT(11, rndx, rndy, rndz, 0, 0, 0, 1, 35) \
"object_set_info 3\n" \
WORLD_ADD_OBJECT(11, rndx, rndy, rndz, 0, 0, 0, 1, 35) \
"object_set_info 3\n" \
WORLD_ADD_OBJECT(11, rndx, rndy, rndz, 0, 0, 0, 1, 35) \
"object_set_info 3\n" \
WORLD_ADD_OBJECT(11, rndx, rndy, rndz, 0, 0, 0, 1, 35) \
"object_set_info 3\n" \
WORLD_ADD_OBJECT(11, rndx, rndy, rndz, 0, 0, 0, 1, 35) \
"object_set_info 3\n" \
WORLD_ADD_OBJECT(11, rndx, rndy, rndz, 0, 0, 0, 1, 35) \
"object_set_info 3\n" \

//"set_world_size 200 100 200\n" // all must be divisible by MAX_WORLD_REGIONS

"set_background_info "BACKGROUND_TEXTURE_STR"\n"

//MESH_CUBES_2(28, 4)
//WORLD_SCALED_FRAME_GL_TERRAIN(1, 83, 4)

// else
GREEN_TENDRILS_1

COIN_LINE_1_4X
COIN_LINE_1_4X
COIN_LINE_1_4X
COIN_LINE_1_4X
COIN_LINE_1_4X
COIN_LINE_1_4X
COIN_LINE_1_4X
COIN_LINE_1_4X

/////////////

// buildings
BUILDING_1(rndx, 0, rndz)
BUILDING_1(rndx, 0, rndz)
BUILDING_1(rndx, 0, rndz)
BUILDING_1(rndx, 0, rndz)
BUILDING_1(rndx, 0, rndz)
BUILDING_1(rndx, 0, rndz)
; // map_trench


const static char* maps_list[] =
{
    initial_map_collection,
    initial_map_deathmatch,
    /*
    initial_map_200x100x200,
    map_400x100x400,
    map_portal_lobby,
     */
    map_200x100x200_pits,
    initial_map_turret,
    pokeball_map,
    map_speedrun,
    NULL
};

const static char* maps_list_names[] =
{
    "map hack-grid",
    "map asteroidfield",
    /*"map canvas",
    "map bigcanvas",
    "lobby",
     */
    "pit",
    "map turret-defense",
    "ball-throwing test",
    "speedrun",
    NULL
};


#endif
