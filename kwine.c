// KWINE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kos32sys.h>
#include "include/windows.h"

#define KWINE_NAME_VERSION "Kwine 0.4"

static inline int IsPowerOf2(uint32_t val) { // is number some power of 2
    if(val == 0)
        return 0;
    return (val & (val - 1)) == 0;
}

static inline void memcpy_fast(void *dst, void *src, size_t len) {
    __asm__ __volatile__ (
    "shrl $2, %%ecx         \n\t"
    "rep movsl"
    :
    :"c"(len),"S"(src),"D"(dst)
    :"cc");
    __asm__ __volatile__ (
    ""
    :::"ecx","esi","edi");
};

static inline int get_used_memory() {
    int eax;
    uint8_t buf[1025] = {0};
    __asm__ __volatile__ (
    "int $0x40"
    :"=a"(eax)
    :"a"(9),"b"(buf),"c"(-1));
    return *(int*)(buf + 0x1A);
}

static inline void set_default_event_mask() {
    int eax;
    __asm__ __volatile__ (
    "int $0x40"
    :"=a"(eax)
    :"a"(40),"b"(7));
}

/*static inline int sys_virtual_alloc(void *base, size_t size) {
    int eax;
    __asm__ __volatile__ (
    "int $0x40"
    :"=a"(eax)
    :"a"(81),"b"(base),"c"(size));
    return eax;
}*/

