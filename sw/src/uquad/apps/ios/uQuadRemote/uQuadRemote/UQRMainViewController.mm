//
//  UQRMainViewController.m
//  uQuadRemote
//
//  Created by Zoran Angelov on 11/2/15.
//  Copyright © 2015 uQuad. All rights reserved.
//

#import "UQRMainViewController.h"
#import "UQRViewControllerDelegate.h"
#import "UQRRemoteViewController.h"

@interface UQRMainViewController () <UQRViewControllerDelegate>

- (void)dismissViewController:(UIViewController*)controller animated:(BOOL)animated completion:(void (^)(void))completion;

- (IBAction)showRemoteViewController:(id)sender;

@end

@implementation UQRMainViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    // Uncomment the following line to preserve selection between presentations
    // self.clearsSelectionOnViewWillAppear = NO;
    
    // Register cell classes
    //[self.collectionView registerClass:[UQRMainViewControllerCell class] forCellWithReuseIdentifier:reuseIdentifier];
    
    // Do any additional setup after loading the view.
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)dismissViewController:(UIViewController*)controller animated:(BOOL)animated completion:(void (^)(void))completion
{
    if(controller == self.presentedViewController)
    {
        [controller dismissViewControllerAnimated:animated completion:completion];
    }
}

- (IBAction)showRemoteViewController:(id)sender
{
    UIStoryboard *storyboard = [UIStoryboard storyboardWithName:@"Main" bundle:nil];
    UQRRemoteViewController *remoteVC = (UQRRemoteViewController*)[storyboard instantiateViewControllerWithIdentifier:@"Remote"];
    remoteVC.viewControllerDelegate = self;

    [self presentViewController:remoteVC animated:YES completion:nil];
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/



@end
