#ifndef __USR_FILE_H__
#define __USR_FILE_H__

int last_access_file(const char *dirpath, char *latest, char *second);

int find_file_in_path(char *dirname, const char *filename, char *outpath);

int open_in_dir(const char *dir, const char *filename);

size_t get_fdisk_size(void);

size_t get_fdisk_free(void);

#endif /* __USR_FILE_H__ */
