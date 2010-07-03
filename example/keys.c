#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <inttypes.h>

#include <dect/libdect.h>
#include <dect/auth.h>
#include "common.h"

static FILE *dect_keyfile_open(const char *mode)
{
	char name[PATH_MAX];

	snprintf(name, sizeof(name), "%s/%s", getenv("HOME"), "dect.keys");
	return fopen(name, mode);
}

int dect_write_uak(const struct dect_ipui *ipui,
		   const uint8_t uak[DECT_AUTH_KEY_LEN])
{
	unsigned int i;
	FILE *f;

	f = dect_keyfile_open("w");
	if (f == NULL)
		return -1;

	fprintf(f, "N|%04x|%05x|", ipui->pun.n.ipei.emc, ipui->pun.n.ipei.psn);
	for (i = 0; i < DECT_AUTH_KEY_LEN; i++)
		fprintf(f, "%.2x", uak[i]);
	fprintf(f, "\n");

	fclose(f);
	return 0;
}

int dect_read_uak(const struct dect_ipui *ipui, uint8_t _uak[DECT_AUTH_KEY_LEN])
{
	struct dect_ipui ripui;
	uint8_t uak[16];
	unsigned int i;
	FILE *f;

	f = dect_keyfile_open("r");
	if (f == NULL)
		return -1;

	if (fscanf(f, "N|%04hx|%05x|", &ripui.pun.n.ipei.emc, &ripui.pun.n.ipei.psn) != 2)
		return -1;

	for (i = 0; i < DECT_AUTH_KEY_LEN; i++) {
		if (fscanf(f, "%02hhx", &uak[i]) != 1)
			return -1;
	}

	if (ipui->pun.n.ipei.emc != ripui.pun.n.ipei.emc ||
	    ipui->pun.n.ipei.psn != ripui.pun.n.ipei.psn)
		return -1;

	memcpy(_uak, uak, DECT_AUTH_KEY_LEN);

	fclose(f);
	return 0;
}
