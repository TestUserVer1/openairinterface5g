/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

/*! \file gNB_scheduler_ulsch.c
 * \brief gNB procedures for the ULSCH transport channel
 * \author Navid Nikaein and Raymond Knopp, Guido Casati
 * \date 2019
 * \email: guido.casati@iis.fraunhofer.de
 * \version 1.0
 * @ingroup _mac
 */


#include "LAYER2/NR_MAC_gNB/mac_proto.h"
#include "executables/softmodem-common.h"
#include "common/utils/nr/nr_common.h"


void nr_process_mac_pdu(
    module_id_t module_idP,
    rnti_t rnti,
    uint8_t CC_id,
    frame_t frameP,
    uint8_t *pduP,
    uint16_t mac_pdu_len)
{

    // This function is adapting code from the old
    // parse_header(...) and ue_send_sdu(...) functions of OAI LTE

    uint8_t *pdu_ptr = pduP, rx_lcid, done = 0;
    int pdu_len = mac_pdu_len;
    uint16_t mac_ce_len, mac_subheader_len, mac_sdu_len;


    //  For both DL/UL-SCH
    //  Except:
    //   - UL/DL-SCH: fixed-size MAC CE(known by LCID)
    //   - UL/DL-SCH: padding
    //   - UL-SCH:    MSG3 48-bits
    //  |0|1|2|3|4|5|6|7|  bit-wise
    //  |R|F|   LCID    |
    //  |       L       |
    //  |0|1|2|3|4|5|6|7|  bit-wise
    //  |R|F|   LCID    |
    //  |       L       |
    //  |       L       |

    //  For both DL/UL-SCH
    //  For:
    //   - UL/DL-SCH: fixed-size MAC CE(known by LCID)
    //   - UL/DL-SCH: padding, for single/multiple 1-oct padding CE(s)
    //   - UL-SCH:    MSG3 48-bits
    //  |0|1|2|3|4|5|6|7|  bit-wise
    //  |R|R|   LCID    |
    //  LCID: The Logical Channel ID field identifies the logical channel instance of the corresponding MAC SDU or the type of the corresponding MAC CE or padding as described in Tables 6.2.1-1 and 6.2.1-2 for the DL-SCH and UL-SCH respectively. There is one LCID field per MAC subheader. The LCID field size is 6 bits;
    //  L: The Length field indicates the length of the corresponding MAC SDU or variable-sized MAC CE in bytes. There is one L field per MAC subheader except for subheaders corresponding to fixed-sized MAC CEs and padding. The size of the L field is indicated by the F field;
    //  F: lenght of L is 0:8 or 1:16 bits wide
    //  R: Reserved bit, set to zero.

    while (!done && pdu_len > 0){
        mac_ce_len = 0;
        mac_subheader_len = 1; //  default to fixed-length subheader = 1-oct
        mac_sdu_len = 0;
        rx_lcid = ((NR_MAC_SUBHEADER_FIXED *)pdu_ptr)->LCID;

        LOG_D(MAC, "LCID received at gNB side: %d \n", rx_lcid);

        switch(rx_lcid){
            //  MAC CE

            /*#ifdef DEBUG_HEADER_PARSING
              LOG_D(MAC, "[UE] LCID %d, PDU length %d\n", ((NR_MAC_SUBHEADER_FIXED *)pdu_ptr)->LCID, pdu_len);
            #endif*/
        case UL_SCH_LCID_RECOMMENDED_BITRATE_QUERY:
              // 38.321 Ch6.1.3.20
              mac_ce_len = 2;
              break;
        case UL_SCH_LCID_CONFIGURED_GRANT_CONFIRMATION:
                // 38.321 Ch6.1.3.7
                break;
        case UL_SCH_LCID_S_BSR:
        	//38.321 section 6.1.3.1
        	//fixed length
        	mac_ce_len =1;
        	/* Extract short BSR value */
        	break;

        case UL_SCH_LCID_S_TRUNCATED_BSR:
        	//38.321 section 6.1.3.1
        	//fixed length
        	mac_ce_len =1;
        	/* Extract short truncated BSR value */
        	break;

        case UL_SCH_LCID_L_BSR:
        	//38.321 section 6.1.3.1
        	//variable length
        	mac_ce_len |= (uint16_t)((NR_MAC_SUBHEADER_SHORT *)pdu_ptr)->L;
        	mac_subheader_len = 2;
        	if(((NR_MAC_SUBHEADER_SHORT *)pdu_ptr)->F){
        		mac_ce_len |= (uint16_t)(((NR_MAC_SUBHEADER_LONG *)pdu_ptr)->L2)<<8;
        		mac_subheader_len = 3;
        	}
        	/* Extract long BSR value */
        	break;

        case UL_SCH_LCID_L_TRUNCATED_BSR:
        	//38.321 section 6.1.3.1
        	//variable length
        	mac_ce_len |= (uint16_t)((NR_MAC_SUBHEADER_SHORT *)pdu_ptr)->L;
        	mac_subheader_len = 2;
        	if(((NR_MAC_SUBHEADER_SHORT *)pdu_ptr)->F){
        		mac_ce_len |= (uint16_t)(((NR_MAC_SUBHEADER_LONG *)pdu_ptr)->L2)<<8;
        		mac_subheader_len = 3;
        	}
        	/* Extract long truncated BSR value */
        	break;


        case UL_SCH_LCID_C_RNTI:
        	//38.321 section 6.1.3.2
        	//fixed length
        	mac_ce_len = 2;
        	/* Extract CRNTI value */
        	break;

        case UL_SCH_LCID_SINGLE_ENTRY_PHR:
        	//38.321 section 6.1.3.8
        	//fixed length
        	mac_ce_len = 2;
        	/* Extract SINGLE ENTRY PHR elements for PHR calculation */
        	break;

        case UL_SCH_LCID_MULTI_ENTRY_PHR_1_OCT:
        	//38.321 section 6.1.3.9
        	//  varialbe length
        	mac_ce_len |= (uint16_t)((NR_MAC_SUBHEADER_SHORT *)pdu_ptr)->L;
        	mac_subheader_len = 2;
        	if(((NR_MAC_SUBHEADER_SHORT *)pdu_ptr)->F){
        		mac_ce_len |= (uint16_t)(((NR_MAC_SUBHEADER_LONG *)pdu_ptr)->L2)<<8;
        		mac_subheader_len = 3;
        	}
        	/* Extract MULTI ENTRY PHR elements from single octet bitmap for PHR calculation */
        	break;

        case UL_SCH_LCID_MULTI_ENTRY_PHR_4_OCT:
        	//38.321 section 6.1.3.9
        	//  varialbe length
        	mac_ce_len |= (uint16_t)((NR_MAC_SUBHEADER_SHORT *)pdu_ptr)->L;
        	mac_subheader_len = 2;
        	if(((NR_MAC_SUBHEADER_SHORT *)pdu_ptr)->F){
        		mac_ce_len |= (uint16_t)(((NR_MAC_SUBHEADER_LONG *)pdu_ptr)->L2)<<8;
        		mac_subheader_len = 3;
        	}
        	/* Extract MULTI ENTRY PHR elements from four octets bitmap for PHR calculation */
        	break;

        case UL_SCH_LCID_PADDING:
        	done = 1;
        	//  end of MAC PDU, can ignore the rest.
        	break;

        // MAC SDUs
        case UL_SCH_LCID_SRB1:
              // todo
              break;
        case UL_SCH_LCID_SRB2:
              // todo
              break;
        case UL_SCH_LCID_SRB3:
              // todo
              break;
        case UL_SCH_LCID_CCCH_MSG3:
              // todo
              break;
        case UL_SCH_LCID_CCCH:
              // todo
              mac_subheader_len = 2;
              break;

        case UL_SCH_LCID_DTCH:
                //  check if LCID is valid at current time.
                if(((NR_MAC_SUBHEADER_SHORT *)pdu_ptr)->F){
                    //mac_sdu_len |= (uint16_t)(((NR_MAC_SUBHEADER_LONG *)pdu_ptr)->L2)<<8;
                    mac_subheader_len = 3;
                    mac_sdu_len = ((uint16_t)(((NR_MAC_SUBHEADER_LONG *) pdu_ptr)->L1 & 0x7f) << 8)
                    | ((uint16_t)((NR_MAC_SUBHEADER_LONG *) pdu_ptr)->L2 & 0xff);

                } else {
                  mac_sdu_len = (uint16_t)((NR_MAC_SUBHEADER_SHORT *)pdu_ptr)->L;
                  mac_subheader_len = 2;
                }

                LOG_D(MAC, "[UE %d] Frame %d : ULSCH -> UL-DTCH %d (gNB %d, %d bytes)\n", module_idP, frameP, rx_lcid, module_idP, mac_sdu_len);
		int UE_id = find_nr_UE_id(module_idP, rnti);
		RC.nrmac[module_idP]->UE_info.mac_stats[UE_id].lc_bytes_rx[rx_lcid] += mac_sdu_len;
                #if defined(ENABLE_MAC_PAYLOAD_DEBUG)
                    LOG_T(MAC, "[UE %d] First 32 bytes of DLSCH : \n", module_idP);

                    for (i = 0; i < 32; i++)
                      LOG_T(MAC, "%x.", (pdu_ptr + mac_subheader_len)[i]);

                    LOG_T(MAC, "\n");
                #endif
                if(IS_SOFTMODEM_NOS1){
                  mac_rlc_data_ind(module_idP,
                      0x1234,
                      module_idP,
                      frameP,
                      ENB_FLAG_YES,
                      MBMS_FLAG_NO,
                      rx_lcid,
                      (char *) (pdu_ptr + mac_subheader_len),
                      mac_sdu_len,
                      1,
                      NULL);
                }
                else{
                  mac_rlc_data_ind(module_idP,
                      rnti,
                      module_idP,
                      frameP,
                      ENB_FLAG_YES,
                      MBMS_FLAG_NO,
                      rx_lcid,
                      (char *) (pdu_ptr + mac_subheader_len),
                      mac_sdu_len,
                      1,
                      NULL);
                }


            break;

        default:
        	return;
        	break;
        }
        pdu_ptr += ( mac_subheader_len + mac_ce_len + mac_sdu_len );
        pdu_len -= ( mac_subheader_len + mac_ce_len + mac_sdu_len );

        if (pdu_len < 0) {
          LOG_E(MAC, "%s() residual mac pdu length < 0!, pdu_len: %d\n", __func__, pdu_len);
          return;
        }
    }
}

