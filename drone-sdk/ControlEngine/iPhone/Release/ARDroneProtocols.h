/**
 * @file ARDroneProtocols.h
 *
 * Copyright 2009 Parrot SA. All rights reserved.
 * @author D HAEYER Frederic
 * @date 2009/10/26
 */
#include "ARDroneTypes.h"

/**
 * Define a protocol to make it possible for the game engine to callback the Parrot library
 */
@protocol ARDroneProtocolIn<NSObject>
/**
 * Change the state of the library.<br/>
 * Note: It is the library sole responsability to handle change of states and modify its behavior; basically, it should pause some of the threads when "paused" then resume everything back to normal when "running".
 *
 * @param inGame Boolean flag that indicates whether the library state shall change to "running" (YES) or "pause" (NO).
 */
- (void)changeState:(BOOL)inGame;

/**
 * Send a command informaton.
 *
 * @commandIn Command structure : identifier, parameters, callback. (each sender has to define its own identifiers; the receiver has to check which sender is calling and react accordingly).
 * @sender Pointer to the object that requests the change of state.
 * @refresh If the menu settings should be refreshed at the end of the command
 */
- (void)executeCommandIn:(ARDRONE_COMMAND_IN_WITH_PARAM)commandIn fromSender:(id)sender refreshSettings:(BOOL)refresh;

/**
 * Send a default configuration for the application (must be sent before the navdata thread is launched)
 *
 * @key Config key to set the default value
 * @value pointer to the application default value of the config key
 */
- (void)setDefaultConfigurationForKey:(ARDRONE_CONFIG_KEYS)key withValue:(void *)value;

/**
 * Send a command informaton.
 *
 * @commandId Command identification number (each sender has to define its own identifiers; the receiver has to check which sender is calling and react accordingly).
 * @parameter parameter for the command
 * @sender Pointer to the object that requests the change of state.
 */
- (void)executeCommandIn:(ARDRONE_COMMAND_IN)commandId withParameter:(void*)parameter fromSender:(id)sender __attribute__((deprecated));

/**
 * Check the state of the library.
 *
 * @return Boolean flag that indicates whether the library is running (YES) or paused (NO).
 */
- (BOOL)checkState;

@optional
/**
 * Get the latest drone's navigation data.
 *
 * @param data Pointer to a navigation data structure.
 */
- (void)navigationData:(ARDroneNavigationData*)data;

/**
 * Get the latest detection camera structure (rotation and translation).
 *
 * @param data Pointer to a detection camera structure.
 */
- (void)detectionCamera:(ARDroneDetectionCamera*)camera;

/**
 * Get the latest drone camera structure (rotation and translation).
 *
 * @param data Pointer to a drone camera structure.
 */
- (void)droneCamera:(ARDroneCamera*)camera;

/**
 * Exchange enemies data.<br/>
 * Note: basically, data should be provided by the Parrot library when in multiplayer mode (enemy type = "HUMAN"), and by the game controller when in single player mode (enemy type = "AI").
 *
 * @param data Pointer to an enemies data structure.
 */
- (void)humanEnemiesData:(ARDroneEnemiesData*)data;

@end

/**
 * Define a protocol to make it possible for the Parrot library to callback the game engine
 */
@protocol ARDroneProtocolOut<NSObject>

/**
 * Send a command informaton.
 *
 * @commandId Command identification number (each sender has to define its own identifiers; the receiver has to check which sender is calling and react accordingly).
 * @sender Pointer to the object that requests the change of state.
 */
- (void)executeCommandOut:(ARDRONE_COMMAND_OUT)commandId withParameter:(void*)parameter fromSender:(id)sender;

@optional
/**
 * Exchange AI enemies data.<br/>
 * Note: basically, data should be provided by the Parrot library when in multiplayer mode (enemy type = "HUMAN"), and by the game controller when in single player mode (enemy type = "AI").
 *
 * @param data Pointer to an enemies data structure.
 */
- (void)AIEnemiesData:(ARDroneEnemiesData*)data;

@end
