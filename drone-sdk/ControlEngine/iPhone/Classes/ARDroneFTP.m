//
//  ARDroneFTP.m
//  ARDroneEngine
//
//  Created by Nicolas BRULEZ on 07/04/11.
//  Copyright 2011 Parrot. All rights reserved.
//

#import "ARDroneFTP.h"
#import <utils/ardrone_ftp.h>

#import "ConstantsAndMacros.h"
#import "wifi.h"

/**
 * List of different FTPs opened, the key is the hex value of the ftp pointer
 * Used inside "C to Obj-C callback to get the correct delegate/callback from
 * multi-thread/multi-ftp contexts.
 */
static NSMutableDictionary *listOfFtps = nil;

@implementation ARDroneFTPCallbackArg
@synthesize status;
@synthesize progress;
@synthesize operation;

- (id)initWithStatus:(ARDFTP_RESULT)_status withProgress:(float)_progress withFileList:(char *)_fileList withOperation:(FTP_OPERATION)_operation
{
    self = [super init];
    if (self)
    {
        status = _status;
        progress = _progress;
        if (nil != _fileList && 0 < strlen (_fileList))
        {
            fileList = [[NSString stringWithCString:_fileList encoding:NSASCIIStringEncoding] copy];
        }
        operation = _operation;
    }
    return self;
}

- (void)setFileList:(char *)_fileList
{
    if (nil != _fileList && 0 < strlen (_fileList))
    {
        fileList = [[NSString stringWithCString:_fileList encoding:NSASCIIStringEncoding] copy];
    }
}

- (void)dealloc
{
	[fileList release];
	
	[super dealloc];
}

- (NSString *)fileList
{
	return [[fileList copy] autorelease];
}

@end

ARDFTP_RESULT convertStatus (_ftp_status status)
{
    ARDFTP_RESULT retVal = ARDFTP_FAIL;
    switch (status)
    {
        case FTP_FAIL:
            retVal = ARDFTP_FAIL;
            break;
        case FTP_SUCCESS:
            retVal = ARDFTP_SUCCESS;
            break;
        case FTP_TIMEOUT:
            retVal = ARDFTP_TIMEOUT;
            break;
        case FTP_BADSIZE:
            retVal = ARDFTP_BADSIZE;
            break;
        case FTP_SAMESIZE:
            retVal = ARDFTP_SAMESIZE;
            break;
        case FTP_PROGRESS:
            retVal = ARDFTP_PROGRESS;
            break;
        case FTP_BUSY:
            retVal = ARDFTP_BUSY;
            break;
        case FTP_ABORT:
            retVal = ARDFTP_ABORT;
            break;
        default:
            break;
    }
    return retVal;
}

void wrapperCallback (_ftp_status status, void *arg, _ftp_t *ftp)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
    if (nil == listOfFtps) // Should never happen ... but just in case
	{
		[pool drain];
        return;
	}
    
    ARDroneFTP *ardftp = [listOfFtps objectForKey:[NSString stringWithFormat:@"%x",ftp]];
    if (nil != ardftp)
    {
        ARDFTP_RESULT locStat = convertStatus(status);
        ARDroneFTPCallbackArg *callbackArg = nil;
        if (ARDFTP_PROGRESS == locStat && NULL != arg)
        {
            callbackArg = [[ARDroneFTPCallbackArg alloc] initWithStatus:locStat withProgress:*(float *)arg withFileList:nil withOperation:ardftp.currentOperation];
        }
        else if (ARDFTP_SUCCESS == locStat && NULL != arg && FTP_LIST == ardftp.currentOperation)
        { 
            callbackArg = [[ARDroneFTPCallbackArg alloc] initWithStatus:locStat withProgress:0.0 withFileList:(char *)arg withOperation:ardftp.currentOperation];
            vp_os_free (arg);
        }
        else
        { 
            callbackArg = [[ARDroneFTPCallbackArg alloc] initWithStatus:locStat withProgress:0.0 withFileList:nil withOperation:ardftp.currentOperation];
        }
		
        if (FTP_BUSY != locStat &&
            FTP_PROGRESS != locStat) // Keep FTP busy if it was already busy before call, or if the current process is still in progress 
        {
            ardftp.isBusy = NO;
        }
		
        [ardftp.delegate performSelector:ardftp.currentOpCallback withObject:[callbackArg autorelease]];
    }
	
	[pool drain];
}

