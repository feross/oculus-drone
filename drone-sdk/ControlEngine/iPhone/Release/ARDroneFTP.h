//
//  ARDroneFTP.h
//  ARDroneEngine
//
//  Wrapper around utils/ardrone_ftp.c functions
//
//  Created by Nicolas BRULEZ on 07/04/11.
//  Copyright 2011 Parrot. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef enum ARDFTP_RESULT_e {
    ARDFTP_FAIL,
    ARDFTP_BUSY,
    ARDFTP_SUCCESS,
    ARDFTP_TIMEOUT,
    ARDFTP_BADSIZE,
    ARDFTP_SAMESIZE,
    ARDFTP_PROGRESS,
    ARDFTP_ABORT,
} ARDFTP_RESULT;

typedef enum FTP_OPERATION_e {
    FTP_NONE,
    FTP_SEND,
    FTP_GET,
    FTP_LIST,
} FTP_OPERATION;

/**
 * Fail / Success test
 *
 * ARDFTP_FAILED is not "return !ARDFTP_SUCCEEDED (result);" because ARDFTP_PROGRESS is not a success
 * nor a failure case
 */


static inline bool ARDFTP_SUCCEEDED (ARDFTP_RESULT result)
{
    return (ARDFTP_SAMESIZE == result ||
            ARDFTP_SUCCESS == result) ? 
    YES : NO;
}

static inline bool ARDFTP_FAILED (ARDFTP_RESULT result)
{
    return (ARDFTP_FAIL == result ||
            ARDFTP_BUSY == result ||
            ARDFTP_TIMEOUT == result ||
            ARDFTP_BADSIZE == result ||
            ARDFTP_ABORT == result) ?
    YES : NO;
}

@interface ARDroneFTPCallbackArg : NSObject {
    ARDFTP_RESULT status;
    float progress; // Zero if status is not "ARDFTP_PROGRESS"
    NSString *fileList; // nil if operation is not "FTP_LIST" and status is not "ARDFTP_SUCCESS"
    FTP_OPERATION operation;
}
@property (readonly) ARDFTP_RESULT status;
@property (readonly) float progress;
@property (readonly) FTP_OPERATION operation;
@property (nonatomic, readonly) NSString *fileList;

- (id)initWithStatus:(ARDFTP_RESULT)_status withProgress:(float)_progress withFileList:(char *)_fileList withOperation:(FTP_OPERATION)_operation;

@end

@interface ARDroneFTP : NSObject {
    struct _ftp_s *ftp;
    id delegate;
    SEL defaultCallback;
    SEL currentOpCallback;
    BOOL isBusy;
    FTP_OPERATION currentOperation;
}
@property (readonly) FTP_OPERATION currentOperation;
@property (readonly) SEL currentOpCallback;
@property (readonly) id delegate;
@property (readwrite) BOOL isBusy;

/* Instanciations of common FTP for AR.Drone */
+ (id) anonymousUpdateFTPwithDelegate:(id)_delegate withDefaultCallback:(SEL)_callback;
+ (id) updateFTPwithUser:(NSString *)user withPassword:(NSString *)password withDelegate:(id)_delegate withDefaultCallback:(SEL)_callback;
+ (id) anonymousStandardFTPwithDelegate:(id)_delegate withDefaultCallback:(SEL)_callback;
+ (id) standardFTPwithUser:(NSString *)user withPassword:(NSString *)password withDelegate:(id)_delegate withDefaultCallback:(SEL)_callback;

/* Open FTP */
- (id)initAnonymousWithIP:(NSString *)ip withPort:(int)port withDelegate:(id)_delegate withDefaultCallback:(SEL)_callback;
- (id)initWithIP:(NSString *)ip withPort:(int)port withUser:(NSString *)user withPassword:(NSString *)password withDelegate:(id)_delegate withDefaultCallback:(SEL)_callback;
/* Change delegate/callback */
- (void)setDelegate:(id)_delegate;
- (void)setDefaultCallback:(SEL)_callback;
/* Close FTP */
- (void)close;

/* Dummy command to keep ftp connexion alive ... return NO if connexion was closed by server */
- (bool)keepConnexionAlive;

/* NOTE : Default callback refers to the callback defined in init... or setDefaultCallback functions */

/* Send file */
- (ARDFTP_RESULT)sendLocalFile:(NSString *)local toDistantFile:(NSString *)distant withResume:(BOOL)resume; // Default callback
- (ARDFTP_RESULT)sendLocalFile:(NSString *)local toDistantFile:(NSString *)distant withResume:(BOOL)resume withCallback:(SEL)_callback; // Specified callback
- (ARDFTP_RESULT)sendSynchronouslyLocalFile:(NSString *)local toDistantFile:(NSString *)distant withResume:(BOOL)resume; // Blocking call until completion
/* Get file */
- (ARDFTP_RESULT)getDistantFile:(NSString *)distant toLocalFile:(NSString *)local withResume:(BOOL)resume; // Default callback
- (ARDFTP_RESULT)getDistantFile:(NSString *)distant toLocalFile:(NSString *)local withResume:(BOOL)resume withCallback:(SEL)_callback; // Specified callback
- (ARDFTP_RESULT)getSynchronouslyDistantFile:(NSString *)distant toLocalFile:(NSString *)local withResume:(BOOL)resume; // Blocking call until completion
/* List current directory */
- (ARDFTP_RESULT)listCurrentDirectory; // Default callback
- (ARDFTP_RESULT)listCurrentDirectoryWithCallback:(SEL)_callback; // Specified callback
- (ARDFTP_RESULT)listCurrentDirectoryIn:(NSString **)result; // Blocking call, result is set in *result

/* Remove file */
- (ARDFTP_RESULT)removeDistantFile:(NSString *)distant;
/* Rename file/directory (Both functions are the same) */
- (ARDFTP_RESULT)renameDistantFileFrom:(NSString *)origin to:(NSString *)dest;
- (ARDFTP_RESULT)renameDistantDirectoryFrom:(NSString *)origin to:(NSString *)dest;
/* Change working directory */
- (ARDFTP_RESULT)changeDirectoryTo:(NSString *)nextDir;
/* Get working directory */
- (ARDFTP_RESULT)getWorkingDirectoryTo:(NSString **)result;
/* Create directory */
- (ARDFTP_RESULT)createDirectoryNamed:(NSString *)dirName;
/* Remove directory */
- (ARDFTP_RESULT)removeDirectoryNamed:(NSString *)dirName;

/* Abort current operation */
- (ARDFTP_RESULT)abortCurrentOperation;

/* Check if instance is available */
- (BOOL)isBusy;

@end
