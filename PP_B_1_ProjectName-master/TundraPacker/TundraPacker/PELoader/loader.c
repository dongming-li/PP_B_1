/**
 * @file loader.c
 * @brief Position independent loader for 32-bit windows portable executable files
 * 
 * All code written for the PE loader must be position independent as it is essentially
 * shellcode which is being run in an arbitrary memory location without having been
 * relocated by the windows loader
 *
 * This means you cannot use any windows APIs which you do not resolve the location
 * of yourself, cannot use global variables, as well as cannot use things like switch
 * statement which could result in the compiler generating jump tables which require
 * relocations
 *
 * For more information, please read see the windows PE/COFF
 * specification available at the link below
 *
 * @see http://www.osdever.net/documents/PECOFF.pdf
 */

#include <Windows.h>

/*
 * When functions are located within the same source/object file the compiler
 * will generate relative jumps. For shellcode, this is what we need so we need
 * to do this instead of using header files. Otherwise the call instructions generated
 * will require relocations and this is not acceptable for shellcode
 *
 * This is one of the rare use-cases where you would actually want
 * to include c sources files using the preprocessor. Although, it is still
 * better practice than leaving all the code in one source file
 */

#include "crt.c"
#include "apis.c"
#include "utils.c"

/**
 * @brief Relocate executable so that it can run at the address we are loading it
 *
 * Relocations are performed by calculating a delta between the preferred base
 * address of the executable and the address we are actually loading the executable
 * at then we process the relocations to adjust the code in the binary so it can run
 * at the appropriate address
 */