void handle_nr_ul_harq(uint16_t slot, NR_UE_sched_ctrl_t *sched_ctrl, NR_mac_stats_t *stats, nfapi_nr_crc_t crc_pdu) {

  int max_harq_rounds = 4; // TODO define macro
  uint8_t hrq_id = crc_pdu.harq_id;
  NR_UE_ul_harq_t *cur_harq = &sched_ctrl->ul_harq_processes[hrq_id];
  if (cur_harq->state==ACTIVE_SCHED) {
    if (!crc_pdu.tb_crc_status) {
      cur_harq->ndi ^= 1;
      cur_harq->round = 0;
      cur_harq->state = INACTIVE; // passed -> make inactive. can be used by scheduder for next grant
#ifdef UL_HARQ_PRINT
      printf("[HARQ HANDLER] Ulharq id %d crc passed, freeing it for scheduler\n",hrq_id);
#endif
    } else {
      cur_harq->round++;
      cur_harq->state = ACTIVE_NOT_SCHED;
#ifdef UL_HARQ_PRINT
      printf("[HARQ HANDLER] Ulharq id %d crc failed, requesting retransmission\n",hrq_id);
#endif
    }

    if (!(cur_harq->round<max_harq_rounds)) {
      cur_harq->ndi ^= 1;
      cur_harq->state = INACTIVE; // failed after 4 rounds -> make inactive
      cur_harq->round = 0;
      LOG_D(MAC,"[HARQ HANDLER] RNTI %x: Ulharq id %d crc failed in all round, freeing it for scheduler\n",crc_pdu.rnti,hrq_id);
      stats->ulsch_errors++;
    }
    return;
  } else
    LOG_E(MAC,"Incorrect ULSCH HARQ process %d or invalid state %d\n",hrq_id,cur_harq->state);
}

