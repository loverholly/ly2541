#include "common.h"

#define MAX_FILES 1024
typedef struct {
	char name[NAME_MAX + 1];
	time_t mtime;
} Entry;

static Entry buf[MAX_FILES];

static int cmp_mtime(const void *a, const void *b)
{
	const Entry *ea = (const Entry *)a;
	const Entry *eb = (const Entry *)b;
	return (eb->mtime > ea->mtime) - (eb->mtime < ea->mtime);
}

int last_access_file(const char *dirpath, char *latest, char *second)
{
	DIR *d = opendir(dirpath ? dirpath : ".");
	if (!d)
		return -1;

	int n = 0;
	struct dirent *de;
	while ((de = readdir(d)) && n < MAX_FILES) {
		if (de->d_type != DT_REG)
			continue;

		struct stat st;
		char path[PATH_MAX];
		snprintf(path, sizeof(path), "%s/%s",
		         dirpath ? dirpath : ".", de->d_name);

		if (stat(path, &st) == -1)
			continue;

		strcpy(buf[n].name, de->d_name);
		buf[n].name[NAME_MAX] = '\0';
		buf[n].mtime = st.st_mtime;
		++n;
	}
	closedir(d);

	if (n == 0) {
		errno = ENOENT;
		return -1;
	}

	qsort(buf, n, sizeof(Entry), cmp_mtime);
	if (latest) {
		strncpy(latest, buf[0].name, NAME_MAX);
		latest[NAME_MAX] = '\0';
	}

	if (second) {
		if (n >= 2) {
			strncpy(second, buf[1].name, NAME_MAX);
			second[NAME_MAX] = '\0';
		} else {
			second[0] = '\0';        /* 只有一个文件 */
		}
	}

	return 0;
}

int find_file_in_path(char *dirname, const char *filename, char *outpath)
{
	if (!filename || !outpath) {
		errno = EINVAL;
		return -1;
	}

	DIR *d = opendir(dirname);
	if (!d)
		return -1;          /* errno 已由 opendir 设置 */

	struct dirent *ent;
	int found = 0;

	while ((ent = readdir(d))) {
		if (ent->d_type != DT_REG && ent->d_type != DT_LNK)
			continue;

		if (strcmp(ent->d_name, filename) == 0) {
			if (!getcwd(outpath, PATH_MAX)) {
				closedir(d);
				return -1;
			}

			if (outpath) {
				strncat(outpath, "/",   PATH_MAX - strlen(outpath) - 1);
				strncat(outpath, filename, PATH_MAX - strlen(outpath) - 1);
			}

			found = 1;
			break;
		}
	}
	closedir(d);

	if (!found) {
		errno = ENOENT;
		return -1;
	}

	return 0;
}

int open_in_dir(const char *dir, const char *filename)
{
	if (!dir || !filename) {
		return -1;
	}

	/* 1. 保存原工作目录以便恢复 */
	char old_cwd[PATH_MAX];
	if (!getcwd(old_cwd, sizeof(old_cwd))) {
		return -1;
	}

	/* 2. 切换到目标目录 */
	if (chdir(dir) == -1) {
		return -1;
	}

	/* 3. 打开文件 */
	int fd = open(filename, O_RDWR);

	/* 4. 无论成功与否，恢复原工作目录 */
	if (chdir(old_cwd) == -1) {
		/* 恢复失败，只能返回错误 */
		close(fd);
		return -1;
	}

	return fd;
}

size_t get_fdisk_size(void)
{
	size_t disk_size = 0;
	struct statvfs s;

	if (statvfs("/", &s) != 0) {
		perror("statvfs");
	}

	disk_size = s.f_frsize * s.f_blocks;
	dbg_printf("disk full size %ld\n", disk_size);
	return disk_size;
}

size_t get_fdisk_free(void)
{
	size_t disk_size = 0;
	struct statvfs st;

	if (statvfs("/", &st) != 0) {
		perror("statvfs");
	}

	disk_size = st.f_bavail * st.f_frsize;
	dbg_printf("disk free size %ld\n", disk_size);
	return disk_size;
}
