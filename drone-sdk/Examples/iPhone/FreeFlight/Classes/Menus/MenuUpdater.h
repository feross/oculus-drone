//
//  MenuUpdater.h
//  Updater
//
//  Created by Robert Ryll on 10-05-14.
//  Copyright Playsoft 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MenuController.h"

#import "ARDroneFTP.h"
#import "FiniteStateMachine.h"

enum
{
	UPDATER_STATE_WAITING_CONNECTION,
	UPDATER_STATE_NOT_CONNECTED,
	UPDATER_STATE_REPAIR,
	UPDATER_STATE_NOT_REPAIRED,
	UPDATER_STATE_CHECK_VERSION,
	UPDATER_STATE_UPDATE_FREEFLIGHT,
	UPDATER_STATE_LAUNCH_FREEFLIGHT,
	UPDATER_STATE_UPDATE_FIRMWARE,
	UPDATER_STATE_NOT_UPDATED,
	UPDATER_STATE_RESTART_DRONE,
	UPDATER_STATE_INSTALLING_FIRMWARE,
	NUMBER_OF_UPDATER_STATES
};

enum
{
	UPDATER_ACTION_FAIL,
	UPDATER_ACTION_SUCCESS,
	UPDATER_ACTION_ASK_FOR_FREEFLIGHT_UPDATE,
	NUMBER_OF_UPDATER_ACTIONS
};

enum
{
	UPDATER_STEP_CONNECTING,
	UPDATER_STEP_REPAIR,
	UPDATER_STEP_CHECK,
	UPDATER_STEP_SEND,
	UPDATER_STEP_RESTART,
	UPDATER_STEP_INSTALL,
	NUMBER_OF_UPDATER_STEPS
};

@interface MenuUpdater : UIViewController <MenuProtocol, ARDroneProtocolOut>
{
	MenuController   *controller;
	
	BOOL compiledForIPad;
	BOOL usingIPad;
	BOOL using2xInterface;
	
	IBOutlet UILabel *firmwareVersionLabel;
	IBOutlet UILabel *statusLabel;
	
	IBOutlet UIButton	*okButton;
	IBOutlet UIButton	*cancelButton;
	
	IBOutlet UIProgressView *sendProgressView;

	UILabel *stepLabel[NUMBER_OF_UPDATER_STEPS];
	UIImageView *stepImageView[NUMBER_OF_UPDATER_STEPS];
	
	UIActivityIndicatorView *stepIndicator;
	
	NSString *firmwarePath;
	NSString *firmwareFileName;
	NSString *firmwareVersion;
	NSString *repairPath;
	NSString *repairFilename;
	NSString *repairVersion;
	NSString *bootldrPath;
	NSString *bootldrFilename;
	
	NSString *droneFirmwareVersion;
	NSString *localIP;
    
    ARDroneFTP *ftp;
    ARDroneFTP *repairFtp;
	
	FiniteStateMachine *fsm;
}

@property (nonatomic, copy) NSString *firmwarePath;
@property (nonatomic, copy) NSString *firmwareFileName;
@property (nonatomic, copy) NSString *repairPath;
@property (nonatomic, copy) NSString *repairFileName;
@property (nonatomic, copy) NSString *repairVersion;
@property (nonatomic, copy) NSString *bootldrPath;
@property (nonatomic, copy) NSString *bootldrFileName;
@property (nonatomic, copy) NSString *firmwareVersion;
@property (nonatomic, copy) NSString *droneFirmwareVersion;
@property (nonatomic, copy) NSString *localIP;

@property (nonatomic, retain) ARDroneFTP *ftp;
@property (nonatomic, retain) ARDroneFTP *repairFtp;
@property (nonatomic, retain) FiniteStateMachine *fsm;

- (void)checkFtp;

// Waiting Connection:
- (void)enterWaitingConnection:(id)_fsm;
- (void)quitWaitingConnection:(id)_fsm;

// Not Connected:
- (void)enterNotConnected:(id)_fsm;

// Repair:
- (void)enterRepair:(id)_fsm;
- (void)quitRepair:(id)_fsm;

// Not Repaired:
- (void)enterNotRepaired:(id)_fsm;

// Check Version:
- (void)enterCheckVersion:(id)_fsm;
- (void)quitCheckVersion:(id)_fsm;

// Update Freeflight:
- (void)enterUpdateFreeflight:(id)_fsm;

// Launch Freeflight:
- (void)enterLaunchFreeflight:(id)_fsm;

// Update Firmware:
- (void)enterUpdateFirmware:(id)_fsm;
- (void)quitUpdateFirmware:(id)_fsm;

// Not Updated:
- (void)enterNotUpdated:(id)_fsm;

// Restart Drone:
- (void)enterRestartDrone:(id)_fsm;
- (void)quitRestartDrone:(id)_fsm;

// Installing Firmware:
- (void)enterInstallingFirmware:(id)_fsm;

// IBActions:
- (IBAction)okAction:(id)sender;
- (IBAction)cancelAction:(id)sender;

@end
