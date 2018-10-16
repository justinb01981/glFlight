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
#import "GameNetworkBonjourManager.h"

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
    
    NSString *docsPrefix = get_docs_prefix();
    
    get_time_ms_init();
    
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
    
    game_ai_init();
    
    //gameInputInit();
    
    console_init();
    
    gameMapFilePrefix((char*) [docsPrefix UTF8String]);
    gameMapFileName("temp");
    
    gameNetwork_init_mem();
    gameNetwork_init(0, gameSettingGameTitle, gameSettingsPlayerName, gameSettingsNetworkFrequency,
                     gameSettingsPortNumber, gameSettingsLocalIPOverride);
    
    if(game_terminated_gracefully) reset_map = 0;
    game_terminated_gracefully = 0;
    appWriteSettings();
    
    if(!reset_map)
    {
        gameMapSetMap(maps_list[0]);
    }
    else
    {
        gameMapSetMap(maps_list[0]);
    };
    
    console_write("Welcome to d0gf1ght %s\n"
                  "http://www.domain17.net/d0gf1ght\n"
                  "^D^Dnow with bluetooth local play!^D^D\n",
                  [[[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"] UTF8String]);
    
    gameInterfaceInit(viewWidth, viewHeight);
}

void glFlightResume(time_t time_last_suspend)
{
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
    
    [NSThread detachNewThreadSelector:@selector(applicationBackgroundWorkerTimer) toTarget:self withObject:NULL];
    
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
    
    console_log_clear(0, 1);
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

- (void) applicationBackgroundWorkerTimer
{
    while(1)
    {
        /* keeping this in glFlight-main render loop for now on iOS */
        //extern void update_time_ms_frame_tick();
        //update_time_ms_frame_tick();
        
        usleep((1000/GAME_TICK_RATE)*1000);
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

void
AppDelegateOpenURL(const char* url)
{
    [[UIApplication sharedApplication] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
}
