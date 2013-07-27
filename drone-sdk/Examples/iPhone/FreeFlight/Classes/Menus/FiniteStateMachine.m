//
//  FiniteStateMachine.m
//  ArDroneGameLib
//
//  Created by clement choquereau on 5/4/11.
//  Copyright 2011 Parrot. All rights reserved.
//

#import "FiniteStateMachine.h"

// FSMXMLParsing needed to know the state of the XML Parser
#define FSM_XML_ROOT					@"fsm"
#define FSM_XML_STATES					@"states"
#define FSM_XML_STATE					@"state"
#define FSM_XML_STATE_NAME				@"name"
#define FSM_XML_STATE_ENTER_CALLBACK	@"enter-callback"
#define FSM_XML_STATE_QUIT_CALLBACK		@"quit-callback"
#define FSM_XML_STATE_OBJECT			@"object"
#define FSM_XML_ACTIONS					@"actions"
#define FSM_XML_ACTION					@"action"
#define FSM_XML_ACTION_NAME				@"name"
#define FSM_XML_ASSOCIATIONS			@"associations"
#define FSM_XML_ASSOCIATION				@"association"
#define FSM_XML_ASSOCIATION_FROM_STATE	@"from-state"
#define FSM_XML_ASSOCIATION_ACTION		@"action"
#define FSM_XML_ASSOCIATION_TO_STATE	@"to-state"

typedef enum
{
	FSM_XML_PARSING_ROOT,
	FSM_XML_PARSING_STATES,
	FSM_XML_PARSING_ACTIONS,
	FSM_XML_PARSING_ASSOCIATIONS,
	FSM_XML_PARSING_ASSOCIATION,
	FSM_XML_PARSING_ERROR
} FSM_XML_PARSING;

@interface FSMXMLParsing : NSObject
{
@public
	FSM_XML_PARSING state;
	
	NSMutableDictionary *states;
	NSMutableDictionary *actions;
}

@end

@implementation FSMXMLParsing

- (id)init
{
	if (self = [super init])
	{
		state = FSM_XML_PARSING_ROOT;
		
		states = [[NSMutableDictionary alloc] init];
		actions = [[NSMutableDictionary alloc] init];
	}
	
	return self;
}

- (void)dealloc
{
	[states removeAllObjects];
	[states release];
	
	[actions removeAllObjects];
	[actions release];
	
	[super dealloc];
}

@end

// FSM implementation:
@implementation FiniteStateMachine

@synthesize delegate;
@synthesize statesCount;
@synthesize actionsCount;

+ (id) fsm
{
    FiniteStateMachine *fsm = [[FiniteStateMachine alloc] init];
    
    return [fsm autorelease];
}

+ (id) fsmWithXML:(NSString *)fileName
{
    FiniteStateMachine *fsm = [[FiniteStateMachine alloc] init];
	
	fsm->xml = [[FSMXMLParsing alloc] init];
	
	NSData *data = [NSData dataWithContentsOfFile:fileName];
	NSXMLParser *xml = [[NSXMLParser alloc] initWithData:data];

	[xml setDelegate:fsm];
	if (![xml parse])
		NSLog(@"XML error: %@ (file: %@)", [xml parserError], fileName);
	else
		NSLog(@"XML parsing OK!");
	
	[xml release];
	
	[fsm->xml release];
	fsm->xml = nil;
    
    return [fsm autorelease];
}

- (id) init
{
    self = [super init];
    
    if (self)
    {
        statesCount = 0;
        actionsCount = 0;
        
        currentState = NO_STATE;
        
        delegate = nil;

        objects = [[NSMutableDictionary alloc] init];
        enterCallbacks = [[NSMutableDictionary alloc] init];
        quitCallbacks = [[NSMutableDictionary alloc] init];
        stateActions = [[NSMutableDictionary alloc] init];
		
		xml = nil;
    }
    
    return self;
}

- (void) dealloc
{
    [delegate release];
    delegate = nil;
    
    [objects removeAllObjects];
    [objects release];
    objects = nil;
    
    [enterCallbacks removeAllObjects];
    [enterCallbacks release];
    enterCallbacks = nil;
    
    [quitCallbacks removeAllObjects];
    [quitCallbacks release];
    quitCallbacks = nil;
    
    [stateActions removeAllObjects];
    [stateActions release];
    stateActions = nil;
    
    [super dealloc];
}

// Current State getter
- (unsigned int)currentState
{
    return currentState;
}

// Current State setter
- (void) setCurrentState:(unsigned int)state
{
    if (state >= statesCount)
        return;
    
    NSString *key = [NSString stringWithFormat:@"%u", currentState];
    
    if ( (delegate) && (currentState != NO_STATE) && ([[quitCallbacks allKeys] containsObject:key]) )
    {
        SEL callback = NSSelectorFromString((NSString *)[quitCallbacks objectForKey:key]);
        [delegate performSelector:callback withObject:self];
    }
    
    currentState = state;
    
    key = [NSString stringWithFormat:@"%u", currentState];
	
    if ( (delegate) && ([[enterCallbacks allKeys] containsObject:key]) )
    {
        SEL callback = NSSelectorFromString((NSString *)[enterCallbacks objectForKey:key]);
        [delegate performSelector:callback withObject:self];
    }
}

