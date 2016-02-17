//
//  UQRAppDelegate.mm
//  uQuadRemote
//
//  Created by Zoran Angelov on 11/2/15.
//  Copyright © 2015 uQuad. All rights reserved.
//

#import "UQRAppDelegate.h"
#include "UQRClient.h"

@interface UQRAppDelegate ()
{
    UQRClient *m_Client;
    UIBackgroundTaskIdentifier m_ExtendBGTask;
}
@end

@implementation UQRAppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    m_Client = new UQRClient();
    m_Client->start();
    m_ExtendBGTask = UIBackgroundTaskInvalid;
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
    m_ExtendBGTask = [application beginBackgroundTaskWithExpirationHandler:^
    {
        m_Client->stop();
        [application endBackgroundTask:m_ExtendBGTask];
        m_ExtendBGTask = UIBackgroundTaskInvalid;
    }];
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
    if(m_ExtendBGTask != UIBackgroundTaskInvalid)
    {
        [application endBackgroundTask:m_ExtendBGTask];
        m_ExtendBGTask = UIBackgroundTaskInvalid;
    }
    
    if(!m_Client->isStarted())
    {
        m_Client->start();
    }
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
    delete m_Client;
    m_Client = nullptr;
}

@end
