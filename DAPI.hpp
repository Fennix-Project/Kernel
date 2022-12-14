#ifndef __FENNIX_DRIVER_API_H__
#define __FENNIX_DRIVER_API_H__

/**
 * @brief The driver API is a set of functions that the kernel provides to the drivers.
 *
 * - The driver is responsible for the memory management.
 * - The kernel will NOT free any memory allocated by the driver. On @see StopReason the driver must free all the memory it allocated and disable the hardware it uses.
 * - The driver image will be freed after the driver is unloaded.
 * - The kernel will unbind the interrupt handlers and the process handlers.
 * - Kernel API will be freed after the driver is unloaded.
 *
 */

enum DriverReturnCode
{
    ERROR,
    OK,
    NOT_IMPLEMENTED,
    NOT_FOUND,
    NOT_READY,
    NOT_AVAILABLE,
    NOT_AUTHORIZED,
    NOT_VALID,
    NOT_ACCEPTED,
    INVALID_KERNEL_API,
    DEVICE_NOT_SUPPORTED,
    SYSTEM_NOT_SUPPORTED,
    KERNEL_API_VERSION_NOT_SUPPORTED
};

enum DriverBindType
{
    BIND_NULL,
    BIND_INTERRUPT,
    BIND_PROCESS,
    BIND_PCI,
    BIND_INPUT
};

struct KernelAPI
{
    struct KAPIVersion
    {
        int Major;
        int Minor;
        int Patch;
    } Version;

    struct KAPIInfo
    {
        unsigned long Offset;
        unsigned long DriverUID;
    } Info;

    struct KAPIMemory
    {
        unsigned long PageSize;
        void *(*RequestPage)(unsigned long Size);
        void (*FreePage)(void *Page, unsigned long Size);
        void (*Map)(void *VirtualAddress, void *PhysicalAddress, unsigned long Flags);
        void (*Unmap)(void *VirtualAddress);
    } Memory;

    struct KAPIPCI
    {
        char *(*GetDeviceName)(unsigned int VendorID, unsigned int DeviceID);
    } PCI;

    struct KAPIUtilities
    {
        void (*DebugPrint)(char *String, unsigned long DriverUID);
        void (*DisplayPrint)(char *Value);
        void *(*memcpy)(void *Destination, void *Source, unsigned long Size);
        void *(*memset)(void *Destination, int Value, unsigned long Size);
    } Util;

    struct KAPIDriverTalk
    {
        /** @brief Connects to the network manager */
        struct
        {
            void (*SendPacket)(unsigned int DriverID, unsigned char *Data, unsigned short Size);
            void (*ReceivePacket)(unsigned int DriverID, unsigned char *Data, unsigned short Size);
        } Network;

        /** @brief Connects to the disk manager */
        struct
        {
            struct
            {
                void (*ReadSector)(unsigned int DriverID, unsigned long Sector, unsigned char *Data, unsigned int SectorCount, unsigned char Port);
                void (*WriteSector)(unsigned int DriverID, unsigned long Sector, unsigned char *Data, unsigned int SectorCount, unsigned char Port);
            } AHCI;
        } Disk;
    } Command;

} __attribute__((packed));

enum CallbackReason
{
    UnknownReason,
    AcknowledgeReason,
    SendReason,
    ReceiveReason,
    ConfigurationReason,
    FetchReason,
    StopReason,
    BindReason,
    UnbindReason,
    InterruptReason,
    ProcessReason,
    InputReason,
};

struct KernelCallback
{
    CallbackReason Reason;
    void *RawPtr;
    unsigned long RawData;

    /** @brief When the kernel wants to send a packet. */
    struct
    {
        struct
        {
            unsigned char *Data;
            unsigned long Length;
        } Send;

        struct
        {
            char Name[128];
            unsigned long MAC;
        } Fetch;
    } NetworkCallback;

    /** @brief When the kernel wants to write to disk. */
    struct
    {
        struct
        {
            unsigned long Sector;
            unsigned long SectorCount;
            unsigned char Port;
            unsigned char *Buffer;
            bool Write;
        } RW;

        struct
        {
            unsigned char Ports;
            int BytesPerSector;
        } Fetch;
    } DiskCallback;

    struct
    {
        struct
        {
            unsigned long X;
            unsigned long Y;
            unsigned long Z;
            struct
            {
                bool Left;
                bool Right;
                bool Middle;
            } Buttons;
        } Mouse;
    } InputCallback;

    struct
    {
        unsigned char Vector;
    } InterruptInfo;
} __attribute__((packed));

#endif // !__FENNIX_DRIVER_API_H__