/*
* When data are received on PHY and transmitted to MAC
*/
void nr_rx_sdu(const module_id_t gnb_mod_idP,
               const int CC_idP,
               const frame_t frameP,
               const sub_frame_t slotP,
               const rnti_t rntiP,
               uint8_t *sduP,
               const uint16_t sdu_lenP,
               const uint16_t timing_advance,
               const uint8_t ul_cqi,
               const uint16_t rssi){
  int current_rnti = 0, UE_id = -1, harq_pid = 0;
  gNB_MAC_INST *gNB_mac = NULL;
  NR_UE_info_t *UE_info = NULL;
  NR_UE_sched_ctrl_t *UE_scheduling_control = NULL;

  current_rnti = rntiP;
  UE_id = find_nr_UE_id(gnb_mod_idP, current_rnti);
  gNB_mac = RC.nrmac[gnb_mod_idP];
  UE_info = &gNB_mac->UE_info;
  int target_snrx10 = gNB_mac->pusch_target_snrx10;

  NR_RA_t *ra = &gNB_mac->common_channels[CC_idP].ra[0];

  if (sduP != NULL) {
    T(T_GNB_MAC_UL_PDU_WITH_DATA, T_INT(gnb_mod_idP), T_INT(CC_idP),
      T_INT(rntiP), T_INT(frameP), T_INT(slotP), T_INT(-1) /* harq_pid */,
      T_BUFFER(sduP, sdu_lenP));
  }

  // random access pusch with TC-RNTI
  if (ra->state == WAIT_Msg3) {
    if (sduP != NULL) { // if the CRC passed

      if (ra->rnti != current_rnti) {
        LOG_E(MAC,
              "expected TC-RNTI %04x to match current RNTI %04x\n",
              ra->rnti,
              current_rnti);
        return;
      }
      free(ra->preambles.preamble_list);
      ra->state = RA_IDLE;
      LOG_I(MAC, "reset RA state information for RA-RNTI %04x\n", ra->rnti);
      const int UE_id = add_new_nr_ue(gnb_mod_idP, ra->rnti);
      UE_info->secondaryCellGroup[UE_id] = ra->secondaryCellGroup;
      compute_csi_bitlen (ra->secondaryCellGroup, UE_info, UE_id);
      UE_info->UE_beam_index[UE_id] = ra->beam_id;
      struct NR_ServingCellConfig__downlinkBWP_ToAddModList *bwpList = ra->secondaryCellGroup->spCellConfig->spCellConfigDedicated->downlinkBWP_ToAddModList;
      AssertFatal(bwpList->list.count == 1,
                  "downlinkBWP_ToAddModList has %d BWP!\n",
                  bwpList->list.count);
      const int bwp_id = 1;
      UE_info->UE_sched_ctrl[UE_id].active_bwp = bwpList->list.array[bwp_id - 1];
      LOG_I(MAC,
            "[gNB %d][RAPROC] PUSCH with TC_RNTI %x received correctly, "
            "adding UE MAC Context UE_id %d/RNTI %04x\n",
            gnb_mod_idP,
            current_rnti,
            UE_id,
            ra->rnti);
    }
    return;
  }

  if (UE_id != -1) {
    UE_scheduling_control = &(UE_info->UE_sched_ctrl[UE_id]);

    UE_info->mac_stats[UE_id].ulsch_total_bytes_rx += sdu_lenP;
    LOG_D(MAC, "[gNB %d][PUSCH %d] CC_id %d %d.%d Received ULSCH sdu from PHY (rnti %x, UE_id %d) ul_cqi %d\n",
          gnb_mod_idP,
          harq_pid,
          CC_idP,
          frameP,
          slotP,
          current_rnti,
          UE_id,
          ul_cqi);

    // if not missed detection (10dB threshold for now)
    if (UE_scheduling_control->ul_rssi < (100+rssi)) {
      UE_scheduling_control->tpc0 = nr_get_tpc(target_snrx10,ul_cqi,30);
      if (timing_advance != 0xffff)
        UE_scheduling_control->ta_update = timing_advance;
      UE_scheduling_control->ul_rssi = rssi;
      LOG_D(MAC, "[UE %d] PUSCH TPC %d and TA %d\n",UE_id,UE_scheduling_control->tpc0,UE_scheduling_control->ta_update);
    }
    else{
      UE_scheduling_control->tpc0 = 1;
    }

#if defined(ENABLE_MAC_PAYLOAD_DEBUG)
    LOG_I(MAC, "Printing received UL MAC payload at gNB side: %d \n");
    for (int i = 0; i < sdu_lenP ; i++) {
	  //harq_process_ul_ue->a[i] = (unsigned char) rand();
	  //printf("a[%d]=0x%02x\n",i,harq_process_ul_ue->a[i]);
	  printf("%02x ",(unsigned char)sduP[i]);
    }
    printf("\n");
#endif

    if (sduP != NULL){
      LOG_D(MAC, "Received PDU at MAC gNB \n");
      nr_process_mac_pdu(gnb_mod_idP, current_rnti, CC_idP, frameP, sduP, sdu_lenP);
    }
    else {

    }
  }
}

