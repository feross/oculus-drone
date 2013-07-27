//
//  MenuController.h
//  ArDroneGameLib
//
//  Created by clement choquereau on 5/4/11.
//  Copyright 2011 Parrot. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "ARDroneProtocols.h"


// States & Actions:

#define FSM_HUD @"hud"

enum
{
	MENU_STATE_UPDATER,
	MENU_STATE_HUD,
	NUMBER_OF_MENU_STATES
};

enum
{
	MENU_ACTION_NEXT,
	NUMBER_OF_MENU_ACTIONS
};

@class MenuController;

@protocol MenuProtocol <ARDroneProtocolOut>

- (id)initWithController:(MenuController *)menuController;

@end

@class FiniteStateMachine;
@class ARDrone;

@interface MenuController : UIViewController <ARDroneProtocolIn, ARDroneProtocolOut>
{
	UIViewController<MenuProtocol>  *currentMenu;
	
	ARDrone							*drone;
	id<ARDroneProtocolIn>			delegate;
    
    FiniteStateMachine              *fsm;
}

/*
 * Set a FSM with UIViewController<MenuProtocol> Class String (see NSStringFromClass) as object attached to each state.
 * Entering a state will allocate and initialize this Class.
 * Quitting a state will release the previously allocated object.
 *
 * You can then use [MenuController doAction:] to communicate with the FSM.
 *
 * /!\ The MenuController will set the FSM delegate to itself.
 */
@property (nonatomic, retain) FiniteStateMachine *fsm;

- (void)doAction:(unsigned int)action;

@property (readonly) unsigned int currentState;

@property (nonatomic, assign) ARDrone *drone;
@property (nonatomic, assign) id<ARDroneProtocolIn> delegate;

@end
