//
//  IVSHMEMUserClient.cpp
//  IVSHMEM
//
//  Created by Ali on 9/6/20.
//  Copyright © 2020 Ali. All rights reserved.
//

#include "IVSHMEMUserClient.hpp"
#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <libkern/OSByteOrder.h>
#include <IOKit/assert.h>
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <os/log.h>

#define super IOUserClient

OSDefineMetaClassAndStructors(IVSHMEMDeviceUserClient, IOUserClient);

bool IVSHMEMDeviceUserClient::initWithTask(task_t owningTask,
                                           void *securityID,
                                           UInt32 type,
                                           OSDictionary *properties)
{
    
    bool success;
    
    success = super::initWithTask(owningTask, securityID, type, properties);
    
    // Can't call getName() until init() has been called.
    IOLog("%s[%p]::%s(type = " UInt32_FORMAT " )\n", getName(), this, __FUNCTION__, type);
    
//    if (success) {
//        // This code will do the right thing on both PowerPC- and Intel-based systems because the cross-endian
//        // property will never be set on PowerPC-based Macs.
//        fCrossEndian = false;
//        if (properties != NULL && properties->getObject(kIOUserClientCrossEndianKey)) {
//            // A connection to this user client is being opened by a user process running using Rosetta.
//
//            // Indicate that this user client can handle being called from cross-endian user processes by
//            // setting its IOUserClientCrossEndianCompatible property in the I/O Registry.
//            if (setProperty(kIOUserClientCrossEndianCompatibleKey, kOSBooleanTrue)) {
//                fCrossEndian = true;
//                IOLog("%s[%p]::%s(): fCrossEndian = true\n", getName(), this, __FUNCTION__);
//            }
//        }
//    }
    
    fTask = owningTask;
    fDriver = NULL;
    
    return success;
}

/*
 * driver initialization after the matching has completed...
 */
bool IVSHMEMDeviceUserClient::start(IOService *provider)
{
    IOLog("%s[%p]::%s(provider = %p)\n", getName(), this, __FUNCTION__, provider);
    
    if (!super::start(provider))
        return false;
    
    /*
     * Our provider should be a IVSHMEMDevice object. Verify that before proceeding.
     */
    
    assert(OSDynamicCast(IVSHMEMDevice, provider));
    fDriver = (IVSHMEMDevice*) provider;
    
    /*
     * Set up some memory to be shared between this user client instance and its
     * client process. The client will call in to map this memory, and I/O Kit
     * will call clientMemoryForType to obtain this memory descriptor.
     */
    
    IOMemoryDescriptor *mem = fDriver->copyGlobalMemory();
    IOByteCount memLength = mem->getLength();
    IOLog("%s[%p]: BAR2 mem->getLength() = %llu \n", getName(), this, memLength);
    
    // TODO: replace this sizeof with a method to get the size of the actual IVSHMEM BAR2 region
    fClientSharedMemory = IOBufferMemoryDescriptor::withOptions(kIOMemoryKernelUserShared, sizeof(DriverSharedMemory));
    if (!fClientSharedMemory)
        return false;

    fClientShared = (DriverSharedMemory *) fClientSharedMemory->getBytesNoCopy();
    
    fClientShared->field1 = 0x11111111; // same in all endianesses...
    fClientShared->field2 = 0x22222222; // ditto
    fClientShared->field3 = 0x33333333; // ditto
    
//    if (fCrossEndian) {
//        // Swap the fields so the user process sees the proper endianness
//        fClientShared->field1 = OSSwapInt32(fClientShared->field1);
//        fClientShared->field2 = OSSwapInt32(fClientShared->field2);
//        fClientShared->field3 = OSSwapInt32(fClientShared->field3);
//    }

    (void) strlcpy(fClientShared->string, "some data", sizeof(fClientShared->string));
    fOpenCount = 1;
    
    return true;
}

/*
 * Kill ourselves off if the client closes its connection or the client dies.
 */
IOReturn IVSHMEMDeviceUserClient::clientClose(void)
{
    IOLog("%s[%p]::%s()\n", getName(), this, __FUNCTION__);
    
    if( !isInactive())
        terminate();
    
    return kIOReturnSuccess;
}

/*
 * stop will be called during the termination process, and should free all resources
 * associated with this client.
 */
