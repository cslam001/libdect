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
	uint8_t uak[DECT_AUTH_KEY_LEN];
	unsigned int i;
	FILE *f;

	f = dect_keyfile_open("r");
	if (f == NULL)
		goto err;

	memset(&ripui, 0, sizeof(ripui));
	ripui.put = DECT_IPUI_N;

	if (fscanf(f, "N|%04hx|%05x|",
		   &ripui.pun.n.ipei.emc,
		   &ripui.pun.n.ipei.psn) != 2)
		goto err;

	for (i = 0; i < DECT_AUTH_KEY_LEN; i++) {
		if (fscanf(f, "%02hhx", &uak[i]) != 1)
			goto err;
	}

	if (dect_ipui_cmp(ipui, &ripui))
		goto err;

	memcpy(_uak, uak, DECT_AUTH_KEY_LEN);

	fclose(f);
	return 0;

err:
	fprintf(stderr, "Could not find UAK for IPUI N %4x %5x, use "
			"'pp-access-rights' to allocate a new one\n",
		ipui->pun.n.ipei.emc, ipui->pun.n.ipei.psn);
	return -1;
}
