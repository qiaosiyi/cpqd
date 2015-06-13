/*
 * Copyright (c) 2015 qsy Networks
 *
 */

#ifndef OFL_EXP_QSY_H
#define OFL_EXP_QSY_H 1



#include "../oflib/ofl-structs.h"
#include "../oflib/ofl-messages.h"


struct ofl_exp_qsy_msg_header {
    struct ofl_msg_experimenter   header; /* QX_VENDOR_ID */

    uint32_t   type;
};

struct ofl_exp_qsy_config_result {
	struct ofl_exp_qsy_msg_header header;
	uint32_t neighbor;
	uint16_t neighbor_table_utilization[8];
};



int
ofl_exp_qsy_msg_pack(struct ofl_msg_experimenter *msg, uint8_t **buf, size_t *buf_len);

ofl_err
ofl_exp_qsy_msg_unpack(struct ofp_header *oh, size_t *len, struct ofl_msg_experimenter **msg);

int
ofl_exp_qsy_msg_free(struct ofl_msg_experimenter *msg);

char *
ofl_exp_qsy_msg_to_string(struct ofl_msg_experimenter *msg);


#endif /* OFL_EXP_QSY_H */
