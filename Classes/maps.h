//
//  maps.h
//  gl_flight
//
//  Created by jbrady on 11/13/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef gl_flight_maps_h
#define gl_flight_maps_h

#define CUBE_TEXTURE_1 19
#define CUBE_TEXTURE_2 28
#define CUBE_TEXTURE_3 52
#define CUBE_TEXTURE_4 57

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

#define WORLD_SCALED_FRAME(x, tex, scale)                                \
"register_params 200 100 200 0 0 0 0 0 0 0 0 0 0 0 0 0\n"                \
"register_params_mul "#x" "#x" "#x" 1 1 1 1 1 1 1 1 1 1 1 1 1\n"         \
"set_world_size r r r\n"                                                 \
"mesh_manip_add wx0.1 " "wy0.1" " wz0.1" " 4 0 0" " 0 0 4" " wx0.2 wz0.2\n" \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(25, 0.93) \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(25, 0.93) \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(25, 0.93) \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(25, 0.93) \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(25, 0.93) \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(25, 0.93) \
WORLD_SCALED_FRAME_MESH_PULL_RANDOM(25, 0.93) \
"mesh_manip_round 0 4 0\n" \
"mesh_manip_complete 2 "#tex" 1 "#scale"\n"

#define MESH_CUBES_2(tex, scale)                                    \
"mesh_manip_add wx0.1 " "-4" " wz0.1" " 4 0 0" " 0 0 4" " 40 40\n"  \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_50 0"" 0.95\n"               \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_50 0"" 0.95\n"               \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_50 0"" 0.95\n"               \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_-10 0"" 0.95\n"              \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_-10 0"" 0.95\n"              \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_-10 0"" 0.95\n"              \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_50 0"" 0.95\n"               \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_50 0"" 0.95\n"               \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_50 0"" 0.95\n"               \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_-10 0"" 0.95\n"              \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_-10 0"" 0.95\n"              \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_-10 0"" 0.95\n"              \
"mesh_manip_complete "/*"2"*/"1"" "/*"57"*/#tex" 1 "#scale"\n"

#define MESH_CUBES_80x80(tex, scale)                                \
"mesh_manip_add wx0.1 " "-4" " wz0.1" " 2 0 0" " 0 0 2" " 80 80\n"  \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_50 0"" 0.95\n"               \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_50 0"" 0.95\n"               \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_50 0"" 0.95\n"               \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_-10 0"" 0.95\n"              \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_-10 0"" 0.95\n"              \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_-10 0"" 0.95\n"              \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_50 0"" 0.95\n"               \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_50 0"" 0.95\n"               \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_50 0"" 0.95\n"               \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_-10 0"" 0.95\n"              \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_-10 0"" 0.95\n"              \
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_-10 0"" 0.95\n"              \
"mesh_manip_complete "/*"2"*/"1"" "/*"57"*/#tex" 1 "#scale"\n"

#define BACKGROUND_1                                             \
"set_background_info 77\n"

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

#define BACKGROUND_TEX_BEGIN "77"
#define BACKGROUND_TEX_LEN /*"3"*/ "1"

#define BACKGROUND_TEX_RANDOM                                                     \
"register_params "BACKGROUND_TEX_BEGIN" 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"          \
"register_params_update rnd_"BACKGROUND_TEX_LEN" 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n" \
"set_background_info r\n"

#define BACKGROUND_TEX_SPACE                                                      \
"register_params "BACKGROUND_TEX_BEGIN" 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"          \
"register_params_update rnd_""1"" 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n" \
"set_background_info r\n"

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
"register_params 2 rndx 0 rndz 0 0 0 4 29 0 0 0 0 0 0 0\n"                 \
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
"add_object r r r r 0 0 0 r r\n"                                           \
"register_params_update 0 rnd_-4_4 4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"   \
"add_object r r r r 0 0 0 r r\n"

