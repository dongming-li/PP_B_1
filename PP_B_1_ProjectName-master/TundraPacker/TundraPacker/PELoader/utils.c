/**
 * TODO:
 */
PBYTE GetImageDirectorySection(
	//TODO:	
                 )
{
        PIMAGE_DATA_DIRECTORY directory;
	PIMAGE_SECTION_HEADER section;

	//
	// Lookup the entry for the corresponding entry in the data directory
	//
	
	directory = &module->header->OptionalHeader.DataDirectory[entry];
	if(directory) {

	}


	//
	// Find the section that corresponds to the directory entry
	//

	//TODO:
	section = ResolveVAToSection(image->file, directory->VirtualAddress);
	if (section) {
		return section->PointerToRawData + module->baseaddr;
	}
}

/**
 * @brief Gets an entry from the image data directory
 *
 * TODO
 */
PIMAGE_SECTION_HEADER GetImageDirectoryEntry(
		IMAGE_NT_HEADER header,
		DWORD entry
		)
{
	PIMAGE_DATA_DIRECTORY directory = &header->OptionalHeader.DataDirectory[Entry];

	if (directory) {

	}

}

/**
 * @brief Finds the section where a specific rva is located 
 *
 * Does this by doing a linear search of all of the section headers until
 * it finds a section header that the section is located in
 */
PIMAGE_SECTION_HEADER ResolveVAToSection(
		IMAGE_NT_HEADER header, 
		uint32_t rva
	)
{
	PIMAGE_SECTION_HEADER sections = IMAGE_FIRST_SECTION(header);

	for(int i = 0; i < NUM_SECTIONS(sections); i++) {

		if(RVA_IS_IN_SECTION(rva, sections[i])) {
			return section;
		}
	}

	return NULL;
}