void IVSHMEMDeviceUserClient::stop(IOService *provider)
{
    IOLog("%s[%p]::%s(provider = %p)\n", getName(), this, __FUNCTION__, provider);
    
    if (fClientSharedMemory) {
        fClientSharedMemory->release();
        fClientSharedMemory = 0;
    }
    
    super::stop(provider);
}

// defining and selecting our external user client methods
IOReturn IVSHMEMDeviceUserClient::externalMethod(uint32_t selector,
                                                 IOExternalMethodArguments *arguments,
                                                 IOExternalMethodDispatch *dispatch,
                                                 OSObject *target,
                                                 void *reference)
{
    
    IOReturn    result;
    
    if (fDriver == NULL || isInactive()) {
        // Return an error if we don't have a provider. This could happen if the user process
        // called either method without calling IOServiceOpen first. Or, the user client could be
        // in the process of being terminated and is thus inactive.
        result = kIOReturnNotAttached;
    }
    else if (!fDriver->isOpen(this)) {
        // Return an error if we do not have the driver open. This could happen if the user process
        // did not call openUserClient before calling this function.
        result = kIOReturnNotOpen;
    }
    
    IOReturn err;
    IOLog("The Leopard and later way to route external methods\n");
    switch (selector)
    {
        case kSampleMethod1:
            err = method1( (UInt32 *) arguments->structureInput,
                          (UInt32 *)  arguments->structureOutput,
                          arguments->structureInputSize, (IOByteCount *) &arguments->structureOutputSize );
            break;
            
//        case kSampleMethod2:
//            err = method2( (SampleStructForMethod2 *) arguments->structureInput,
//                          (SampleResultsForMethod2 *)  arguments->structureOutput,
//                          arguments->structureInputSize, (IOByteCount *) &arguments->structureOutputSize );
//            break;
            
        default:
            err = kIOReturnBadArgument;
            break;
    }
    
    IOLog("externalMethod(%d) 0x%x", selector, err);
    
    return (err);
}

/*
 * Implement each of the external methods described above.
 */

IOReturn IVSHMEMDeviceUserClient::method1(UInt32 *dataIn,
                                          UInt32 *dataOut,
                                          IOByteCount inputSize,
                                          IOByteCount *outputSize )
{
    
    IOReturn    ret;
    IOItemCount    count;
    
    IOLog("IVSHMEMDeviceUserClient::method1(");
    os_log(OS_LOG_DEFAULT, "AAAAAAAAAAAAAAAAAAAAAAAAAaaa");
    
    if (*outputSize < inputSize)
        return( kIOReturnNoSpace );
    
    count = inputSize / sizeof(UInt32);
    for (UInt32 i = 0; i < count; i++ ) {
//        // Client app is running using Rosetta
//        if (fCrossEndian) {
//            dataIn[i] = OSSwapInt32(dataIn[i]);
//        }
        IOLog("" UInt32_x_FORMAT ", ", dataIn[i]);
//        dataOut[i] = dataIn[i] ^ 0xffffffff;
        dataOut[i] = dataIn[i] + 5;
//        // Rosetta again
//        if (fCrossEndian) {
//            dataOut[i] = OSSwapInt32(dataOut[i]);
//        }
        
    }
    
    ret = kIOReturnSuccess;
    IOLog(")\n");
    *outputSize = count * sizeof( UInt32 );
    
    return( ret );
}

/*
 * Shared memory support. Supply a IOMemoryDescriptor instance to describe
 * each of the kinds of shared memory available to be mapped into the client
 * process with this user client.
 */

IOReturn IVSHMEMDeviceUserClient::clientMemoryForType(
                                                      UInt32 type,
                                                      IOOptionBits* options,
                                                      IOMemoryDescriptor** memory) {

    // Return a memory descriptor reference for some memory a client has requested
    // be mapped into its address space.
    
    IOReturn ret;
    
    IOLog("SamplePCIUserClient::clientMemoryForType(" UInt32_FORMAT ")\n", type);
    
    switch( type ) {
            
//        case kSamplePCIMemoryType1:
//            // give the client access to some shared data structure
//            // (shared between this object and the client)
//            fClientSharedMemory->retain();
//            *memory  = fClientSharedMemory;
//            ret = kIOReturnSuccess;
//            break;
            
        case kSamplePCIMemoryType2:
            // Give the client access to some of the card's memory
            // (all clients get the same)
            *memory  = fDriver->copyGlobalMemory();
            
            ret = kIOReturnSuccess;
            break;
            
        default:
            ret = kIOReturnBadArgument;
            break;
    }
    
    return ret;
}