long get_K2(NR_BWP_Uplink_t *ubwp, int time_domain_assignment, int mu) {
  DevAssert(ubwp);
  const NR_PUSCH_TimeDomainResourceAllocation_t *tda_list = ubwp->bwp_Common->pusch_ConfigCommon->choice.setup->pusch_TimeDomainAllocationList->list.array[time_domain_assignment];
  if (tda_list->k2)
    return *tda_list->k2;
  else if (mu < 2)
    return 1;
  else if (mu == 2)
    return 2;
  else
    return 3;
}

int8_t select_ul_harq_pid(NR_UE_sched_ctrl_t *sched_ctrl) {
  const uint8_t max_ul_harq_pids = 3; // temp: for testing
  // schedule active harq processes
  for (uint8_t hrq_id = 0; hrq_id < max_ul_harq_pids; hrq_id++) {
    NR_UE_ul_harq_t *cur_harq = &sched_ctrl->ul_harq_processes[hrq_id];
    if (cur_harq->state == ACTIVE_NOT_SCHED) {
      LOG_D(MAC, "Found ulharq id %d, scheduling it for retransmission\n", hrq_id);
      return hrq_id;
    }
  }

  // schedule new harq processes
  for (uint8_t hrq_id=0; hrq_id < max_ul_harq_pids; hrq_id++) {
    NR_UE_ul_harq_t *cur_harq = &sched_ctrl->ul_harq_processes[hrq_id];
    if (cur_harq->state == INACTIVE) {
      LOG_D(MAC, "Found new ulharq id %d, scheduling it\n", hrq_id);
      return hrq_id;
    }
  }
  LOG_E(MAC, "All UL HARQ processes are busy. Cannot schedule ULSCH\n");
  return -1;
}

