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
    kSampleNumMethods,
    
    getMemorySizeMethod = 3,
    getInterruptsEnabledMethod = 4
};

// types f	or IOServiceOpen()
enum {
    kSamplePCIConnectType = 23
};

// types for IOConnectMapMemory()
enum {
    kSamplePCIMemoryType1 = 100,
    kBAR2MemoryType = 101,
};

//enum {
//    // KVM Inter-VM shared memory device register offsets
//    intrMaskOffset        = 0x00,     // Interrupt Mask
//    intrStatusOffset      = 0x10,     // Interrupt Status
//    IVPositionOffset      = 0x08,     // VM ID
//    doorbellOffset        = 0x20,     // Doorbell
////    ShmOK = 1               // Everything is OK
//};

// https://fossies.org/linux/qemu/docs/specs/ivshmem-spec.txt
// "There is NO GOOD WAY for software to find out whether the device is configured for interrupts."
// workaround found in IVSHMEM.cpp (checking for BAR1)
//enum {
//    /* KVM Inter-VM shared memory device register offsets */
//    IntrMaskOffset        = 0x00,    /* Interrupt Mask */
//    IntrStatusOffset      = 0x04,    /* Interrupt Status */
//    IVPositionOffset      = 0x08,    /* VM ID */
//    DoorbellOffset        = 0x0c,    /* Doorbell */
//};

// memory structure to be shared between the kernel and userland.
typedef struct DriverSharedMemory {
    uint32_t    field1;
    uint32_t    field2;
    uint32_t    field3;
    char        string[100];
} DriverSharedMemory;

//typedef struct IVSHMEMRegisters {
//    uint32_t    intrMask;
//    uint32_t    intrStatus;
//    uint32_t    IVPosition // (VM ID 0 if )
//} DeviceRegisters;

#endif /* IVSHMEMShared_hpp */