extern char drone_address[];

@implementation ARDroneFTP
@synthesize currentOperation;
@synthesize currentOpCallback;
@synthesize delegate;
@synthesize isBusy;


+ (id) anonymousUpdateFTPwithDelegate:(id)_delegate withDefaultCallback:(SEL)_callback
{
    NSString *ip = [NSString stringWithCString:drone_address encoding:NSUTF8StringEncoding];
    const int port = 5551;
    
    ARDroneFTP *ftp = [[ARDroneFTP alloc] initAnonymousWithIP:ip withPort:port withDelegate:_delegate withDefaultCallback:_callback];
    
    return [ftp autorelease];
}

+ (id) updateFTPwithUser:(NSString *)user withPassword:(NSString *)password withDelegate:(id)_delegate withDefaultCallback:(SEL)_callback
{
    NSString *ip = [NSString stringWithCString:drone_address encoding:NSUTF8StringEncoding];
    const int port = 5551;
    
    ARDroneFTP *ftp = [[ARDroneFTP alloc] initWithIP:ip withPort:port withUser:user withPassword:password withDelegate:_delegate withDefaultCallback:_callback];
    
    return [ftp autorelease];
}

+ (id) anonymousStandardFTPwithDelegate:(id)_delegate withDefaultCallback:(SEL)_callback
{
    NSString *ip = [NSString stringWithCString:drone_address encoding:NSUTF8StringEncoding];
    const int port = 21;
    
    ARDroneFTP *ftp = [[ARDroneFTP alloc] initAnonymousWithIP:ip withPort:port withDelegate:_delegate withDefaultCallback:_callback];
    
    return [ftp autorelease];
}

+ (id) standardFTPwithUser:(NSString *)user withPassword:(NSString *)password withDelegate:(id)_delegate withDefaultCallback:(SEL)_callback
{
    NSString *ip = [NSString stringWithCString:drone_address encoding:NSUTF8StringEncoding];
    const int port = 21;
    
    ARDroneFTP *ftp = [[ARDroneFTP alloc] initWithIP:ip withPort:port withUser:user withPassword:password withDelegate:_delegate withDefaultCallback:_callback];
    
    return [ftp autorelease];
}

- (id)initAnonymousWithIP:(NSString *)ip withPort:(int)port withDelegate:(id)_delegate withDefaultCallback:(SEL)_callback
{
    self = [super init];
    if (self)
    {
        _ftp_status initResult = FTP_FAIL;
        ftp = ftpConnect ([ip cStringUsingEncoding:NSASCIIStringEncoding],
                          port,
                          "anonymous",
                          "",
                          &initResult);
        if (FTP_FAILED (initResult))
        {
            NSLog(@"Connexion to FTP anonymous@%@:%d failed\n", ip, port);
        }
        delegate = _delegate;
        defaultCallback = _callback;
        isBusy = NO;
        currentOperation = FTP_NONE;
        if (nil == listOfFtps)
        {
            listOfFtps = [[NSMutableDictionary alloc] initWithCapacity:1];
        }
        [listOfFtps setObject:self forKey:[NSString stringWithFormat:@"%x",ftp]];
    }
    return self;
}

