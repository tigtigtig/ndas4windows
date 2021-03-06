
                                         SpPowerIdleTimeout,
                                         PowerDeviceD3);

    //
    // Initialize WMI support.
    //

    if (commonExtension->WmiInitialized == FALSE) {

        //
        // Build the SCSIPORT WMI registration information buffer for this PDO.
        //

        SpWmiInitializeSpRegInfo(DeviceObject);

        //
        // Register this device object only if the miniport supports WMI and/or
        // SCSIPORT will support certain WMI GUIDs on behalf of the miniport.
        //

        if (commonExtension->WmiMiniPortSupport ||
            commonExtension->WmiScsiPortRegInfoBuf) {

            //
            // Register this physical device object as a WMI data provider,
            // instructing WMI that it is ready to receive WMI IRPs.
            //

            IoWMIRegistrationControl(DeviceObject, WMIREG_ACTION_REGISTER);
            commonExtension->WmiInitialized = TRUE;

        }

        //
        // Allocate several WMI_MINIPORT_REQUEST_ITEM blocks to satisfy a
        // potential onslaught of WMIEvent notifications by the miniport.
        //

        if (commonExtension->WmiMiniPortSupport) {

            //
            // Currently we only allocate two per new SCSI target (PDO).
            //
            SpWmiInitializeFreeRequestList(DeviceObject, 2);
        }
    }

    //
    // Build a device map entry for this logical unit.
    //

    SpBuildDeviceMapEntry(DeviceObject);

    return status;
}

#endif __INTERRUPT__

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        /*++

Copyright (C) Microsoft Corporation, 1990 - 1999

Module Name:

    pnp.c

Abstract:

    This is the NT SCSI port driver.  This file contains the self-contained plug
    and play code.

Authors:

    Peter Wieland

Environment:

    kernel mode only

Notes:

    This module is a driver dll for scsi miniports.

Revision History:

--*/

#include "port.h"

#ifdef __INTERRUPT__

#include <wdmguid.h>

#ifdef __MODULE__
#undef __MODULE__
#endif // __MODULE__
#define __MODULE__ "LSMP_PNP"

#if DBG
static const char *__file__ = __FILE__;
#endif

#define NUM_DEVICE_TYPE_INFO_ENTRIES 18

extern SCSIPORT_DEVICE_TYPE DeviceTypeInfo[];

ULONG SpAdapterStopRemoveSupported = TRUE;

NTSTATUS
SpQueryCapabilities(
    IN PADAPTER_EXTENSION Adapter
    );

PWCHAR
ScsiPortAddGenericControllerId(
    IN PWCHAR IdList
    );

VOID
CopyField(
    IN PUCHAR Destination,
    IN PUCHAR Source,
    IN ULONG Count,
    IN UCHAR Change
    );

NTSTATUS
ScsiPortInitPnpAdapter(
    IN PDEVICE_OBJECT Fdo
    );

NTSTATUS
SpStartLowerDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
SpGetSlotNumber(
    IN PDEVICE_OBJECT Fdo,
    IN PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    IN PCM_RESOURCE_LIST ResourceList
    );

BOOLEAN
SpGetInterrupt(
    IN PCM_RESOURCE_LIST FullResourceList,
    OUT ULONG *Irql,
    OUT ULONG *Vector,
    OUT ULONG *Affinity
    );

//
// Routines start
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ScsiPortAddDevice)
#pragma alloc_text(PAGE, ScsiPortUnload)
#pragma alloc_text(PAGE, ScsiPortFdoPnp)
#pragma alloc_text(PAGE, ScsiPortStartAdapter)
#pragma alloc_text(PAGE, ScsiPortGetDeviceId)
#pragma alloc_text(PAGE, ScsiPortGetInstanceId)
#pragma alloc_text(PAGE, ScsiPortGetHardwareIds)
#pragma alloc_text(PAGE, ScsiPortGetCompatibleIds)
#pragma alloc_text(PAGE, CopyField)
#pragma alloc_text(PAGE, SpFindInitData)
#pragma alloc_text(PAGE, SpStartLowerDevice)
#pragma alloc_text(PAGE, SpGetSlotNumber)
#pragma alloc_text(PAGE, SpGetDeviceTypeInfo)
#pragma alloc_text(PAGE, ScsiPortAddGenericControllerId)
#pragma alloc_text(PAGE, SpQueryCapabilities)
#pragma alloc_text(PAGE, SpGetInterrupt)