#define BASE_FRIENDLY_1                        \
"register_params 2 rndx 50 rndz 0 0 0 8 42 0 0 0 0 0 0 0\n"                 \
"add_object r r r r r r r r " "r" "\n" \
"object_set_info 8\n" \
\
"register_params_update 0 0 -8 0 0 0 0 -4 44 0 0 0 0 0 0 0\n"   \
"add_object r r r r 0 0 0 r r\n"                                           \
"register_params_update 0 0 16 0 0 0 0 0 0 0 0 0 0 0 0 0\n"   \
"add_object r r r r 0 0 0 r r\n" \
"register_params_update 0 0 -8 0 0 0 0 0 0 0 0 0 0 0 0 0\n"   \
\
"register_params_update 0 -8 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"   \
"add_object r r r r 0 0 0 r r\n"                                           \
"register_params_update 0 16 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"   \
"add_object r r r r 0 0 0 r r\n"                                           \
"register_params_update 0 -8 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"   \
\
"register_params_update 0 0 0 -8 0 0 0 0 0 0 0 0 0 0 0 0\n"   \
"add_object r r r r 0 0 0 r r\n"                                           \
"register_params_update 0 0 0 16 0 0 0 0 0 0 0 0 0 0 0 0\n"   \
"add_object r r r r 0 0 0 r r\n"                                           \
"register_params_update 0 0 0 -8 0 0 0 0 0 0 0 0 0 0 0 0\n"

#define BASE_ENEMY_1                           \
"add_object 2 rndx 50 rndz 0 0 0 8 58\n"       \
"object_set_info 15\n"

#define ASTEROID_FIELD_BEGIN_1(x, y, z)                      \
"register_params 15 """#x" "#y" "#z""" 0 0 0 4 14 0 0 0 0 0 0 0\n"

#define ASTEROID_FIELD_ADD(sp, scale)                                                              \
"register_params_update 0 rnd_-"#sp"_"#sp" rnd_-"#sp"_"#sp" rnd_-"#sp"_"#sp" 0 0 0 0 0 0 0 0 0 0 0 0\n" \
"add_object r r r r 0 0 0 "#scale" r\n"

#define ASTEROID_FIELD_5 \
ASTEROID_FIELD_BEGIN_1(rndx, rndy, rndz) \
ASTEROID_FIELD_ADD(16, rnd_6) \
ASTEROID_FIELD_ADD(16, rnd_6) \
ASTEROID_FIELD_ADD(16, rnd_6) \
ASTEROID_FIELD_ADD(16, rnd_6) \
ASTEROID_FIELD_ADD(16, rnd_6)

#define BUILDING_1(x, y, z)                         \
"set_vector "#x" "#y" "#z" 0 1 0\n"                 \
"add_object 1 vecx_0 vecy_4 vecz_0 0 0 0 4 16\n"    \
"add_object 1 vecx_0 vecy_8 vecz_0 0 0 0 4 16\n"    \
"add_object 1 vecx_0 vecy_12 vecz_0 0 0 0 4 16\n"   \
"add_object 1 vecx_0 vecy_16 vecz_0 0 0 0 4 16\n"   \
"add_object 1 vecx_0 vecy_20 vecz_0 0 0 0 4 16\n"   \
"add_object 1 vecx_0 vecy_24 vecz_0 0 0 0 4 16\n"   \
"add_object 1 vecx_0 vecy_28 vecz_0 0 0 0 4 16\n"   \
"add_object 1 vecx_0 vecy_32 vecz_0 0 0 0 4 16\n"

///////////////////////////////////////////////////////
const static char initial_map_200x100x200[] = ""

"set_world_size 300 100 300\n" // all must be divisible by MAX_WORLD_REGIONS (50 currently)

"set_world_plane 0 0 10 0 1 0 0 0 0 1\n" // idx, origin[3], v1[3], v2[3]

BACKGROUND_1

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

const static char initial_map_asteroid1[] = ""
BACKGROUND_TEX_SPACE

WORLD_SCALED_FRAME(1, 80, 4)

BUILDING_1(rndx, 8, rndz)
BUILDING_1(rndx, 8, rndz)
BUILDING_1(rndx, 8, rndz)
BUILDING_1(rndx, 8, rndz)
BUILDING_1(rndx, 8, rndz)
BUILDING_1(rndx, 8, rndz)

"register_params 2 rndx rndy rndz 0 0 0 4 29 0 0 0 0 0 0 0\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
;

const static char initial_map_collection[] = ""

//"set_world_size 200 100 200\n" // all must be divisible by MAX_WORLD_REGIONS

BACKGROUND_TEX_RANDOM

//MESH_CUBES_2(28, 4)
WORLD_SCALED_FRAME(1, 57, 4)

// else
GREEN_TENDRILS_1

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
"add_object 2 100 20 100 0 0 0 11 56\n"
"object_set_info 4\n" // moving block
"object_set_velocity 0 0.5 0\n"
 */

// add test tower
// add base
BASE_FRIENDLY_1

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

const static char initial_map_deathmatch[] = ""

