//
//  UQRViewControllerDelegate.h
//  uQuadRemote
//
//  Created by Zoran Angelov on 11/2/15.
//  Copyright © 2015 uQuad. All rights reserved.
//

#import <UIKit/UIKit.h>

@protocol UQRViewControllerDelegate <NSObject>

- (void)dismissViewController:(UIViewController*)controller animated:(BOOL)animated completion:(void (^)(void))completion;

@end