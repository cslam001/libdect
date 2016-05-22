/*
 * DECT PP List Access (LiA) example
 *
 * Copyright (c) 2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>

#include <dect/libdect.h>
#include <dect/auth.h>
#include <dect/lia.h>
#include "common.h"

static void dect_iwu_info_req(struct dect_handle *dh, struct dect_call *call,
                        uint8_t list)
{
	struct dect_ie_iwu_to_iwu iwu_to_iwu;
	struct dect_mncc_iwu_info_param req = {
		.iwu_to_iwu	= &iwu_to_iwu,
	};

	iwu_to_iwu.sr		= true;
	iwu_to_iwu.pd		= DECT_IWU_TO_IWU_PD_LIST_ACCESS;
	iwu_to_iwu.len		= 3;
	iwu_to_iwu.data[0]	= DECT_LIA_CMD_START_SESSION;
	iwu_to_iwu.data[1]	= list;
	iwu_to_iwu.data[2]	= 0x0;

	dect_mncc_iwu_info_req(dh, call, &req);
}

static void dect_iwu_info_close(struct dect_handle *dh, struct dect_call *call,
                                uint8_t session)
{
        struct dect_ie_iwu_to_iwu iwu_to_iwu;
        struct dect_mncc_iwu_info_param req = {
                .iwu_to_iwu	= &iwu_to_iwu,
        };

        iwu_to_iwu.sr		= true;
        iwu_to_iwu.pd		= DECT_IWU_TO_IWU_PD_LIST_ACCESS;
        iwu_to_iwu.len		= 3;
        iwu_to_iwu.data[0]	= DECT_LIA_CMD_END_SESSION;
        iwu_to_iwu.data[1]	= session;
        iwu_to_iwu.data[2]	= 0x0;

        dect_mncc_iwu_info_req(dh, call, &req);
}

static void dect_iwu_info_ask_entries(struct dect_handle *dh, struct dect_call *call,
                                uint8_t session)
{
        struct dect_ie_iwu_to_iwu iwu_to_iwu;
        struct dect_mncc_iwu_info_param req = {
                .iwu_to_iwu	= &iwu_to_iwu,
        };

        iwu_to_iwu.sr		= true;
        iwu_to_iwu.pd		= DECT_IWU_TO_IWU_PD_LIST_ACCESS;
        iwu_to_iwu.len		= 3;
        iwu_to_iwu.data[0]	= DECT_LIA_CMD_QUERY_ENTRY_FIELDS;
        iwu_to_iwu.data[1]	= session;
        iwu_to_iwu.data[2]	= 0x0;

        dect_mncc_iwu_info_req(dh, call, &req);
}

static void dect_iwu_info_read_entry(struct dect_handle *dh, struct dect_call *call,
                                uint8_t session, uint8_t id, uint8_t index)
{
        struct dect_ie_iwu_to_iwu iwu_to_iwu;
        struct dect_mncc_iwu_info_param req = {
                .iwu_to_iwu	= &iwu_to_iwu,
        };

        iwu_to_iwu.sr		= true;
        iwu_to_iwu.pd		= DECT_IWU_TO_IWU_PD_LIST_ACCESS;
        iwu_to_iwu.len		= 6;
        iwu_to_iwu.data[0]	= DECT_LIA_CMD_READ_ENTRIES;
        iwu_to_iwu.data[1]	= session;
        iwu_to_iwu.data[2]	= 0x80 + index;
        iwu_to_iwu.data[3]	= 1;
        iwu_to_iwu.data[4]	= 0x0;
        iwu_to_iwu.data[5]	= id;

        dect_mncc_iwu_info_req(dh, call, &req);
}

static void dect_iwu_parse_data(uint8_t *data, uint8_t data_length)
{
        uint8_t id, len;
        uint8_t field_id, field_len;
        uint8_t pos = 0, t_pos, i;

        printf("parsing data packet with length: %d\n", data_length);
        while (pos < data_length) {
                id = data[pos++];
                len = data[pos++];
                if ((!(id & 0x80)) || !(len & 0x80)) {
                        printf("identifiers larger than 127 are not supported\n");
                        return;
                }
                id &= ~0x80;
                len &= ~0x80;

                printf("entry with id %d and length %d:\n", id, len);

                t_pos = pos;
                while (pos < (t_pos+len)) {
                        field_id = data[pos++];
                        field_len = data[pos++];
                        if (!(field_len & 0x80)) {
                                printf("identifiers larger than 127 are not supported\n");
                                return;
                        }
                        field_len &= ~0x80;

                        printf("\tfield with id 0x%x and length %d:\n", field_id, field_len);
                        printf("\t");
                        for (i = 0; i < field_len; i++) {
                                if ((data[pos] >= 0x80) || (data[pos] <= 0x1f))
                                        printf(" 0x%X ", data[pos++]);
                                else
                                        printf("%c", data[pos++]);
                        }
                        printf("\n");
                }
        }
}

static void dect_iwu_info_edit_entry(struct dect_handle *dh, struct dect_call *call,
                                uint8_t session, uint8_t entry, uint8_t field)
{
        struct dect_ie_iwu_to_iwu iwu_to_iwu;
        struct dect_mncc_iwu_info_param req = {
                .iwu_to_iwu	= &iwu_to_iwu,
        };

        iwu_to_iwu.sr		= true;
        iwu_to_iwu.pd		= DECT_IWU_TO_IWU_PD_LIST_ACCESS;
        iwu_to_iwu.len		= 4;
        iwu_to_iwu.data[0]	= DECT_LIA_CMD_EDIT_ENTRY;
        iwu_to_iwu.data[1]	= session;
        iwu_to_iwu.data[2]	= 0x80 + entry;
        iwu_to_iwu.data[3]	= field;

        dect_mncc_iwu_info_req(dh, call, &req);
}

/* #include <sys/time.h> */
/* void msleep (unsigned int ms) { */
/*         int microsecs; */
/*         struct timeval tv; */
/*         microsecs = ms * 1000; */
/*         tv.tv_sec  = microsecs / 1000000; */
/*         tv.tv_usec = microsecs % 1000000; */
/*         select (0, NULL, NULL, NULL, &tv);   */
/* } */

