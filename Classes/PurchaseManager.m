//
//  PurchaseManager.m
//  WalkieTalkie
//
//  Created by Justin Brady on 12/13/23.
//


/* sandbox test account: justinb+01981@gmail.com */



#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>
#import "PurchaseManager.h"

extern void generateGUIDPurchaseManager(char*);

// MARK: -- private helpers

static NSString* keyForProduct(NSString* s) {
    return [NSString stringWithFormat:@"purchased_%@", s];
}


// MARK: -- private types

@interface MyProduct: SKProduct
@end

@implementation MyProduct

- (NSString*) productIdentifier {
    return PRODUCT_UPGRADE1_ID;
}

@end


// MARK: -- PurchaseManager singleton
static PurchaseManager* gPurchaseManager;

@interface PurchaseManager() <SKProductsRequestDelegate, SKPaymentTransactionObserver>
{
    BOOL warned;
    BOOL purchased;
    SKStoreProductViewController* vc;
    MyProduct* prod;
}

@property (nonatomic) BOOL purchased;
@property (copy) void(^requestPurchase)(UIViewController*);
@property (copy) void(^uponSuccess)(void);
@property (retain) SKProductsRequest* req;

@end


@implementation PurchaseManager
@synthesize  req;

- (void)setPurchased:(BOOL)purchased {
    [NSUserDefaults.standardUserDefaults setBool:purchased forKey:keyForProduct(PRODUCT_UPGRADE1_ID)];
}

- (BOOL)getPurchased:(BOOL)purchased {
    return [NSUserDefaults.standardUserDefaults boolForKey:keyForProduct(PRODUCT_UPGRADE1_ID)];
}

+ (id)shared {
    if(!gPurchaseManager) {
        gPurchaseManager = [[PurchaseManager alloc ] init];
    }
    return gPurchaseManager;
}

- (instancetype)init {
    NSLog(@"testPurchases init\n");

    warned = NO;

    // https://developer.apple.com/documentation/storekit/transaction/3851204-currententitlements

    [SKPaymentQueue.defaultQueue addTransactionObserver:self];

    return self;
}

- (void)dealloc {
    [super dealloc];
    [SKPaymentQueue.defaultQueue removeTransactionObserver:self];
}

- (void) purchase: (void(^)(UIViewController*))cb {
    self.requestPurchase = cb;

    req = [[SKProductsRequest alloc] init];
    [req initWithProductIdentifiers:  [NSSet setWithArray:@[PRODUCT_UPGRADE1_ID]]   ] ;
    req.delegate = self;

    [req start];

}

-(void) uponActivation: (void (^)(void))doThis {
    self.uponSuccess = doThis;

    if (self.purchased) {   // already purchased?
        self.uponSuccess();
    }
}

- (void)paymentQueue:(nonnull SKPaymentQueue *)queue updatedTransactions:(nonnull NSArray<SKPaymentTransaction *> *)transactions {

    // MARK: -- warning: product will not show up as valid until it is in READY FOR SUBMIT state! not "missing metadata"
    SKPaymentTransaction* nextTrans = [transactions lastObject];

    // transactionIdentifier is only set when purchased or restored
    if(nextTrans.transactionIdentifier != nil) {
        NSLog(@"found transaction ID + restoring purchase");
        //purchased = YES;
    }

    if ([nextTrans transactionState] == SKPaymentTransactionStatePurchasing)
    {
        if( !purchased) {
            assert(nextTrans.transactionIdentifier == nil);

            prod = [[MyProduct alloc] init];
            vc = [[SKStoreProductViewController alloc] init];

            NSLog(@"launching product store view");
            [vc loadProductWithParameters: @{PRODUCT_UPGRADE1_ID: @"PURCHASED"} completionBlock:^(BOOL result, NSError * _Nullable error) {
                self.requestPurchase(vc);
            }];
        }
    }
    else if ([nextTrans transactionState] == SKPaymentTransactionStatePurchased) {
        [self activate];
        assert(purchased);

        self.uponSuccess();
    }
    else if ([nextTrans transactionState] == SKPaymentTransactionStateRestored) {
        [self activate];
        assert(purchased);

        self.uponSuccess();
    }
    else {
    }

    if(vc != nil) {
        [vc dismissViewControllerAnimated:YES completion:^{

        }];
    }


}

-(void)activate {
    
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
    // store a new encryption key

    purchased = YES;
}

// MARK: -- delegate stuff

- (BOOL)paymentQueue:(nonnull SKPaymentQueue *)queue shouldAddStorePayment:(nonnull SKPayment *)payment forProduct:(nonnull SKProduct *)product {
//    if(!warned) {
//        // TODO: user must read eula
//        warned = YES;
//        return NO;
//    }
    return YES;
}

- (void)productsRequest:(nonnull SKProductsRequest *)request didReceiveResponse:(nonnull SKProductsResponse *)response {
    if ( [response.products.firstObject.productIdentifier compare: PRODUCT_UPGRADE1_ID] == NSOrderedSame ) {

        SKMutablePayment* pay = [SKMutablePayment paymentWithProduct:[[MyProduct alloc] init]];
//        pay.simulatesAskToBuyInSandbox = YES;

        [SKPaymentQueue.defaultQueue addPayment:pay];
    }
    else {
        assert(false);
    }
}

@end



@implementation PurchaseManager (SKPaymentTransactionObserver)

- (void) requestDidFinish:(SKRequest *)request {
    NSLog(@"PurchaseManager requestDidFinish ok");

}
@end

