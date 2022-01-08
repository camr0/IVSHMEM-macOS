//
//  IVSHMEMUserClient.hpp
//  IVSHMEM
//
//  Created by Ali on 9/6/20.
//  Copyright © 2020 Ali. All rights reserved.
//

#ifndef IVSHMEMUserClient_hpp
#define IVSHMEMUserClient_hpp

#include <IOKit/IOUserClient.h>
#include "IVSHMEM.hpp"

// Forward declarations
class IOBufferMemoryDescriptor;

class IVSHMEMDeviceUserClient : public IOUserClient {
    
    OSDeclareDefaultStructors(IVSHMEMDeviceUserClient)
    
private:
    IVSHMEMDevice               *fDriver;
    IOBufferMemoryDescriptor    *fClientSharedMemory;
    DriverSharedMemory          *fClientShared;
    task_t                          fTask;
    int32_t                         fOpenCount;

public:
    // IOService overrides
    virtual bool start(IOService *provider);
    virtual void stop(IOService *provider);
    
    // IOUserClient overrides
    virtual bool initWithTask(task_t owningTask, void *securityID, UInt32 type, OSDictionary *properties);
    virtual IOReturn clientClose(void);
    
    virtual IOReturn externalMethod(uint32_t selector, IOExternalMethodArguments *arguements,
                                    IOExternalMethodDispatch *dispatch, OSObject *target, void *reference);
    
    virtual IOReturn clientMemoryForType(UInt32 type, IOOptionBits *options, IOMemoryDescriptor **memory);
    
    // External methods
    virtual IOReturn getBAR2MemorySize(UInt32 *dataIn, UInt32 *dataOut, IOByteCount inputCount, IOByteCount *outputCount);
    virtual IOReturn getInterruptsEnabled(UInt32 *dataIn, UInt32 *dataOut, IOByteCount inputCount, IOByteCount *outputCount);
    
};

#endif /* IVSHMEMUserClient_hpp */
