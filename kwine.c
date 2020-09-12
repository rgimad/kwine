// KWINE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kos32sys.h>
#include "include/windows.h"

static inline int IsPowerOf2(uint32_t val) // is number some power of 2
{
    if(val == 0)
        return 0;
    return (val & (val - 1)) == 0;
}


int kwine_validate_pe(void *raw, size_t raw_size, int is_exec) // is_exec = 1 if we check if exe is correct, else check as dll
{
    PIMAGE_DOS_HEADER dos;
    PIMAGE_NT_HEADERS32 nt;

    dos = (PIMAGE_DOS_HEADER)raw;
    if( !raw || raw_size < sizeof(IMAGE_DOS_HEADER) ) {return 0;}

    if( dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0) {return 0;}

    nt =  (PIMAGE_NT_HEADERS32)((uint32_t)dos + (uint32_t)dos->e_lfanew);

    if( (uint32_t)nt < (uint32_t)raw) {return 0;}

    if(nt->Signature != IMAGE_NT_SIGNATURE) { return 0;}

    if(nt->FileHeader.Machine != IMAGE_FILE_MACHINE_I386) {return 0;}

    if(is_exec && (nt->FileHeader.Characteristics & IMAGE_FILE_DLL)) {return 0;}

    if(nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC) {return 0;}

    if( is_exec && nt->OptionalHeader.ImageBase != 0) {return 0;}

    if(nt->OptionalHeader.SectionAlignment < 4096) {
        if(nt->OptionalHeader.FileAlignment != nt->OptionalHeader.SectionAlignment) {return 0;}
    } else if(nt->OptionalHeader.SectionAlignment < nt->OptionalHeader.FileAlignment) {return 0;}

    if(!IsPowerOf2(nt->OptionalHeader.SectionAlignment) || !IsPowerOf2(nt->OptionalHeader.FileAlignment)) {return 0;}

    if(nt->FileHeader.NumberOfSections > 96) {return 0;}

    return 1;
}


uint32_t kwine_get_address_of_entry_point(void *raw) // RVA of entry point
{
    PIMAGE_DOS_HEADER dos;
    PIMAGE_NT_HEADERS32 nt;
    dos = (PIMAGE_DOS_HEADER)raw;
    nt = (PIMAGE_NT_HEADERS32)((uint32_t)dos + (uint32_t)dos->e_lfanew);
    return (void*)(nt->OptionalHeader.AddressOfEntryPoint);
}

uint32_t kwine_get_image_base(void *raw) // VA of image base
{
    PIMAGE_DOS_HEADER dos;
    PIMAGE_NT_HEADERS32 nt;
    dos = (PIMAGE_DOS_HEADER)raw;
    nt = (PIMAGE_NT_HEADERS32)((uint32_t)dos + (uint32_t)dos->e_lfanew);
    return (void*)(nt->OptionalHeader.ImageBase);
}

PIMAGE_SECTION_HEADER kwine_get_section_containing_rva(void *raw, uint32_t rva) // returns raw address of section
{
	PIMAGE_DOS_HEADER dos;
    PIMAGE_NT_HEADERS32 nt;
    dos = (PIMAGE_DOS_HEADER)raw;
    nt = (PIMAGE_NT_HEADERS32)((uint32_t)dos + (uint32_t)dos->e_lfanew);
    IMAGE_SECTION_HEADER* cur_sect_ptr = IMAGE_FIRST_SECTION(nt);
    int i;
	for (i = 0; i < nt->FileHeader.NumberOfSections; i++, cur_sect_ptr++)
	{
		// if RVA is located within section, then return pointer to its header
    	if (rva >= cur_sect_ptr->VirtualAddress && rva < cur_sect_ptr->VirtualAddress + cur_sect_ptr->Misc.VirtualSize)
			return cur_sect_ptr;
	}
	return NULL; // section not found
}

uint32_t kwine_rva_to_raw(void *raw, uint32_t rva) // RVA to RAW address
{
	PIMAGE_DOS_HEADER dos;
    PIMAGE_NT_HEADERS32 nt;
    dos = (PIMAGE_DOS_HEADER)raw;
    nt = (PIMAGE_NT_HEADERS32)((uint32_t)dos + (uint32_t)dos->e_lfanew);
    IMAGE_SECTION_HEADER* sect_hdr_ptr = kwine_get_section_containing_rva(raw, rva);
	if (sect_hdr_ptr == NULL) 
		return rva; // if section not found then raw addr is equal to rva
	// otherwise calculate raw adderess (raw means from the beginning of the file)
	return sect_hdr_ptr->PointerToRawData + (rva - sect_hdr_ptr->VirtualAddress);
}

