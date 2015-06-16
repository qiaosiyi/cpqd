extern int tb[PORTNUM][TTPORTNUM*2];
uint32_t ip_src=0;
uint32_t ip_dst=0;
uint32_t ip_proto=0;
uint32_t src_port=0;
uint32_t dst_port=0;

// FILE *fp=fopen("1.txt","a");
// fprintf(fp, "dealing dp_actions_output_port\n");
// fclose(fp);

void
dp_actions_output_port(struct packet *pkt, uint32_t out_port, uint32_t out_queue, uint16_t max_len, uint64_t cookie) {
	//printf("dp actions output port\n");
	if(pkt->handle_std->table_miss){
       // printf("dealing dp_actions_output_port table_miss\n");
        static int conter3 = 0;
        conter3++;
        FILE *fp=fopen("qsy.txt","a");
        fprintf(fp, "dealing %d dp_actions_output_port table_miss\n",conter3);
        printf("dealing %d dp_actions_output_port table_miss\n",conter3);
        time_t timep;
        time (&timep);
        printf("\n\n%s \ta miss packet,dpid=%llu\n\n",ctime(&timep),pkt -> dp ->id);
        
		uint32_t temp;
        temp = out_port;
        printf("temp=%d\n",temp);
        if(pkt->handle_std->proto->ipv4 != NULL){
			ip_proto=pkt->handle_std->proto->ipv4->ip_proto;
			ip_src=pkt->handle_std->proto->ipv4->ip_src;
			ip_dst=pkt->handle_std->proto->ipv4->ip_dst;
		}
		if( (ip_proto == 6) || (ip_proto == 0) ){
			if (pkt->handle_std->proto->tcp!=NULL){
				src_port=pkt->handle_std->proto->tcp->tcp_src;
				dst_port=pkt->handle_std->proto->tcp->tcp_dst;
			}
		}
		if( ip_proto == 17 ){
			if(pkt->handle_std->proto->udp != NULL){
				src_port=pkt->handle_std->proto->udp->udp_src;
				dst_port=pkt->handle_std->proto->udp->udp_dst;
			}
		}
		//pirme 999907  999917  999931  999953  999959  999961  999979  999983
		int prime[5]={999907,999917,999931,999953,999959};
		int result;
		result = 0;
		result = ip_proto % prime[0] + ip_src % prime[1] + ip_dst % prime[2]+ src_port % prime[3]+ dst_port % prime[4];
		result = result % 99999 + 1;//100000;
        printf("result=%d\n",result);
		int inpn;
		inpn = pkt->in_port - 1;
        printf("inpn=%d\n",inpn);
        int i;
        if(tb[0][TTPORTNUM*2-1] == 0){
            out_port = temp;
            printf("out_port = temp =%d\n", temp);
        }else{
            for(i=0;i<TTPORTNUM;i++){
                if((tb[inpn][2*i]<=result)&&(result<=tb[inpn][2*i+1])){
                    if(i!=TTPORTNUM-1){
                        out_port = i+1;
                        printf("out_port = %d\n",out_port );
                    }
                    if(i==TTPORTNUM-1){
                        out_port = OFPP_CONTROLLER;
                        printf("out_port = OFPP_CONTROLLER\n");
                    }
                    break;
                }
            }
        }
		// for( i=0;i<TTPORTNUM;i++){//TTPORTNUM
		// 	if((tb[inpn][2*i]<=result)&&(result<=tb[inpn][2*i+1])){
		// 		if(i!=TTPORTNUM-1)
		// 			out_port = i+1;
		// 		if(i==TTPORTNUM-1)
		// 			out_port = OFPP_CONTROLLER;
		// 	}else{
		// 		out_port = OFPP_CONTROLLER;
  //               printf("out_port=OFPP_CONTROLLER\n");
  //           }
		// }
        if(pkt->handle_std->proto->ipv4 == NULL ){
            out_port = temp;//don't care proto packets
        }
        //out_port = OFPP_CONTROLLER;//unable the spread function, this is the default packet-in action.
        fprintf(fp, "out_port=%x in_port=%d proto=%d \n",out_port,inpn,ip_proto);
        printf("out_port=%x in_port=%d proto=%d \n",out_port,inpn,ip_proto);
       // fprintf(fp, "dealing finish dp_actions_output_port table_miss\n");
        fclose(fp);
	}


    switch (out_port) {
        case (OFPP_TABLE): {
            if (pkt->packet_out) {
                // NOTE: hackish; makes sure packet cannot be resubmit to pipeline again.
                pkt->packet_out = false;
                pipeline_process_packet(pkt->dp->pipeline, pkt);
            } else {
                VLOG_WARN_RL(LOG_MODULE, &rl, "Trying to resubmit packet to pipeline.");
            }
            break;
        }
        case (OFPP_IN_PORT): {
            dp_ports_output(pkt->dp, pkt->buffer, pkt->in_port, 0);
            break;
        }
        case (OFPP_CONTROLLER): {
            //fprintf(fp, "dealing finish out_port OFPP_CONTROLLER \n");
            //fclose(fp);
            struct ofl_msg_packet_in msg;
            msg.header.type = OFPT_PACKET_IN;
            msg.total_len   = pkt->buffer->size;
            msg.reason = pkt->handle_std->table_miss? OFPR_NO_MATCH:OFPR_ACTION;
            msg.table_id = pkt->table_id;
            msg.data        = pkt->buffer->data;
            msg.cookie = cookie;

            if (pkt->dp->config.miss_send_len != OFPCML_NO_BUFFER){
                dp_buffers_save(pkt->dp->buffers, pkt);
                msg.buffer_id = pkt->buffer_id;
                msg.data_length = MIN(max_len, pkt->buffer->size);
            }
            else {
                msg.buffer_id = OFP_NO_BUFFER;
                msg.data_length =  pkt->buffer->size;
            }

            if (!pkt->handle_std->valid){
                packet_handle_std_validate(pkt->handle_std);
            }
            /* In this implementation the fields in_port and in_phy_port
                always will be the same, because we are not considering logical
                ports*/
            msg.match = (struct ofl_match_header*) &pkt->handle_std->match;
            dp_send_message(pkt->dp, (struct ofl_msg_header *)&msg, NULL);
            break;
        }
        case (OFPP_FLOOD):
        case (OFPP_ALL): {
            dp_ports_output_all(pkt->dp, pkt->buffer, pkt->in_port, out_port == OFPP_FLOOD);
            break;
        }
        case (OFPP_NORMAL):
            // TODO Zoltan: Implement
        case (OFPP_LOCAL):
        default: {
            if (pkt->in_port == out_port) {
                VLOG_WARN_RL(LOG_MODULE, &rl, "can't directly forward to input port.");
            } else {
                VLOG_DBG_RL(LOG_MODULE, &rl, "Outputting packet on port %u.", out_port);
                dp_ports_output(pkt->dp, pkt->buffer, out_port, out_queue);///
            }
        }
    }
}
