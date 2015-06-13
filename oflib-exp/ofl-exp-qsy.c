/*
 * Copyright (c) 2015 qsy Networks
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>

#include "openflow/openflow.h"
#include "openflow/qsy-ext.h"
#include "ofl-exp-qsy.h"
#include "../oflib/ofl-print.h"
#include "../oflib/ofl-log.h"

#define LOG_MODULE ofl_exp_nx
OFL_LOG_INIT(LOG_MODULE)
/*
enum qsy_type {
    QXT_ENABLE=1,
	QXT_ENABLE_GOOD,
	
	QXT_ACT_SET_CONFIG,
	QXT_ACT_SET_CONFIG_GOOD,
	
	QXT_DISABLE,
	QXT_DISABLE_GOOD
 };*/

extern int tb[PORTNUM][TTPORTNUM*2];

ofl_err
ofl_exp_qsy_msg_unpack(struct ofp_header *oh, size_t *len, struct ofl_msg_experimenter **msg) {
    //printf("dealing ofl_exp_msg_unpack QX_VENDOR_ID\n");
    struct qsy_header *exp;

    if (*len < sizeof(struct qsy_header)) {
        OFL_LOG_WARN(LOG_MODULE, "Received EXPERIMENTER message has invalid length (%zu).", *len);
        return ofl_error(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
    }

    exp = (struct qsy_header *)oh;

    if (ntohl(exp->vendor) == QX_VENDOR_ID) {//1

        switch (ntohl(exp->subtype)) {
            case (QXT_ENABLE):{
                static int conter1 = 0;
                conter1++;
                struct qxt_enable *src;

                if (*len < sizeof(struct qxt_enable)) {
                    OFL_LOG_WARN(LOG_MODULE, "Received QXT_ENABLE message has invalid length (%zu).", *len);
                    return ofl_error(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
                }
                *len -= sizeof(struct qxt_enable);

                src = (struct qxt_enable *)exp;
               
                printf("Received %d QXT_ENABLE %c \n",conter1,src->enable);
                return -1;
            }
            case (QXT_ACT_SET_CONFIG): {//2
				static int conter2 = 0;
                conter2++;
                int i1=1;
                struct qx_config *src;
				//struct ofl_exp_qsy_config_result *dst;
				

 				if (*len < sizeof(struct qx_config)) {
                    OFL_LOG_WARN(LOG_MODULE, "Received QXT_ACT_SET_CONFIG message has invalid length (%zu).", *len);
                    return ofl_error(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
                }
                *len -= sizeof(struct qx_config);

                src = (struct qx_config *)exp;

				int i=0,j=0;
				for(i=0;i<PORTNUM;i++){
					for(j=0;j<TTPORTNUM*2;j++){
						tb[i][j] = src->neighbor_table_utilization[i][j];
                        printf("%x %d ",tb[i][j],i1++);
					}
                    printf("\n");
                    i1=1;
				}
				printf("%d Received CONFIG MESSAGE\n",conter2);
				//free(msg);
                return 1;
            
            }

			case (QXT_DISABLE):
            default: {
                OFL_LOG_WARN(LOG_MODULE, "Trying to unpack unknown Qsy Experimenter message.");
                return ofl_error(OFPET_BAD_REQUEST, OFPBRC_BAD_EXPERIMENTER);
            }
        }
    } else {
        OFL_LOG_WARN(LOG_MODULE, "Trying to unpack Qsy Experimenter message.");
        return ofl_error(OFPET_BAD_REQUEST, OFPBRC_BAD_EXPERIMENTER);
    }
    free(msg);
    return 0;
}
