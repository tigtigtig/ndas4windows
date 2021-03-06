#include <pch.cxx>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include "error.hxx"
#include "path.hxx"
#include "ifssys.hxx"
#include "filter.hxx"
#include "system.hxx"
#include "dir.hxx"
#include "rcache.hxx"
#ifdef DBLSPACE_ENABLED
#include "dblentry.hxx"
#endif // DBLSPACE_ENABLED

#ifndef __in
#	define __in
#endif

#ifndef __out
#	define __out
#endif

#ifndef __inout
#	define __inout
#endif

extern "C" {
    #include "nturtl.h"
	#include <ndas/ndasvol.h>
	#include <ndas/ndasuser.h>

}

#include "message.hxx"
#include "rtmsg.h"
#include "ifsserv.hxx"
#include "supera.hxx"




BOOLEAN
FAR APIENTRY
Chkdsk(
    IN      PCWSTRING   NtDriveName,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     Fix,
    IN      BOOLEAN     Verbose,
    IN      BOOLEAN     OnlyIfDirty,
    IN      BOOLEAN     Recover,
    IN      PPATH       PathToCheck,
    IN      BOOLEAN     Extend,
    IN      BOOLEAN     ResizeLogFile,
    IN      ULONG       LogFileSize,
    IN      PULONG      ExitStatus
   )
{

	PXIXFS_VOL	XixFsVol;
	DWORD		oldErrorMode;
	ULONG		flags;
	BOOLEAN		result = FALSE;

    if (Extend || ResizeLogFile || (LogFileSize != 0)) {

        *ExitStatus = CHKDSK_EXIT_COULD_NOT_CHK;
        return FALSE;

    }

	oldErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS );

	if( !(XixFsVol = NEW XIXFS_VOL)) {
		Message->Set( MSG_FMT_NO_MEMORY );
        Message->Display( "" );
        return FALSE;		
	}
	
	
	if(	!(XixFsVol->Initialize(NtDriveName, Message)) ) {
		DELETE(XixFsVol);
		SetErrorMode(oldErrorMode);
		*ExitStatus = CHKDSK_EXIT_COULD_NOT_CHK;
		return FALSE;
	}

	SetErrorMode(oldErrorMode);

	flags = (Verbose ? CHKDSK_VERBOSE:0);
	flags |= (OnlyIfDirty ? CHKDSK_CHECK_IF_DIRTY:0);
	flags |= (Recover? (CHKDSK_RECOVER_FREE_SPACE| CHKDSK_RECOVER_ALLOC_SPACE) : 0 );

	if(Fix) {
		if( !XixFsVol->Lock()) {
			DELETE(XixFsVol);
            *ExitStatus = CHKDSK_EXIT_COULD_NOT_CHK;
            return FALSE;
		}
	}

	result = XixFsVol->ChkDsk(	
						(Fix?TotalFix:CheckOnly),
						Message,
						flags
						); 
	

	DELETE(XixFsVol);
	
    return result;
}


