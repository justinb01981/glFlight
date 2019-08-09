//
//  gameSettings.h
//  gl_flight
//
//  Created by Justin Brady on 7/1/13.
//
//

#ifndef gl_flight_gameSettings_h
#define gl_flight_gameSettings_h

#include "gamePlay.h"
#include "gameInterface.h"
#include "glFlight.h"
#include "gameAudio.h"
#include "textures.h"
#include "gamePlatform.h"
#include "gameNetwork.h"

typedef struct
{
    char map_name[128];
    char server_name[128];
    char player_name[128];
    char directory_hostname[128];
    int net_update_frequency;
    int hosting;
    unsigned short host_port;
    char hidden_config[128];
    char local_inet_addr[128];
} glFlightPrefs;

// TODO:bump this every time settings change
const static int settings_version = 37;

// HACK: externs built in .m files
extern double dz_roll, dz_pitch, dz_yaw;

extern int gameSettingsLaunchCount;
extern int gameSettingsRatingGiven;
extern char gameSettingsPlayerName[255];
extern char gameSettingsPlayerNameDefault[255];
extern char gameSettingsMapName[255];
extern char gameSettingGameTitle[255];
extern char gameSettingsGameNameDefault[255];
extern char gameSettingsGameName[255];
extern int gameSettingsNetworkFrequency;
extern int gameSettingsPortNumber;
extern char gameSettingsLocalIPOverride[255];
extern int gameSettingsComplexControls;
extern char gameSettingsVersionStr[255];
extern char gameSettingsKillHistory[4096];
static char gameSettingsDirectoryServerNameREMOVED[255];

static char* nullableStrings[] = {
    gameSettingsKillHistory,
    gameSettingsPlayerName,
    gameSettingsMapName,
    gameSettingGameTitle,
    gameSettingsDirectoryServerNameREMOVED,
    gameSettingsLocalIPOverride,
    gameSettingsVersionStr
};

static void
gameSettingsStatsReset()
{
    for(int s = 0; s < GAME_TYPE_LAST; s++) gameStateSinglePlayer.high_score[s] = 0;
    gameStateSinglePlayer.lifetime_credits = 0;
}

static void
gameSettingsPlatformInit(const char *playerName, const char* serverName)
{
    strcpy(gameSettingsPlayerNameDefault, playerName);
    strcpy(gameSettingsGameNameDefault, "serverName");
}

static void
gameSettingsDefaults()
{
    // default values
    gameStateSinglePlayer.difficulty = 1;
    gameStateSinglePlayer.setting_bot_intelligence = 1;
    gameAudioMuted = 0;
    texture_id_playership = TEXTURE_ID_SHIP1;
    block_scale = 1.0;
    game_terminated_gracefully = 0;
    gameSettingsStatsReset();
    GYRO_DC = 3;
    model_my_ship = MODEL_SHIP1;
    gameStateSinglePlayer.last_game.difficulty = 0;
    gameStateSinglePlayer.last_game.score = 0;
    gameSettingsLaunchCount = 0;
    gameSettingsRatingGiven = 0;
    strcpy(gameSettingsPlayerName, gameSettingsPlayerNameDefault);
    strcpy(gameSettingsMapName, "MAP1");
    strcpy(gameSettingGameTitle, "dogfight1");
    strcpy(gameSettingsLocalIPOverride, "0.0.0.0");
    strcpy(gameSettingsKillHistory, "");
    gameSettingsNetworkFrequency = 50;
    gameSettingsPortNumber = 52000;
    gameStateSinglePlayer.lifetime_credits = 0;
    gameSettingsComplexControls = 0;
    strcpy(gameSettingsVersionStr, GAME_VERSION_STR);
}

static void
gameSettingsWrite(const char *filename)
{
    FILE* fp;
    int i;
    
    // HACK: don't allow "empty" strings to corrupt the (awful) settings format
    for(i = 0; i < sizeof(nullableStrings)/sizeof(char*); i++)
    {
        if(strlen(nullableStrings[i]) == 0)
        {
            strcpy(nullableStrings[i], "null");
        }
    }
    
    fp = fopen(filename, "w");
    if(fp)
    {
        // append at END
        fprintf(fp, "%d\n", settings_version);
        fprintf(fp, "%d\n", 0); // previously gameStateSinglePlayer.setting_n_bots
        fprintf(fp, "%d\n", gameStateSinglePlayer.difficulty);
        fprintf(fp, "%d\n", texture_id_playership);
        fprintf(fp, "%d\n", gameAudioMuted);
        fprintf(fp, "%d\n", gameStateSinglePlayer.setting_bot_intelligence);
        fprintf(fp, "%f\n", block_scale);
        fprintf(fp, "%d\n", game_terminated_gracefully);
        for(int s = 0; s < GAME_TYPE_LAST; s++) fprintf(fp, "%d\n", gameStateSinglePlayer.high_score[s]);
        fprintf(fp, "%f\n", GYRO_DC);
        fprintf(fp, "%d\n", model_my_ship);
        fprintf(fp, "%d\n", gameStateSinglePlayer.last_game.difficulty);
        fprintf(fp, "%d\n", gameStateSinglePlayer.last_game.score);
        fprintf(fp, "%d\n", gameSettingsLaunchCount);
        fprintf(fp, "%d\n", gameSettingsRatingGiven);
        fprintf(fp, "%s\n", strlen(gameSettingsPlayerName) > 0 ? gameSettingsPlayerName : "null");
        fprintf(fp, "%s\n", strlen(gameSettingsMapName) > 0 ? gameSettingsMapName : "NULL");
        fprintf(fp, "%s\n", strlen(gameSettingGameTitle) > 0 ? gameSettingGameTitle : "NULL");
        fprintf(fp, "%s\n", "d0gf1ght.domain17.net");
        fprintf(fp, "%s\n", strlen(gameSettingsLocalIPOverride) > 0 ? gameSettingsLocalIPOverride : "NULL");
        fprintf(fp, "%d\n", gameSettingsNetworkFrequency);
        fprintf(fp, "%d\n", gameSettingsPortNumber);
        fprintf(fp, "%lu\n", gameStateSinglePlayer.lifetime_credits);
        fprintf(fp, "%d\n", gameSettingsComplexControls);
        fprintf(fp, "%s\n", GAME_VERSION_STR);
        fprintf(fp, "%s\n", gameSettingsKillHistory);
        
        fclose(fp);
    }
    
    for(i = 0; i < sizeof(nullableStrings)/sizeof(char*); i++)
    {
        if(!strcmp(nullableStrings[i], "null"))
        {
            nullableStrings[i][0] = '\0';
        }
    }
}

