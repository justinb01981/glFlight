//
//  gameInput.m
//  gl_flight
//
//  Created by Justin Brady on 2/1/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

// TODO: google yaw, pitch, roll rotation matrix
// and an input of roll results in changing the camera y,p,r relative to it's current orientation
// see euler angles
// http://mathworld.wolfram.com/EulerAngles.html
//
// 3-21-2012:
// after many blind alleys.. I have something close to working.. but the drift is just horrible
// ... you can't move on one axis for more than a few hundred frames before you are way off
// so I'm thinking that I can pre-compute positions and interpolate between them
// ...googling doesn't fill me with a lot of confidence..
//
// 5-11-2012:
// Now im using 3 vectors to represent our axes, and rotating 2 (using quaternion_rotate)
// about the axis we want to rotate.. but this is prone to drift, so you have to reset the
// vectors to be perpendicular to each other every so often. Got that working.
// One thing that really fucked me hard forever was the difference between euler and tait-bryan
// angles.. since I'm getting my information from multiple sources, I wasn't realizing there
// was such a difference in the formulas.. euler rotations re-use the already rotated axes
// NOTE: 'yaw' 'pitch' and 'roll' are misleading terms usually associated with tait-bryan
// right now I still have the body-quaternion 'b', even though I'm not using it, I'm
// deriving the camera rotations via geometry from the 3 vectors.. bettr way would be to just
// conver the quaternion 'b' to euler, but haven't bothered debugging that yet.
//

#import <math.h>
#import "cocoaGameInput.h"
#import "CoreMotion/CMDeviceMotion.h"
#import "gl_flightViewController.h"
#import "quaternions.h"
#import "gameInterface.h"
#import "gameCamera.h"
#include "gameShip.h"
#include "gameUtils.h"
#include "gameDialogs.h"


@implementation cocoaGyroManager

CMDeviceMotion* refMotion;
int resumeCountDown = 0;

+(cocoaGyroManager*) instance
{
    static cocoaGyroManager* gameInputSingleton = NULL;
    
	if(!gameInputSingleton)
	{
		gameInputSingleton = [[cocoaGyroManager alloc] initialize];
	}
	return gameInputSingleton;
}

+(void) handleDeviceMotionUpdate: (CMDeviceMotion*) motion withError:(NSError*) err
{
    if(resumeCountDown > 0)
    {
        resumeCountDown--;
        return;
    }
    
#if DEVICEMOTION_USE_GYRO_DATA
    if(!refMotion || gameInputTrimPending())
    {
        if(refMotion) [refMotion release];
        
        refMotion = [motion copy];
    }
    
    CMAttitude *attitude = [motion attitude];

    if(attitude && refMotion && [refMotion attitude])
    {
        [attitude multiplyByInverseOfAttitude:[refMotion attitude]];
        
        gameInputGyro([attitude roll], [attitude pitch], [attitude yaw]);
    }
#else
    CMRotationRate r = [motion rotationRate];
    
    motionRoll += r.y*0.01;
    motionPitch += r.x*0.01;
    motionYaw += r.z*0.01;
#endif
}

-(cocoaGyroManager*) initialize
{
	[self init];
    
	initialized = false;
    refMotion = NULL;
    
    self.showsDeviceMovementDisplay = YES;
    
	[self setDeviceMotionUpdateInterval:1/30];
    
	return self;
}

-(void) startUpdates
{
    CMDeviceMotionHandler handler = ^(CMDeviceMotion *motion, NSError *error)
    {
        [cocoaGyroManager handleDeviceMotionUpdate:motion withError:error];
    };
    
    [self startDeviceMotionUpdatesUsingReferenceFrame:CMAttitudeReferenceFrameXArbitraryCorrectedZVertical
                                              toQueue:[[NSOperationQueue alloc] init] withHandler:handler];
}

-(void) resumeUpdates
{
    resumeCountDown = 120;
}

@end