BOOLEAN
FAR APIENTRY
ChkdskEx(
    IN      PCWSTRING           NtDriveName,
    IN OUT  PMESSAGE            Message,
    IN      BOOLEAN             Fix,
    IN      PCHKDSKEX_FN_PARAM  Param,
    IN      PULONG              ExitStatus
   )
{
	PXIXFS_VOL	XixFsVol;
	DWORD		oldErrorMode;
	ULONG		flags;
	BOOLEAN		result = FALSE;


	if (Param->Major != 1 || (Param->Minor != 0 && Param->Minor != 1)) {

		*ExitStatus = CHKDSK_EXIT_COULD_NOT_CHK;
		return FALSE;

	}

	if ((Param->Flags & CHKDSK_EXTEND) || (Param->Flags & CHKDSK_RESIZE_LOGFILE)) {

		*ExitStatus = CHKDSK_EXIT_COULD_NOT_CHK;
		return FALSE;

	}
	
	oldErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS );

	if( !(XixFsVol = NEW XIXFS_VOL)) {
		Message->Set( MSG_FMT_NO_MEMORY );
        Message->Display( "" );
        return FALSE;		
	}
	
	
	if(	!(XixFsVol->Initialize(NtDriveName, Message)) ) {
		DELETE(XixFsVol);
		SetErrorMode(oldErrorMode);
		*ExitStatus = CHKDSK_EXIT_COULD_NOT_CHK;
		return FALSE;
	}

	SetErrorMode(oldErrorMode);

	if(Fix) {
		if( !XixFsVol->Lock()) {
			DELETE(XixFsVol);
            *ExitStatus = CHKDSK_EXIT_COULD_NOT_CHK;
            return FALSE;
		}
	}

	result = XixFsVol->ChkDsk(	
						(Fix?TotalFix:CheckOnly),
						Message,
						Param->Flags
						); 
	

	DELETE(XixFsVol);
	
    return result;
}



BOOLEAN
FAR APIENTRY
Format(
    IN      PCWSTRING           NtDriveName,
    IN OUT  PMESSAGE            Message,
    IN      BOOLEAN             Quick,
    IN      BOOLEAN             BackwardCompatible,
    IN      MEDIA_TYPE          MediaType,
    IN      PCWSTRING           LabelString,
    IN      ULONG               ClusterSize
    )
/*++

Routine Description:

    This routine formats a volume for the XIXFS file system.

Arguments:

    NtDriveName - Supplies the NT style drive name of the volume to format.
    Message     - Supplies an outlet for messages.
    Quick       - Supplies whether or not to do a quick format.
    BackwardCompatible - Supplies whether or not the newly formatted volume
                         will be compatible with FAT16.
    MediaType   - Supplies the media type of the drive.
    LabelString - Supplies a volume label to be set on the volume after
                    format.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
	PDP_DRIVE   DpDrive;
	PXIXFS_VOL	XixFsVol;
	DSTRING		FileSystemName;
	BOOLEAN     ret_status;
	FORMAT_ERROR_CODE   errcode;
	USHORT      format_type;
    BOOLEAN     send_fmt_cmd, fmt_cmd_capable;
	


    if( !(DpDrive = NEW DP_DRIVE) ) {

        Message->Set( MSG_FMT_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }




	if (!DpDrive->Initialize(NtDriveName, Message)) {
		DELETE( DpDrive );
		return FALSE;
	}

    if (DpDrive->IsFloppy()) {
        DELETE( DpDrive );
        
		Message->DisplayMsg(MSG_NTFS_FORMAT_NO_FLOPPIES);
        return FALSE;
    }

	DELETE(DpDrive);
	
	if( !(XixFsVol = NEW XIXFS_VOL)	) {
		Message->Set( MSG_FMT_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
	}
	

	errcode = XixFsVol->Initialize(NtDriveName, Message);
	if( errcode == NoError ) {
		errcode = XixFsVol->Format(LabelString, Message);
	}

	DELETE(XixFsVol);


	if(errcode == LockError) {
		Message->DisplayMsg(MSG_CANT_LOCK_THE_DRIVE);
		return FALSE;		
	}else{
		return (errcode == NoError);
	}
	
    
}


/*

BOOL
CheckAccess(
	LPCGUID lpHostGuid,
	ACCESS_MASK Access,
	LPVOID lpContext
)
{
	LPDWORD lpnHosts = (LPDWORD ) lpContext;

	if(Access | (GENERIC_READ | GENERIC_WRITE)){
		(*lpnHosts) = (*lpnHosts) + 1;
	}

	if((*lpnHosts) > 1) return FALSE;

	return TRUE;
}

*/



BOOLEAN
FAR APIENTRY
FormatEx(
    IN      PCWSTRING           NtDriveName,
    IN OUT  PMESSAGE            Message,
    IN      PFORMATEX_FN_PARAM  Param,
    IN      MEDIA_TYPE          MediaType
    )
