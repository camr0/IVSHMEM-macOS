//
//  IVSHMEMShared.hpp
//  IVSHMEM
//
//  Created by Ali on 9/6/20.
//  Copyright © 2020 Ali. All rights reserved.
//

#ifndef IVSHMEMShared_hpp
#define IVSHMEMShared_hpp

enum {
    kSampleMethod1 = 0,
    kSampleMethod2 = 1,
    kSampleMethod3 = 2,
    kSampleNumMethods
};

// types for IOConnectMapMemory()
enum {
    kSamplePCIMemoryType1 = 100,
    kSamplePCIMemoryType2 = 101,
};

enum {
    // KVM Inter-VM shared memory device register offsets
    IntrMask        = 0x00,     // Interrupt Mask
    IntrStatus      = 0x10,     // Interrupt Status
    Doorbell        = 0x20,     // Doorbell
    ShmOK = 1               // Everything is OK
};

// memory structure to be shared between the kernel and userland.
typedef struct DriverSharedMemory {
    uint32_t    field1;
    uint32_t    field2;
    uint32_t    field3;
    char        string[100];
} DriverSharedMemory;

#endif /* IVSHMEMShared_hpp */