- (id)initWithIP:(NSString *)ip withPort:(int)port withUser:(NSString *)user withPassword:(NSString *)password withDelegate:(id)_delegate withDefaultCallback:(SEL)_callback
{
    self = [super init];
    if (self)
    {
        _ftp_status initResult = FTP_FAIL;
        ftp = ftpConnect ([ip cStringUsingEncoding:NSASCIIStringEncoding],
                          port,
                          [user cStringUsingEncoding:NSASCIIStringEncoding],
                          [password cStringUsingEncoding:NSASCIIStringEncoding],
                          &initResult);
        if (FTP_FAILED (initResult))
        {
            NSLog(@"Connexion to FTP %@@%@:%d failed\n", user, ip, port);
        }
        delegate = _delegate;
        defaultCallback = _callback;
        isBusy = NO;
        currentOperation = FTP_NONE;
        if (nil == listOfFtps)
        {
            listOfFtps = [[NSMutableDictionary alloc] initWithCapacity:1];
        }
        [listOfFtps setObject:self forKey:[NSString stringWithFormat:@"%x",ftp]];
    }
    return self;
}

- (void)setDelegate:(id)_delegate
{
    delegate = _delegate;
}

- (void)setDefaultCallback:(SEL)_callback
{
    defaultCallback = _callback;
}

- (void)close
{
    [listOfFtps removeObjectForKey:[NSString stringWithFormat:@"%x",ftp]];
	
	if (![listOfFtps count])
	{
		[listOfFtps release];
		listOfFtps = nil;
	}
	
    ftpClose (&ftp);
}

- (bool)keepConnexionAlive
{
    return ARDFTP_SUCCEEDED([self changeDirectoryTo:@"."]);
}

- (ARDFTP_RESULT)sendLocalFile:(NSString *)local toDistantFile:(NSString *)distant withResume:(BOOL)resume
{
    return [self sendLocalFile:local toDistantFile:distant withResume:resume withCallback:defaultCallback];
}

- (ARDFTP_RESULT)sendLocalFile:(NSString *)local toDistantFile:(NSString *)distant withResume:(BOOL)resume withCallback:(SEL)_callback
{
    if (isBusy)
    {
        return ARDFTP_BUSY;
    }
    _ftp_status retVal = FTP_FAIL;
    isBusy = YES;
    currentOperation = FTP_SEND;
    currentOpCallback = _callback;
	
    if (nil != currentOpCallback)
    {
        retVal = ftpPut (ftp,
                         [local cStringUsingEncoding:NSASCIIStringEncoding],
                         [distant cStringUsingEncoding:NSASCIIStringEncoding],
                         (resume) ? 1 : 0,
                         wrapperCallback);
    }
    return convertStatus(retVal);
}

- (ARDFTP_RESULT)sendSynchronouslyLocalFile:(NSString *)local toDistantFile:(NSString *)distant withResume:(BOOL)resume
{
    if (isBusy)
    {
        return ARDFTP_BUSY;
    }
    _ftp_status retVal = FTP_FAIL;
    isBusy = YES;
    currentOperation = FTP_SEND;
    retVal = ftpPut (ftp,
                     [local cStringUsingEncoding:NSASCIIStringEncoding],
                     [distant cStringUsingEncoding:NSASCIIStringEncoding],
                     (resume) ? 1 : 0,
                     NULL);
    currentOperation = FTP_NONE;
    isBusy = NO;
    return convertStatus(retVal);
}

- (ARDFTP_RESULT)getDistantFile:(NSString *)distant toLocalFile:(NSString *)local withResume:(BOOL)resume
{
    return [self getDistantFile:distant toLocalFile:local withResume:resume withCallback:defaultCallback];
}

- (ARDFTP_RESULT)getDistantFile:(NSString *)distant toLocalFile:(NSString *)local withResume:(BOOL)resume withCallback:(SEL)_callback
{
    if (isBusy)
    {
        return ARDFTP_BUSY;
    }
    _ftp_status retVal = FTP_FAIL;
    isBusy = YES;
    currentOperation = FTP_GET;
    currentOpCallback = _callback;
    if (nil != currentOpCallback)
    {
        retVal = ftpGet(ftp,
                        [distant cStringUsingEncoding:NSASCIIStringEncoding],
                        [local cStringUsingEncoding:NSASCIIStringEncoding],
                        (resume) ? 1 : 0,
                        wrapperCallback);
    }
    return convertStatus(retVal);
}