// Current Object getter
- (id) currentObject
{
    NSString *key = [NSString stringWithFormat:@"%u", currentState];
    
    if ( ![[objects allKeys] containsObject:key] )
        return nil;
    
    return [objects objectForKey:key];
}

// Managing states:
- (unsigned int) createState
{
    statesCount++;
    
    return (statesCount - 1);
}

- (void) createStates:(unsigned int)count inArray:(unsigned int *)states
{
    for (unsigned int i = 0 ; i < count ; i++)
    {
        unsigned int state = [self createState];
        
        if (states)
            states[i] = state;
    }
}

- (void) setObject:(id)object forState:(unsigned int)state
{
	if ( (state >= statesCount) || (!object) )
        return;
    
    NSString *key = [NSString stringWithFormat:@"%u", state];
    [objects setObject:object forKey:key];
}

- (void) setEnterStateCallback:(SEL)enter forState:(unsigned int)state
{
    if ( (state >= statesCount) || (!enter) )
        return;
    
    NSString *key = [NSString stringWithFormat:@"%u", state];
    id object = NSStringFromSelector(enter);
    
    [enterCallbacks setObject:object forKey:key];
}

- (void) setQuitStateCallback:(SEL)quit forState:(unsigned int)state
{
	if ( (state >= statesCount) || (!quit) )
        return;
    
    NSString *key = [NSString stringWithFormat:@"%u", state];
    id object = NSStringFromSelector(quit);
    
    [quitCallbacks setObject:object forKey:key];
}

- (unsigned int) createStateWithObject:(id)object andEnterStateCallback:(SEL)enter andQuitStateCallback:(SEL)quit
{
	unsigned int state = [self createState];
	
	[self setObject:object forState:state];
	[self setEnterStateCallback:enter forState:state];
	[self setQuitStateCallback:quit forState:state];
	
	return state;
}

- (void) createStates:(unsigned int)count withObjects:(id *)items andEnterStateCallbacks:(SEL *)enters andQuitStateCallbacks:(SEL *)quits inArray:(unsigned int *)states
{
    for (unsigned int i = 0 ; i < count ; i++)
    {
		unsigned int state = [self createState];
		
		if (items)
			[self setObject:items[i] forState:state];
		
		if (enters)
			[self setEnterStateCallback:enters[i] forState:state];
		
		if (quits)
			[self setQuitStateCallback:quits[i] forState:state];
        
        if (states)
            states[i] = state;
    }
}

// Managing actions:
- (unsigned int) createAction
{
    actionsCount++;
    
    return (actionsCount - 1);
}

- (void)createActions:(unsigned int)count inArray:(unsigned int *)actions
{
    for (unsigned int i = 0 ; i < count ; i++)
    {
        unsigned int action = [self createAction];
        
        if (actions)
            actions[i] = action;
    }
}

// Associations:
- (void) createAssociationFromState:(unsigned int)start withAction:(unsigned int)action toState:(unsigned int)end
{
    if ( (action >= actionsCount) || (start >= statesCount) || (end >= statesCount) )
        return;
    
    NSString *key = NSStringFromRange(NSMakeRange(start, action));
    id object = [NSString stringWithFormat:@"%u", end];
    
    [stateActions setObject:object forKey:key];
}

- (void) createAssociationFromState:(unsigned int)start withActions:(unsigned int *)actions toStates:(unsigned int *)ends andNumAssociations:(unsigned int)count
{
	if ( (start >= statesCount) || (!actions) || (!ends) || (!count) )
		return;
	
	for (unsigned int i = 0 ; i < count ; i++)
		[self createAssociationFromState:start withAction:actions[i] toState:ends[i]];
}

- (void) doAction:(unsigned int)action
{
    if  ( (action >= actionsCount) || (currentState == NO_STATE) )
        return;
    
    NSString *key = NSStringFromRange(NSMakeRange(currentState, action));
    id object = [stateActions objectForKey:key];
    
    self.currentState = ((unsigned int)[(NSString *)object intValue]);
}

