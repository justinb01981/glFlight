//
//  gameInput.h
//  gl_flight
//
//  Created by Justin Brady on 2/1/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreMotion/CoreMotion.h>
#import "quaternions.h"
#include "gameInput.h"

@interface cocoaGyroManager : CMMotionManager {

	bool initialized;
	float yaw_m, pitch_m, roll_m;
}

+(cocoaGyroManager*) instance;

-(cocoaGyroManager*) initialize;

-(void) startUpdates;

-(void) resumeUpdates;

@end