static void dect_iwu_info_save_entry(struct dect_handle *dh, struct dect_call *call,
                                uint8_t session, uint8_t entry, uint8_t field,
                                uint16_t data_len, uint8_t *data)
{
        struct dect_ie_iwu_to_iwu iwu_to_iwu;
        struct dect_mncc_iwu_info_param req = {
                .iwu_to_iwu	= &iwu_to_iwu,
        };
        uint16_t part_len, i;
        uint16_t left_data_len = data_len;
        uint16_t pos = 0;
        bool last;

        iwu_to_iwu.sr		= true;
        iwu_to_iwu.pd		= DECT_IWU_TO_IWU_PD_LIST_ACCESS;
        iwu_to_iwu.len		= 3;
        iwu_to_iwu.data[0]	= DECT_LIA_CMD_SAVE_ENTRY;
        iwu_to_iwu.data[1]	= session;
        iwu_to_iwu.data[2]	= 0x80 + entry;

        dect_mncc_iwu_info_req(dh, call, &req);

        while (left_data_len > 0) {
                if (left_data_len > 40) {
                        part_len = 40;
                        last = false;
                } else {
                        part_len = left_data_len;
                        last = true;
                }
                left_data_len -= part_len;

                iwu_to_iwu.sr		= true;
                iwu_to_iwu.pd		= DECT_IWU_TO_IWU_PD_LIST_ACCESS;
                iwu_to_iwu.len		= 6 + part_len;
                iwu_to_iwu.data[0]	= last ? DECT_LIA_CMD_DATA_PACKET_LAST : DECT_LIA_CMD_DATA_PACKET;
                iwu_to_iwu.data[1]	= session;
                iwu_to_iwu.data[2]	= 0x80 + entry;
                iwu_to_iwu.data[3]	= 0x80 + part_len + 2; /* entry len */
                iwu_to_iwu.data[4]	= field;
                iwu_to_iwu.data[5]	= 0x80 + part_len; /* field len */
                for (i = 0; i < part_len; i++) {
                        iwu_to_iwu.data[6+i] = data[pos++];
                }

                /* msleep(100); */
                dect_mncc_iwu_info_req(dh, call, &req);
        }
}

static void dect_mncc_iwu_info_ind(struct dect_handle *dh, struct dect_call *call,
                                struct dect_mncc_iwu_info_param *param)
{
        struct dect_ie_iwu_to_iwu *iwu_to_iwu = param->iwu_to_iwu;
        uint8_t command = iwu_to_iwu->data[0];
        static uint8_t data_to_parse = 0;

