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
    
    IOMemoryDescriptor *mem = fDriver->copyGlobalMemory(); // BAR2 (shared mem)
    IOByteCount memLength = mem->getLength(); // Length of BAR2 (shared mem)
    IOLog("%s[%p]: BAR2 mem->getLength() = %llu \n", getName(), this, memLength);
    
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
//    IOLog("The Leopard and later way to route external methods\n");
    switch (selector)
    {
        // Returns the size of the shared memory device (BAR2)
        case getMemorySizeMethod:
            err = getBAR2MemorySize( (UInt32 *) arguments->structureInput,
                          (UInt32 *)  arguments->structureOutput,
                          arguments->structureInputSize, (IOByteCount *) &arguments->structureOutputSize );
            break;
        
        // Interrupts are not currently supported, this method is a placeholder
        case getInterruptsEnabledMethod:
            err = getInterruptsEnabled( (UInt32 *) arguments->structureInput,
                          (UInt32 *)  arguments->structureOutput,
                          arguments->structureInputSize, (IOByteCount *) &arguments->structureOutputSize );
            break;
            
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
        case kBAR2MemoryType:
            // Give the client access to the card's memory
            // (all clients get the same)
            *memory  = fDriver->copyGlobalMemory(); // BAR2
            
            ret = kIOReturnSuccess;
            break;
            
        default:
            ret = kIOReturnBadArgument;
            break;
    }
    
    return ret;
}

IOReturn IVSHMEMDeviceUserClient::getBAR2MemorySize(UInt32 *dataIn,
                                          UInt32 *dataOut,
                                          IOByteCount inputSize,
                                          IOByteCount *outputSize )
{
    IOReturn    ret;
    
//    IOLog("IVSHMEMDeviceUserClient::getMemorySize\n");
    
    if (*outputSize < inputSize)
        return( kIOReturnNoSpace );
    
    IOMemoryDescriptor *mem = fDriver->copyGlobalMemory(); // BAR2
    *dataOut = mem->getLength(); // Length of BAR2(shared mem)
    
    ret = kIOReturnSuccess;
    *outputSize = sizeof(UInt32);
    return( ret );
}

IOReturn IVSHMEMDeviceUserClient::getInterruptsEnabled(UInt32 *dataIn,
                                          UInt32 *dataOut,
                                          IOByteCount inputSize,
                                          IOByteCount *outputSize )
{
    IOReturn    ret;
    
//    IOLog("IVSHMEMDeviceUserClient::getInterruptsEnabled\n");
    
    if (*outputSize < inputSize)
        return( kIOReturnNoSpace );
    
    *dataOut = fDriver->interruptsEnabled;
    
    ret = kIOReturnSuccess;
    *outputSize = sizeof(UInt32);
    return( ret );
}