static int
gameSettingsRead(const char *filename)
{
    FILE* fp = fopen(filename, "r");
    int settings_ver;
    int err = 0;
    int ignored = 0;
    int i;
    
    while(fp)
    {
        // append at END
        if(fscanf(fp, "%d", &settings_ver) < 1) break;
        
        if(settings_version == 32 && settings_ver == 31)
        {
            // migrate from 31->32
            gameSettingsRatingGiven = 0;
            gameSettingsLaunchCount = 0;
            
        }
        else if(settings_version == 33 && settings_ver == 32)
        {
            gameSettingsComplexControls = 0;
        }
        else if(settings_version == 37 && settings_ver == 36)
        {
            strcpy(gameSettingsKillHistory, "");
        }
        else
        // settings are from old binary, force defaults
        if(settings_ver != settings_version) break;
        
        if(fscanf(fp, "%d", &ignored) < 1) break;
        if(fscanf(fp, "%d", &gameStateSinglePlayer.difficulty) < 1) break;
        if(fscanf(fp, "%d", &texture_id_playership) < 1) break;
        if(fscanf(fp, "%d", &gameAudioMuted) < 1) break;
        if(fscanf(fp, "%d", &gameStateSinglePlayer.setting_bot_intelligence) < 1) break;
        if(fscanf(fp, "%f", &block_scale) < 1) break;
        if(fscanf(fp, "%d", &game_terminated_gracefully) < 1) break;
        for(int s = 0; s < GAME_TYPE_LAST; s++)
        {
            if(fscanf(fp, "%d", &gameStateSinglePlayer.high_score[s]) < 1)
            {
                err = 0;
            }
        }
        if(fscanf(fp, "%f", &GYRO_DC) < 1) break;
        if(GYRO_DC < 0) GYRO_DC = 1;
        
        if(fscanf(fp, "%d", &model_my_ship) < 1) break;
        if(fscanf(fp, "%d", &gameStateSinglePlayer.last_game.difficulty) < 1) break;
        if(fscanf(fp, "%d", &gameStateSinglePlayer.last_game.score) < 1) break;
        if(fscanf(fp, "%d", &gameSettingsLaunchCount) < 1) break;
        if(fscanf(fp, "%d", &gameSettingsRatingGiven) < 1) break;
        if(fscanf(fp, "%s", gameSettingsPlayerName) < 1) break;
        if(fscanf(fp, "%s", gameSettingsMapName) < 1) break;
        if(fscanf(fp, "%s", gameSettingGameTitle) < 1) break;
        if(fscanf(fp, "%s", gameSettingsDirectoryServerNameREMOVED) < 1) break;
        if(fscanf(fp, "%s", gameSettingsLocalIPOverride) < 1) break;
        if(fscanf(fp, "%d", &gameSettingsNetworkFrequency) < 1) break;
        if(fscanf(fp, "%d", &gameSettingsPortNumber) < 1) break;
        if(fscanf(fp, "%lu", &gameStateSinglePlayer.lifetime_credits) < 1) break;
        if(fscanf(fp, "%d", &gameSettingsComplexControls) < 1) break;
        if(fscanf(fp, "%s", gameSettingsVersionStr) < 1) break;
        if(fscanf(fp, "%s", gameSettingsKillHistory) < 1) break;
        
        fclose(fp);
        
        err = 1;
    }
    
    for(i = 0; i < sizeof(nullableStrings)/sizeof(char*); i++)
    {
        if(!strcmp(nullableStrings[i], "null"))
        {
            nullableStrings[i][0] = '\0';
        }
    }
    
    // version changed
    if(strncmp(gameSettingsVersionStr, GAME_VERSION_STR, sizeof(gameSettingsVersionStr)) != 0)
    {
        gameSettingsRatingGiven = 0;
    }
    
    if(fp) fclose(fp);
    return err;
}

#endif
