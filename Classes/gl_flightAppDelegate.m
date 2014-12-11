//
//  gl_flightAppDelegate.m
//  gl_flight
//
//  Created by Justin Brady on 12/14/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "gl_flightAppDelegate.h"
#import "gl_flightViewController.h"
#import "cocoaGameInput.h"
#import "cocoaNetworkThread.h"
#import "cocoaAudioThread.h"

#include "glFlight.h"
#include "world.h"
#include "gameGraphics.h"
#include "gameAudio.h"
#include "gameCamera.h"
#include "textures.h"
#include "gameInterface.h"
#include "gameNetwork.h"
#include "gameAI.h"
#include "world_file.h"
#include "gameSettings.h"
#include "maps.h"
#include "gameDialogs.h"

@implementation gl_flightAppDelegate

@synthesize window;
@synthesize viewController;

void readPrefs(glFlightPrefs* prefs)
{
    NSUserDefaults* defaults = [[NSUserDefaults alloc] init];

    NSString *defaultPlayerName = [NSString stringWithFormat:@"player_%@",
                                   [[[UIDevice currentDevice] name] componentsSeparatedByString:@" "][0]];
    NSString *defaultServerName = [NSString stringWithFormat:@"game_%@",
                                   [[[UIDevice currentDevice] name] componentsSeparatedByString:@" "][0]];
    
    memset(prefs, 0, sizeof(*prefs));
    
    NSString *key_map = @"map_name";
    NSString *key_server = @"host_preference";
    NSString *key_name = @"name_preference";
    NSString *key_update_frequency = @"update_frequency_ms_preference";
    NSString *key_directory_name = @"game_directory_preference";
    NSString *key_host_port = @"game_host_port";
    NSString *key_hidden_cfg = @"game_extra_cfg";
    NSString *key_local_inet_addr_cfg = @"local_inet_addr_preference";
    
    // set default values
    if(![defaults stringForKey:key_map]) [defaults setValue:@"map1" forKey:key_map];
    if(![defaults stringForKey:key_server]) [defaults setValue:defaultServerName forKey:key_server];
    if(![defaults stringForKey:key_name]) [defaults setValue:defaultPlayerName forKey:key_name];
    if(![defaults integerForKey:key_update_frequency]) [defaults setInteger:50 forKey:key_update_frequency];
    if(![defaults stringForKey:key_directory_name]) [defaults setValue:@"d0gfight.domain17.net" forKey:key_directory_name];
    if(![defaults stringForKey:key_host_port]) [defaults setValue:@GAME_NETWORK_PORT_STR forKey:key_host_port];
    if(![defaults stringForKey:key_hidden_cfg]) [defaults setValue:@"" forKey:key_hidden_cfg];
    if(![defaults stringForKey:key_local_inet_addr_cfg]) [defaults setValue:@"0.0.0.0" forKey:key_local_inet_addr_cfg];
    
    strcpy(prefs->map_name, [[defaults stringForKey:key_map] UTF8String]);
    strcpy(prefs->server_name, [[defaults stringForKey:key_server] UTF8String]);
    strcpy(prefs->player_name, [[defaults stringForKey:key_name] UTF8String]);
    strcpy(prefs->directory_hostname, [[defaults stringForKey:key_directory_name] UTF8String]);
    strcpy(prefs->hidden_config, [[defaults stringForKey:key_hidden_cfg] UTF8String]);
    strcpy(prefs->local_inet_addr, [[defaults stringForKey:key_local_inet_addr_cfg] UTF8String]);
    prefs->host_port = [defaults integerForKey:key_host_port];
    
    prefs->net_update_frequency = [defaults integerForKey:key_update_frequency];
    prefs->hosting = 0;
    
    [defaults release];
}

void appWriteSettings()
{
    gameSettingsWrite([[NSString stringWithFormat:@"%@/settings.txt", get_docs_prefix()] UTF8String]);
}

NSString* get_docs_prefix()
{
    return [NSHomeDirectory() stringByAppendingPathComponent:
            [NSString stringWithFormat:@"Documents"]];
}

