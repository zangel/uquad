//
//  UQRRemoteViewController.h
//  uQuadRemote
//
//  Created by Zoran Angelov on 11/3/15.
//  Copyright © 2015 uQuad. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "UQRViewControllerDelegate.h"

@interface UQRRemoteViewController : UIViewController

@property (strong) id<UQRViewControllerDelegate> viewControllerDelegate;

@end