- (ARDFTP_RESULT)getSynchronouslyDistantFile:(NSString *)distant toLocalFile:(NSString *)local withResume:(BOOL)resume
{
    if (isBusy)
    {
        return ARDFTP_BUSY;
    }
    _ftp_status retVal = FTP_FAIL;
    isBusy = YES;
    currentOperation = FTP_GET;
    retVal = ftpGet (ftp,
                     [distant cStringUsingEncoding:NSASCIIStringEncoding],
                     [local cStringUsingEncoding:NSASCIIStringEncoding],
                     (resume) ? 1 : 0,
                     NULL);
    currentOperation = FTP_NONE;
    isBusy = NO;
    return convertStatus(retVal);
}

- (ARDFTP_RESULT)listCurrentDirectory
{
    return [self listCurrentDirectoryWithCallback:defaultCallback];
}

- (ARDFTP_RESULT)listCurrentDirectoryWithCallback:(SEL)_callback
{
    if (isBusy)
    {
        return ARDFTP_BUSY;
    }
    _ftp_status retVal = FTP_FAIL;
    isBusy = YES;
    currentOperation = FTP_LIST;
    currentOpCallback = _callback;
    if (nil != currentOpCallback)
    {
        retVal = ftpList(ftp,
                         NULL,
                         wrapperCallback);
    }
    return convertStatus (retVal);
}

- (ARDFTP_RESULT)listCurrentDirectoryIn:(NSString **)result
{
    if (isBusy)
    {
        return ARDFTP_BUSY;
    }
    _ftp_status retVal = FTP_FAIL;
    isBusy = YES;
    currentOperation = FTP_LIST;
    char *buffer = NULL;
    retVal = ftpList (ftp,
                      &buffer,
                      NULL);
    currentOperation = FTP_NONE;
    isBusy = NO;
    if (NULL != buffer)
    {
        *result = [NSString stringWithCString:buffer encoding:NSASCIIStringEncoding];
    }
    return convertStatus(retVal);
}

- (ARDFTP_RESULT)removeDistantFile:(NSString *)distant
{
    return convertStatus (ftpRemove (ftp,
                                     [distant cStringUsingEncoding:NSASCIIStringEncoding]));
}

- (ARDFTP_RESULT)renameDistantFileFrom:(NSString *)origin to:(NSString *)dest
{
    return convertStatus (ftpRename (ftp,
                                     [origin cStringUsingEncoding:NSASCIIStringEncoding],
                                     [dest cStringUsingEncoding:NSASCIIStringEncoding]));
}

- (ARDFTP_RESULT)renameDistantDirectoryFrom:(NSString *)origin to:(NSString *)dest
{
    return [self renameDistantFileFrom:origin to:dest];
}

- (ARDFTP_RESULT)changeDirectoryTo:(NSString *)nextDir
{
    return convertStatus (ftpCd (ftp,
                                 [nextDir cStringUsingEncoding:NSASCIIStringEncoding]));
}

- (ARDFTP_RESULT)getWorkingDirectoryTo:(NSString **)result
{
    char buffer [1024] = {0};
    _ftp_status retVal = ftpPwd (ftp,
                                 buffer,
                                 sizeof (buffer));
    *result = [NSString stringWithCString:buffer encoding:NSASCIIStringEncoding];
    return convertStatus (retVal);
}

- (ARDFTP_RESULT)createDirectoryNamed:(NSString *)dirName
{
    return convertStatus (ftpMkdir (ftp,
                                    [dirName cStringUsingEncoding:NSASCIIStringEncoding]));
}

- (ARDFTP_RESULT)removeDirectoryNamed:(NSString *)dirName
{
    return convertStatus (ftpMkdir (ftp,
                                    [dirName cStringUsingEncoding:NSASCIIStringEncoding]));
}

- (ARDFTP_RESULT)abortCurrentOperation
{
    return convertStatus(ftpAbort (ftp));
}

- (void)dealloc
{
    [self close];
	
    [super dealloc];
}

@end