void glFlightInit(gl_flightViewController* viewController)
{
    int reset_map = 1;
    glFlightPrefs prefs;
    
    NSString *docsPrefix = get_docs_prefix();
    get_time_ms();
    
    rand_seed(time_ms);
    
    NSString *defaultPlayerName = [NSString stringWithFormat:@"player_%@",
                                   [[[UIDevice currentDevice] name] componentsSeparatedByString:@" "][0]];
    NSString *defaultServerName = [NSString stringWithFormat:@"game_%@",
                                   [[[UIDevice currentDevice] name] componentsSeparatedByString:@" "][0]];
    
    gameSettingsPlatformInit([defaultPlayerName UTF8String], [defaultServerName UTF8String]);
    
    if(!gameSettingsRead([[NSString stringWithFormat:@"%@/settings.txt", get_docs_prefix()] UTF8String]))
    {
        gameSettingsDefaults();
    }
    
    initTextures([[NSString stringWithFormat:@"%@/", [[NSBundle mainBundle] resourcePath]] UTF8String]);
    
    viewWidth = [viewController.view bounds].size.width;
    viewHeight = [viewController.view bounds].size.height;
    
    gameCamera_init(0, 0, 0, 0, 0, 0);
    game_init();
    gameAudioInit();
    world_lock_init();
    
    readPrefs(&prefs);
    
    game_ai_init();
    
    gameInputInit();
    
    gameMapFilePrefix((char*) [docsPrefix UTF8String]);
    gameMapFileName("temp");
    
    gameNetwork_init(0, gameSettingsGameName, gameSettingsPlayerName, gameSettingsNetworkFrequency,
                     gameSettingsDirectoryServerName, gameSettingsPortNumber, gameSettingsLocalIPOverride);
    
    if(game_terminated_gracefully) reset_map = 0;
    game_terminated_gracefully = 0;
    appWriteSettings();
    
    if(!reset_map)
    {
        //gameMapSetMapData(gameNetworkState.server_map_data);
        gameMapSetMap(maps_list[0]);
    }
    else
    {
        gameMapSetMap(maps_list[0]);
    };
    
    console_write("Welcome to d0gf1ght %s\n"
                  "http://www.domain17.net/d0gf1ght\n",
                  [[[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleVersion"]UTF8String]);
    
    gameInterfaceInit(viewWidth, viewHeight);
    gameInterfaceControls.trim.blinking = 1;
    
    gameDialogCalibrate();
}

void glFlightResume(time_t time_last_suspend)
{
    glFlightPrefs prefs;
    
    readPrefs(&prefs);
    
    if(time(NULL) - time_last_suspend > 30)
    {
    }
    
    gameNetwork_resume();
    
    gameDialogResume();
    
    [[cocoaGyroManager instance] resumeUpdates];
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    [application setIdleTimerDisabled:TRUE];
    
    [self.window addSubview:self.viewController.view];
    
    [self performSelectorOnMainThread:@selector(applicationInit) withObject:NULL waitUntilDone:true];
    
    [NSThread detachNewThreadSelector:@selector(run) toTarget:[[cocoaAudioThread alloc] init] withObject:NULL];

    [NSThread detachNewThreadSelector:@selector(applicationBackgroundWorker) toTarget:self withObject:NULL];
    
    time_last_suspend = 0;
    
    gameSettingsLaunchCount++;
    
    [self.window setFrame:[[UIScreen mainScreen] bounds]];
    
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    [self.viewController stopAnimation];
    appWriteSettings();
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    [[cocoaGyroManager instance] startUpdates];
    [self.viewController startAnimation];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    [self.viewController stopAnimation];
    
    gameNetwork_disconnect();
    
    if(save_map)
    {
        world_lock();
        gameMapWrite();
        world_unlock();
    }
    
    game_terminated_gracefully = 1;
    appWriteSettings();
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Handle any background procedures not related to animation here.
    time_last_suspend = time(NULL);
    game_paused = 1;
    gameNetwork_disconnect();
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Handle any foreground procedures not related to animation here.
    glFlightResume(time_last_suspend);
}

- (void) applicationInit
{
    glFlightInit(self.viewController);
    
    [self.viewController startAnimation];
}

- (void) applicationBackgroundWorker
{
    while(1)
    {
        if(gameDialogState.ratingDesired)
        {
            gameDialogState.ratingDesired = 0;
            [[UIApplication sharedApplication] openURL:[NSURL URLWithString:@APP_ITUNES_URL]];
        }
        
        /*
        if(gameAIState.started)
        {
            world_lock();
            game_ai_run();
            world_unlock();
        }
        
        world_lock();
        do_game_network_write();
        world_unlock();
         */
        
        // throttle this thread in select(), not here
        do_game_network_read();
    }
}

- (void)dealloc
{
    [viewController release];
    [window release];
    
    [super dealloc];
}

- (void) doAlertMessage:(NSString*)str
{
    UIAlertView* v = [[UIAlertView alloc] initWithTitle:@"Alert" message:str delegate:self cancelButtonTitle:@"OK" otherButtonTitles:NULL, nil];
    
    [v show];
    [v release];
}

@end