// NSXMLParserDelegate functions:
- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName attributes:(NSDictionary *)attributeDict
{
	switch (xml->state)
	{
		case FSM_XML_PARSING_ROOT:
			if ([elementName compare:FSM_XML_ROOT] == NSOrderedSame)
			{
				xml->state = FSM_XML_PARSING_STATES;
				break;
			}
			
			NSLog(@"Missing root");
			xml->state = FSM_XML_PARSING_ERROR;
			
			break;
		case FSM_XML_PARSING_STATES:
			
			if ([elementName compare:FSM_XML_STATES] == NSOrderedSame)
			{
				break;
			}
			else if ([elementName compare:FSM_XML_STATE] == NSOrderedSame)
			{
				// Create state:
				NSString *key = nil;
				
				if ([[attributeDict allKeys] containsObject:FSM_XML_STATE_NAME])
					key = [attributeDict objectForKey:FSM_XML_STATE_NAME];
				
				if (!key)
				{
					NSLog(@"Error while parsing state: attribute '%@' missing", FSM_XML_STATE_NAME);
					xml->state = FSM_XML_PARSING_ERROR;
					break;
				}
				
				id object = nil;
				SEL enter = nil;
				SEL quit = nil;
				
				if ([[attributeDict allKeys] containsObject:FSM_XML_STATE_OBJECT])
					object = [attributeDict objectForKey:FSM_XML_STATE_OBJECT];
				
				if ([[attributeDict allKeys] containsObject:FSM_XML_STATE_ENTER_CALLBACK])
					enter = NSSelectorFromString([attributeDict objectForKey:FSM_XML_STATE_ENTER_CALLBACK]);
				
				if ([[attributeDict allKeys] containsObject:FSM_XML_STATE_QUIT_CALLBACK])
					quit = NSSelectorFromString([attributeDict objectForKey:FSM_XML_STATE_QUIT_CALLBACK]);
				
				unsigned int state = [self createStateWithObject:object andEnterStateCallback:enter andQuitStateCallback:quit];
				
				[xml->states setObject:[NSNumber numberWithUnsignedInt:state] forKey:key];
				
				break;
			}
			else if ([elementName compare:FSM_XML_ACTIONS] == NSOrderedSame)
			{
				xml->state = FSM_XML_PARSING_ACTIONS;
				break;
			}
			else if ([elementName compare:FSM_XML_ASSOCIATIONS] == NSOrderedSame)
			{
				xml->state = FSM_XML_PARSING_ASSOCIATIONS;
				break;
			}
			
			NSLog(@"Error while parsing states (found %@)", elementName);
			xml->state = FSM_XML_PARSING_ERROR;
			
			break;
		case FSM_XML_PARSING_ACTIONS:
			
			if ([elementName compare:FSM_XML_ACTIONS] == NSOrderedSame)
			{
				break;
			}
			else if ([elementName compare:FSM_XML_ACTION] == NSOrderedSame)
			{
				// Create action:
				NSString *key = nil;
				
				if ([[attributeDict allKeys] containsObject:FSM_XML_ACTION_NAME])
					key = [attributeDict objectForKey:FSM_XML_ACTION_NAME];
				
				if (!key)
				{
					NSLog(@"Error while parsing action: attribute '%@' missing", FSM_XML_ACTION_NAME);
					xml->state = FSM_XML_PARSING_ERROR;
					break;
				}
				
				unsigned int action = [self createAction];
				
				[xml->actions setObject:[NSNumber numberWithUnsignedInt:action] forKey:key];
				
				break;
			}
			else if ([elementName compare:FSM_XML_ASSOCIATIONS] == NSOrderedSame)
			{
				xml->state = FSM_XML_PARSING_ASSOCIATIONS;
				break;
			}
			
			NSLog(@"Error while parsing actions (found %@)", elementName);
			xml->state = FSM_XML_PARSING_ERROR;
			
			break;
		case FSM_XML_PARSING_ASSOCIATIONS:
			
			if ([elementName compare:FSM_XML_ASSOCIATIONS] == NSOrderedSame)
			{
				break;
			}
			else if ([elementName compare:FSM_XML_ASSOCIATION] == NSOrderedSame)
			{
				// Create association:
				NSString *fromState = nil;
				NSString *withAction = nil;
				NSString *toState = nil;
				
				if ([[attributeDict allKeys] containsObject:FSM_XML_ASSOCIATION_FROM_STATE])
					fromState = [attributeDict objectForKey:FSM_XML_ASSOCIATION_FROM_STATE];
				
				if ([[attributeDict allKeys] containsObject:FSM_XML_ASSOCIATION_ACTION])
					withAction = [attributeDict objectForKey:FSM_XML_ASSOCIATION_ACTION];
				
				if ([[attributeDict allKeys] containsObject:FSM_XML_ASSOCIATION_TO_STATE])
					toState = [attributeDict objectForKey:FSM_XML_ASSOCIATION_TO_STATE];
				
				if ( (!fromState) || (!withAction) || (!toState) )
				{
					NSLog(@"Error while parsing association: attributes '%@', '%@' or '%@' missing", FSM_XML_ASSOCIATION_FROM_STATE, FSM_XML_ASSOCIATION_ACTION, FSM_XML_ASSOCIATION_TO_STATE);
					xml->state = FSM_XML_PARSING_ERROR;
					break;
				}
				
				unsigned int from = [(NSNumber *)[xml->states objectForKey:fromState] unsignedIntValue];
				unsigned int action = [(NSNumber *)[xml->actions objectForKey:withAction] unsignedIntValue];
				unsigned int to = [(NSNumber *)[xml->states objectForKey:toState] unsignedIntValue];
				
				[self createAssociationFromState:from withAction:action toState:to];
				
				break;
			}
			
			NSLog(@"Error while parsing associations (found %@)", elementName);
			xml->state = FSM_XML_PARSING_ERROR;
			
			break;
		default:
			break;
	}
}

@end
