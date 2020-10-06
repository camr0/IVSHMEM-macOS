#include <IOKit/IOService.h>
#include "IVSHMEMShared.hpp"

#define UInt32_FORMAT        "%u"
#define UInt32_x_FORMAT      "0x%08x"
//#define UInt64_FORMAT        "%llu"
#define PhysAddr_FORMAT      "0x%016llx"
#define PhysLen_FORMAT       "%llu"
#define VirtAddr_FORMAT      "0x%016llx"
#define ByteCount_FORMAT     "%llu"

// Forward declarations
class IOPCIDevice;
class IOMemoryDescriptor;

class IVSHMEMDevice : public IOService {
    
    OSDeclareDefaultStructors(IVSHMEMDevice)

private:
    IOPCIDevice             *fPCIDevice;
//    IOMemoryDescriptor      *fLowMemory;
    
public:
    // IOService overrides
    virtual bool init(OSDictionary *dictionary = 0);
    virtual void free(void);
    virtual IOService *probe(IOService *provider, SInt32 *score);
    virtual bool start(IOService *provider);
    virtual void stop(IOService *provider);
//    virtual IOReturn setProperties(OSObject *properties);
    
    // Other methods
    IOMemoryDescriptor* copyGlobalMemory(void);
//    IOReturn generateDMAAddresses(IOMemoryDescriptor *memDesc);
//    void updateRegistry(UInt32 value);
};