#pragma alloc_text(PAGELOCK, ScsiPortInitPnpAdapter)
#endif

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif

SCSIPORT_DEVICE_TYPE DeviceTypeInfo[NUM_DEVICE_TYPE_INFO_ENTRIES] = {
    {"Disk",        "GenDisk",          L"DiskPeripheral",                  TRUE},
    {"Sequential",  "",                 L"TapePeripheral",                  TRUE},
    {"Printer",     "GenPrinter",       L"PrinterPeripheral",               FALSE},
    {"Processor",   "",                 L"OtherPeripheral",                 FALSE},
    {"Worm",        "GenWorm",          L"WormPeripheral",                  TRUE},
    {"CdRom",       "GenCdRom",         L"CdRomPeripheral",                 TRUE},
    {"Scanner",     "GenScanner",       L"ScannerPeripheral",               FALSE},
    {"Optical",     "GenOptical",       L"OpticalDiskPeripheral",           TRUE},
    {"Changer",     "ScsiChanger",      L"MediumChangerPeripheral",         TRUE},
    {"Net",         "ScsiNet",          L"CommunicationsPeripheral",        FALSE},
    {"ASCIT8",      "ScsiASCIT8",       L"ASCPrePressGraphicsPeripheral",   FALSE},
    {"ASCIT8",      "ScsiASCIT8",       L"ASCPrePressGraphicsPeripheral",   FALSE},
    {"Array",       "ScsiArray",        L"ArrayPeripheral",                 FALSE},
    {"Enclosure",   "ScsiEnclosure",    L"EnclosurePeripheral",             FALSE},
    {"RBC",         "ScsiRBC",          L"RBCPeripheral",                   TRUE},
    {"CardReader",  "ScsiCardReader",   L"CardReaderPeripheral",            FALSE},
    {"Bridge",      "ScsiBridge",       L"BridgePeripheral",                FALSE},
    {"Other",       "ScsiOther",        L"OtherPeripheral",                 FALSE}
};

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif


NTSTATUS
ScsiPortAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )

/*++

Routine Description:

    This routine handles add-device requests for the scsi port driver

Arguments:

    DriverObject - a pointer to the driver object for this device

    PhysicalDeviceObject - a pointer to the PDO we are being added to

Return Value:

    STATUS_SUCCESS

--*/

{
    PSCSIPORT_DRIVER_EXTENSION driverExtension;
    PDEVICE_OBJECT newFdo;

    NTSTATUS status;

    PAGED_CODE();

    status = SpCreateAdapter(DriverObject, &newFdo);

    if(newFdo != NULL) {

        PADAPTER_EXTENSION adapter;
        PCOMMON_EXTENSION commonExtension;

        PDEVICE_OBJECT newStack;

        adapter = newFdo->DeviceExtension;
        commonExtension = &(adapter->CommonExtension);

        adapter->IsMiniportDetected = FALSE;
        adapter->IsPnp = TRUE;

        driverExtension = IoGetDriverObjectExtension(DriverObject, 
                                                     ScsiPortInitialize);

        switch(driverExtension->BusType) {
#if 0
            case BusTypeFibre: {
                adapter->DisablePower = TRUE;
                adapter->DisableStop = TRUE;
                break;
            }
#endif

            default: {
                adapter->DisablePower = FALSE;
                adapter->DisableStop = FALSE;
                break;
            }
        }

        newStack = IoAttachDeviceToDeviceStack(newFdo, PhysicalDeviceObject);

        adapter->CommonExtension.LowerDeviceObject = newStack;
        adapter->LowerPdo = PhysicalDeviceObject;

        if(newStack == NULL) {

            status =  STATUS_UNSUCCESSFUL;

        } else {

            status =  STATUS_SUCCESS;
        }
    }

    return status;
}


