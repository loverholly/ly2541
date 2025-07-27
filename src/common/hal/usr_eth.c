#include "common.h"

#define INTERFACES_FILE "/etc/network/interfaces"
#define BACKUP_FILE     "/etc/interfaces.bak"

int set_eth0_static_ip(char *ip, char *netmask, char *gateway)
{
	if (!ip && !netmask && !gateway) {
		fprintf(stderr, "NULL argument\n");
		return -1;
	}

	struct stat st;
	/* if file not exist, backup the file */
	if (stat(BACKUP_FILE, &st) != 0) {
		if (rename(INTERFACES_FILE, BACKUP_FILE) != 0 && errno != ENOENT) {
			perror("rename backup");
			return -1;
		}
	}

	FILE *fin  = fopen(BACKUP_FILE, "r");
	if (!fin) {
		fin = fopen(INTERFACES_FILE, "r");
	}

	FILE *fout = fopen(INTERFACES_FILE, "w");
	if (!fin || !fout) {
		perror("open interfaces");
		if (fin)
			fclose(fin);
		if (fout)
			fclose(fout);
		return -1;
	}

	char line[512];
	int inside_eth0 = 0;

	while (fgets(line, sizeof(line), fin)) {
		if (strstr(line, "iface eth0") && strstr(line, "static"))
			inside_eth0 = 1;

		if (inside_eth0 &&
		    (line[0] == '\n' ||
		     (strncmp(line, "iface", 5) == 0 && !strstr(line, "eth0"))))
			inside_eth0 = 0;

		if (inside_eth0) {
			if (strstr(line, "address"))
				fprintf(fout, "    address %s\n", ip);
			else if (strstr(line, "netmask") && netmask)
				fprintf(fout, "    netmask %s\n", netmask);
			else if (strstr(line, "gateway") && gateway)
				fprintf(fout, "    gateway %s\n", gateway);
			else
				fputs(line, fout);
		} else {
			fputs(line, fout);
		}
	}

	fclose(fin);
	fclose(fout);
	return 0;
}
