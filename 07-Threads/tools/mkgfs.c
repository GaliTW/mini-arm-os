#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <dirent.h>
#include <string.h>

#define TYPE_DIR 0
#define TYPE_FILE 1

FILE *outfile;

void usage(const char *binname)
{
	printf("Usage: %s <dir> [outfile]\n", binname);
	exit(-1);
}

static inline void wb(uint8_t v)
{
	fwrite(&v, 1, 1, outfile);
}

static inline void ww(uint32_t v)
{
	wb((v >>  0) & 0xff);
	wb((v >>  8) & 0xff);
	wb((v >> 16) & 0xff);
	wb((v >> 24) & 0xff);
}

static inline void wcptr(const char *cptr, size_t size)
{
	fwrite(cptr, size, 1, outfile);
}

static inline void wfile(const char *cptr, size_t size)
{
	fwrite(cptr, size, 1, outfile);
}

void processdir(const char *path, DIR *dirp, const uint32_t parent, uint32_t children)
{
	char fullpath[1024];
	char buf[16 * 1024];
	struct dirent *ent;
	DIR * rec_dirp;

	uint32_t size, w;
	uint32_t sibling, pre_sibling = 0;
	FILE *infile;

	strcpy(fullpath, path);
	strcat(fullpath, "/");
	char *sub_path = fullpath + strlen(fullpath);

	while ((ent = readdir(dirp))) {
		strcpy(sub_path, ent->d_name);

		if (ent->d_type == DT_DIR) {
			if (strcmp(ent->d_name, ".") == 0)
				continue;
			if (strcmp(ent->d_name, "..") == 0)
				continue;

			rec_dirp = opendir(fullpath);
			if (!rec_dirp) {
				perror("opening input dir");
				exit(-1);
			}

			wb(TYPE_DIR);
			uint32_t my_parent = ftell(outfile);
			ww(parent);

			sibling = ftell(outfile);
			ww(children);
			if (pre_sibling == 0) {
				fseek(outfile, children, SEEK_SET);
				ww(sibling);
			} else {
				fseek(outfile, pre_sibling, SEEK_SET);
				ww(sibling);
			}
			fseek(outfile, sibling + 4, SEEK_SET);
			pre_sibling = sibling;

			uint32_t my_children = ftell(outfile);
			ww(my_children);

			wcptr(ent->d_name, strlen(ent->d_name));
			wb(0);

			processdir(fullpath, rec_dirp, my_parent, my_children);
			closedir(rec_dirp);
		} else {
			infile = fopen(fullpath, "rb");
			if (!infile) {
				perror("opening input file");
				exit(-1);
			}

			wb(TYPE_FILE);
			ww(parent);

			sibling = ftell(outfile);
			ww(children);
			if (pre_sibling == 0) {
				fseek(outfile, children, SEEK_SET);
				ww(sibling);
			} else {
				fseek(outfile, pre_sibling, SEEK_SET);
				ww(sibling);
			}
			fseek(outfile, sibling + 4, SEEK_SET);
			pre_sibling = sibling;

			fseek(infile, 0, SEEK_END);
			size = ftell(infile);
			fseek(infile, 0, SEEK_SET);
			ww(size);

			wcptr(ent->d_name, strlen(ent->d_name));
			wb(0);

			while (size) {
				w = size > 16 * 1024 ? 16 * 1024 : size;
				fread(buf, 1, w, infile);
				fwrite(buf, 1, w, outfile);
				size -= w;
			}
			fclose(infile);
		}
	}
}

int main(int argc, char **argv)
{
	char path[1024];
	DIR *dirp;
	uint32_t parent, sibling, children;

	if (argc == 3)
		outfile = fopen(argv[2], "wb");
	else if (argc == 2)
		outfile = stdout;
	else
		usage(argv[0]);

	realpath(argv[1], path);
	dirp = opendir(path);

	if (!outfile) {
		perror("opening output file");
		exit(-1);
	}

	if (!dirp) {
		perror("opening directory");
		exit(-1);
	}

	/* special code */
	ww(0x006AF500);

	/* gen root dir */
	wb(TYPE_DIR);                // type

	parent = ftell(outfile);
	ww(parent);                  // parent = self

	sibling = ftell(outfile);
	ww(sibling);                 // sibling = self

	children = ftell(outfile);
	ww(children);                // children = self

	wb('/');                     // name
	wb('\0');

	processdir(path, dirp, parent, children);

	if (outfile != stdout)
		fclose(outfile);
	closedir(dirp);

	return 0;
}
