//
//  world_file.c
//  gl_flight
//
//  Created by jbrady on 11/8/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include "world_file.h"
#include "world.h"
#include "gameGlobals.h"
#include "maps.h"
#include "textures.h"
#include "gameUtils.h"
#include "glFlight.h"
#include "gamePlay.h"
#include "gameDebug.h"

FILE* map_fp = NULL;
unsigned long map_file_size;
char map_path_prefix[1024];
char map_file_path[1024];
char *world_data_rendered = NULL;

const static float PARAM_ESCAPE_USE_REGISTER = 1234.1234;

static int map_file_open(int writing)
{
    map_fp = fopen(map_file_path, writing? "w": "r");
    if(map_fp)
    {
        fseek(map_fp, 0, SEEK_END);
        map_file_size = ftell(map_fp);
        fseek(map_fp, 0, SEEK_SET);
    }
    return (map_fp? 1: 0);
}

static void map_file_close()
{
    if(map_fp) fclose(map_fp);
    map_fp = NULL;
    map_file_size = 0;
}

static void map_write_line(char* line, char *cmd, float p[], int p_len)
{
    line[0] = '\0';
    
    strcat(line, cmd);
    for(int i = 0; i < p_len; i++)
    {
        char f[64];
        sprintf(f, " %f", p[i]);
        strcat(line, f);
    }
    strcat(line, "\n");
}

