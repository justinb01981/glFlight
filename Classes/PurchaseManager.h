//
//  PurchaseManager.h
//  WalkieTalkie
//
//  Created by Justin Brady on 12/13/23.
//

#ifndef PurchaseManager_h
#define PurchaseManager_h

#import <StoreKit/StoreKit.h>

static NSString* PRODUCT_UPGRADE1_ID = @"2001"; // FROM APP STORE CONNECT

//static NSString* PREFS_KEY_CRYPTOVOX_EKEY = @"encrypt_key_cryptovox";
//static NSString* PREFS_KEY_CRYPTOVOX_CHAN = @"chan_priv_cryptovox";

@interface PurchaseManager: NSObject

+(instancetype)shared;

@property (readonly) BOOL purchased;

- (void) purchase: (void(^)(UIViewController*))cb;

- (void) uponActivation: (void(^)(void))doThis;

@end



#endif /* PurchaseManager_h */
