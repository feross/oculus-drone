//
//  MenuController.m
//  ArDroneGameLib
//
//  Created by clement choquereau on 5/4/11.
//  Copyright 2011 Parrot. All rights reserved.
//

#import "MenuController.h"

#import "ARDrone.h"
#import "FiniteStateMachine.h"

@interface MenuController()

- (void) enterState:(id)state;
- (void) quitState:(id)state;

@end

@implementation MenuController

@synthesize fsm;
@synthesize drone;
@synthesize delegate;

- (void)viewDidLoad
{
	[super viewDidLoad];
    
	currentMenu = nil;
	
	self.fsm = [FiniteStateMachine fsmWithXML:[[NSBundle mainBundle] pathForResource:@"menus_fsm" ofType:@"xml"]];
	
	fsm.currentState = MENU_STATE_UPDATER;
}

- (void) dealloc
{
	// Release the current menu
	if (currentMenu)
	{
		[currentMenu.view removeFromSuperview];
		[currentMenu release];
		currentMenu = nil;
	}
	
    [fsm release];
    fsm = nil;
	
	[super dealloc];
}

/*
 * Finite State Machine setter
 *
 * 
 */
- (void)setFsm:(FiniteStateMachine *)_fsm
{
    [_fsm retain];
    [fsm release];
    fsm = _fsm;
    
    [fsm setDelegate:self];
    
    unsigned int n = fsm.statesCount;
    
    for (unsigned int state = 0 ; state < n ; state++)
    {
        [fsm setEnterStateCallback:@selector(enterState:) forState:state];
        [fsm setQuitStateCallback:@selector(quitState:) forState:state];
    }
}

/*
 * enterState
 *
 * 
 */
- (void) enterState:(id)_fsm
{
	if (![fsm currentObject])
		return;
	
	NSString *object = [fsm currentObject];
	
	if ([object compare:FSM_HUD] == NSOrderedSame)
	{
		[delegate changeState:YES];
		[self changeState:YES];
		
		return;
	}
	
    Class menuClass = NSClassFromString(object);
    
    currentMenu = [(UIViewController<MenuProtocol> *)[menuClass alloc] initWithController:self];
	if (![delegate checkState])
		[self.view addSubview:currentMenu.view];
}

/*
 * quitState
 *
 * 
 */
- (void) quitState:(id)_fsm
{
	NSString *object = nil;
	
	if (fsm)
		object = [fsm currentObject];
	
	if ( (object) && ([object compare:FSM_HUD] == NSOrderedSame) )
	{
		[self changeState:NO];
		[delegate changeState:NO];
		
		return;
	}
	
    [currentMenu.view removeFromSuperview];
    [currentMenu release];
	currentMenu = nil;
}

/*
 * doAction
 *
 * 
 */
- (void)doAction:(unsigned int)action
{
    if (!fsm)
        return;
    
    [fsm doAction:action];
}

/*
 * currentState
 *
 *
 */
- (unsigned int)currentState
{
	if (!fsm)
		return NO_STATE;
	
	return [fsm currentState];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation 
{	
	if([delegate checkState] == NO)
		return UIInterfaceOrientationIsLandscape(interfaceOrientation);
	else
		return FALSE;
}

// ARDrone protocols:
// ProtocolIn
- (void)changeState:(BOOL)inGame
{
	if(!inGame)
	{
        // If not in game, the MenuController removes the drone view, and adds a menu view.
		if (drone)
			[drone.view removeFromSuperview];
		
		if (currentMenu)
			[self.view addSubview:currentMenu.view];
	}
	else
	{
        // If in game, the MenuController removes the active menu view, and adds the drone view.
		if (currentMenu)
			[currentMenu.view removeFromSuperview];
		
		if (drone)
			[self.view addSubview:drone.view];
	}
}

- (void) executeCommandIn:(ARDRONE_COMMAND_IN_WITH_PARAM)commandIn fromSender:(id)sender refreshSettings:(BOOL)refresh
{
}

- (void)executeCommandIn:(ARDRONE_COMMAND_IN)commandId withParameter:(void *)parameter fromSender:(id)sender
{
}

- (BOOL)checkState
{
	return YES;
}

- (void)setDefaultConfigurationForKey:(ARDRONE_CONFIG_KEYS)key withValue:(void *)value
{
    
}

// ProtocolOut
- (void)executeCommandOut:(ARDRONE_COMMAND_OUT)commandId withParameter:(void *)parameter fromSender:(id)sender
{
	if (currentMenu)
		[currentMenu executeCommandOut:commandId withParameter:parameter fromSender:sender];
}

@end
