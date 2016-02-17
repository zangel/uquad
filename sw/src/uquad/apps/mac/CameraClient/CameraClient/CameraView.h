//
//  CameraView.h
//  CameraClient
//
//  Created by Zoran Angelov on 10/29/15.
//  Copyright © 2015 uQuad. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <AVFoundation/AVFoundation.h>

@interface CameraView : NSView

@property (nonatomic, strong) AVSampleBufferDisplayLayer *videoLayer;

- (id)initWithCoder:(NSCoder *)coder;

@end

@interface CameraViewController : NSObject
{
@public
    IBOutlet CameraView *cameraView;
    IBOutlet NSButton *connectButton;
}

- (IBAction) connectOrDisconnect:(id) sender;

@end