void nr_schedule_ulsch(module_id_t module_id,
                       frame_t frame,
                       sub_frame_t slot,
                       int num_slots_per_tdd,
                       int ul_slots,
                       uint64_t ulsch_in_slot_bitmap) {
  gNB_MAC_INST *nr_mac = RC.nrmac[module_id];
  NR_COMMON_channels_t *cc = nr_mac->common_channels;
  NR_ServingCellConfigCommon_t *scc = cc->ServingCellConfigCommon;

  const int bwp_id=1;
  const int mu = scc->uplinkConfigCommon->initialUplinkBWP->genericParameters.subcarrierSpacing;
  const int UE_id = 0;
  NR_UE_info_t *UE_info = &RC.nrmac[module_id]->UE_info;
  AssertFatal(UE_info->active[UE_id],"Cannot find UE_id %d is not active\n",UE_id);

  NR_CellGroupConfig_t *secondaryCellGroup = UE_info->secondaryCellGroup[UE_id];
  AssertFatal(secondaryCellGroup->spCellConfig->spCellConfigDedicated->downlinkBWP_ToAddModList->list.count == 1,
              "downlinkBWP_ToAddModList has %d BWP!\n",
              secondaryCellGroup->spCellConfig->spCellConfigDedicated->downlinkBWP_ToAddModList->list.count);
  NR_BWP_Downlink_t *bwp =
      secondaryCellGroup->spCellConfig->spCellConfigDedicated
          ->downlinkBWP_ToAddModList->list.array[bwp_id - 1];

  NR_BWP_Uplink_t *ubwp =
      secondaryCellGroup->spCellConfig->spCellConfigDedicated->uplinkConfig
          ->uplinkBWP_ToAddModList->list.array[bwp_id - 1];

  const int tda = 1; // hardcoded for the moment
  const struct NR_PUSCH_TimeDomainResourceAllocationList *tdaList =
    ubwp->bwp_Common->pusch_ConfigCommon->choice.setup->pusch_TimeDomainAllocationList;
  AssertFatal(tda < tdaList->list.count,
              "time domain assignment %d >= %d\n",
              tda,
              tdaList->list.count);

  int K2 = get_K2(ubwp, tda, mu);
  /* check if slot is UL, and for phy test verify that it is in first TDD
   * period, slot 8 (for K2=2, this is at slot 6 in the gNB; because of UE
   * limitations).  Note that if K2 or the TDD configuration is changed, below
   * conditions might exclude each other and never be true */
  const int slot_idx = (slot + K2) % num_slots_per_tdd;
  if (is_xlsch_in_slot(ulsch_in_slot_bitmap, slot_idx)
        && (!get_softmodem_params()->phy_test || slot_idx == 8)) {

    const int target_ss = NR_SearchSpace__searchSpaceType_PR_ue_Specific;
    NR_SearchSpace_t *ss = get_searchspace(bwp, target_ss);
    uint8_t nr_of_candidates, aggregation_level;
    find_aggregation_candidates(&aggregation_level, &nr_of_candidates, ss);
    NR_ControlResourceSet_t *coreset = get_coreset(bwp, ss, 1 /* dedicated */);
    const int cid = coreset->controlResourceSetId;
    const uint16_t Y = UE_info->Y[UE_id][cid][nr_mac->current_slot];
    const int m = UE_info->num_pdcch_cand[UE_id][cid];
    int CCEIndex = allocate_nr_CCEs(nr_mac,
                                    bwp,
                                    coreset,
                                    aggregation_level,
                                    Y,
                                    m,
                                    nr_of_candidates);
    if (CCEIndex < 0) {
      LOG_E(MAC, "%s(): CCE list not empty, couldn't schedule PUSCH\n", __func__);
      return;
    }
    UE_info->num_pdcch_cand[UE_id][cid]++;

    const uint8_t mcs = 9;
    const uint16_t rbStart = 0;
    const uint16_t rbSize = get_softmodem_params()->phy_test ?
      50 : NRRIV2BW(ubwp->bwp_Common->genericParameters.locationAndBandwidth,275);

    uint16_t rnti = UE_info->rnti[UE_id];

    int first_ul_slot = num_slots_per_tdd - ul_slots;
    NR_sched_pusch *pusch_sched = &UE_info->UE_sched_ctrl[UE_id].sched_pusch[slot+K2-first_ul_slot];
    pusch_sched->frame = frame;
    pusch_sched->slot = slot + K2;
    pusch_sched->active = true;

    LOG_D(MAC, "Scheduling UE specific PUSCH\n");

    const int startSymbolAndLength = tdaList->list.array[tda]->startSymbolAndLength;
    int StartSymbolIndex, NrOfSymbols;
    SLIV2SL(startSymbolAndLength,&StartSymbolIndex,&NrOfSymbols);


    // --------------------------------------------------------------------------------------------------------------------------------------------

    //Pusch Allocation in frequency domain [TS38.214, sec 6.1.2.2]
    //Optional Data only included if indicated in pduBitmap
    int8_t harq_id = select_ul_harq_pid(&UE_info->UE_sched_ctrl[UE_id]);
    if (harq_id < 0) return;
    NR_UE_ul_harq_t *cur_harq = &UE_info->UE_sched_ctrl[UE_id].ul_harq_processes[harq_id];

    cur_harq->state = ACTIVE_SCHED;
    cur_harq->last_tx_slot = pusch_sched->slot;

    nfapi_nr_ul_dci_request_t *UL_dci_req = &RC.nrmac[module_id]->UL_dci_req[0];
    UL_dci_req->SFN = frame;
    UL_dci_req->Slot = slot;

    uint8_t tpc0 = UE_info->UE_sched_ctrl[UE_id].tpc0;
    nr_fill_nfapi_ul_pdu(module_id,
                         UL_dci_req,
                         pusch_sched,
                         secondaryCellGroup,
                         bwp,
                         ubwp,
                         rnti,
                         ss,
                         coreset,
                         aggregation_level,
                         CCEIndex,
                         tda,
                         StartSymbolIndex,
                         NrOfSymbols,
                         mcs,
                         rbStart,
                         rbSize,
                         tpc0,
                         harq_id,
                         cur_harq);

    UE_info->mac_stats[UE_id].ulsch_rounds[cur_harq->round]++;
    if (cur_harq->round == 0)
      UE_info->mac_stats[UE_id].ulsch_total_bytes_scheduled += pusch_sched->pusch_pdu.pusch_data.tb_size;
  }
}