        switch (command) {
        case DECT_LIA_CMD_START_SESSION_CONFIRM: {
                uint8_t session = iwu_to_iwu->data[2];

                dect_iwu_info_ask_entries(dh, call, session);
                
                break;
        }
        case DECT_LIA_CMD_QUERY_ENTRY_FIELDS_CONFIRM: {
                uint8_t session = iwu_to_iwu->data[1];
                int i, j;
                uint8_t rw_entries = iwu_to_iwu->data[2];
                uint8_t ro_entries = iwu_to_iwu->data[2+1+rw_entries];

                printf("editable entries:\n");
                for (i = 0; i < rw_entries; i++) {
                        printf("0x%x ", iwu_to_iwu->data[2+1+i]);
                }
                printf("\n");

                printf("noneditable entries:\n");
                for (i = 0; i < ro_entries; i++) {
                        printf("0x%x ", iwu_to_iwu->data[2+1+rw_entries+1+i]);
                }
                printf("\n");

                dect_iwu_info_edit_entry(dh, call, session, 1, 0xb);
                data_to_parse++;

                break;
        }
        case DECT_LIA_CMD_READ_ENTRIES_CONFIRM: {
                uint8_t session = iwu_to_iwu->data[1];
                uint8_t index = iwu_to_iwu->data[2];
                uint8_t cnt = iwu_to_iwu->data[3];
                
                printf("confirm. index: %x, cnt: %x\n", index, cnt);

                break;
        }
        case DECT_LIA_CMD_EDIT_ENTRY_CONFIRM: {
                uint8_t session = iwu_to_iwu->data[1];
                
                printf("edit confirm.\n");

                break;
        }
        case DECT_LIA_CMD_SAVE_ENTRY_CONFIRM: {
                uint8_t session = iwu_to_iwu->data[1];
                
                printf("save confirm.\n");

                printf("closing session 0x%x\n", session);
                dect_iwu_info_close(dh, call, session);

                break;
        }
        case DECT_LIA_CMD_DATA_PACKET:
        case DECT_LIA_CMD_DATA_PACKET_LAST: {
                uint8_t session = iwu_to_iwu->data[1];
                uint8_t data_length = iwu_to_iwu->len;
                uint8_t data[256];

                data_to_parse--;
                dect_iwu_parse_data(&iwu_to_iwu->data[2], data_length-2);

                /* if (data_to_parse == 0) { */
                /*         printf("closing session 0x%x\n", session); */
                /*         dect_iwu_info_close(dh, call, session); */
                /* } */

                memset(data, 'c', sizeof(data));

                data[0] = 0xc0;
                data[1] = '1';
                dect_iwu_info_save_entry(dh, call, session, 1, 0x3, 2, &data[0]);

                break;
        }
        case DECT_LIA_CMD_END_SESSION_CONFIRM: {
                uint8_t session = iwu_to_iwu->data[1];
                printf("session 0x%x closed\n", session);
                dect_common_cleanup(dh);
                return;
        }
        }
}

static void dect_mncc_call_proc_ind(struct dect_handle *dh, struct dect_call *call,
				    struct dect_mncc_call_proc_param *param)
{
        /* dect_iwu_info_req(dh, call, DECT_LIA_LIST_INTERNAL_NAMES); */
	/* dect_iwu_info_req(dh, call, DECT_LIA_LIST_LINE_SETTINGS); */
	/* dect_iwu_info_req(dh, call, DECT_LIA_LIST_CONTACTS); */
	dect_iwu_info_req(dh, call, DECT_LIA_LIST_DECT_SYSTEM_SETTINGS);
}

static void dect_mncc_reject_ind(struct dect_handle *dh, struct dect_call *call,
				 enum dect_causes cause,
				 struct dect_mncc_release_param *param)
{
	dect_event_loop_stop();
}

static void dect_mncc_release_ind(struct dect_handle *dh, struct dect_call *call,
				  struct dect_mncc_release_param *param)
{
	struct dect_mncc_release_param res = {};

	dect_mncc_release_res(dh, call, &res);
	dect_event_loop_stop();
}

static void dect_open_call(struct dect_handle *dh, const struct dect_ipui *ipui)
{
	struct dect_ie_basic_service basic_service;
	struct dect_mncc_setup_param req = {
		.basic_service	= &basic_service,
	};
	struct dect_call *call;

	call = dect_call_alloc(dh);
	if (call == NULL)
		return;

	basic_service.class   = DECT_CALL_CLASS_LIA_SERVICE_SETUP;
	basic_service.service = DECT_SERVICE_WIDEBAND_SPEECH;

	dect_mncc_setup_req(dh, call, ipui, &req);
}

static void dect_dl_establish_cfm(struct dect_handle *dh, bool success,
				  struct dect_data_link *ddl,
				  const struct dect_mac_conn_params *mcp)
{
	dect_open_call(dh, &ipui);
}

static struct dect_cc_ops cc_ops = {
	.mncc_call_proc_ind	= dect_mncc_call_proc_ind,
	.mncc_release_ind	= dect_mncc_release_ind,
	.mncc_reject_ind	= dect_mncc_reject_ind,
        .mncc_iwu_info_ind      = dect_mncc_iwu_info_ind,
};

static struct dect_lce_ops lce_ops = {
	.dl_establish_cfm	= dect_dl_establish_cfm,
};

static struct dect_ops ops = {
	.lce_ops		= &lce_ops,
	.cc_ops			= &cc_ops,
};

int main(int argc, char **argv)
{
	const struct dect_fp_capabilities *fpc;
	struct dect_mac_conn_params mcp = {
		.service	= DECT_SERVICE_IN_MIN_DELAY,
                .slot       = DECT_FULL_SLOT,
	};

	dect_pp_common_options(argc, argv);
	dect_pp_common_init(&ops, cluster, &ipui);

	fpc = dect_llme_fp_capabilities(dh);
	if (!(fpc->ehlc2 & DECT_EHLC2_LIST_ACCESS_FEATURES)) {
		fprintf(stderr, "FP does not support List Access (LiA)\n");
		goto out;
	}

	dect_dl_establish_req(dh, &ipui, &mcp);
	dect_event_loop();
out:
	dect_common_cleanup(dh);
	return 0;
}
