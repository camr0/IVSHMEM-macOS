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
//#include <IOKit/IOMemoryDescriptor.h>
//#include <IOKit/IOBufferMemoryDescriptor.h>
#include <CoreFoundation/CoreFoundation.h>
//#include libkern

#include "IVSHMEMShared.hpp"

void TestUserClient( io_service_t service );
void TestSharedMemory( io_connect_t connect );
 
#define arrayCnt(var) (sizeof(var) / sizeof(var[0]))

void TestUserClient( io_service_t service )
{
    kern_return_t                kr;
    io_connect_t                connect;
    size_t                        structureOutputSize;
//    SampleStructForMethod2        method2Param;
//    SampleResultsForMethod2        method2Results;
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

//    // test shared memory
//    TestSharedMemory( connect );
//
//    // test method with out of line memory.
//    // Use anonymous mmap to ensure we get a single VM object.
//    bigBufferLen = 0x4321;
//    bigBuffer = (uint32_t *) mmap(NULL, bigBufferLen, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
//    if (bigBuffer == MAP_FAILED) {
//        perror("mmap() call error:");
//        return;
//    }
//
//    printf("buffer is created @ %p\n", bigBuffer);
//
//    strcpy( (char *) (bigBuffer + (32 / 4)), "some out of line data");

//    method2Param.parameter1   = 0x12345678;
//    method2Param.data_pointer = (uintptr_t) bigBuffer;
//    method2Param.data_length  = bigBufferLen;
//
//    structureOutputSize = sizeof(method2Results);
//
//    kr = IOConnectCallStructMethod( connect, kSampleMethod2,
//                                    inputStructure
//                                   &method2Param, sizeof(method2Param),
//                                    ouputStructure
//                                   &method2Results, &structureOutputSize );

//    assert( KERN_SUCCESS == kr );
//    printf("kSampleMethod2 result 0x%" PRIx64 "\n", method2Results.results1);

    munmap( bigBuffer, bigBufferLen );
}

void TestSharedMemory( io_service_t service )
{
    kern_return_t        kr;
    io_connect_t         connect = 0;
    DriverSharedMemory   *shared;

    #if __LP64__
        mach_vm_address_t     addr;
        mach_vm_size_t        size;
    #else
        vm_address_t     addr;
        vm_size_t        size;
    #endif

    kr = IOServiceOpen(service, mach_task_self(), 0, &connect);
    if (kr != KERN_SUCCESS) {
        printf("error: IOServiceOpen returned 0xx\n", kr);
    }
    
    kr = IOConnectMapMemory( connect, kBAR2MemoryType,
                            mach_task_self(), &addr, &size,
                            kIOMapAnywhere | kIOMapDefaultCache );
//    assert( KERN_SUCCESS == kr );
//    assert( size == sizeof( DriverSharedMemory ));

    shared = (DriverSharedMemory *) addr;

    printf("From DriverSharedMemory: %u, %s\n", shared->buildNumber, shared->message);
    
    printf("Writing Memory...\n");
    shared->buildNumber = 4;
    strcpy( shared->message, "Hello World from a memory map!" );
    
    printf("From DriverSharedMemory: %u, %s\n", shared->buildNumber, shared->message);
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

//void TestRead( io_service_t service) {
//    kern_return_t                kr;
//    io_connect_t                connect;
//    size_t                        structureOutputSize;
////    SampleStructForMethod2        method2Param;
////    SampleResultsForMethod2        method2Results;
//    char                    message[] = "Hello World!";
//    //TODO: CHANGE OUTPUT TO EXIT CODE (UINT PREFERRABLY) 0 FOR SUCCESS, ETC.
////    IOByteCount                    bigBufferLen;
////    uint32_t *                    bigBuffer;
//
//    // connecting to driver
//    //    printf("CONNECTING TO DRIVER\n");
//    kr = IOServiceOpen( service, mach_task_self(), kSamplePCIConnectType, &connect );
//    assert( KERN_SUCCESS == kr );
//
//    // test a simple struct in/out method
//    //TODO: OUTPUT SIZE SET TO THE SIZE OF THE HARDCORDED TEST INPUT
////    int len = sizeof(message)/sizeof(message[0]);
//    char messageReturned[strlen(message)];
//    structureOutputSize = strlen(message);
//
//    kr = IOConnectCallStructMethod( connect, readMemoryMethod,
//                                   // inputStructure
//                                   &message, strlen(message),
//                                   // ouputStructure
//                                   &messageReturned, &structureOutputSize );
//    assert( KERN_SUCCESS == kr );
//    printf("Reading Memory: %s\n", messageReturned);
//
////
////    kr = IOConnectCallStructMethod( connect, writeMemoryMethod,
////                                   // inputStructure
////                                   &message, sizeof(message),
////                                   // ouputStructure
////                                   &messageReturned, sizeof(messageReturned)); // &structureOutputSize giving an error?
////    assert( KERN_SUCCESS == kr );
////    printf("Writing Memory...\n");
//}
//
//void TestWrite( io_service_t service, char messageIn[]) {
//    kern_return_t                kr;
//    io_connect_t                connect;
//    size_t                        structureOutputSize;
////    SampleStructForMethod2        method2Param;
////    SampleResultsForMethod2        method2Results;
//    char                    message[strlen(messageIn)];
//    strcpy(message, messageIn);
//    //TODO: CHANGE OUTPUT TO EXIT CODE (UINT PREFERRABLY) 0 FOR SUCCESS, ETC.
////    IOByteCount                    bigBufferLen;
////    uint32_t *                    bigBuffer;
//
//    // connecting to driver
//    //    printf("CONNECTING TO DRIVER\n");
//    kr = IOServiceOpen( service, mach_task_self(), kSamplePCIConnectType, &connect );
//    assert( KERN_SUCCESS == kr );
//
//    // test a simple struct in/out method
//    //TODO: OUTPUT SIZE SET TO THE SIZE OF THE HARDCORDED TEST INPUT
////    int len = sizeof(message)/sizeof(message[0]);
////    printf("izeof(message): %llu -- sizeof(message[0]): %llu\n", sizeof(message), sizeof(message[0]));
////    printf("strlen(message): %llu\n", strlen(message));
////    printf("sizeof(message[0]): %llu\n", sizeof(&message[0]));
//
//    char messageReturned[strlen(message)];
//    structureOutputSize = strlen(message);
//
//    kr = IOConnectCallStructMethod( connect, writeMemoryMethod,
//                                   // inputStructure
//                                   &message, strlen(message),
//                                   // ouputStructure
//                                   &messageReturned, &structureOutputSize);
//    assert( KERN_SUCCESS == kr );
//    printf("Writing Memory...\n");
//}



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
        
        // Test getting and setting properties
//        TestProperties( service );
        
        // Test the user client
        TestUserClient( service );
        
        TestSharedMemorySize(service);
        TestInterruptStatus(service);
        
        TestSharedMemory(service);
//        TestRead(service);
//        char message[] = "Hello World!";
//        TestWrite(service, message);
//        TestRead(service);
        
    }
    IOObjectRelease(iter);
	
    return EXIT_SUCCESS;
}