"set_world_size 100 100 100\n" // all must be divisible by MAX_WORLD_REGIONS
//"set_background_info 27\n"
"set_background_info ""27""\n"

// stuff
"register_params 2 rndx rndy rndz 0 0 0 4 29 0 0 0 0 0 0 0\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"

"register_params 2 rndx rndy rndz 0 0 0 4 29 0 0 0 0 0 0 0\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"

"register_params 2 rndx rndy rndz 0 0 0 4 29 0 0 0 0 0 0 0\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"

"register_params 2 rndx rndy rndz 0 0 0 4 29 0 0 0 0 0 0 0\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"

"register_params 2 rndx rndy rndz 0 0 0 4 29 0 0 0 0 0 0 0\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"

"register_params 2 rndx rndy rndz 0 0 0 4 29 0 0 0 0 0 0 0\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"

"register_params 2 rndx rndy rndz 0 0 0 4 29 0 0 0 0 0 0 0\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"

"register_params 2 rndx rndy rndz 0 0 0 4 29 0 0 0 0 0 0 0\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"

"register_params 2 rndx rndy rndz 0 0 0 4 29 0 0 0 0 0 0 0\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"
"register_params_update 0 rnd_-4_4 rnd_-4_4 rnd_-4_4 0 0 0 0 0 0 0 0 0 0 0 0\n"
"add_object r r r r 0 0 0 r r\n"

// asteroid
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
ASTEROID_FIELD_5
;

const static char initial_map_survival[] = ""

"set_world_size 200 100 200\n" // all must be divisible by MAX_WORLD_REGIONS
//"set_background_info 27\n"
"set_background_info ""27""\n"

"mesh_manip_add wx0.1 " "-4" " wz0.1" " 4 0 0" " 0 0 4" " 40 40\n"
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_50 0"" 0.95\n"
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_50 0"" 0.95\n"
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_50 0"" 0.95\n"
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_-10 0"" 0.95\n"
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_-10 0"" 0.95\n"
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_-10 0"" 0.95\n"

"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_50 0"" 0.95\n"
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_50 0"" 0.95\n"
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_50 0"" 0.95\n"
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_-10 0"" 0.95\n"
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_-10 0"" 0.95\n"
"mesh_manip_pull rnd_50 rnd_50"" 0 rnd_-10 0"" 0.95\n"
"mesh_manip_complete "/*"2"*/"1"" "/*"57"*/"28"" 1 4\n"

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
 "add_object 2 100 20 100 0 0 0 11 56\n"
 "object_set_info 4\n" // moving block
 "object_set_velocity 0 0.5 0\n"
 */

// add test tower
// add base
"add_object 2 rndx 50 rndz 0 0 0 8 " "42" "\n"
"object_set_info 8\n"

// enemy base
"add_object 2 rndx 50 rndz 0 0 0 8 58\n"
"object_set_info 15\n"
// enemy base
"add_object 2 rndx 50 rndz 0 0 0 8 58\n"
"object_set_info 15\n"
// enemy base
"add_object 2 rndx 50 rndz 0 0 0 8 58\n"
"object_set_info 15\n"
// enemy base
"add_object 2 rndx 50 rndz 0 0 0 8 58\n"
"object_set_info 15\n"

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
"set_background_info 27\n"

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
"set_background_info 27\n"

// mesh of cubes
"mesh_manip_add wx1.0 " "wy0.0" " wz1.0" " -8 0 0" " 0 0 -8" " 50 50\n"
"mesh_manip_complete 1 19 1 8\n"
;

const static char map_200x100x200_pits[] = ""

"set_world_size 200 100 200\n" // all must be divisible by MAX_WORLD_REGIONS (50 currently)

"set_world_plane 0 0 10 0 1 0 0 0 0 1\n" // idx, origin[3], v1[3], v2[3]
"set_background_info 27\n"

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
"set_background_info 27\n"
"mesh_manip_add wx-0.1 " "4" " wz-0.1" " 4 0 0" " 0 0 4" " 10 10\n"
"mesh_manip_complete 1 19 1 4\n"
;

const static char* maps_list[] =
{
    initial_map_collection,
    initial_map_deathmatch,
    initial_map_200x100x200,
    map_400x100x400,
    map_portal_lobby,
    map_200x100x200_pits,
    initial_map_asteroid1,
    NULL
};

const static char* maps_list_names[] =
{
    "map hack-grid",
    "map asteroidfield",
    "map canvas",
    "map bigcanvas",
    "lobby",
    "pit",
    "asteroid",
    NULL
};


#endif
