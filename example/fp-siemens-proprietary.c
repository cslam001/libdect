#include <dect/libdect.h>
#include <dect/terminal.h>
#include "common.h"

#define DECT_EMC_SIEMENS	2

#define DECT_IE_SIEMENS_TIME_DATE	0x3b
#define DECT_IE_REQUEST_SEQ		0x5b
#define DECT_IE_REQUEST_ACK		0x59
#define DECT_IE_SIEMENS_DISPLAY		0x54

static uint8_t seq;

static uint8_t time_date_req[] = {
	DECT_IE_SIEMENS_TIME_DATE, 9,
	0x03,				/* Unknown */
	0x12, 0x10, 0x10,		/* Date */
	0x00, 0x04,			/* Unknown */
	0x23, 0x45, 0x00		/* Time */
};

static uint8_t display_req[] = {
	DECT_IE_SIEMENS_DISPLAY, 11,
	0x1,				/* Unknown */
	'm', 'o', 'b', 'i', 'l', 't', 'e', 'i', 'l', '1'
};

static uint8_t display_test1[] = {
	0x58, 0x16,
	0x03, 0x14,			/* Subtype display, len 20 */
	DECT_C_CLEAR_DISPLAY,		/* Clear display and return home */
	0x91, 0x02, 0x02, 0x0f, 	/* IEC 2022: Private use 1 */
	DECT_C_ESC, 0x2d, 0x41,		/* IEC 2022: G1-designate 96-set ISO 8859-1 */
	'S', 'e', 'r', 'v', 'e', 'r', ' ', 'n', 'i', 'c', 'h', 't'
};

static uint8_t display_test2[] = {
	0x58, 0x15,
	0x03, 0x13,			/* Subtype display, len 19 */
	DECT_C_MOVE_NEXT_ROW,		/* Move down one row */
	0x91, 0x02, 0x02, 0x0e, 	/* IEC 2022: Private Use 1 */
	DECT_C_ESC, 0x2d, 0x41,		/* IEC 2022: G1-designate 96-set ISO 8859-1*/
	'e', 'r', 'r', 'e', 'i', 'c', 'h', 'b', 'a', 'r', '!'
};

static uint8_t events_req[] = {
	0x58, 0x0e,
	0x02, 0x0c,			/* Subtype events notification, len 12 */
	0x01, 0x01,			/* SMS waiting */
	0x04, 0x00,			/* missed calls */
	0x09, 0x00,			/* Unknown */
	0x0a, 0x00,			/* Unknown */
	0x0b, 0x01,			/* Unknown */
	0x0c, 0x00			/* Unknown */
};

static uint8_t prefix_req[] = {
	0x58, 0x10,
	0x09, 0x1, '0',			/* National area code prefix subtype */
	0x0a, 0x2, '0', '0',		/* Country code prefix subtype */
	0x0b, 0x3, '7', '6', '1',	/* Local area code subtype */
	0x0c, 0x2, '4', '9',		/* Country code subtype */
};

static void dect_facility_req(struct dect_handle *dh,
			      struct dect_ss_endpoint *sse,
			      const void *data, unsigned int len)
{
	struct dect_ie_escape_to_proprietary escape_to_proprietary;
	struct dect_mnss_param param = {
		.escape_to_proprietary	= &escape_to_proprietary,
	};

	escape_to_proprietary.emc		= DECT_EMC_SIEMENS;
	escape_to_proprietary.len		= len;
	memcpy(escape_to_proprietary.content, data, len);

	escape_to_proprietary.content[len]	= DECT_IE_REQUEST_SEQ;
	escape_to_proprietary.content[len + 1]	= 1;
	escape_to_proprietary.content[len + 2]	= seq++;
	escape_to_proprietary.len		+= 3;

	dect_mnss_facility_req(dh, sse, &param);
	dect_event_loop();
}

static void mnss_release_ind(struct dect_handle *dh, struct dect_ss_endpoint *sse,
		             struct dect_mnss_param *param)

{
	dect_event_loop_stop();
}

static struct dect_ss_ops ss_ops = {
	.mnss_release_ind	= mnss_release_ind,
};

static struct dect_ops ops = {
	.ss_ops			= &ss_ops,
};

int main(int argc, char **argv)
{
	struct dect_ss_endpoint *sse;

	dect_fp_common_options(argc, argv);
	dect_common_init(&ops, cluster);

	sse = dect_ss_endpoint_alloc(dh, &ipui);
	if (sse == NULL)
		return -1;

	dect_facility_req(dh, sse, time_date_req, sizeof(time_date_req));
	dect_facility_req(dh, sse, display_req, sizeof(display_req));
	dect_facility_req(dh, sse, display_test1, sizeof(display_test1));
	dect_facility_req(dh, sse, display_test2, sizeof(display_test2));
	dect_facility_req(dh, sse, events_req, sizeof(events_req));
	dect_facility_req(dh, sse, prefix_req, sizeof(prefix_req));

	return 0;
}