static int parse_command(char* command_line, world_map_command* map_cmd)
{
    int success = 0;
    
    char* line1 = strdup(command_line);
    if(!line1) return 0;
    char* line;
    
    char* token = strtok_r(line1, " ", &line);
    while(token)
    {
        int n_params = 0;
        
        map_cmd->n_params = 0;
        
        if(strcmp(token, "add_object") == 0)
        {
            map_cmd->type = MAP_ADD_OBJECT;
            n_params = 9;
        }
        else if(strcmp(token, "set_block_scale") == 0)
        {
            map_cmd->type = MAP_SET_BLOCK_SCALE;
            n_params = 1;
        }
        else if(strcmp(token, "object_identify") == 0)
        {
            map_cmd->type = MAP_IDENT_OBJECT;
            if(line) strncpy(map_cmd->cparams, line, sizeof(map_cmd->cparams)-1);
            n_params = 1;
        }
        else if(strcmp(token, "object_set_sub_info") == 0)
        {
            map_cmd->type = MAP_SET_OBJECT_SUB_INFO;
            n_params = 1;
        }
        else if(strcmp(token, "object_set_info") == 0)
        {
            map_cmd->type = MAP_SET_OBJECT_INFO;
            n_params = 1;
        }
        else if(strcmp(token, "add_bounding_vec") == 0)
        {
            map_cmd->type = MAP_ADD_BOUNDING_VEC;
            n_params = 6;
        }
        else if(strcmp(token, "add_enemy_jet") == 0)
        {
            map_cmd->type = MAP_ADD_ENEMY_JET;
            n_params = 2;
        }
        else if(strcmp(token, "add_enemy_turret") == 0)
        {
            map_cmd->type = MAP_ADD_ENEMY_TURRET;
            n_params = 2;
        }
        else if(strcmp(token, "add_objects_packed") == 0)
        {
            map_cmd->type = MAP_ADD_OBJECTS_PACKED;
            n_params = 14;
        }
        else if(strcmp(token, "add_spawn") == 0)
        {
            map_cmd->type = MAP_ADD_SPAWN_POINT;
            n_params = 6;
        }
        else if(strcmp(token, "add_teleport_region") == 0)
        {
            /*
            int add_object
            (int map_id, Model type, float x, float y, float z,
             float alpha, float beta, float gamma,
             float scale, int texture_id);
             
             -- last 3 fields are x y z destination for teleporter
            */
            map_cmd->type = MAP_ADD_TELEPORTER;
            n_params = 13;
        }
        else if(strcmp(token, "add_mesh") == 0)
        {
            /*
             void
             world_add_mesh(float x, float y, float z,
             float dx_r, float dy_r, float dz_r,
             float dx_c, float dy_c, float dz_c,
             unsigned int mesh_c, unsigned int mesh_r,
             float vis_norm_x, float vis_norm_y, float vis_norm_z);
             */
            map_cmd->type = MAP_ADD_MESH;
            n_params = 16;
        }
        else if(strcmp(token, "mesh_manip_add") == 0)
        {
            map_cmd->type = MAP_MANIP_MESH_ADD;
            n_params = 11;
        }
        else if(strcmp(token, "mesh_manip_object_info") == 0)
        {
            map_cmd->type = MAP_MANIP_MESH_INFO;
            n_params = 3;
        }
        else if(strcmp(token, "mesh_manip_pull") == 0)
        {
            map_cmd->type = MAP_MANIP_MESH_PULL;
            n_params = 6;
        }
        else if(strcmp(token, "mesh_manip_round") == 0)
        {
            map_cmd->type = MAP_MANIP_MESH_SET_ROUND;
            n_params = 3;
        }
        else if(strcmp(token, "mesh_manip_complete") == 0)
        {
            map_cmd->type = MAP_MANIP_MESH_COMPLETE;
            n_params = 7;
        }
        else if(strcmp(token, "mesh_manip_gltriangles_complete") == 0)
        {
            map_cmd->type = MAP_MANIP_MESH_TO_GLTRIANGLES;
            n_params = 1;
        }
        else if(strcmp(token, "set_world_size") == 0)
        {
            map_cmd->type = MAP_SET_SIZE;
            n_params = 3;
        }
        else if(strcmp(token, "define_world_plane") == 0)
        {
            map_cmd->type = MAP_DEFINE_PLANE;
            n_params = 10;
        }
        else if(strcmp(token, "register_params") == 0)
        {
            map_cmd->type = MAP_REGISTER_PARAMS;
            n_params = WORLD_MAP_COMMAND_PARAMS_MAX;
        }
        else if(strcmp(token, "register_params_update") == 0)
        {
            map_cmd->type = MAP_REGISTER_UPDATE_PARAMS;
            n_params = WORLD_MAP_COMMAND_PARAMS_MAX;
        }
        else if(strcmp(token, "register_params_clear") == 0)
        {
            map_cmd->type = MAP_REGISTER_CLEAR_PARAMS;
            n_params = WORLD_MAP_COMMAND_PARAMS_MAX;
        }
        else if(strcmp(token, "register_params_mul") == 0)
        {
            map_cmd->type = MAP_REGISTER_UPDATE_MUL_PARAMS;
            n_params = WORLD_MAP_COMMAND_PARAMS_MAX;
        }
        else if(strcmp(token, "set_background_info") == 0)
        {
            map_cmd->type = MAP_SET_BG_INFO;
            n_params = 1;
        }
        else if(strcmp(token, "set_vector") == 0)
        {
            map_cmd->type = MAP_ADD_VECTOR;
            n_params = 6;
        }
        else if(strcmp(token, "object_set_velocity") == 0)
        {
            map_cmd->type = MAP_SET_OBJECT_VELOCITY;
            n_params = 3;
        }
        else
        {
            break;
        }
        
        while(map_cmd->n_params < n_params)
        {
            float f;
            unsigned int tx;
            
            token = strtok_r(NULL, " ", &line);
            if(!token) break;
            
            if(strncmp(token, "rnd", 3) == 0)
            {
                char *rptr;
                long rb, re;
                
                switch(token[3])
                {
                case 'x':
                    f = rand_in_range(1, gWorld->bound_x-1);
                    break;
                case 'y':
                    f = rand_in_range(1, gWorld->bound_y-1);
                    break;
                case 'z':
                    f = rand_in_range(1, gWorld->bound_z-1);
                    break;
                case 'r':
                    f = ((float) (rand() % (int) (M_PI*2 * 100000)) / 100000.0);
                    break;
                        
                case '_':
                    rptr = &token[4];
                    rb = strtol(rptr, NULL, 10);
                        
                    while(*rptr == '-' || (*rptr >= '0' && *rptr <= '9')) rptr++;
                    if(*rptr == '_') rptr++;
                    if(*rptr == '-' || (*rptr >= '0' && *rptr <= '9'))
                    {
                        re = strtol(rptr, NULL, 10);
                    }
                    else
                    {
                        re = rb;
                        rb = 0;
                    }
                    f = rand_in_range(rb, re);
                    break;
                        
                default:
                    break;
                }
            }
            else if(strncmp(token, "wx", 2) == 0)
            {
                sscanf(&token[2], "%f", &f);
                f *= gWorld->bound_x;
            }
            else if(strncmp(token, "wy", 2) == 0)
            {
                sscanf(&token[2], "%f", &f);
                f *= gWorld->bound_y;
            }
            else if(strncmp(token, "wz", 2) == 0)
            {
                sscanf(&token[2], "%f", &f);
                f *= gWorld->bound_z;
            }
            else if(strncmp(token, "mx", 2) == 0)
            {
                sscanf(&token[2], "%f", &f);
                f *= world_mesh_pending_x();
            }
            else if(strncmp(token, "my", 2) == 0)
            {
                sscanf(&token[2], "%f", &f);
                f *= world_mesh_pending_y();
            }
            else if(strncmp(token, "vecx_", 5) == 0)
            {
                sscanf(&token[5], "%f", &f);
                f = gWorld->vec[0] + gWorld->vec[3] * f;
            }
            else if(strncmp(token, "vecy_", 5) == 0)
            {
                sscanf(&token[5], "%f", &f);
                f = gWorld->vec[1] + gWorld->vec[4] * f;
            }
            else if(strncmp(token, "vecz_", 5) == 0)
            {
                sscanf(&token[5], "%f", &f);
                f = gWorld->vec[2] + gWorld->vec[5] * f;
            }
            else if(strncmp(token, "r", 1) == 0)
            {
                f = PARAM_ESCAPE_USE_REGISTER;
            }
            else if(!sscanf(token, "%f", &f))
            {
                break;
            }
            else if(strncmp(token, "tx", 2) == 0)
            {
                sscanf(&token[2], "%u", &tx);
                if(tx < TEXTURE_ID_TABLE_IDX_LAST) f = texture_id_table[tx];
            }
            
            map_cmd->params[map_cmd->n_params++] = f;
        }
        
        success = 1;
        
        strtok_r(NULL, " ", &line);
    }
    
    free(line1);
    
    return success;
}

