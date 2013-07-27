//
//  ARUpdaterViewController.m
//  AR.Updater
//
//  Created by Robert Ryll on 10-05-14.
//  Copyright Playsoft 2010. All rights reserved.
//

#import "MenuUpdater.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "plf.h"

#define UPDATER_ASSERT(var, value)									\
if (var != value)													\
{																	\
	NSLog(@"oO ! "#var" is #%u (should be #%u)", (var), (value));	\
	return;															\
}

static NSArray *documentPath = nil;

static const NSString *stepKeys[] =
{
	@"Connecting",
	@"Checking/updating bootloader",
	@"Checking version",
	@"Sending file",
	@"Restarting AR.Drone",
	@"Installing",
};

enum
{
	STEP_IMAGE_EMPTY,
	STEP_IMAGE_PASS,
	STEP_IMAGE_FAIL,
	STEP_IMAGE_PROBLEM
};

#define LINE_H			 22
#define STATUS_LINE_NR	  2
#define INDICATOR_S		 10

#define IPAD_LINE_H			  50
#define IPAD_INDICATOR_S	  20
#define IPAD_OFFSET_X		  50

#define OFFSET_Y		(SCREEN_H - (STATUS_LINE_NR + NUMBER_OF_UPDATER_STEPS + 3) * LINE_H)
#define IMG_H			(OFFSET_Y - LINE_H)
#define STATUS_Y		(OFFSET_Y + LINE_H)
#define STEP_LINE_Y		(STATUS_Y + LINE_H * STATUS_LINE_NR + LINE_H)
#define INDICATOR_M		((LINE_H - INDICATOR_S) / 2)

#define IPAD_OFFSET_Y		((IPAD_SCREEN_H - (STATUS_LINE_NR + NUMBER_OF_UPDATER_STEPS) * IPAD_LINE_H) / 2)
#define IPAD_IMG_H			(IPAD_OFFSET_Y - IPAD_LINE_H)
#define IPAD_STATUS_Y		(IPAD_OFFSET_Y + IPAD_LINE_H)
#define IPAD_STEP_LINE_Y	(IPAD_STATUS_Y + IPAD_LINE_H * STATUS_LINE_NR + IPAD_LINE_H)
#define IPAD_INDICATOR_M	((IPAD_LINE_H - IPAD_INDICATOR_S) / 2)

#define MAX_RETRIES 5

#define VERSION_TXT @"version.txt"
#define REPAIR_VERSION_TXT @"repair_version.txt"

@implementation MenuUpdater
@synthesize firmwareVersion;
@synthesize firmwarePath;
@synthesize firmwareFileName;
@synthesize repairPath;
@synthesize repairFileName;
@synthesize repairVersion;
@synthesize bootldrPath;
@synthesize bootldrFileName;
@synthesize droneFirmwareVersion;
@synthesize localIP;

@synthesize ftp;
@synthesize repairFtp;
@synthesize fsm;

#pragma mark init
- (id) initWithController:(MenuController*)menuController
{
	NSArray *targetArray = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"UIDeviceFamily"];
	compiledForIPad = NO;
	self.localIP = nil;
	
	for (int i = 0; i < [targetArray count]; i++)
	{
		NSNumber* num = (NSNumber*)[targetArray objectAtIndex:i];
		int value;
		[num getValue:&value];
		if (2 == value)
		{
			compiledForIPad = YES;
			break;
		}
	}
	
	usingIPad = NO;
	if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
	{
		usingIPad = YES;
	}
	
	using2xInterface = NO;
	if ([UIScreen instancesRespondToSelector:@selector(scale)]) {
		CGFloat scale = [[UIScreen mainScreen] scale];
		if (scale > 1.0) {
			using2xInterface = YES;
		}
	}
	
	if (!documentPath)
		documentPath = [[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0] copy];
	
	if (usingIPad && compiledForIPad)
	{
		self = [super initWithNibName:@"MenuUpdater-iPad" bundle:nil];
	}
	else
	{
		self = [super initWithNibName:@"MenuUpdater" bundle:nil];
	}
	
	if (self)
	{
		controller = menuController;
        ftp = nil;
	}
	return self;
}

