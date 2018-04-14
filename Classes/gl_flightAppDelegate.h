//
//  gl_flightAppDelegate.h
//  gl_flight
//
//  Created by Justin Brady on 12/14/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#include <time.h>

@class gl_flightViewController;

@interface gl_flightAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    gl_flightViewController *viewController;
    
    time_t time_last_suspend;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet gl_flightViewController *viewController;

@end

