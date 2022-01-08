//
//  main.m
//  IVSHMEM Client
//
//  Created by Ali on 9/7/20.
//  Copyright © 2020 Ali. All rights reserved.
//

#include <AvailabilityMacros.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>

#include "IVSHMEMShared.hpp"


void TestUserClient( io_service_t service );
void TestSharedMemory( io_connect_t connect );
 
#define arrayCnt(var) (sizeof(var) / sizeof(var[0]))

void TestUserClient( io_service_t service )
{
    kern_return_t                kr;
    io_connect_t                connect;
    size_t                        structureOutputSize;
    uint32_t                    varStructParam[3] = { 1, 2, 3 };
    IOByteCount                    bigBufferLen;
    uint32_t *                    bigBuffer;

    // connecting to driver
//    printf("CONNECTING TO DRIVER\n");
    kr = IOServiceOpen( service, mach_task_self(), kSamplePCIConnectType, &connect );
    assert( KERN_SUCCESS == kr );

    // test a simple struct in/out method
    structureOutputSize = sizeof(varStructParam);

    kr = IOConnectCallStructMethod( connect, kSampleMethod1,
                                   // inputStructure
                                   &varStructParam, sizeof(varStructParam),
                                   // ouputStructure
                                   &varStructParam, &structureOutputSize );

    assert( KERN_SUCCESS == kr );
    printf("kSampleMethod1 results 0x%08" PRIx32 ", 0x%08" PRIx32 ", 0x%08" PRIx32 "\n",
           varStructParam[0], varStructParam[1], varStructParam[2]);

    munmap( bigBuffer, bigBufferLen );
}

void TestSharedMemory( io_service_t service )
{
    kern_return_t        kr;
    io_connect_t         connect = 0;
//    DriverSharedMemory   *shared;

    #if __LP64__
        mach_vm_address_t     addr;
        mach_vm_size_t        size;
    #else
        vm_address_t     addr;
        vm_size_t        size;
    #endif

    kr = IOServiceOpen(service, mach_task_self(), 0, &connect);
    if (kr != KERN_SUCCESS) {
        printf("error: IOServiceOpen returned 0xx: %i\n", kr);
    }
    
    kr = IOConnectMapMemory( connect, kBAR2MemoryType,
                            mach_task_self(), &addr, &size,
                            kIOMapAnywhere | kIOMapDefaultCache );
//    assert( KERN_SUCCESS == kr );
//    assert( size == sizeof( DriverSharedMemory ));

//    shared = (DriverSharedMemory *) addr;
    char *shared = (char *) addr;
    printf("From DriverSharedMemory: %s\n", shared);
    printf("Writing Memory...\n");
    strcpy( shared, "Hello World from a memory map?" );
    printf("From DriverSharedMemory: %s\n", shared);

//    printf("From DriverSharedMemory: %u, %s\n", shared->buildNumber, shared->message);
    
//    printf("Writing Memory...\n");
//    shared->buildNumber = 1;
//    strcpy( shared->message, "Hello World from a memory map?" );
    
//    printf("From DriverSharedMemory: %u, %s\n", shared->buildNumber, shared->message);
}

void TestSharedMemorySize( io_service_t service )
{
    kern_return_t                kr;
    io_connect_t                connect;
    size_t                        structureOutputSize;
//    SampleStructForMethod2        method2Param;
//    SampleResultsForMethod2        method2Results;
    uint32_t                    memorySize;
    IOByteCount                    bigBufferLen;
    uint32_t *                    bigBuffer;

    // connecting to driver
//    printf("CONNECTING TO DRIVER\n");
    kr = IOServiceOpen( service, mach_task_self(), kSamplePCIConnectType, &connect );
    assert( KERN_SUCCESS == kr );

    // test a simple struct in/out method
    structureOutputSize = sizeof(memorySize);

    kr = IOConnectCallStructMethod( connect, getMemorySizeMethod,
                                   // inputStructure
                                   &memorySize, sizeof(memorySize),
                                   // ouputStructure
                                   &memorySize, &structureOutputSize );

    assert( KERN_SUCCESS == kr );
    printf("Shared Memory Size: %i bytes (%i MB)\n", memorySize, memorySize / 1048576);
}

void TestInterruptStatus( io_service_t service )
{
    kern_return_t                kr;
    io_connect_t                connect;
    size_t                        structureOutputSize;
//    SampleStructForMethod2        method2Param;
//    SampleResultsForMethod2        method2Results;
    uint32_t                    interruptsEnabled;
    IOByteCount                    bigBufferLen;
    uint32_t *                    bigBuffer;

    // connecting to driver
//    printf("CONNECTING TO DRIVER\n");
    kr = IOServiceOpen( service, mach_task_self(), kSamplePCIConnectType, &connect );
    assert( KERN_SUCCESS == kr );

    // test a simple struct in/out method
    structureOutputSize = sizeof(interruptsEnabled);

    kr = IOConnectCallStructMethod( connect, getInterruptsEnabledMethod,
                                   // inputStructure
                                   &interruptsEnabled, sizeof(interruptsEnabled),
                                   // ouputStructure
                                   &interruptsEnabled, &structureOutputSize );

    assert( KERN_SUCCESS == kr );
    printf("Interrupts Enabled: %i\n", interruptsEnabled);
}


int main(int argc, const char * argv[]) {
    io_iterator_t            iter;
    io_service_t            service;
    kern_return_t            kr;
    bool                    driverFound = false;
    
    // Look up the object we wish to open. This example uses simple class
    // matching (IOServiceMatching()) to look up the object that is the
    // SamplePCI driver class instantiated by the kext.
    //
    // From the userland perspective, a process must look for any of the class names it is prepared to talk
    // to.
    
    kr = IOServiceGetMatchingServices( kIOMasterPortDefault,
                                       IOServiceMatching( "IVSHMEMDevice" ), &iter);
    assert( KERN_SUCCESS == kr );
    
    for( ;
        (service = IOIteratorNext(iter));
        IOObjectRelease(service)) {
        io_string_t path;
        kr = IORegistryEntryGetPath(service, kIOServicePlane, path);
        assert( KERN_SUCCESS == kr );
        
        driverFound = true;
        printf("Found a device of class 'IVSHMEMDevice': %s\n", path);
        
        // Test the user client
        TestUserClient( service );
        
        TestSharedMemorySize(service);
        TestInterruptStatus(service);
        
        TestSharedMemory(service);
    }
    IOObjectRelease(iter);
	
    return EXIT_SUCCESS;
}