void kwine_print_directory_table(void *raw_img)
{
	PIMAGE_DOS_HEADER dos;
    PIMAGE_NT_HEADERS32 nt;
    dos = (PIMAGE_DOS_HEADER)raw_img;
    nt = (PIMAGE_NT_HEADERS32)((uint32_t)dos + (uint32_t)dos->e_lfanew);

    IMAGE_DATA_DIRECTORY* data_dir_ptr = nt->OptionalHeader.DataDirectory;
	//printf("%x %x\n", (pDataDir-1)->VirtualAddress, (pDataDir-1)->Size);
	printf("N\tVirtaddr\tSize\tRawaddr\n");
	for (UINT i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++, data_dir_ptr++) {
    	//if (pDataDir->Size != 0) {
    	ULONG nFilePtr = (ULONG)(ULONG_PTR)kwine_rva_to_raw(raw_img, data_dir_ptr->VirtualAddress);
    	//cout << dec << i << ":\t" << hex << pDataDir->VirtualAddress << '\t' << pDataDir->Size << '\t' << nFilePtr << endl;
    	printf("%8d:\t0x%08x\t0x%08x\t0x%08x\n", i, data_dir_ptr->VirtualAddress, data_dir_ptr->Size, nFilePtr);
    	//}
  	}
}

void kwine_print_section_table(void *raw_img)
{
	PIMAGE_DOS_HEADER dos;
    PIMAGE_NT_HEADERS32 nt;
    dos = (PIMAGE_DOS_HEADER)raw_img;
    nt = (PIMAGE_NT_HEADERS32)((uint32_t)dos + (uint32_t)dos->e_lfanew);

    char section_name[9] = {0};
    IMAGE_SECTION_HEADER* cur_sect_ptr = IMAGE_FIRST_SECTION(nt);
    int i;
    printf("N\tName\tVirtsize\tVirtaddr\tRawsize\tRawaddr\n");
	for (i = 0; i < nt->FileHeader.NumberOfSections; i++, cur_sect_ptr++)
	{
		memcpy(section_name, cur_sect_ptr->Name, 8);
		printf("%8d:\t%s\t0x%08x\t0x%08x\t0x%08x\t0x%08x\n", i, section_name, cur_sect_ptr->Misc.VirtualSize, cur_sect_ptr->VirtualAddress, cur_sect_ptr->SizeOfRawData, cur_sect_ptr->PointerToRawData);
	}
}


int main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		printf("[-] error: no exe file specified.\n Usage: kwine <file>\n");
		return -1;
	}

	ufile_t uf;
    void *raw_img;
    size_t raw_size;
    void *img_base = NULL;

    char *exe_path = argv[1];

    uf = load_file(exe_path);
    raw_img  = uf.data;
    raw_size = uf.size;

    if(raw_img == NULL)
    {
    	printf("[-] error: file %s not found.\n", exe_path);
        return -1;
    }

    if( kwine_validate_pe(raw_img, raw_size, 0) == 0)
    {
        printf("module %s is NOT valid\n", exe_path);
        user_free(raw_img);
        return -1;
    }
    printf("module %s is valid\n", exe_path);
    printf("RVA of entry point = 0x%x\n", (uint32_t)kwine_get_address_of_entry_point(raw_img));
    printf("VA of image base = 0x%x\n", (uint32_t)kwine_get_image_base(raw_img));
    //printf("%d", IMAGE_NUMBEROF_DIRECTORY_ENTRIES);

    kwine_print_directory_table(raw_img);
    printf("\n\n");
    kwine_print_section_table(raw_img);


	return 0;
}


/*



The AddressOfEntryPoint is the relative virtual address of the entry point, not the raw offset in the file. It holds the address of the first instruction that will be executed when the program starts.
Usually this is not the same as the beginning of the code section. If you want to get the beginning of the code section, you should see the BaseOfCode field.



*/