- (void) viewDidLoad
{
	NSString *plistPath = [[NSBundle mainBundle] pathForResource:@"Firmware" ofType:@"plist"];
	NSDictionary *plistDict = [NSDictionary dictionaryWithContentsOfFile:plistPath];
	self.firmwareFileName = [plistDict objectForKey:@"FirmwareFileName"];
	self.repairFileName = [plistDict objectForKey:@"RepairFileName"];
	self.bootldrFileName = [plistDict objectForKey:@"BootldrFileName"];
	self.repairVersion = [plistDict objectForKey:@"RepairVersion"];
	
	self.firmwarePath = [[NSBundle mainBundle] pathForResource:firmwareFileName ofType:@"plf"];
	self.repairPath = [[NSBundle mainBundle] pathForResource:repairFileName ofType:@"bin"];
	self.bootldrPath = [[NSBundle mainBundle] pathForResource:bootldrFileName ofType:@"bin"];
	plf_phdr plf_header;
	

	if(plf_get_header([self.firmwarePath cStringUsingEncoding:NSUTF8StringEncoding], &plf_header) != 0)
		memset(&plf_header, 0, sizeof(plf_phdr));
	
	self.firmwareVersion = [NSString stringWithFormat:@"%d.%d.%d", plf_header.p_ver, plf_header.p_edit, plf_header.p_ext];
	
#ifdef DEBUG
	firmwareVersionLabel.text = [NSString stringWithFormat:@"v%s_D", [[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"] cStringUsingEncoding:NSUTF8StringEncoding]];
#else
	firmwareVersionLabel.text = [NSString stringWithFormat:@"v%s", [[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"] cStringUsingEncoding:NSUTF8StringEncoding]];
#endif
	
	if (usingIPad && compiledForIPad)
	{
		for (NSInteger i = 0; i < NUMBER_OF_UPDATER_STEPS ; i++)
		{
			stepLabel[i] = [UILabel new];
			stepLabel[i].backgroundColor = [UIColor clearColor];
			stepLabel[i].textColor = NORMAL_COLOR;
			stepLabel[i].frame = CGRectMake(IPAD_LINE_H + IPAD_OFFSET_X, IPAD_STEP_LINE_Y + IPAD_LINE_H * i, IPAD_SCREEN_H - IPAD_LINE_H, IPAD_LINE_H);
			stepLabel[i].text = NSLocalizedString((NSString *)stepKeys[i],);
			[self.view addSubview:stepLabel[i]];
			
			stepImageView[i] = [UIImageView new];
			stepImageView[i].frame = CGRectMake(IPAD_OFFSET_X, IPAD_STEP_LINE_Y + i * IPAD_LINE_H, IPAD_LINE_H, IPAD_LINE_H);
			stepImageView[i].image = [UIImage imageNamed:@"Empty-iPad.png"];
			[self.view addSubview:stepImageView[i]];
			
			if(i > UPDATER_STEP_CHECK)
			{
				stepLabel[i].hidden = YES;
				stepImageView[i].hidden = YES;	
			}
		}
	}
	else
	{
		for (NSInteger i = 0; i < NUMBER_OF_UPDATER_STEPS ; i++)
		{
			stepLabel[i] = [UILabel new];
			stepLabel[i].backgroundColor = [UIColor clearColor];
			stepLabel[i].textColor = NORMAL_COLOR;
			stepLabel[i].frame = CGRectMake(LINE_H, STEP_LINE_Y + LINE_H * i, SCREEN_H - LINE_H, LINE_H);
			stepLabel[i].text = NSLocalizedString((NSString *)stepKeys[i],);
			[self.view addSubview:stepLabel[i]];
			
			stepImageView[i] = [UIImageView new];
			stepImageView[i].frame = CGRectMake(0, STEP_LINE_Y + i * LINE_H, LINE_H, LINE_H);
			stepImageView[i].image = [UIImage imageNamed:@"Empty.png"];
			[self.view addSubview:stepImageView[i]];
			
			if(i > UPDATER_STEP_CHECK)
			{
				stepLabel[i].hidden = YES;
				stepImageView[i].hidden = YES;	
			}
		}
	}
	
	[okButton setTitle:NSLocalizedString(@"Ok",) forState:UIControlStateNormal];
	[cancelButton setTitle:NSLocalizedString(@"Cancel",) forState:UIControlStateNormal];

	stepIndicator = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle: UIActivityIndicatorViewStyleWhite];
	stepIndicator.hidesWhenStopped = YES;
	[self.view addSubview:stepIndicator];
	[stepIndicator release];
	
	okButton.hidden = YES;
	cancelButton.hidden = YES;
	sendProgressView.hidden = YES;
	
	self.fsm = [FiniteStateMachine fsmWithXML:[[NSBundle mainBundle] pathForResource:@"updater_fsm" ofType:@"xml"]];
	fsm.delegate = self;
	fsm.currentState = UPDATER_STATE_WAITING_CONNECTION;
	
	[self performSelector:@selector(checkFtp) withObject:nil afterDelay:5.];
}

- (void)didReceiveMemoryWarning
{
	[super didReceiveMemoryWarning];
}

- (void)dealloc
{
	for (NSInteger i = 0 ; i < NUMBER_OF_UPDATER_STEPS ; i++) 
	{
		[stepLabel[i] release];
		[stepImageView[i] release];
	}
	
	self.firmwarePath = nil;
	self.firmwareFileName = nil;
	self.repairPath = nil;
	self.repairFileName = nil;
	self.repairVersion = nil;
	self.bootldrPath = nil;
	self.bootldrFileName = nil;
	self.firmwareVersion = nil;
	self.droneFirmwareVersion = nil;
	self.localIP = nil;
	
	self.ftp = nil;
	self.repairFtp = nil;
	self.fsm = nil;
	
	if (documentPath)
		[documentPath release];
	
	[super dealloc];
}

// NSURLConnection needed to ping the ARDrone before creating the FTP.
- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
	NSLog(@"%@", response);
	[connection release];
	
	self.ftp = [ARDroneFTP anonymousUpdateFTPwithDelegate:self withDefaultCallback:nil];
	
	if ([ftp keepConnexionAlive])
		[fsm doAction:UPDATER_ACTION_SUCCESS];
	else
		[fsm doAction:UPDATER_ACTION_FAIL];
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
	NSLog(@"%@", error);
	[connection release];
	
	[fsm doAction:UPDATER_ACTION_FAIL];
}

- (void)executeCommandOut:(ARDRONE_COMMAND_OUT)commandId withParameter:(void*)parameter fromSender:(id)sender
{
	switch(commandId)
	{
		case ARDRONE_COMMAND_RUN:
			self.localIP = [NSString stringWithCString:(char*)parameter encoding:NSUTF8StringEncoding];
			
			NSURLConnection *ping = [[NSURLConnection connectionWithRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:[NSString stringWithFormat:FTP_ADDRESS, localIP, FTP_PORT, @""]]] delegate:self] retain];
			[ping start];
			
			break;
			
		default:
			break;
	}
}

- (void)checkFtp
{
	if (fsm.currentState == UPDATER_STATE_WAITING_CONNECTION)
		[fsm doAction:UPDATER_ACTION_FAIL];
}

// Updates the display with the FSM object.
- (void) updateStatusLabel
{
	NSString *key = [(NSString *)fsm.currentObject stringByReplacingOccurrencesOfString:@"\\n" withString:@"\n"];
	
	switch (fsm.currentState)
	{
		case UPDATER_STATE_NOT_CONNECTED:
		case UPDATER_STATE_NOT_REPAIRED:
		case UPDATER_STATE_NOT_UPDATED:
			statusLabel.text = [NSString stringWithFormat:NSLocalizedString(key,), [[UIDevice currentDevice] model]];
			break;
		case UPDATER_STATE_CHECK_VERSION:
		{
			switch ([firmwareVersion compare:droneFirmwareVersion options:NSNumericSearch])
			{
				case NSOrderedAscending:
					statusLabel.text = [NSString stringWithFormat:NSLocalizedString(key,), droneFirmwareVersion, NSLocalizedString(@"Please update this application",), NSLocalizedString(@"Launching Application...",)];
					break;
				case NSOrderedSame:
					statusLabel.text = [NSString stringWithFormat:NSLocalizedString(key,), droneFirmwareVersion, NSLocalizedString(@"AR.Drone up to date",), NSLocalizedString(@"Launching Application...",)];
					break;
				default:
					statusLabel.text = [NSString stringWithFormat:NSLocalizedString(key,), droneFirmwareVersion, NSLocalizedString(@"Firmware update available",), NSLocalizedString(@"Do you want to update the AR.Drone ?",)];
					break;
			}
		}
			break;
		default:
			statusLabel.text = NSLocalizedString(key,);
			break;
	}
}

- (void) updateStepIndicator:(unsigned int)index
{
	stepIndicator.frame =	(usingIPad && compiledForIPad) ?
	/* iPad */				CGRectMake(IPAD_OFFSET_X + IPAD_INDICATOR_M, IPAD_STEP_LINE_Y + IPAD_INDICATOR_M + IPAD_LINE_H * index, IPAD_INDICATOR_S, IPAD_INDICATOR_S) :
	/* iPhone */			CGRectMake(INDICATOR_M, STEP_LINE_Y + INDICATOR_M + LINE_H * index, INDICATOR_S, INDICATOR_S);
}

- (void) updateStepImage:(unsigned int)index withImage:(unsigned int)image
{
	switch (image)
	{
		case STEP_IMAGE_EMPTY:
			stepImageView[index].image = [UIImage imageNamed:(usingIPad && compiledForIPad) ? @"Empty-iPad.png" : @"Empty.png"];
			break;
			
		case STEP_IMAGE_PASS:
			stepImageView[index].image = [UIImage imageNamed:(usingIPad && compiledForIPad) ? @"Pass-iPad.png" : @"Pass.png"];
			break;
			
		case STEP_IMAGE_FAIL:
			stepImageView[index].image = [UIImage imageNamed:(usingIPad && compiledForIPad) ? @"Fail-iPad.png" : @"Fail.png"];
			break;
			
		case STEP_IMAGE_PROBLEM:
			stepImageView[index].image = [UIImage imageNamed:(usingIPad && compiledForIPad) ? @"Problem-iPad.png" : @"Problem.png"];
			break;
			
		default:
			break;
	}
}

/*
 * Waiting Connection State:
 *
 *
 */

// Enter
- (void)enterWaitingConnection:(id)_fsm
{
	[self updateStatusLabel];
	
	[self updateStepIndicator:UPDATER_STEP_CONNECTING];
	[stepIndicator startAnimating];
}

// Quit
- (void)quitWaitingConnection:(id)_fsm
{
	[stepIndicator stopAnimating];
}

/*
 * Not Connected State:
 *
 *
 */

// Enter
- (void)enterNotConnected:(id)_fsm
{
	[self updateStatusLabel];
	
	[self updateStepImage:UPDATER_STEP_CONNECTING withImage:STEP_IMAGE_FAIL];
}

/*
 * Repair State:
 *
 *
 */
- (BOOL) repair
{
	int socket_desc;
	struct sockaddr_in address;
	
	socket_desc=socket(AF_INET,SOCK_STREAM,0);
	if (socket_desc>-1)
	{
		char buffer[1024];
		int size;
		printf("Socket created (ip: %s)\n", [localIP cStringUsingEncoding:NSUTF8StringEncoding]);
		
		/* type of socket created in socket() */
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = inet_addr([localIP cStringUsingEncoding:NSUTF8StringEncoding]);
		
		/* TELNET_PORT is the port to use for connections */
		address.sin_port = htons(TELNET_PORT);
		/* connect the socket to the port specified above */
		connect(socket_desc, (struct sockaddr *)&address, sizeof(struct sockaddr));
		
		// Change access right to XR 
		sprintf(buffer, "cd `find /data -name \"repair\" -exec dirname {} \\;` && chmod 755 repair\n");
		size = send(socket_desc, buffer, strlen(buffer), 0);
		if(size != strlen(buffer))
		{
			printf("Cannot change access right ...\n");
			return NO;
		}
		
		// execute repair binary file 
		sprintf(buffer, "cd `find /data -name \"repair\" -exec dirname {} \\;` && ./repair\n");
		size = send(socket_desc, buffer, strlen(buffer), 0);
		if(size != strlen(buffer))
		{
			printf("Cannot execute binary to repair AR.Drone ...\n");
			return NO;
		}
		
		close(socket_desc);
	}
	else
	{
		perror("Can't create socket");
		return NO;
	}
	
	return YES;
}

// Called back when repair is needed:
- (void)repairFileCallback:(ARDroneFTPCallbackArg *)arg
{
	static unsigned int count = 0;
	
	UPDATER_ASSERT(fsm.currentState, UPDATER_STATE_REPAIR)
	UPDATER_ASSERT(arg.operation, FTP_SEND)
	
	if (ARDFTP_FAILED(arg.status))
	{
		count++;
		
		if (MAX_RETRIES == count)
		{
			[fsm doAction:UPDATER_ACTION_FAIL];
			return;
		}
		
		[repairFtp sendLocalFile:repairPath toDistantFile:repairFileName withResume:YES withCallback:@selector(repairFileCallback:)];
		
		return;
	}
	
	if (!ARDFTP_SUCCEEDED(arg.status))
		return;
	
	NSLog(@"'repair' sent");
	self.repairFtp = nil;
	
	if ([self repair])
		[fsm doAction:UPDATER_ACTION_SUCCESS];
	else
		[fsm doAction:UPDATER_ACTION_FAIL];
}

// Called back when bootldr is needed:
- (void)bootldrFileCallback:(ARDroneFTPCallbackArg *)arg
{
	static unsigned int count = 0;
	
	UPDATER_ASSERT(fsm.currentState, UPDATER_STATE_REPAIR)
	UPDATER_ASSERT(arg.operation, FTP_SEND)
	
	if (ARDFTP_FAILED(arg.status))
	{
		count++;
		
		if (MAX_RETRIES == count)
		{
			[fsm doAction:UPDATER_ACTION_FAIL];
			return;
		}
		
		[repairFtp sendLocalFile:bootldrPath toDistantFile:[NSString stringWithFormat:@"%@.bin", bootldrFilename] withResume:YES withCallback:@selector(bootldrFileCallback:)];
		
		return;
	}
	
	if (!ARDFTP_SUCCEEDED(arg.status))
		return;
	
	NSLog(@"'bootldr.bin' sent");
	[repairFtp sendLocalFile:repairPath toDistantFile:repairFileName withResume:NO withCallback:@selector(repairFileCallback:)];
}

// Checks drone version:
- (void)checkRepairVersion
{
	switch ([droneFirmwareVersion compare:repairVersion options:NSNumericSearch])
	{
		case NSOrderedAscending:
			self.repairFtp = [ARDroneFTP anonymousStandardFTPwithDelegate:self withDefaultCallback:nil];
			
			[repairFtp sendLocalFile:bootldrPath toDistantFile:[NSString stringWithFormat:@"%@.bin", bootldrFilename] withResume:NO withCallback:@selector(bootldrFileCallback:)];
			
			break;
			
		default:
			[fsm doAction:UPDATER_ACTION_SUCCESS];
			break;
	}
}

// Called back when drone version is found:
- (void)repairVersionTxtCallback:(ARDroneFTPCallbackArg *)arg
{
	static unsigned int count = 0;
	
	UPDATER_ASSERT(fsm.currentState, UPDATER_STATE_REPAIR)
	UPDATER_ASSERT(arg.operation, FTP_GET)
	
	NSString *path = [NSString stringWithFormat:@"%@/%@", documentPath, REPAIR_VERSION_TXT];
	
	if (ARDFTP_FAILED(arg.status))
	{
		count++;
		
		if (MAX_RETRIES == count)
		{
			[fsm doAction:UPDATER_ACTION_FAIL];
			return;
		}
		
		[ftp getDistantFile:VERSION_TXT toLocalFile:path withResume:YES withCallback:@selector(repairVersionTxtCallback:)];
		return;
	}
		
	if (!ARDFTP_SUCCEEDED(arg.status))
		return;
	
	NSError *error = nil;
	self.droneFirmwareVersion = [[NSString stringWithContentsOfFile:path encoding:NSASCIIStringEncoding error:&error] stringByReplacingOccurrencesOfString:@"\n" withString:@""];
	
	[self performSelectorOnMainThread:@selector(checkRepairVersion) withObject:nil waitUntilDone:NO];
}

// Enter
- (void)enterRepair:(id)_fsm
{
	[self updateStatusLabel];
	
	[self updateStepIndicator:UPDATER_STEP_REPAIR];
	[stepIndicator startAnimating];
	
	[self updateStepImage:UPDATER_STEP_CONNECTING withImage:STEP_IMAGE_PASS];
	
	NSString *path = [NSString stringWithFormat:@"%@/%@", documentPath, REPAIR_VERSION_TXT];
	
	NSError *error = nil;
	
	if ([[NSFileManager defaultManager] fileExistsAtPath:path])
		[[NSFileManager defaultManager] removeItemAtPath:path error:&error];
	
	[ftp getDistantFile:VERSION_TXT toLocalFile:path withResume:NO withCallback:@selector(repairVersionTxtCallback:)];
}

// Quit
- (void)quitRepair:(id)_fsm
{
	[stepIndicator stopAnimating];
}

/*
 * Repair State:
 *
 *
 */
// Enter
- (void)enterNotRepaired:(id)_fsm
{
	[self updateStatusLabel];
	
	[self updateStepImage:UPDATER_STEP_REPAIR withImage:STEP_IMAGE_FAIL];
}

/*
 * Check Version State:
 *
 *
 */
// Enter
- (void)enterCheckVersion:(id)_fsm
{
	[self updateStatusLabel];
	
	[self updateStepIndicator:UPDATER_STEP_CHECK];
	[stepIndicator startAnimating];
	
	[self updateStepImage:UPDATER_STEP_REPAIR withImage:STEP_IMAGE_PASS];
	
	switch ([firmwareVersion compare:droneFirmwareVersion options:NSNumericSearch])
	{
		case NSOrderedAscending:
			[fsm doAction:UPDATER_ACTION_ASK_FOR_FREEFLIGHT_UPDATE];
			break;
		case NSOrderedSame:
			[fsm doAction:UPDATER_ACTION_SUCCESS];
			break;
		default:
			okButton.hidden = NO;
			cancelButton.hidden = NO;
			break;
	}
}

// Quit
- (void)quitCheckVersion:(id)_fsm
{
	[stepIndicator stopAnimating];
	
	okButton.hidden = YES;
	cancelButton.hidden = YES;
}

/*
 * Update Freeflight State:
 *
 *
 */
// Enter
- (void)enterUpdateFreeflight:(id)_fsm
{
	[self updateStepImage:UPDATER_STEP_CHECK withImage:STEP_IMAGE_PROBLEM];
	[self performSelector:@selector(cancelAction:) withObject:cancelButton afterDelay:TIME_BEFORE_LAUNCH];
}

/*
 * Launch Freeflight State:
 *
 *
 */
// Enter
- (void)enterLaunchFreeflight:(id)_fsm
{
	for (unsigned int i = 0 ; i < NUMBER_OF_UPDATER_STEPS ; i++)
		[self updateStepImage:i withImage:STEP_IMAGE_PASS];
	
	[self performSelector:@selector(cancelAction:) withObject:cancelButton afterDelay:TIME_BEFORE_LAUNCH];
}

/*
 * Update Firmware State:
 *
 *
 */
// Send Progress View can only be updated on Main Thread
- (void)updateSendProgressView:(NSNumber *)progress
{
	sendProgressView.progress = [progress floatValue];
}

// Called back when file sending is in progress.
- (void)sendFileCallback:(ARDroneFTPCallbackArg *)arg
{
	static unsigned int count = 0;
	
	UPDATER_ASSERT(fsm.currentState, UPDATER_STATE_UPDATE_FIRMWARE)
	UPDATER_ASSERT(arg.operation, FTP_SEND)
	
	if (ARDFTP_FAILED(arg.status))
	{
		count++;
		
		if (MAX_RETRIES == count)
		{
			[fsm doAction:UPDATER_ACTION_FAIL];
			return;
		}
		
		[ftp sendLocalFile:firmwarePath toDistantFile:[NSString stringWithFormat:@"%@.plf", firmwareFileName] withResume:YES withCallback:@selector(sendFileCallback:)];
		return;
	}
	
	[self performSelectorOnMainThread:@selector(updateSendProgressView:) withObject:[NSNumber numberWithFloat:arg.progress/100.f] waitUntilDone:NO];
	
	if (!ARDFTP_SUCCEEDED(arg.status))
		return;
	
	NSLog(@"'firmware.plf' sent");
	[fsm doAction:UPDATER_ACTION_SUCCESS];
}

// Enter
- (void)enterUpdateFirmware:(id)_fsm
{
	[self updateStatusLabel];
	
	[self updateStepImage:UPDATER_STEP_CHECK withImage:STEP_IMAGE_PASS];
	
	for (unsigned int i = 0 ; i < NUMBER_OF_UPDATER_STEPS ; i++)
	{
		stepLabel[i].hidden = NO;
		stepImageView[i].hidden = NO;
	}
	
	[self updateStepIndicator:UPDATER_STEP_SEND];
	[stepIndicator startAnimating];
	
	sendProgressView.hidden = NO;
	sendProgressView.progress = 0.f;
	
	[ftp sendLocalFile:firmwarePath toDistantFile:[NSString stringWithFormat:@"%@.plf", firmwareFileName] withResume:NO withCallback:@selector(sendFileCallback:)];
}

// Quit
- (void)quitUpdateFirmware:(id)_fsm
{
	[stepIndicator stopAnimating];
	
	sendProgressView.hidden = YES;
}

/*
 * Not Updated State:
 *
 *
 */
// Enter
- (void)enterNotUpdated:(id)_fsm
{
	[self updateStatusLabel];
	
	[self updateStepImage:UPDATER_STEP_SEND withImage:STEP_IMAGE_FAIL];
}

/*
 * Restart Drone State:
 *
 *
 */
// Checks if the drone is powered off
- (void)checkDroneRestarted
{
	UPDATER_ASSERT(fsm.currentState, UPDATER_STATE_RESTART_DRONE)
	
	if ([ftp keepConnexionAlive])
		[self performSelector:@selector(checkDroneRestarted) withObject:nil afterDelay:1];
	else
		[fsm doAction:UPDATER_ACTION_SUCCESS];
}

// Enter
- (void)enterRestartDrone:(id)_fsm
{
	[self updateStatusLabel];
	
	[self updateStepImage:UPDATER_STEP_SEND withImage:STEP_IMAGE_PASS];
	
	[self updateStepIndicator:UPDATER_STEP_RESTART];
	[stepIndicator startAnimating];
	
	[self performSelectorOnMainThread:@selector(checkDroneRestarted) withObject:nil waitUntilDone:NO];
}

// Quit
- (void)quitRestartDrone:(id)_fsm
{
	[stepIndicator stopAnimating];
}

/*
 * Installing Firmware State:
 *
 *
 */
// Enter
- (void)enterInstallingFirmware:(id)_fsm
{
	[self updateStatusLabel];
	
	[self updateStepImage:UPDATER_STEP_RESTART withImage:STEP_IMAGE_PASS];
	
	[self updateStepIndicator:UPDATER_STEP_INSTALL];
	[stepIndicator startAnimating];
}

// IBActions:
- (IBAction)okAction:(id)sender 
{
	[fsm doAction:UPDATER_ACTION_FAIL];
}

- (IBAction)cancelAction:(id)sender 
{	
	[controller doAction:MENU_ACTION_NEXT];
}

@end