void gameMapFilePrefix(char* prefix)
{
    sprintf(map_path_prefix, "%s", prefix);
}

void gameMapFileName(char *name)
{
    sprintf(map_file_path, "%s/map_%s.txt", map_path_prefix, name);
}

void map_clear()
{    
    if(!map_file_open(1))
    {
        DBPRINTF(("map_file_open failed\n"));
        return;
    }
    
    fwrite(initial_map_200x100x200, sizeof(char), strlen(initial_map_200x100x200), map_fp);
    
    map_file_close();
}

void map_render(char *map_buf)
{
    // build world from map
    char* line_token = "\n";
    char* map_tmp = strdup(map_buf);
    char* line_tok = strtok(map_tmp, line_token);
    int last_object_id = WORLD_ELEM_ID_INVALID;
    float register_params[WORLD_MAP_COMMAND_PARAMS_MAX];
    
    for(int i = 0; i < WORLD_MAP_COMMAND_PARAMS_MAX; i++) register_params[i] = 0;
    
    while(line_tok)
    {
        world_map_command map_cmd;
        
        do {
            memset(&map_cmd, 0, sizeof(map_cmd));
            
            if(parse_command(line_tok, &map_cmd))
            {
                float vx, vy, vz, vsteps;
                float rc[2], d_xyz[3];
                
                if(map_cmd.type == MAP_REGISTER_PARAMS)
                {
                    for(int i = 0; i < WORLD_MAP_COMMAND_PARAMS_MAX; i++)
                        register_params[i] = map_cmd.params[i];
                    
                    line_tok = strtok(NULL, line_token);
                    continue;
                }
                else if(map_cmd.type == MAP_REGISTER_CLEAR_PARAMS)
                {
                    for(int i = 0; i < WORLD_MAP_COMMAND_PARAMS_MAX; i++)
                        if(map_cmd.params[i] != 0) register_params[i] = 0;
                    
                    line_tok = strtok(NULL, line_token);
                    continue;
                }
                else if(map_cmd.type == MAP_REGISTER_UPDATE_PARAMS)
                {
                    for(int i = 0; i < sizeof(map_cmd.params)/sizeof(map_cmd.params[0]); i++)
                        register_params[i] += map_cmd.params[i];
                    line_tok = strtok(NULL, line_token);
                    continue;
                }
                else if(map_cmd.type == MAP_REGISTER_UPDATE_MUL_PARAMS)
                {
                    for(int i = 0; i < sizeof(map_cmd.params)/sizeof(map_cmd.params[0]); i++)
                        register_params[i] *= map_cmd.params[i];
                    line_tok = strtok(NULL, line_token);
                    continue;
                }
                
                // replace some params with stored
                for(int i = 0; i < sizeof(register_params)/sizeof(register_params[0]); i++)
                {
                    if(map_cmd.params[i] == PARAM_ESCAPE_USE_REGISTER) map_cmd.params[i] = register_params[i];
                }
                
                switch(map_cmd.type)
                {
                    case MAP_SET_SIZE:
                        world_init(map_cmd.params[0], map_cmd.params[1], map_cmd.params[2]);
                        break;
                        
                    case MAP_SET_BG_INFO:
                        texture_id_background = map_cmd.params[0];
                        break;
                        
                    case MAP_DEFINE_PLANE:
                        world_set_plane(map_cmd.params[0],
                                        map_cmd.params[1], map_cmd.params[2], map_cmd.params[3],
                                        map_cmd.params[4], map_cmd.params[5], map_cmd.params[6],
                                        map_cmd.params[7], map_cmd.params[8], map_cmd.params[9]);
                        break;
                        
                    case MAP_ADD_BOUNDING_VEC:
                        // done automatically now
                        /*
                         boundingRegionAddVec(gWorld->boundingRegion,
                         map_cmd.params[0], map_cmd.params[1], map_cmd.params[2],
                         map_cmd.params[3], map_cmd.params[4], map_cmd.params[5]);
                         */
                        break;
                        
                    case MAP_ADD_OBJECT:
                        last_object_id =
                        world_add_object(map_cmd.params[0], map_cmd.params[1], map_cmd.params[2],
                                         map_cmd.params[3], map_cmd.params[4], map_cmd.params[5],
                                         map_cmd.params[6], map_cmd.params[7], map_cmd.params[8]);
                        break;
                        
                    case MAP_SET_OBJECT_SUB_INFO:
                        world_get_last_object()->stuff.subtype = map_cmd.params[0];
                        break;
                        
                    case MAP_SET_OBJECT_INFO:
                        world_get_last_object()->object_type = map_cmd.params[0];
                        if(!object_is_static(world_get_last_object()->object_type))
                        {
                            update_object_velocity(world_get_last_object()->elem_id, 0.0, 0.0, 0.0, 0);
                        }
                        
                        switch(world_get_last_object()->object_type)
                        {
                            case OBJ_TURRET:
                                game_elem_setup_turret(world_get_last_object(), gameStateSinglePlayer.enemy_intelligence);
                                break;
                                
                            case OBJ_SHIP:
                                game_elem_setup_ship(world_get_last_object(), gameStateSinglePlayer.enemy_intelligence);
                                break;
                                
                            case OBJ_POWERUP_GENERIC:
                                game_elem_setup_powerup(world_get_last_object());
                                break;
                                
                            case OBJ_SPAWNPOINT:
                                game_elem_setup_spawnpoint(world_get_last_object());
                                break;
                                
                            case OBJ_SPAWNPOINT_ENEMY:
                                game_elem_setup_spawnpoint_enemy(world_get_last_object());
                                break;
                                
                            default:
                                break;
                        }
                        break;
                        
                    case MAP_IDENT_OBJECT:
                        game_elem_identify(world_get_last_object(), map_cmd.cparams);
                        break;
                        
                    case MAP_SET_OBJECT_VELOCITY:
                        if(!object_is_static(world_get_last_object()->object_type))
                        {
                            update_object_velocity(world_get_last_object()->elem_id, map_cmd.params[0],
                                                   map_cmd.params[1], map_cmd.params[2], 0);
                        }
                        break;
                        
                    case MAP_ADD_OBJECTS_PACKED:
                        vx = map_cmd.params[9];
                        vy = map_cmd.params[10];
                        vz = map_cmd.params[11];
                        vsteps = map_cmd.params[12];
                        
                        for(float v = 0; v < vsteps; v++)
                        {
                            world_add_object(map_cmd.params[0],
                                             map_cmd.params[1] + vx*v,
                                             map_cmd.params[2] + vy*v,
                                             map_cmd.params[3] + vz*v,
                                             map_cmd.params[4], map_cmd.params[5],
                                             map_cmd.params[6], map_cmd.params[7], map_cmd.params[8]);
                        }
                        break;
                        
                        /*
                         case MAP_ADD_SPAWN_POINT:
                         last_object_id =
                         world_add_object(1,
                         map_cmd.params[0], map_cmd.params[1], map_cmd.params[2],
                         map_cmd.params[3], map_cmd.params[4], map_cmd.params[5],
                         10, TEXTURE_ID_SPAWNPOINT);
                         world_get_last_object()->object_type = OBJ_SPAWNPOINT;
                         break;
                         */
                        
                    case MAP_ADD_MESH:
                        world_add_mesh(map_cmd.params[0], map_cmd.params[1], map_cmd.params[2],
                                       map_cmd.params[3], map_cmd.params[4], map_cmd.params[5],
                                       map_cmd.params[6], map_cmd.params[7], map_cmd.params[8],
                                       map_cmd.params[9], map_cmd.params[10], map_cmd.params[11],
                                       map_cmd.params[12], map_cmd.params[13], map_cmd.params[14],
                                       map_cmd.params[15]);
                        break;
                        
                    case MAP_ADD_TELEPORTER:
                        world_add_object(map_cmd.params[0], map_cmd.params[1], map_cmd.params[2],
                                         map_cmd.params[3], map_cmd.params[4], map_cmd.params[5],
                                         map_cmd.params[6], map_cmd.params[7], map_cmd.params[8]);
                        world_get_last_object()->invisible = 1;
                        world_get_last_object()->stuff.u.teleporter.x = map_cmd.params[9];
                        world_get_last_object()->stuff.u.teleporter.y = map_cmd.params[10];
                        world_get_last_object()->stuff.u.teleporter.z = map_cmd.params[11];
                        break;
                        
                    case MAP_MANIP_MESH_ADD:
                        world_prepare_mesh(map_cmd.params[0], map_cmd.params[1], map_cmd.params[2],
                                           map_cmd.params[3], map_cmd.params[4], map_cmd.params[5],
                                           map_cmd.params[6], map_cmd.params[7], map_cmd.params[8],
                                           map_cmd.params[9], map_cmd.params[10]);
                        break;
                        
                    case MAP_MANIP_MESH_INFO:
                        world_set_mesh_object_info(map_cmd.params[0], map_cmd.params[1], map_cmd.params[2]);
                        break;
                        
                    case MAP_MANIP_MESH_PULL:
                        rc[0] = map_cmd.params[0];
                        rc[1] = map_cmd.params[1];
                        d_xyz[0] = map_cmd.params[2];
                        d_xyz[1] = map_cmd.params[3];
                        d_xyz[2] = map_cmd.params[4];
                        world_manip_mesh(rc, d_xyz, map_cmd.params[5]);
                        break;
                        
                    case MAP_MANIP_MESH_SET_ROUND:
                        world_manip_mesh_round(map_cmd.params[0], map_cmd.params[1], map_cmd.params[2]);
                        break;
                        
                    case MAP_MANIP_MESH_COMPLETE:
                        world_complete_mesh(map_cmd.params[0], map_cmd.params[1], map_cmd.params[2], map_cmd.params[3]);
                        break;
                        
                    case MAP_MANIP_MESH_TO_GLTRIANGLES:
                        world_convert_mesh_to_gltriangles(map_cmd.params[0]);
                        break;
                        
                    case MAP_SET_BLOCK_SCALE:
                        block_scale = map_cmd.params[0];
                        break;
                        
                    case MAP_ADD_VECTOR:
                        gWorld->vec[0] = map_cmd.params[0];
                        gWorld->vec[1] = map_cmd.params[1];
                        gWorld->vec[2] = map_cmd.params[2];
                        gWorld->vec[3] = map_cmd.params[3];
                        gWorld->vec[4] = map_cmd.params[4];
                        gWorld->vec[5] = map_cmd.params[5];
                        break;
                        
                    default:
                        break;
                }
            }
            
            line_tok = strtok(NULL, line_token);
        } while(0);
    }
    
    if(map_tmp) free(map_tmp);
}