// is_exec = 1 if we check if exe is correct, else check as dll
int kwine_validate_pe(void *raw, size_t raw_size, int is_exec) { 
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


uint32_t kwine_get_address_of_entry_point(void *raw) {// RVA of entry point
    PIMAGE_DOS_HEADER dos;
    PIMAGE_NT_HEADERS32 nt;
    dos = (PIMAGE_DOS_HEADER)raw;
    nt = (PIMAGE_NT_HEADERS32)((uint32_t)dos + (uint32_t)dos->e_lfanew);
    return (uint32_t)(nt->OptionalHeader.AddressOfEntryPoint);
}

uint32_t kwine_get_image_base(void *raw) { // VA of image base
    PIMAGE_DOS_HEADER dos;
    PIMAGE_NT_HEADERS32 nt;
    dos = (PIMAGE_DOS_HEADER)raw;
    nt = (PIMAGE_NT_HEADERS32)((uint32_t)dos + (uint32_t)dos->e_lfanew);
    return (uint32_t)(nt->OptionalHeader.ImageBase);
}

PIMAGE_SECTION_HEADER kwine_get_section_containing_rva(void *raw, uint32_t rva) { // returns raw address of section
	PIMAGE_DOS_HEADER dos;
    PIMAGE_NT_HEADERS32 nt;
    dos = (PIMAGE_DOS_HEADER)raw;
    nt = (PIMAGE_NT_HEADERS32)((uint32_t)dos + (uint32_t)dos->e_lfanew);
    IMAGE_SECTION_HEADER* cur_sect_ptr = IMAGE_FIRST_SECTION(nt);
    int i;
	for (i = 0; i < nt->FileHeader.NumberOfSections; i++, cur_sect_ptr++) {
		// if RVA is located within section, then return pointer to its header
    	if (rva >= cur_sect_ptr->VirtualAddress && rva < cur_sect_ptr->VirtualAddress + cur_sect_ptr->Misc.VirtualSize)
			return cur_sect_ptr;
	}
	return NULL; // section not found
}

uint32_t kwine_rva_to_raw(void *raw, uint32_t rva) { // RVA to RAW address
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

void kwine_print_directory_table(void *raw_img) {
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
    	printf("%8d:\t0x%08x\t0x%08x\t0x%08x\n", i, (uint32_t)data_dir_ptr->VirtualAddress, (uint32_t)data_dir_ptr->Size, (uint32_t)nFilePtr);
    	//}
  	}
}

void kwine_print_section_table(void *raw_img) {
	PIMAGE_DOS_HEADER dos;
    PIMAGE_NT_HEADERS32 nt;
    dos = (PIMAGE_DOS_HEADER)raw_img;
    nt = (PIMAGE_NT_HEADERS32)((uint32_t)dos + (uint32_t)dos->e_lfanew);

    char section_name[9] = {0};
    IMAGE_SECTION_HEADER* cur_sect_ptr = IMAGE_FIRST_SECTION(nt);
    int i;
    printf("N\tName\tVirtsize\tVirtaddr\tRawsize\tRawaddr\n");
	for (i = 0; i < nt->FileHeader.NumberOfSections; i++, cur_sect_ptr++) {
		memcpy(section_name, cur_sect_ptr->Name, 8);
		printf("%8d:\t%s\t0x%08x\t0x%08x\t0x%08x\t0x%08x\n", i, section_name, (uint32_t)cur_sect_ptr->Misc.VirtualSize, (uint32_t)cur_sect_ptr->VirtualAddress, (uint32_t)cur_sect_ptr->SizeOfRawData, (uint32_t)cur_sect_ptr->PointerToRawData);
	}
}

void kwine_load_sections(void *raw_img) {
    PIMAGE_DOS_HEADER dos;
    PIMAGE_NT_HEADERS32 nt;
    dos = (PIMAGE_DOS_HEADER)raw_img;
    nt = (PIMAGE_NT_HEADERS32)((uint32_t)dos + (uint32_t)dos->e_lfanew);

    IMAGE_SECTION_HEADER* cur_sect_ptr = IMAGE_FIRST_SECTION(nt);
    int i;
    for (i = 0; i < nt->FileHeader.NumberOfSections; i++, cur_sect_ptr++) {
        void *dst_ptr = (void*)(nt->OptionalHeader.ImageBase + cur_sect_ptr->VirtualAddress);
        void *src_ptr = (void*)(raw_img + cur_sect_ptr->PointerToRawData);
        memcpy_fast(dst_ptr, src_ptr, cur_sect_ptr->SizeOfRawData);
    }
}

BOOL kwine_load_exe_image(void *raw_img) {
    PIMAGE_DOS_HEADER dos;
    PIMAGE_NT_HEADERS32 nt;
    dos = (PIMAGE_DOS_HEADER)raw_img;
    nt = (PIMAGE_NT_HEADERS32)((uint32_t)dos + (uint32_t)dos->e_lfanew);

    if (!(nt->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)) {
        return FALSE;
    }

    // allocate such amount of memory so that we can load exe at its ImageBase (almost always 0x00400000)
    int x = /*get_used_memory() + */nt->OptionalHeader.ImageBase + nt->OptionalHeader.SizeOfImage;
    void *tmp = user_alloc(x);
    //printf("x = %d (bytes) tmp = %d (decimal)\n", x, (int)tmp);
    //printf("used memory = %d bytes\n------\n", get_used_memory());
    kwine_load_sections(raw_img);

    uint32_t esp;
    asm( "mov %%esp, %0" : "=r" ( esp ));
    printf("esp = %x\n", esp);

    // jump to the entry point
    ((void(*)(void))(nt->OptionalHeader.ImageBase + nt->OptionalHeader.AddressOfEntryPoint))();
    return TRUE;
}


int main(int argc, char *argv[]) {

    set_default_event_mask();

    printf("%s\n", KWINE_NAME_VERSION);

	if (argc <= 1) {
		printf("[-] error: no exe file specified.\n Usage: kwine <file>\n");
		return -1;
	}

	ufile_t uf;
    void *raw_img;
    size_t raw_size;
    void *img_base = NULL;

    char *exe_path = argv[1];

    //printf("used memory = %d bytes\n", get_used_memory());
    uf = load_file(exe_path);
    raw_img  = uf.data;
    raw_size = uf.size;
    //printf("uf.data = %d (decimal)\n", uf.data);
    //printf("used memory = %d bytes\n", get_used_memory());

    if(raw_img == NULL) {
    	printf("[-] error: file %s not found.\n", exe_path);
        return -1;
    }

    if( kwine_validate_pe(raw_img, raw_size, 0) == 0) {
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

    kwine_load_exe_image(raw_img);


	return 0;
}


/*



The AddressOfEntryPoint is the relative virtual address of the entry point, not the raw offset in the file. It holds the address of the first instruction that will be executed when the program starts.
Usually this is not the same as the beginning of the code section. If you want to get the beginning of the code section, you should see the BaseOfCode field.



*/