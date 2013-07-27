//
//  FiniteStateMachine.h
//  ArDroneGameLib
//
//  Created by clement choquereau on 5/4/11.
//  Copyright 2011 Parrot. All rights reserved.
//

#import <Foundation/Foundation.h>

#define NO_STATE (-1)

@class FSMXMLParsing;

@interface FiniteStateMachine : NSObject <NSXMLParserDelegate>
{
    id delegate;
    
    unsigned int statesCount;
    unsigned int actionsCount;
    unsigned int currentState;
    
    NSMutableDictionary *objects;
    NSMutableDictionary *enterCallbacks;
    NSMutableDictionary *quitCallbacks;
    NSMutableDictionary *stateActions;
	
	// should always be nil:
	FSMXMLParsing *xml;
}

@property (nonatomic, retain) id delegate;
@property (readonly) id currentObject;

@property (readonly) unsigned int statesCount;
@property (readonly) unsigned int actionsCount;
@property unsigned int currentState;

/*
 * You can create a FSM with a XML file:
 * (For element 'state', attributes 'enter-callback', 'quit-callback' and 'object' aren't mandatory.)
 *
 * <?xml version="1.0"?>
 * <fsm>
 *     <states>
 *         <state name="myState" enter-callback="onEnterState:" quit-callback="onQuitState:" object="aStringObject" />
 *         ...
 *     </states>
 *     <actions>
 *         <action name="myAction" />
 *         ...
 *     </actions>
 *     <associations>
 *         <association from-state="aState" action="anAction" to-state="anotherState" />
 *     </associations>
 * </fsm>
 */
+ (id) fsm;
+ (id) fsmWithXML:(NSString *)fileName;

// Managing states:
- (unsigned int) createState;
- (void) createStates:(unsigned int)count inArray:(unsigned int *)states;
- (void) setObject:(id)object forState:(unsigned int)state;
- (void) setEnterStateCallback:(SEL)enter forState:(unsigned int)state;
- (void) setQuitStateCallback:(SEL)quit forState:(unsigned int)state;

- (unsigned int) createStateWithObject:(id)object andEnterStateCallback:(SEL)enter andQuitStateCallback:(SEL)quit;
- (void) createStates:(unsigned int)count withObjects:(id *)items andEnterStateCallbacks:(SEL *)enters andQuitStateCallbacks:(SEL *)quits inArray:(unsigned int *)states;

// Managing actions:
- (unsigned int) createAction;
- (void) createActions:(unsigned int)count inArray:(unsigned int *)actions;

// Associations:
- (void) createAssociationFromState:(unsigned int)start withAction:(unsigned int)action toState:(unsigned int)end;
- (void) createAssociationFromState:(unsigned int)start withActions:(unsigned int *)actions toStates:(unsigned int *)ends andNumAssociations:(unsigned int)count;
- (void) doAction:(unsigned int)action;

@end