/*++

Routine Description:

    This routine formats a volume for the XIXFS file system.

Arguments:

    NtDriveName - Supplies the NT style drive name of the volume to format.
    Message     - Supplies an outlet for messages.
    Param       - Supplies the format parameter block
    MediaType   - Supplies the media type of the drive.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    PDP_DRIVE   DpDrive;
    BIG_INT     Sectors;
    DSTRING     FileSystemName;
    BOOLEAN     ret_status;
    FORMAT_ERROR_CODE   errcode;
	PXIXFS_VOL	XixFsVol;

 
	DebugPrint( "enter FormatEx 1!");

    if (Param->Major != 1 || Param->Minor != 0) {
        return FALSE;
    }

	DebugPrint( "enter FormatEx 2!");
    if( !(DpDrive = NEW DP_DRIVE) ) {

        Message->Set( MSG_FMT_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

	DebugPrint( "enter FormatEx 3!");

	if (!DpDrive->Initialize(NtDriveName, Message)) {
		DELETE( DpDrive );
		return FALSE;
	}

	DebugPrint( "enter FormatEx 4!");

    if (DpDrive->IsFloppy()) {
        DELETE( DpDrive );
        
		Message->DisplayMsg(MSG_NTFS_FORMAT_NO_FLOPPIES);
        return FALSE;
    }
	DELETE(DpDrive);
	
	DebugPrint( "enter FormatEx 5!");

	if( !(XixFsVol = NEW XIXFS_VOL)	) {
		Message->Set( MSG_FMT_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
	}
	
	DebugPrint( "enter FormatEx 6!");

	errcode = XixFsVol->Initialize(NtDriveName, Message);

	DebugPrint( "enter FormatEx 7-1!");
	
#pragma warning(disable: 4995)
	if((errcode == NoError ) || (errcode == LockError)){
		HRESULT hr;
		NDAS_LOCATION location;

//		NDAS_LOGICALDEVICE_ID logicalDevId;
		DWORD nROHosts = 0; 
		DWORD nRWHosts = 0;
//		DWORD nHosts = 0;

		DebugPrintTrace(("NtDriveName %s\n",NtDriveName->QuerySTR()));
		hr = NdasIsNdasVolume(XixFsVol->QueryDriveHandle());
		if(FAILED(hr)){
			Message->DisplayMsg(MSG_DASD_ACCESS_DENIED);
			DebugPrint( "IS NOT NDAS_VOLUME!");
			DELETE(XixFsVol);
			return FALSE;	
		}

		DebugPrint( "Querry Volume!");

		hr = NdasGetNdasLocationForVolume(XixFsVol->QueryDriveHandle(), &location);
				
		if (FAILED(hr))
		{
			DebugPrint( "ERROR FAILED!");
			Message->DisplayMsg(MSG_DASD_ACCESS_DENIED);
			DELETE(XixFsVol);
			return FALSE;		
		}
		else if (S_FALSE == hr)
		{
			DebugPrint( "ERROR IS NOT NDAS_VOLUME!");
			Message->DisplayMsg(MSG_DASD_ACCESS_DENIED);
			DELETE(XixFsVol);
			return FALSE;		
		}
		else // S_OK
		{
			DebugPrintTrace(("NDAS_LOCATION=%d\n", location));
		}


	


/*


		if(!NdasQueryNdasLogicalDeviceId(location, &logicalDevId)){
			DebugPrint( "ERROR Get LogicalDeviceId Information!");
			DELETE(XixFsVol);
			return FALSE;		
		}

		DebugPrintTrace(("NDAS_LogicalId=%d\n", logicalDevId));



		//--> It's not implemented
		if(!NdasQueryHostsForLogicalDevice(logicalDevId, CheckAccess, (LPVOID)&nHosts)){
			DebugPrintTrace(( "ERROR NOt allowed! Host(%d)\n", nHosts));
			DELETE(XixFsVol);
			return FALSE;		
		}


		DebugPrintTrace(( "Num Host(%d)\n", nHosts));
*/

		if(!NdasQueryUnitDeviceHostStats(location/10, 0, &nROHosts, &nRWHosts)){
			DebugPrint( "ERROR Get Host Information!");
			Message->DisplayMsg(MSG_DASD_ACCESS_DENIED);
			DELETE(XixFsVol);
			return FALSE;		
		}
	

		DebugPrintTrace(("Num of Host (%d)\n", (nROHosts + nRWHosts)));

		if(nRWHosts == 1) {

			if((nROHosts) > 1){
				DebugPrint( "ERROR An other host using this volume!");
				Message->DisplayMsg(MSG_DASD_ACCESS_DENIED);
				DELETE(XixFsVol);
				return FALSE;		
			}
		}else{
			DebugPrint( "ERROR An other host using this volume!");
			Message->DisplayMsg(MSG_DASD_ACCESS_DENIED);
			DELETE(XixFsVol);
			return FALSE;		
		}

		


	}else{
		Message->DisplayMsg(MSG_DASD_ACCESS_DENIED);
		DELETE(XixFsVol);
		return FALSE;	
	}

	if( errcode == NoError ) {
		errcode = XixFsVol->Format(Param->LabelString, Message);
		DebugPrint( "enter FormatEx 7-2!");
	}


	DebugPrint( "enter FormatEx 7-3!");
	if(errcode == LockError) {
		if(	!(Param->Flags & FORMAT_FORCE) ) {
			Message->DisplayMsg(MSG_FMT_FORCE_DISMOUNT_PROMPT);

			if (Message->IsYesResponse(FALSE) &&
				IFS_SYSTEM::DismountVolume(NtDriveName)) {
				Message->DisplayMsg(MSG_VOLUME_DISMOUNTED);
			}
		}else if ( IFS_SYSTEM::DismountVolume(NtDriveName)){
				Message->DisplayMsg(MSG_VOLUME_DISMOUNTED);
		}
	


		errcode = XixFsVol->Initialize(NtDriveName, Message);
		if( errcode == NoError ) {
			errcode = XixFsVol->Format(Param->LabelString, Message);
		}

		if(errcode == LockError) {
			DELETE(XixFsVol);
			Message->DisplayMsg(MSG_CANT_LOCK_THE_DRIVE);
			return FALSE;
		}else {
			DELETE(XixFsVol);
			return (errcode == NoError);
		}

	} else {
		DELETE(XixFsVol);
		return (errcode == NoError);	
	}
}


