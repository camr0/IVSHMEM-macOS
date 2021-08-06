#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/pci/IOPCIDevice.h>

#if IOMEMORYDESCRIPTOR_SUPPORTS_DMACOMMAND
#include <IOKit/IODMACommand.h>
#endif

#include "IVSHMEM.hpp"
#include <IOKit/IOLib.h>
#include <IOKit/assert.h>

// This required macro defines the class's constructors, destructors, and several other methods I/O Kit requires.
OSDefineMetaClassAndStructors(IVSHMEMDevice, IOService)

// Define the driver's superclass.
#define super IOService

bool IVSHMEMDevice::init(OSDictionary *dictionary)
{
    bool result = super::init(dictionary);
    IOLog("Initializing...\n");
    return result;
}

void IVSHMEMDevice::free(void)
{
    IOLog("Freeing...\n");
    super::free();
}


IOService *IVSHMEMDevice::probe(IOService *provider, SInt32 *score)
{
    IOService *result = super::probe(provider, score);
    IOLog("Probing...\n");
    return result;
}


bool IVSHMEMDevice::start(IOService *provider)
{
    IOLog("Starting...\n");
    
    IOMemoryDescriptor    *mem;
    IOMemoryMap           *map;
    
    IOLog("%s[%p]::%s(%p)\n", getName(), this, __FUNCTION__, provider);
    
    if (!super::start(provider))
        return false;
    
    /*
     * Our provider class is specified in the driver property table
     * as IOPCIDevice, so the provider must be of that class.
     * The assert is just to make absolutely sure for debugging.
     */
    
    assert(OSDynamicCast(IOPCIDevice, provider));
    fPCIDevice = (IOPCIDevice *) provider;
    
    /*
     * Enable memory response from the card
     */
    fPCIDevice->setMemoryEnable(true);
    
    /*
     * Log some info about the device
     */
    
    /* Print all the device's memory ranges */
    for ( uint32_t index = 0; index < fPCIDevice->getDeviceMemoryCount(); index++ ) {
        
        mem = fPCIDevice->getDeviceMemoryWithIndex( index );
        assert( mem );
        IOLog("Range[%d] " PhysAddr_FORMAT ":" ByteCount_FORMAT "\n", index,
             mem->getPhysicalAddress(), mem->getLength());
    }
    
    /* look up a range based on its config space base address register */
    mem = fPCIDevice->getDeviceMemoryWithRegister(
                                                  kIOPCIConfigBaseAddress0 );
    if ( mem )
        IOLog("BAR0 Range@0x%x " PhysAddr_FORMAT ":" ByteCount_FORMAT "\n", kIOPCIConfigBaseAddress0,
             mem->getPhysicalAddress(), mem->getLength());
    
    /* look up a range based on its config space base address register */
    IOMemoryDescriptor *mem1 = fPCIDevice->getDeviceMemoryWithRegister(
                                                  kIOPCIConfigBaseAddress1 );
    if ( mem1 )
        IOLog("BAR1 Range@0x%x " PhysAddr_FORMAT ":" ByteCount_FORMAT "\n", kIOPCIConfigBaseAddress1,
             mem1->getPhysicalAddress(), mem1->getLength());
    
    /* Map a range based on its config space base address register,
     * This is how the driver gets access to its memory-mapped registers.
     * The getVirtualAddress() method returns a kernel virtual address
     * for the register mapping */
    
    map = fPCIDevice->mapDeviceMemoryWithRegister(
                                                  kIOPCIConfigBaseAddress0 );
    if ( map ) {
        IOLog("BAR0 Range@0x%x (" PhysAddr_FORMAT ") mapped to kernel virtual address " VirtAddr_FORMAT "\n",
              kIOPCIConfigBaseAddress0,
              map->getPhysicalAddress(),
              map->getVirtualAddress()
              );
        
        /* Release the map object, and the mapping itself */
        map->release();
    }
    
    /* Read a config space register */
    IOLog("Config register@0x%x = " UInt32_FORMAT "\n", kIOPCIConfigCommand,
            fPCIDevice->configRead32(kIOPCIConfigCommand) );
    IOLog("Config register@0x%x(status register) = " UInt32_FORMAT "\n", kIOPCIConfigStatus,
    fPCIDevice->configRead32(kIOPCIConfigStatus) );
    
//    // Construct a memory descriptor for a buffer below the 4Gb physical line &
//    // so addressable by 32-bit DMA. This could be used for a
//    // DMA program buffer, for example.
//
//    IOBufferMemoryDescriptor *bmd = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(
//                                                         // task to hold the memory
//                                                         kernel_task,
//                                                         // options
//                                                         kIOMemoryPhysicallyContiguous,
//                                                         // size
//                                                         64*1024,
//                                                         // physicalMask - 32 bit addressable and page aligned
//                                                         0x00000000FFFFF000ULL);
//
//    if (bmd) {
//        generateDMAAddresses(bmd);
//    } else {
//        IOLog("IOBufferMemoryDescriptor::inTaskWithPhysicalMask failed\n");
//    }
//    fLowMemory = bmd;
    
    /* Publish ourselves so clients can find us */
    registerService();
    
    return true;
}


void IVSHMEMDevice::stop(IOService *provider)
{
    IOLog("Stopping...\n");
    IOLog("%s[%p]::%s(%p)\n", getName(), this, __FUNCTION__, provider);
    super::stop(provider);
}

/*
 * Method to supply an IOMemoryDescriptor for the user client to map into
 * the client process. This sample just supplies all of the hardware memory
 * associated with the PCI device's Base Address Register 2 (host shared memory object).
 */

IOMemoryDescriptor * IVSHMEMDevice::copyGlobalMemory(void)
{
    IOMemoryDescriptor *memory;
    
    memory = fPCIDevice->getDeviceMemoryWithRegister(kIOPCIConfigBaseAddress2);
    if(memory)
        memory->retain();
    
    return memory;
}