char* gameMapRead()
{
    int read_len;
    int reopen_retries = 0;
    static char *map_read_last = NULL;
    
    read_retry:
    if(!map_file_open(0))
    {
        map_clear();
        DBPRINTF(("map_file_open(0) failed\n"));
        reopen_retries++;
        if(reopen_retries <= 1) goto read_retry;
    }
    
    if(map_read_last)
    {
        free(map_read_last);
        map_read_last = NULL;
    }
    
    map_read_last = malloc(map_file_size+1);
    if(!map_read_last)
    {
        fclose(map_fp);
        map_clear();
        return NULL;
    }
    
    read_len = fread(map_read_last, sizeof(char), map_file_size, map_fp);
    map_read_last[read_len] = '\0';
    
    DBPRINTF(("read map:\n%s\n", map_read_last));
    
    map_file_close();
    
    return map_read_last;
}

char* world_write_buffer()
{
    int line_bytes = 256;
    size_t buffer_len = line_bytes;
    char *p = NULL;
    
    p = malloc(buffer_len);
    if(p)
    {
        sprintf(p, "set_world_size %f %f %f\n"
                "set_background_info %f\n", gWorld->bound_x, gWorld->bound_y, gWorld->bound_z,
                (float) texture_id_background);
        
        WorldElemListNode* pElemListNode = gWorld->elements_list.next;
        while(pElemListNode)
        {
            char line[1024];
            float params[16];
            int i = 0;
            
            if(pElemListNode->elem->head_elem == NULL && pElemListNode->elem->lifetime == 0)
            {
                if(pElemListNode->elem->elem_id == my_ship_id)
                {
                    /* exclude the player ship */
                }
                else if(object_is_static(pElemListNode->elem->object_type) &&
                       pElemListNode->elem->physics.ptr->velocity == 0)
                {
                    params[i++] = pElemListNode->elem->type;
                    params[i++] = pElemListNode->elem->physics.ptr->x;
                    params[i++] = pElemListNode->elem->physics.ptr->y;
                    params[i++] = pElemListNode->elem->physics.ptr->z;
                    params[i++] = pElemListNode->elem->physics.ptr->alpha;
                    params[i++] = pElemListNode->elem->physics.ptr->beta;
                    params[i++] = pElemListNode->elem->physics.ptr->gamma;
                    params[i++] = pElemListNode->elem->scale;
                    params[i++] = pElemListNode->elem->texture_id;
                    
                    map_write_line(line, "add_object", params, i);
                    buffer_len += strlen(line);
                    p = realloc(p, buffer_len);
                    if(!p) return NULL;
                    strcat(p, line);
                    
                    if(pElemListNode->elem->object_type != OBJ_UNKNOWN)
                    {
                        i = 0;
                        params[i++] = pElemListNode->elem->object_type;
                        map_write_line(line, "object_set_info", params, i);
                        buffer_len += strlen(line);
                        p = realloc(p, buffer_len);
                        if(!p) return NULL;
                        strcat(p, line);
                    }
                    
                    printf("%s:%d(%s): %s\n", __FILE__, __LINE__, __FUNCTION__, line);
                }
            }
            
            pElemListNode = pElemListNode->next;
        }
    }
    return p;
}

void gameMapWrite()
{
    map_file_open(1);
    
    char *world_buf = world_write_buffer();
    if(world_buf)
    {
        fwrite(world_buf, 1, strlen(world_buf), map_fp);
        free(world_buf);
    }
    
    map_file_close();
    
}

void gameMapSetMap(const char *data)
{
    if(world_data) free(world_data);
    world_data = data? strdup(data): NULL;
    world_inited = 0;
}

void gameMapReRender()
{
    world_free();
    if(world_data == NULL || strlen(world_data) < 1)
    {
        if(world_data) free(world_data);
        world_data = strdup(maps_list[0]);
    }
    map_render(world_data);
    world_optimize();
    
    if(world_data) free(world_data);
    world_data = NULL;
    world_inited = 1;
}

char* gameMapReadRendered()
{
    char *tmp = world_write_buffer();
    
    if(world_data_rendered) free(world_data_rendered);
    world_data_rendered = tmp;
    return world_data_rendered;
}