VOID
ScsiPortUnload(
    IN PDRIVER_OBJECT DriverObject
    )

/*++

Routine Description:

    This routine will shut down all device objects for this miniport and
    clean up all allocated resources

Arguments:

    DriverObject - the driver being unloaded

Return Value:

    none

--*/

{
    PAGED_CODE();
    return;
}


NTSTATUS
ScsiPortFdoPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PADAPTER_EXTENSION adapter = DeviceObject->DeviceExtension;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

    NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;

    ULONG isRemoved;

    BOOLEAN sendDown = TRUE;

    PAGED_CODE();

    isRemoved = SpAcquireRemoveLock(DeviceObject, Irp);

    switch(irpStack->MinorFunction) {

        case IRP_MN_QUERY_PNP_DEVICE_STATE: {

            //
            // If the device is in the paging path then mark it as
            // not-disableable.
            //

            PPNP_DEVICE_STATE deviceState;
            deviceState = (PPNP_DEVICE_STATE) &(Irp->IoStatus.Information);

            DebugPrint((1, "QUERY_DEVICE_STATE for FDO %#x\n",
                        DeviceObject));

            *deviceState = adapter->DeviceState;

            if(commonExtension->PagingPathCount != 0) {
                SET_FLAG((*deviceState), PNP_DEVICE_NOT_DISABLEABLE);
                DebugPrint((1, "QUERY_DEVICE_STATE: %#x - not disableable\n",
                            DeviceObject));
            }

            SpReleaseRemoveLock(DeviceObject, Irp);

            Irp->IoStatus.Status = STATUS_SUCCESS;

            IoCopyCurrentIrpStackLocationToNext(Irp);
            return IoCallDriver(commonExtension->LowerDeviceObject, Irp);
        }

        case IRP_MN_START_DEVICE: {

            PSCSIPORT_DRIVER_EXTENSION driverExtension =
                IoGetDriverObjectExtension(DeviceObject->DriverObject,
                                           ScsiPortInitialize);

            PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
            PCM_RESOURCE_LIST allocatedResources =
                irpStack->Parameters.StartDevice.AllocatedResources;
            PCM_RESOURCE_LIST translatedResources =
                irpStack->Parameters.StartDevice.AllocatedResourcesTranslated;
            ULONG interfaceFlags;

            sendDown = FALSE;

            //
            // Make sure this device was created by an add rather than the
            // one that was found by port or miniport.
            //

            if(adapter->IsPnp == FALSE) {

                DebugPrint((1, "ScsiPortFdoPnp - asked to start non-pnp "
                               "adapter\n"));
                status = STATUS_UNSUCCESSFUL;
                break;
            }

            if(commonExtension->CurrentPnpState == IRP_MN_START_DEVICE) {

                DebugPrint((1, "ScsiPortFdoPnp - already started - nothing "
                               "to do\n"));
                status = STATUS_SUCCESS;
                break;
            }

            //
            // Now make sure that pnp handed us some resources.  It may not
            // if this is a phantom of a PCI device which we reported on a
            // previous boot.  In that case pnp thinks we'll allocate the
            // resources ourselves.
            //

            if(allocatedResources == NULL) {

                //
                // This happens with reported devices when PCI mvoes them from
                // boot to boot.
                //

                Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
                break;
            }

            ASSERT(allocatedResources);
            ASSERT(allocatedResources->Count);

            //
            // Make sure that adapters with this interface type can be
            // initialized as pnp drivers.
            //

            interfaceFlags = SpQueryPnpInterfaceFlags(
                                driverExtension,
                                allocatedResources->List[0].InterfaceType);

            if(interfaceFlags == SP_PNP_NOT_SAFE) {

                //
                // Nope - not safe.  We cannot start this device so return
                // failure.
                //

                DebugPrint((0, "ScsiPortFdoPnp - Miniport cannot be run in "
                               "pnp mode for interface type %#08lx\n",
                            allocatedResources->List[0].InterfaceType));

                //
                // Mark the device as not being pnp - that way we won't get
                // removed.
                //

                adapter->IsPnp = FALSE;

                status = STATUS_UNSUCCESSFUL;
                break;
            }

            //
            // Check to see if this interface requires slot/function numbers.
            // If not then zero out the virtual slot number.
            //

            if(!TEST_FLAG(interfaceFlags, SP_PNP_NEEDS_LOCATION)) {
                adapter->VirtualSlotNumber.u.AsULONG = 0;
            }


            //
            // Determine if we should have found this device during
            // our detection scan.  We do this by checking to see if the pdo
            // has a pnp bus type.  If not and the detection flag is set then
            // assume duplicate detection has failed and don't start this
            // device.
            //

            {
                status = SpGetBusTypeGuid(adapter);

                if((status == STATUS_OBJECT_NAME_NOT_FOUND) &&
                   ((driverExtension->LegacyAdapterDetection == TRUE) &&
                    (interfaceFlags & SP_PNP_NON_ENUMERABLE))) {

                    DbgPrint("ScsiPortFdoPnp: device has no pnp bus type but "
                             "was not found as a duplicate during "
                             "detection\n");

                    status = STATUS_UNSUCCESSFUL;

                    //
                    // make sure this one doesn't get removed though - if it's
                    // removed then the resources may be disabled for the
                    // ghost.
                    //

                    adapter->IsPnp = FALSE;

                    break;
                }
            }

            //
            // Finally, if this is a PCI adapter make sure we were given 
            // an interrupt.  The current assumption is that there aren't 
            // any polled-mode PCI SCSI adapters in the market.
            //

            if(TEST_FLAG(interfaceFlags, SP_PNP_INTERRUPT_REQUIRED)) {

                ULONG irql, vector, affinity;

                if(SpGetInterrupt(allocatedResources,
                                  &irql,
                                  &vector,
                                  &affinity) == FALSE) {

                    PIO_ERROR_LOG_PACKET error = 
                        IoAllocateErrorLogEntry(DeviceObject, 
                                                sizeof(IO_ERROR_LOG_PACKET));

                    status = STATUS_DEVICE_CONFIGURATION_ERROR;

                    if(error != NULL) {
                        error->MajorFunctionCode = IRP_MJ_PNP;
                        error->UniqueErrorValue = 0x418;
                        error->ErrorCode = IO_ERR_INCORRECT_IRQL;
                        IoWriteErrorLogEntry(error);
                    }
                    break;
                }
            }

            status = SpStartLowerDevice(DeviceObject, Irp);

            if(NT_SUCCESS(status)) {

                //
                // If we haven't allocated a HwDeviceExtension for this thing 
                // yet then we'll need to set it up
                //

                if(commonExtension->IsInitialized == FALSE) {

                    DebugPrint((1, "ScsiPortFdoPnp - find and init adapter %#p\n",
                                   DeviceObject));

                    if(allocatedResources == NULL) {
                        status = STATUS_INVALID_PARAMETER;
                    } else {

                        adapter->AllocatedResources =
                            RtlDuplicateCmResourceList(
                                NonPagedPool,
                                allocatedResources,
                                SCSIPORT_TAG_RESOURCE_LIST);

                        adapter->TranslatedResources =
                            RtlDuplicateCmResourceList(
                                NonPagedPool,
                                translatedResources,
                                SCSIPORT_TAG_RESOURCE_LIST);

                        commonExtension->IsInitialized = TRUE;

                        status = ScsiPortInitPnpAdapter(DeviceObject);
                    }

                    if(!NT_SUCCESS(status)) {

                        DebugPrint((1, "ScsiPortInitializeAdapter failed "
                                       "%#08lx\n", status));
                        break;
      