BOOLEAN
FAR APIENTRY
Recover(
   IN       PPATH    RecFilePath,
   IN OUT   PMESSAGE Message
   )
{
    PXIXFS_VOL  XixFsVol;
    PWSTRING    FullPath;
    PWSTRING    DosDriveName;
    DSTRING     NtDriveName;
    BOOLEAN     Result;

    FullPath = RecFilePath->QueryDirsAndName();
    DosDriveName = RecFilePath->QueryDevice();

    if ( DosDriveName == NULL ||
         !IFS_SYSTEM::DosDriveNameToNtDriveName(DosDriveName,
                                                &NtDriveName) ||
         FullPath == NULL ) {

        DELETE(DosDriveName);
        DELETE(FullPath);
        return FALSE;
    }

	if( !(XixFsVol = NEW XIXFS_VOL)	) {
		Message->Set( MSG_FMT_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
	}	
	


    Message->DisplayMsg(MSG_RECOV_BEGIN,
                     "%W", DosDriveName);
    Message->WaitForUserSignal();

    Result = ( XixFsVol->Initialize( &NtDriveName, Message ) &&
               XixFsVol->Recover( FullPath, Message ) );

    DELETE(DosDriveName);
    DELETE(FullPath);
	DELETE(XixFsVol);
    return Result;
}