PVOID PerformRelocations(
		PEIMAGE image,
		DWORD hash				
		)
{
	DWORD delta, reloc_count;
	PDWORD reloc_addr;
	PIMAGE_BASE_RELOCATION relocations;

	//
	// Calculate difference between image base address and the address that we are actually
	// loading at and the preferred base address of the binary if we are loading at the preferred
	// base address this calculation result will be zero and so it will still work no matter where
	// we load the binary (at preferred base address or not)
	//

	delta = image->baseaddr - image->OptionalHeader.ImageBase;

	//
	// Relocations are stored in blocks which specify relocations for every 0x1000 bytes
	// of code in the binary. 
	//

	while(relocations->SizeOfBlock != 0) {

		reloc_count = (pRelocTable->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(RELOCATION);

		for(int i = 0; i < reloc_count; reloc_count++) {
			//UINT_PTR* pVal = module->ImageBase + pRelocTable->VirtualAddress + pReloc[i].Offset);

			//
			// We don't want to use a switch statement here because compiler
			// could generate a jump table which would require relocations
			//
			
			if(relocations[i].Type == IMAGE_REL_BASED_DIR64) {
				*reloc_addr += delta;
			} else if(relocations[i].Type == IMAGE_REL_BASED_HIGHLOW) {
				*reloc_addr += delta;
			} else if(relocations[i].Type == IMAGE_REL_BASED_HIGH) {
				*reloc_addr += HIWORD(delta);
			} else if(relocations[i].Type == IMAGE_REL_BASED_LOW) {
				*reloc_addr += LOWORD(delta);
			}
		}

		relocations += relocations->SizeOfBlock;
	}
}

/**
 * @brief Resolves all imports in import address table for PE
 *
 * We resolve the addresses of all of the functions which are imported by the
 * executable, we do this by calling the LoadLibrary function as well as the
 * GetProcAddress functions to find the address of the required function and
 * then write the returned pointer into the import address table entry 
 * corresponding to that function
 */
PVOID ResolveImports(
		//TODO:
		)
{
	HMODULE library;
	LPCSTR name;
	PIMAGE_IMPORT_DESCRIPTOR descriptor;
	PIMAGE_THUNK_DATA thunk;
	PIMAGE_THUNK_DATA thunkAddr;

	//TODO: lookup import descriptor and error check shit

	//
	// TODO:
	//

	while (pDescriptor->Name != NULL) {

		//TODO: name = (LPCSTR)((UINT_PTR)LoadedImage.ImageBase + pDescriptor->Name);
		library = LoadLibrary(name); //TODO: position independent`

		if (!library) {
			//TODO: Debug print
			return;
		}

		//
		// TODO:
		//

		if (pDescriptor->OriginalFirstThunk) {
			pThunk = (PIMAGE_THUNK_DATA)((UINT_PTR)LoadedImage.ImageBase + pDescriptor->OriginalFirstThunk);
		} else {
			pThunk = (PIMAGE_THUNK_DATA)((UINT_PTR)LoadedImage.ImageBase + pDescriptor->FirstThunk);
		}

		pAddrThunk = (PIMAGE_THUNK_DATA)((UINT_PTR)LoadedImage.ImageBase + pDescriptor->FirstThunk);

		//
		// TODO:
		//

		while (pAddrThunk && pThunk && pThunk->u1.AddressOfData) {

			if (IMAGE_SNAP_BY_ORDINAL(pThunk->u1.Ordinal)) {
				LPCSTR Ordinal = (LPCSTR)IMAGE_ORDINAL(pAddrThunk->u1.Ordinal);
				pAddrThunk->u1.Function = (DWORD)pFunTable->fnGetProcAddress(hLib, Ordinal);
			} else {
				PIMAGE_IMPORT_BY_NAME pImport = (PIMAGE_IMPORT_BY_NAME)((UINT_PTR)LoadedImage.ImageBase + pThunk->u1.AddressOfData);
				pAddrThunk->u1.Function = (DWORD)pFunTable->fnGetProcAddress(hLib, pImport->Name);
			}

			++pThunk;
			++pAddrThunk;
		}
	} 

	//TODO: pDescriptor++;
}

/**
 * TODO: Ya ne znayu
 */
PVOID MapExecutable(
		//TODO:
	)
{

	//
	// Allocate memory for the 
	//

	uiBaseAddress = VirtualAlloc(NULL,  //TODO: Position independent code
			module->header->OptionalHeader.SizeOfImage, 
			MEM_RESERVE | MEM_COMMIT, 
			PAGE_EXECUTE_READWRITE);

	// we must now copy over the headers
	uiValueA = ((PIMAGE_NT_HEADERS)uiHeaderValue)->OptionalHeader.SizeOfHeaders;
	uiValueB = uiLibraryAddress;
	uiValueC = uiBaseAddress;

	// TODO: Custom memcpy
	// while( uiValueA-- )
	// 	*(BYTE *)uiValueC++ = *(BYTE *)uiValueB++;

	// STEP 3: load in all of our sections...

	// uiValueA = the VA of the first section
	uiValueA = ( (ULONG_PTR)&((PIMAGE_NT_HEADERS)uiHeaderValue)->OptionalHeader + ((PIMAGE_NT_HEADERS)uiHeaderValue)->FileHeader.SizeOfOptionalHeader );
	
	//
	// TODO:
	//

	uiValueE = ((PIMAGE_NT_HEADERS)uiHeaderValue)->FileHeader.NumberOfSections;
	while( uiValueE-- ) {
		// uiValueB is the VA for this section
		uiValueB = ( uiBaseAddress + ((PIMAGE_SECTION_HEADER)uiValueA)->VirtualAddress );

		// uiValueC if the VA for this sections data
		uiValueC = ( uiLibraryAddress + ((PIMAGE_SECTION_HEADER)uiValueA)->PointerToRawData );

		// copy the section over
		uiValueD = ((PIMAGE_SECTION_HEADER)uiValueA)->SizeOfRawData;

		// TODO: Custom memcpy
		//while( uiValueD-- )
		//	*(BYTE *)uiValueB++ = *(BYTE *)uiValueC++;

		// get the VA of the next section
		uiValueA += sizeof( IMAGE_SECTION_HEADER );
	}
}



/**
 * @brief Function used in debug build of application to test pe loader code
 */
int main() {


}
