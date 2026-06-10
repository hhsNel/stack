#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <elf.h>

/* debug */
//#include <stdio.h>

uint8_t *loaded_file;
uint64_t total_len;

void
mem_zero(void *dst, uint64_t size)
{
	while(size-- > 0) {
		((uint8_t *)dst)[size] = 0;
	}
}

void
elf_wrap()
{
	int64_t ret;
	Elf64_Ehdr ehdr;
	Elf64_Phdr phdr;

	mem_zero(&ehdr, sizeof(ehdr));
	mem_zero(&phdr, sizeof(phdr));

	ehdr.e_ident[EI_MAG0] = ELFMAG0;
	ehdr.e_ident[EI_MAG1] = ELFMAG1;
	ehdr.e_ident[EI_MAG2] = ELFMAG2;
	ehdr.e_ident[EI_MAG3] = ELFMAG3;
	ehdr.e_ident[EI_CLASS] = ELFCLASS64;
	ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
	ehdr.e_ident[EI_VERSION] = EV_CURRENT;
	ehdr.e_ident[EI_OSABI] = ELFOSABI_LINUX;
	ehdr.e_type = ET_EXEC;
	ehdr.e_machine = EM_X86_64;
	ehdr.e_version = EV_CURRENT;
	ehdr.e_entry = 0x400000 + sizeof(ehdr) + sizeof(phdr);
	ehdr.e_phoff = sizeof(ehdr);
	ehdr.e_ehsize = sizeof(ehdr);
	ehdr.e_phentsize = sizeof(phdr);
	ehdr.e_phnum = 1;

	phdr.p_type = PT_LOAD;
	phdr.p_offset = 0;
	phdr.p_vaddr = 0x400000;
	phdr.p_paddr = 0x400000;
	phdr.p_filesz = sizeof(ehdr) + sizeof(phdr) + total_len;
	phdr.p_memsz = sizeof(ehdr) + sizeof(phdr) + total_len;
	phdr.p_flags = PF_R | PF_X;
	phdr.p_align = 0x1000;

	ret = write(1, &ehdr, sizeof(ehdr));
	if(ret < 0) {
		write(2, "couldn't write\n", 15);
		exit(1);
	}
	ret = write(1, &phdr, sizeof(phdr));
	if(ret < 0) {
		write(2, "couldn't write\n", 15);
		exit(1);
	}
	ret = write(1, loaded_file, total_len);
	if(ret < 0) {
		write(2, "couldn't write\n", 15);
		exit(1);
	}
}

void
process_fd(uint64_t fd)
{
	int64_t read_bytes;

	read_bytes = 0;

	while(1) {
		read_bytes = read(fd, loaded_file + total_len, 0x1000);

		/* debug */
		//fprintf(stderr, "read_bytes: %lld\n", read_bytes);

		if(read_bytes < 0) {
			write(2, "couldn't read\n", 14);
			exit(1);
		}
		if(read_bytes == 0) {
			break;
		}

		total_len += read_bytes;
	}

	elf_wrap();
}

void
process_file(char *filename)
{
	int64_t fd;

	fd = open(filename, 0);
	if(fd < 0) {
		write(2, "couldn't open\n", 14);
		exit(1);
	}

	process_fd(fd);
}

int main(int argc, char **argv) {
	uint64_t i;
	char *arg;

	loaded_file = (uint8_t *)mmap(NULL, 0x8000, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if(loaded_file == MAP_FAILED) { write(2, "loaded_file\n", 12); exit(1); }

	for(i = 1; i < (uint64_t)argc; ++i) {
		arg = argv[i];
		if(*arg == '-') {
			++arg;
			if(*arg == 'h') {
				write(1, "-h to help. -f <FILE> to read from FILE. Otherwise read from stdin\n", 67);
				exit(0);
			}
			if(*arg == 'f') {
				if(i + 1 >= (uint64_t)argc) {
					write(2, "expected filename after -f\n", 26);
					exit(1);
				}
				++i;
				process_file(argv[i]);
				return 0;
			}
		}
	}

	process_fd(0);
	return 0;
}

