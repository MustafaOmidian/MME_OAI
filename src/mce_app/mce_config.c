/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the Apache License, Version 2.0  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
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

/*! \file mce_config.c
  \brief
  \author Lionel Gauthier
  \company Eurecom
  \email: lionel.gauthier@eurecom.fr
*/

#if HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include <libconfig.h>

#include <arpa/inet.h>          /* To provide inet_addr */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "assertions.h"
#include "dynamic_memory_check.h"
#include "log.h"
#include "conversions.h"
#include "intertask_interface.h"
#include "common_defs.h"
#include "mce_config.h"
#include "m2ap_common.h"
#include "m2ap_mce_mbms_sa.h"

struct mce_config_s                       mce_config = {.rw_lock = PTHREAD_RWLOCK_INITIALIZER, 0};

//------------------------------------------------------------------------------
int mce_config_find_mnc_length (
  const char mcc_digit1P,
  const char mcc_digit2P,
  const char mcc_digit3P,
  const char mnc_digit1P,
  const char mnc_digit2P,
  const char mnc_digit3P)
{
  uint16_t                                mcc = 100 * mcc_digit1P + 10 * mcc_digit2P + mcc_digit3P;
  uint16_t                                mnc3 = 100 * mnc_digit1P + 10 * mnc_digit2P + mnc_digit3P;
  uint16_t                                mnc2 = 10 * mnc_digit1P + mnc_digit2P;
  int                                     plmn_index = 0;

  AssertFatal ((mcc_digit1P >= 0) && (mcc_digit1P <= 9)
               && (mcc_digit2P >= 0) && (mcc_digit2P <= 9)
               && (mcc_digit3P >= 0) && (mcc_digit3P <= 9), "BAD MCC PARAMETER (%d%d%d)!\n", mcc_digit1P, mcc_digit2P, mcc_digit3P);
  AssertFatal ((mnc_digit2P >= 0) && (mnc_digit2P <= 9)
               && (mnc_digit1P >= 0) && (mnc_digit1P <= 9), "BAD MNC PARAMETER (%d.%d.%d)!\n", mnc_digit1P, mnc_digit2P, mnc_digit3P);

  while (plmn_index < mce_config.served_tai.nb_tai) {
    if (mce_config.served_tai.plmn_mcc[plmn_index] == mcc) {
      if ((mce_config.served_tai.plmn_mnc[plmn_index] == mnc2) && (mce_config.served_tai.plmn_mnc_len[plmn_index] == 2)) {
        return 2;
      } else if ((mce_config.served_tai.plmn_mnc[plmn_index] == mnc3) && (mce_config.served_tai.plmn_mnc_len[plmn_index] == 3)) {
        return 3;
      }
    }
    plmn_index += 1;
  }

  return 0;
}

//------------------------------------------------------------------------------
static
int mme_app_compare_plmn (
  const plmn_t * const plmn)
{
  int                                     i = 0;
  uint16_t                                mcc = 0;
  uint16_t                                mnc = 0;
  uint16_t                                mnc_len = 0;

  DevAssert (plmn != NULL);
  /** Get the integer values from the PLMN. */
  PLMN_T_TO_MCC_MNC ((*plmn), mcc, mnc, mnc_len);

  mce_config_read_lock (&mce_config);

  for (i = 0; i < mce_config.served_tai.nb_tai; i++) {
    OAILOG_TRACE (LOG_MME_APP, "Comparing plmn_mcc %d/%d, plmn_mnc %d/%d plmn_mnc_len %d/%d\n",
        mce_config.served_tai.plmn_mcc[i], mcc, mce_config.served_tai.plmn_mnc[i], mnc, mce_config.served_tai.plmn_mnc_len[i], mnc_len);

    if ((mce_config.served_tai.plmn_mcc[i] == mcc) &&
        (mce_config.served_tai.plmn_mnc[i] == mnc) &&
        (mce_config.served_tai.plmn_mnc_len[i] == mnc_len))
      /*
       * There is a matching plmn
       */
      return MBMS_SA_LIST_AT_LEAST_ONE_MATCH;
  }

  mce_config_unlock (&mce_config);
  return MBMS_SA_LIST_NO_MATCH;
}

//------------------------------------------------------------------------------
/* @brief compare a MBMS-SA
 * @return the first matching MBMS Service Area value.
*/
static
mbms_service_area_id_t mme_app_compare_mbms_sa (const mbms_service_area_t * mbms_service_area)
{
  int                                     i = 0, j = 0;
  mce_config_read_lock (&mce_config);
  for (j = 0; j < mbms_service_area->num_service_area; j++) {
    if(mbms_service_area->serviceArea[j] == INVALID_MBMS_SERVICE_AREA_ID){
      OAILOG_ERROR(LOG_MME_APP, "Skipping invalid MBMS Service Area ID (0). \n");
      continue;
    }
    /** Check if it is a global MBMS SAI. */
    if(mbms_service_area->serviceArea[j] <= mce_config.mbms.mbms_global_service_area_types) {
      /** Global MBMS Service Area Id received. */
      OAILOG_INFO(LOG_MME_APP, "Found a matching global MBMS Service Area ID " MBMS_SERVICE_AREA_ID_FMT ". \n", mbms_service_area->serviceArea[j]);
      return mbms_service_area->serviceArea[j];
    }
    /** Check if it is in bounds for the local service areas. */
    int val = mbms_service_area->serviceArea[j] - (mce_config.mbms.mbms_global_service_area_types +1);
    int local_area = val / mce_config.mbms.mbms_local_service_area_types;
    int local_area_type = val % mce_config.mbms.mbms_local_service_area_types;
    if(local_area < mce_config.mbms.mbms_local_service_area_types){
      OAILOG_INFO(LOG_MME_APP, "Found a valid MBMS Service Area ID " MBMS_SERVICE_AREA_ID_FMT ". \n", mbms_service_area->serviceArea[j]);
      return mbms_service_area->serviceArea[j];
    }
  }
  mce_config_unlock (&mce_config);
  return INVALID_MBMS_SERVICE_AREA_ID;
}

//------------------------------------------------------------------------------
mbms_service_area_id_t mce_app_check_mbms_sa_exists(const plmn_t * target_plmn, const mbms_service_area_t * mbms_service_area){
  if(MBMS_SA_LIST_AT_LEAST_ONE_MATCH == mme_app_compare_plmn(target_plmn)){
  	mbms_service_area_id_t mbms_service_area_id;
    if((mbms_service_area_id = mme_app_compare_mbms_sa(mbms_service_area)) != INVALID_MBMS_SERVICE_AREA_ID){
      OAILOG_DEBUG (LOG_MME_APP, "MBMS-SA " MBMS_SERVICE_AREA_ID_FMT " and PLMN " PLMN_FMT " are matching. \n", mbms_service_area_id, PLMN_ARG(target_plmn));
      return mbms_service_area_id;
    }
  }
  OAILOG_DEBUG (LOG_MME_APP, "MBMS-SA or PLMN " PLMN_FMT " are not matching. \n", PLMN_ARG(target_plmn));
  return INVALID_MBMS_SERVICE_AREA_ID;
}

//------------------------------------------------------------------------------
static void mce_config_init (mce_config_t * config_pP)
{
  memset(&mce_config, 0, sizeof(mce_config));
  pthread_rwlock_init (&config_pP->rw_lock, NULL);
  config_pP->log_config.output             = NULL;
  config_pP->log_config.is_output_thread_safe = false;
  config_pP->log_config.color              = false;
  config_pP->log_config.udp_log_level      = MAX_LOG_LEVEL; // Means invalid
  config_pP->log_config.gtpv1u_log_level   = MAX_LOG_LEVEL; // will not overwrite existing log levels if MME and S-GW bundled in same executable
  config_pP->log_config.gtpv2c_log_level   = MAX_LOG_LEVEL;
  config_pP->log_config.sctp_log_level     = MAX_LOG_LEVEL;

  config_pP->log_config.nas_log_level      = MAX_LOG_LEVEL;
  config_pP->log_config.mme_app_log_level  = MAX_LOG_LEVEL;
  config_pP->log_config.spgw_app_log_level = MAX_LOG_LEVEL;
  config_pP->log_config.s11_log_level      = MAX_LOG_LEVEL;
  config_pP->log_config.s10_log_level      = MAX_LOG_LEVEL;
  config_pP->log_config.s6a_log_level      = MAX_LOG_LEVEL;
  config_pP->log_config.secu_log_level     = MAX_LOG_LEVEL;
  config_pP->log_config.util_log_level     = MAX_LOG_LEVEL;
  /** MBMS Logs. */
  config_pP->log_config.s1ap_log_level     = MAX_LOG_LEVEL;
  config_pP->log_config.mce_app_log_level  = MAX_LOG_LEVEL;
  config_pP->log_config.sm_log_level       = MAX_LOG_LEVEL;

  config_pP->log_config.msc_log_level      = MAX_LOG_LEVEL;
  config_pP->log_config.xml_log_level      = MAX_LOG_LEVEL;
  config_pP->log_config.mme_scenario_player_log_level = MAX_LOG_LEVEL;
  config_pP->log_config.itti_log_level     = MAX_LOG_LEVEL;

  config_pP->log_config.asn1_verbosity_level = 0;
  config_pP->config_file = NULL;
  config_pP->max_s1_enbs 		= 2;
  config_pP->max_ues     		= 2;
  config_pP->unauthenticated_imsi_supported = 0;
  config_pP->dummy_handover_forwarding_enabled = 1;
  config_pP->run_mode    = RUN_MODE_BASIC;

  /*
   * EPS network feature support
   */
  config_pP->eps_network_feature_support.emergency_bearer_services_in_s1_mode = 0;
  config_pP->eps_network_feature_support.extended_service_request = 0;
  config_pP->eps_network_feature_support.ims_voice_over_ps_session_in_s1 = 0;
  config_pP->eps_network_feature_support.location_services_via_epc = 0;

  config_pP->s1ap_config.port_number = S1AP_PORT_NUMBER;
  /*
   * IP configuration
   */
  config_pP->ip.if_name_s1_mme = NULL;
  config_pP->ip.s1_mme_v4.s_addr = INADDR_ANY;
  config_pP->ip.s1_mme_v6 = in6addr_any;
  config_pP->ip.if_name_s11 = NULL;
  config_pP->ip.s11_mme_v4.s_addr = INADDR_ANY;
  config_pP->ip.s11_mme_v6 = in6addr_any;
  config_pP->ip.port_s11 = 2123;
  config_pP->ip.s10_mme_v4.s_addr = INADDR_ANY;
  config_pP->ip.s10_mme_v6 = in6addr_any;
  config_pP->ip.port_s10 = 2123;
  config_pP->ip.s10_mme_v4.s_addr = INADDR_ANY;
  config_pP->ip.s10_mme_v6 = in6addr_any;
  config_pP->ip.port_s10 = 2123;

  config_pP->s6a_config.conf_file = bfromcstr(S6A_CONF_FILE);
  config_pP->itti_config.queue_size = ITTI_QUEUE_MAX_ELEMENTS;
  config_pP->itti_config.log_file = NULL;
  config_pP->sctp_config.in_streams = SCTP_IN_STREAMS;
  config_pP->sctp_config.out_streams = SCTP_OUT_STREAMS;
  config_pP->relative_capacity = RELATIVE_CAPACITY;
  config_pP->mme_statistic_timer = MME_STATISTIC_TIMER_S;

  // todo: sgw address?
//  config_pP->ipv4.sgw_s11 = 0;

  /** Add the timers for handover/idle-TAU completion on both sides. */
  config_pP->mme_mobility_completion_timer = MME_MOBILITY_COMPLETION_TIMER_S;
  config_pP->mme_s10_handover_completion_timer = MME_S10_HANDOVER_COMPLETION_TIMER_S;

  config_pP->gummei.nb = 1;
  config_pP->gummei.gummei[0].mme_code = MMEC;
  config_pP->gummei.gummei[0].mme_gid = MMEGID;
  config_pP->gummei.gummei[0].plmn.mcc_digit1 = 0;
  config_pP->gummei.gummei[0].plmn.mcc_digit2 = 0;
  config_pP->gummei.gummei[0].plmn.mcc_digit3 = 1;
  config_pP->gummei.gummei[0].plmn.mcc_digit1 = 0;
  config_pP->gummei.gummei[0].plmn.mcc_digit2 = 1;
  config_pP->gummei.gummei[0].plmn.mcc_digit3 = 0x0F;

  config_pP->nas_config.t3346_sec = T3346_DEFAULT_VALUE;
  config_pP->nas_config.t3402_min = T3402_DEFAULT_VALUE;
  config_pP->nas_config.t3412_min = T3412_DEFAULT_VALUE;
  config_pP->nas_config.t3422_sec = T3422_DEFAULT_VALUE;
  config_pP->nas_config.t3450_sec = T3450_DEFAULT_VALUE;
  config_pP->nas_config.t3460_sec = T3460_DEFAULT_VALUE;
  config_pP->nas_config.t3470_sec = T3470_DEFAULT_VALUE;
  config_pP->nas_config.t3485_sec = T3485_DEFAULT_VALUE;
  config_pP->nas_config.t3486_sec = T3486_DEFAULT_VALUE;
  config_pP->nas_config.t3489_sec = T3489_DEFAULT_VALUE;
  config_pP->nas_config.t3495_sec = T3495_DEFAULT_VALUE;
  config_pP->nas_config.force_tau = MME_FORCE_TAU_S;
  config_pP->nas_config.force_reject_sr  = true;
  config_pP->nas_config.disable_esm_information = false;

  /*
   * Set the TAI
   */
  config_pP->served_tai.nb_tai = 1;
  config_pP->served_tai.plmn_mcc = calloc (1, sizeof (*config_pP->served_tai.plmn_mcc));
  config_pP->served_tai.plmn_mnc = calloc (1, sizeof (*config_pP->served_tai.plmn_mnc));
  config_pP->served_tai.plmn_mnc_len = calloc (1, sizeof (*config_pP->served_tai.plmn_mnc_len));
  config_pP->served_tai.tac = calloc (1, sizeof (*config_pP->served_tai.tac));
  config_pP->served_tai.plmn_mcc[0] = PLMN_MCC;
  config_pP->served_tai.plmn_mnc[0] = PLMN_MNC;
  config_pP->served_tai.plmn_mnc_len[0] = PLMN_MNC_LEN;
  config_pP->served_tai.tac[0] = PLMN_TAC;
  config_pP->s1ap_config.outcome_drop_timer_sec = S1AP_OUTCOME_TIMER_DEFAULT;
}

//------------------------------------------------------------------------------
void mce_config_exit (void)
{
  pthread_rwlock_destroy (&mce_config.rw_lock);
  bdestroy_wrapper(&mce_config.log_config.output);
  bdestroy_wrapper(&mce_config.realm);
  bdestroy_wrapper(&mce_config.config_file);

  /*
   * IP configuration
   */
  bdestroy_wrapper(&mce_config.ip.if_name_s1_mme);
  bdestroy_wrapper(&mce_config.ip.if_name_s11);
  bdestroy_wrapper(&mce_config.ip.if_name_s10);
  bdestroy_wrapper(&mce_config.mbms.ip.if_name_mc);
  bdestroy_wrapper(&mce_config.s6a_config.conf_file);
  bdestroy_wrapper(&mce_config.s6a_config.hss_host_name);
  bdestroy_wrapper(&mce_config.itti_config.log_file);

  free_wrapper((void**)&mce_config.served_tai.plmn_mcc);
  free_wrapper((void**)&mce_config.served_tai.plmn_mnc);
  free_wrapper((void**)&mce_config.served_tai.plmn_mnc_len);
  free_wrapper((void**)&mce_config.served_tai.tac);

  for (int i = 0; i < mce_config.e_dns_emulation.nb_service_entries; i++) {
    bdestroy_wrapper(&mce_config.e_dns_emulation.service_id[i]);
  }

#if TRACE_XML
  bdestroy_wrapper(&mce_config.scenario_player_config.scenario_file);
#endif
}
//------------------------------------------------------------------------------
static int mce_config_parse_file (mce_config_t * config_pP)
{
  config_t                                cfg = {0};
  config_setting_t                       *setting_mme = NULL;
  config_setting_t                       *setting = NULL;
  config_setting_t                       *subsetting = NULL;
  config_setting_t                       *sub2setting = NULL;
  config_setting_t                       *setting_mce = NULL;
  int                                     aint = 0;
  int                                     adouble  = 0;
  int                                     aint_s10 = 0;
  int                                     aint_s11 = 0;
  int                                     aint_mc = 0;

  int                                     i = 0,n = 0,
                                          stop_index = 0,
                                          num = 0;
  const char                             *astring = NULL;
  const char                             *tac = NULL;
  const char                             *mcc = NULL;
  const char                             *mnc = NULL;
  const char                             *mbms_sa = NULL;
  char                                   *if_name_s1_mme = NULL;
  char                                   *s1_mme_v4 = NULL;
  char                                   *s1_mme_v6 = NULL;
  char                                   *if_name_s11 = NULL;
  char                                   *s11_mme_v4 = NULL;
  char                                   *s11_mme_v6 = NULL;
  char                                   *sgw_ip_address_for_s11 = NULL;
  char                                   *mme_ip_address_for_s10 = NULL;

  char                                   *if_name_s10 = NULL;
  char                                   *s10_mme_v4 = NULL;
  char                                   *s10_mme_v6 = NULL;
  char                                   *ngh_s10 = NULL;

  char                                   *if_name_mc = NULL;
  char                                   *mc_mce_v4 = NULL;
  char                                   *mc_mce_v6 = NULL;

  bool                                    swap = false;
  bstring                                 address = NULL;
  bstring                                 cidr = NULL;
  bstring                                 mask = NULL;
  struct in_addr                          in_addr_var = {0};
  struct in6_addr                         in6_addr_var = {0};

  config_init (&cfg);

  if (config_pP->config_file != NULL) {
    /*
     * Read the file. If there is an error, report it and exit.
     */
    if (!config_read_file (&cfg, bdata(config_pP->config_file))) {
      OAILOG_ERROR (LOG_CONFIG, ": %s:%d - %s\n", bdata(config_pP->config_file), config_error_line (&cfg), config_error_text (&cfg));
      config_destroy (&cfg);
      AssertFatal (1 == 0, "Failed to parse MME configuration file %s!\n", bdata(config_pP->config_file));
    }
  } else {
    OAILOG_ERROR (LOG_CONFIG, " No MME configuration file provided!\n");
    config_destroy (&cfg);
    AssertFatal (0, "No MME configuration file provided!\n");
  }

  setting_mme = config_lookup (&cfg, MME_CONFIG_STRING_MME_CONFIG);

  if (setting_mme != NULL) {

    // LOGGING setting
    setting = config_setting_get_member (setting_mme, LOG_CONFIG_STRING_LOGGING);
    if (setting != NULL) {
      if (config_setting_lookup_string (setting, LOG_CONFIG_STRING_OUTPUT, (const char **)&astring)) {
        if (astring != NULL) {
          if (config_pP->log_config.output) {
            bassigncstr(config_pP->log_config.output , astring);
          } else {
            config_pP->log_config.output = bfromcstr(astring);
          }
        }
      }

      if (config_setting_lookup_string (setting, LOG_CONFIG_STRING_OUTPUT_THREAD_SAFE, (const char **)&astring)) {
        if (astring != NULL) {
          if (strcasecmp (astring, "yes") == 0) {
            config_pP->log_config.is_output_thread_safe = true;
          } else {
            config_pP->log_config.is_output_thread_safe = false;
          }
        }
      }

      if (config_setting_lookup_string (setting, LOG_CONFIG_STRING_COLOR, (const char **)&astring)) {
        if (0 == strcasecmp("yes", astring)) config_pP->log_config.color = true;
        else config_pP->log_config.color = false;
      }

      if (config_setting_lookup_string (setting, LOG_CONFIG_STRING_SCTP_LOG_LEVEL, (const char **)&astring))
        config_pP->log_config.sctp_log_level = OAILOG_LEVEL_STR2INT (astring);

      if (config_setting_lookup_string (setting, LOG_CONFIG_STRING_S1AP_LOG_LEVEL, (const char **)&astring))
        config_pP->log_config.s1ap_log_level = OAILOG_LEVEL_STR2INT (astring);

      if (config_setting_lookup_string (setting, LOG_CONFIG_STRING_NAS_LOG_LEVEL, (const char **)&astring))
        config_pP->log_config.nas_log_level = OAILOG_LEVEL_STR2INT (astring);

      if (config_setting_lookup_string (setting, LOG_CONFIG_STRING_MME_APP_LOG_LEVEL, (const char **)&astring))
        config_pP->log_config.mme_app_log_level = OAILOG_LEVEL_STR2INT (astring);

      if (config_setting_lookup_string (setting, LOG_CONFIG_STRING_MCE_APP_LOG_LEVEL, (const char **)&astring))
        config_pP->log_config.mce_app_log_level = OAILOG_LEVEL_STR2INT (astring);

      if (config_setting_lookup_string (setting, LOG_CONFIG_STRING_S6A_LOG_LEVEL, (const char **)&astring))
        config_pP->log_config.s6a_log_level = OAILOG_LEVEL_STR2INT (astring);

      if (config_setting_lookup_string (setting, LOG_CONFIG_STRING_SECU_LOG_LEVEL, (const char **)&astring))
        config_pP->log_config.secu_log_level = OAILOG_LEVEL_STR2INT (astring);

      if (config_setting_lookup_string (setting, LOG_CONFIG_STRING_GTPV2C_LOG_LEVEL, (const char **)&astring))
        config_pP->log_config.gtpv2c_log_level = OAILOG_LEVEL_STR2INT (astring);

      if (config_setting_lookup_string (setting, LOG_CONFIG_STRING_UDP_LOG_LEVEL, (const char **)&astring))
        config_pP->log_config.udp_log_level = OAILOG_LEVEL_STR2INT (astring);

      if (config_setting_lookup_string (setting, LOG_CONFIG_STRING_S11_LOG_LEVEL, (const char **)&astring))
        config_pP->log_config.s11_log_level = OAILOG_LEVEL_STR2INT (astring);

      if (config_setting_lookup_string (setting, LOG_CONFIG_STRING_S10_LOG_LEVEL, (const char **)&astring))
        config_pP->log_config.s10_log_level = OAILOG_LEVEL_STR2INT (astring);

      if (config_setting_lookup_string (setting, LOG_CONFIG_STRING_SM_LOG_LEVEL, (const char **)&astring))
        config_pP->log_config.sm_log_level = OAILOG_LEVEL_STR2INT (astring);

      if (config_setting_lookup_string (setting, LOG_CONFIG_STRING_UTIL_LOG_LEVEL, (const char **)&astring))
        config_pP->log_config.util_log_level = OAILOG_LEVEL_STR2INT (astring);

      if (config_setting_lookup_string (setting, LOG_CONFIG_STRING_MSC_LOG_LEVEL, (const char **)&astring))
        config_pP->log_config.msc_log_level = OAILOG_LEVEL_STR2INT (astring);

      if (config_setting_lookup_string (setting, LOG_CONFIG_STRING_XML_LOG_LEVEL, (const char **)&astring))
        config_pP->log_config.xml_log_level = OAILOG_LEVEL_STR2INT (astring);

      if (config_setting_lookup_string (setting, LOG_CONFIG_STRING_MME_SCENARIO_PLAYER_LOG_LEVEL, (const char **)&astring))
        config_pP->log_config.mme_scenario_player_log_level = OAILOG_LEVEL_STR2INT (astring);

      if (config_setting_lookup_string (setting, LOG_CONFIG_STRING_ITTI_LOG_LEVEL, (const char **)&astring))
        config_pP->log_config.itti_log_level = OAILOG_LEVEL_STR2INT (astring);

      if ((config_setting_lookup_string (setting_mme, MME_CONFIG_STRING_ASN1_VERBOSITY, (const char **)&astring))) {
        if (strcasecmp (astring, MME_CONFIG_STRING_ASN1_VERBOSITY_NONE) == 0)
          config_pP->log_config.asn1_verbosity_level = 0;
        else if (strcasecmp (astring, MME_CONFIG_STRING_ASN1_VERBOSITY_ANNOYING) == 0)
          config_pP->log_config.asn1_verbosity_level = 2;
        else if (strcasecmp (astring, MME_CONFIG_STRING_ASN1_VERBOSITY_INFO) == 0)
          config_pP->log_config.asn1_verbosity_level = 1;
        else
          config_pP->log_config.asn1_verbosity_level = 0;
      }
    }

    // GENERAL MME SETTINGS

    // todo:
//    if ((config_setting_lookup_string (setting_mme, MME_CONFIG_STRING_RUN_MODE, (const char **)&astring))) {
//      if (strcasecmp (astring, MME_CONFIG_STRING_RUN_MODE_TEST) == 0)
//        config_pP->run_mode = RUN_MODE_TEST;
//      else
//        config_pP->run_mode = RUN_MODE_OTHER;
//    }
    if ((config_setting_lookup_string (setting_mme, MME_CONFIG_STRING_REALM, (const char **)&astring))) {
      config_pP->realm = bfromcstr (astring);
    }

    if ((config_setting_lookup_string (setting_mme,
                                       MME_CONFIG_STRING_PID_DIRECTORY,
                                       (const char **)&astring))) {
      config_pP->pid_dir = bfromcstr (astring);
    }

    if ((config_setting_lookup_int (setting_mme, MME_CONFIG_STRING_INSTANCE, &aint))) {
      config_pP->instance = (uint32_t) aint;
    }

    if ((config_setting_lookup_int (setting_mme, MME_CONFIG_STRING_S1_MAXENB, &aint))) {
      config_pP->max_s1_enbs = (uint32_t) aint;
    }

    if ((config_setting_lookup_int (setting_mme, MME_CONFIG_STRING_MAXUE, &aint))) {
      config_pP->max_ues = (uint32_t) aint;
    }

    if ((config_setting_lookup_int (setting_mme, MME_CONFIG_STRING_RELATIVE_CAPACITY, &aint))) {
      config_pP->relative_capacity = (uint8_t) aint;
    }

    if ((config_setting_lookup_int (setting_mme, MME_CONFIG_STRING_STATISTIC_TIMER, &aint))) {
      config_pP->mme_statistic_timer = (uint32_t) aint;
    }

    if ((config_setting_lookup_int (setting_mme, MME_CONFIG_STRING_MME_MOBILITY_COMPLETION_TIMER, &aint))) {
      config_pP->mme_mobility_completion_timer = (uint32_t) aint;
    }

    if ((config_setting_lookup_int (setting_mme, MME_CONFIG_STRING_MME_S10_HANDOVER_COMPLETION_TIMER, &aint))) {
      config_pP->mme_s10_handover_completion_timer = (uint32_t) aint;
    }

    /**
     * Other configurations..
     */

    if ((config_setting_lookup_string (setting_mme, EPS_NETWORK_FEATURE_SUPPORT_EMERGENCY_BEARER_SERVICES_IN_S1_MODE, (const char **)&astring))) {
      if (strcasecmp (astring, "yes") == 0)
        config_pP->eps_network_feature_support.emergency_bearer_services_in_s1_mode = 1;
      else
        config_pP->eps_network_feature_support.emergency_bearer_services_in_s1_mode = 0;
    }
    if ((config_setting_lookup_string (setting_mme, EPS_NETWORK_FEATURE_SUPPORT_EXTENDED_SERVICE_REQUEST, (const char **)&astring))) {
      if (strcasecmp (astring, "yes") == 0)
        config_pP->eps_network_feature_support.extended_service_request = 1;
      else
        config_pP->eps_network_feature_support.extended_service_request = 0;
    }
    if ((config_setting_lookup_string (setting_mme, EPS_NETWORK_FEATURE_SUPPORT_IMS_VOICE_OVER_PS_SESSION_IN_S1, (const char **)&astring))) {
      if (strcasecmp (astring, "yes") == 0)
        config_pP->eps_network_feature_support.ims_voice_over_ps_session_in_s1 = 1;
      else
        config_pP->eps_network_feature_support.ims_voice_over_ps_session_in_s1 = 0;
    }
    if ((config_setting_lookup_string (setting_mme, EPS_NETWORK_FEATURE_SUPPORT_LOCATION_SERVICES_VIA_EPC, (const char **)&astring))) {
      if (strcasecmp (astring, "yes") == 0)
        config_pP->eps_network_feature_support.location_services_via_epc = 1;
      else
        config_pP->eps_network_feature_support.location_services_via_epc = 0;
    }

    if ((config_setting_lookup_string (setting_mme, MME_CONFIG_STRING_UNAUTHENTICATED_IMSI_SUPPORTED, (const char **)&astring))) {
      if (strcasecmp (astring, "yes") == 0)
        config_pP->unauthenticated_imsi_supported = 1;
      else
        config_pP->unauthenticated_imsi_supported = 0;
    }

    if ((config_setting_lookup_string (setting_mme, MME_CONFIG_STRING_DUMMY_HANDOVER_FORWARDING_ENABLED, (const char **)&astring))) {
      if (strcasecmp (astring, "yes") == 0)
        config_pP->dummy_handover_forwarding_enabled = 1;
      else
        config_pP->dummy_handover_forwarding_enabled = 0;
    }

    // ITTI SETTING
    setting = config_setting_get_member (setting_mme, MME_CONFIG_STRING_INTERTASK_INTERFACE_CONFIG);

    if (setting != NULL) {
      if ((config_setting_lookup_int (setting, MME_CONFIG_STRING_INTERTASK_INTERFACE_QUEUE_SIZE, &aint))) {
        config_pP->itti_config.queue_size = (uint32_t) aint;
      }
    }
    // S6A SETTING
    setting = config_setting_get_member (setting_mme, MME_CONFIG_STRING_S6A_CONFIG);

    if (setting != NULL) {
      if ((config_setting_lookup_string (setting, MME_CONFIG_STRING_S6A_CONF_FILE_PATH, (const char **)&astring))) {
        if (astring != NULL) {
          if (config_pP->s6a_config.conf_file) {
            bassigncstr(config_pP->s6a_config.conf_file , astring);
          } else {
            config_pP->s6a_config.conf_file = bfromcstr(astring);
          }
        }
      }

      if ((config_setting_lookup_string (setting, MME_CONFIG_STRING_S6A_HSS_HOSTNAME, (const char **)&astring))) {
        if (astring != NULL) {
          if (config_pP->s6a_config.hss_host_name) {
            bassigncstr(config_pP->s6a_config.hss_host_name , astring);
          } else {
            config_pP->s6a_config.hss_host_name = bfromcstr(astring);
          }
        } else
          AssertFatal (1 == 0, "You have to provide a valid HSS hostname %s=...\n", MME_CONFIG_STRING_S6A_HSS_HOSTNAME);
      }

      // todo: check if MME hostname is needed
      if ((config_setting_lookup_string (setting, MME_CONFIG_STRING_S6A_MME_HOSTNAME, (const char **)&astring))) {
        if (astring != NULL) {
          if (config_pP->s6a_config.mme_host_name) {
            bassigncstr(config_pP->s6a_config.mme_host_name , astring);
          } else {
            config_pP->s6a_config.mme_host_name = bfromcstr(astring);
          }
        } else
          AssertFatal (1 == 0, "You have to provide a valid MME hostname %s=...\n", MME_CONFIG_STRING_S6A_MME_HOSTNAME);
      }
    }
    // SCTP SETTING
    setting = config_setting_get_member (setting_mme, MME_CONFIG_STRING_SCTP_CONFIG);

    if (setting != NULL) {
      if ((config_setting_lookup_int (setting, MME_CONFIG_STRING_SCTP_INSTREAMS, &aint))) {
        config_pP->sctp_config.in_streams = (uint16_t) aint;
      }

      if ((config_setting_lookup_int (setting, MME_CONFIG_STRING_SCTP_OUTSTREAMS, &aint))) {
        config_pP->sctp_config.out_streams = (uint16_t) aint;
      }
    }
    // S1AP SETTING
    setting = config_setting_get_member (setting_mme, MME_CONFIG_STRING_S1AP_CONFIG);

    if (setting != NULL) {
      if ((config_setting_lookup_int (setting, MME_CONFIG_STRING_S1AP_OUTCOME_TIMER, &aint))) {
        config_pP->s1ap_config.outcome_drop_timer_sec = (uint8_t) aint;
      }

      if ((config_setting_lookup_int (setting, MME_CONFIG_STRING_S1AP_PORT, &aint))) {
        config_pP->s1ap_config.port_number = (uint16_t) aint;
      }
    }

    // GUMMEI SETTING
    setting = config_setting_get_member (setting_mme, MME_CONFIG_STRING_GUMMEI_LIST);
    config_pP->gummei.nb = 0;
    if (setting != NULL) {
      num = config_setting_length (setting);
      AssertFatal(num == 1, "Only one GUMMEI supported for this version of MME");
      for (i = 0; i < num; i++) {
        sub2setting = config_setting_get_elem (setting, i);

        if (sub2setting != NULL) {
          if ((config_setting_lookup_string (sub2setting, MME_CONFIG_STRING_MCC, &mcc))) {
            AssertFatal( 3 == strlen(mcc), "Bad MCC length, it must be 3 digit ex: 001");
            char c[2] = { mcc[0], 0};
            config_pP->gummei.gummei[i].plmn.mcc_digit1 = (uint8_t) atoi (c);
            c[0] = mcc[1];
            config_pP->gummei.gummei[i].plmn.mcc_digit2 = (uint8_t) atoi (c);
            c[0] = mcc[2];
            config_pP->gummei.gummei[i].plmn.mcc_digit3 = (uint8_t) atoi (c);
          }

          if ((config_setting_lookup_string (sub2setting, MME_CONFIG_STRING_MNC, &mnc))) {
            AssertFatal( (3 == strlen(mnc)) || (2 == strlen(mnc)) , "Bad MNC length, it must be 3 digit ex: 001");
            char c[2] = { mnc[0], 0};
            config_pP->gummei.gummei[i].plmn.mnc_digit1 = (uint8_t) atoi (c);
            c[0] = mnc[1];
            config_pP->gummei.gummei[i].plmn.mnc_digit2 = (uint8_t) atoi (c);
            if (3 == strlen(mnc)) {
              c[0] = mnc[2];
              config_pP->gummei.gummei[i].plmn.mnc_digit3 = (uint8_t) atoi (c);
            } else {
              config_pP->gummei.gummei[i].plmn.mnc_digit3 = 0x0F;
            }
          }

          if ((config_setting_lookup_string (sub2setting, MME_CONFIG_STRING_MME_GID, &mnc))) {
            config_pP->gummei.gummei[i].mme_gid = (uint16_t) atoi (mnc);
          }
          if ((config_setting_lookup_string (sub2setting, MME_CONFIG_STRING_MME_CODE, &mnc))) {
            config_pP->gummei.gummei[i].mme_code = (uint8_t) atoi (mnc);
          }
          config_pP->gummei.nb += 1;
        }
      }
    }

    // NETWORK INTERFACE SETTING
    setting = config_setting_get_member (setting_mme, MME_CONFIG_STRING_NETWORK_INTERFACES_CONFIG);

    config_setting_lookup_string (setting, MME_CONFIG_STRING_IPV4_ADDRESS_FOR_S1_MME, (const char **)&s1_mme_v4);
	config_setting_lookup_string (setting, MME_CONFIG_STRING_IPV6_ADDRESS_FOR_S1_MME, (const char **)&s1_mme_v6);
	config_setting_lookup_string (setting, MME_CONFIG_STRING_IPV4_ADDRESS_FOR_S11, (const char **)&s11_mme_v4);
	config_setting_lookup_string (setting, MME_CONFIG_STRING_IPV6_ADDRESS_FOR_S11, (const char **)&s11_mme_v6);
	config_setting_lookup_string (setting, MME_CONFIG_STRING_IPV4_ADDRESS_FOR_S10, (const char **)&s10_mme_v4);
	config_setting_lookup_string (setting, MME_CONFIG_STRING_IPV6_ADDRESS_FOR_S10, (const char **)&s10_mme_v6);

    if (setting != NULL) {
    	/** Process Mandatory Interfaces. */
        if (
        	(
        		/** S1. */
        		config_setting_lookup_string (setting, MME_CONFIG_STRING_INTERFACE_NAME_FOR_S1_MME, (const char **)&if_name_s1_mme) && (s1_mme_v4 || s1_mme_v6)
				/** S11. */
				&& config_setting_lookup_string (setting, MME_CONFIG_STRING_INTERFACE_NAME_FOR_S11, (const char **)&if_name_s11) && (s11_mme_v4 || s11_mme_v6)
				&& config_setting_lookup_int (setting, MME_CONFIG_STRING_MME_PORT_FOR_S11, &aint_s11)
            )
          ) {
        	config_pP->ip.port_s11 = (uint16_t)aint_s11;
        	struct bstrList *list = NULL;

        	/** S1: IPv4. */
        	config_pP->ip.if_name_s1_mme = bfromcstr(if_name_s1_mme);
        	if(s1_mme_v4) {
        		cidr = bfromcstr (s1_mme_v4);
        		list = bsplit (cidr, '/');
        		AssertFatal(2 == list->qty, "Bad CIDR address %s", bdata(cidr));
        		address = list->entry[0];
        		mask    = list->entry[1];
        		IPV4_STR_ADDR_TO_INADDR (bdata(address), config_pP->ip.s1_mme_v4, "BAD IP ADDRESS FORMAT FOR S1-MME !\n");
        		config_pP->ip.s1_mme_cidrv4 = atoi ((const char*)mask->data);
        		bstrListDestroy(list);
        		in_addr_var.s_addr = config_pP->ip.s1_mme_v4.s_addr;
        		OAILOG_INFO (LOG_MME_APP, "Parsing configuration file found S1-MME: %s/%d on %s\n",
        				inet_ntoa (in_addr_var), config_pP->ip.s1_mme_cidrv4, bdata(config_pP->ip.if_name_s1_mme));
        		bdestroy_wrapper(&cidr);
        	}
        	/** S1: IPv6. */
        	if(s1_mme_v6) {
        		cidr = bfromcstr (s1_mme_v6);
        		list = bsplit (cidr, '/');
        		AssertFatal(2 == list->qty, "Bad CIDR address %s", bdata(cidr));
        		address = list->entry[0];
        		mask    = list->entry[1];
        		IPV6_STR_ADDR_TO_INADDR (bdata(address), config_pP->ip.s1_mme_v6, "BAD IPV6 ADDRESS FORMAT FOR S1-MME !\n");
        		//    	config_pP->ipv6.netmask_s1_mme = atoi ((const char*)mask->data);
        		bstrListDestroy(list);
        		//    	memcpy(in6_addr_var.s6_addr, config_pP->ipv6.s1_mme.s6_addr, 16);

        		char strv6[16];
        		OAILOG_INFO (LOG_MME_APP, "Parsing configuration file found S1-MME: %s/%d on %s\n",
        				inet_ntop(AF_INET6, &in6_addr_var, strv6, 16), config_pP->ip.s1_mme_cidrv6, bdata(config_pP->ip.if_name_s1_mme));
        		bdestroy_wrapper(&cidr);
        	}
        	/** S11: IPv4. */
        	config_pP->ip.if_name_s11 = bfromcstr(if_name_s11);
        	if(s11_mme_v4) {
        		cidr = bfromcstr (s11_mme_v4);
        		list = bsplit (cidr, '/');
        		AssertFatal(2 == list->qty, "Bad CIDR address %s", bdata(cidr));
        		address = list->entry[0];
        		mask    = list->entry[1];
            IPV4_STR_ADDR_TO_INADDR (bdata(address), config_pP->ip.s11_mme_v4, "BAD IP ADDRESS FORMAT FOR S11 !\n");
            config_pP->ip.s11_mme_cidrv4 = atoi ((const char*)mask->data);
            bstrListDestroy(list);
            bdestroy_wrapper(&cidr);
            in_addr_var.s_addr = config_pP->ip.s11_mme_v4.s_addr;
            OAILOG_INFO (LOG_MME_APP, "Parsing configuration file found S11: %s/%d on %s\n",
            		inet_ntoa (in_addr_var), config_pP->ip.s11_mme_cidrv4, bdata(config_pP->ip.if_name_s11));
        }
        /** S11: IPv6. */
        if(s11_mme_v6) {
      	  config_pP->ip.port_s11 = (uint16_t)aint_s11;
      	  cidr = bfromcstr (s11_mme_v6);
      	  list = bsplit (cidr, '/');
      	  AssertFatal(2 == list->qty, "Bad CIDR address %s", bdata(cidr));
      	  address = list->entry[0];
      	  mask    = list->entry[1];
      	  IPV6_STR_ADDR_TO_INADDR (bdata(address), config_pP->ip.s11_mme_v6, "BAD IPV6 ADDRESS FORMAT FOR S11-MME !\n");
      	  //    	config_pP->ipv6.netmask_s1_mme = atoi ((const char*)mask->data);
      	  bstrListDestroy(list);
  //    	  memcpy(in6_addr_var.s6_addr, config_pP->ipv6.s1_mme.s6_addr, 16);
      	  char strv6[16];
      	  OAILOG_INFO (LOG_MME_APP, "Parsing configuration file found S11-MME: %s/%d on %s\n",
      			  inet_ntop(AF_INET6, &in6_addr_var, strv6, 16), config_pP->ip.s11_mme_cidrv6, bdata(config_pP->ip.if_name_s11));
      	  bdestroy_wrapper(&cidr);
  //      	config_pP->ipv6.netmask_s11 = atoi ((const char*)mask->data);
        }
      }

      /** Process Optional Interfaces. */
      /** S10. */
      if (
          (
        	/** S10. */
        	config_setting_lookup_string (setting, MME_CONFIG_STRING_INTERFACE_NAME_FOR_S10, (const char **)&if_name_s10) && (s10_mme_v4 || s10_mme_v6)
			&& config_setting_lookup_int (setting, MME_CONFIG_STRING_MME_PORT_FOR_S10, &aint_s10)
          )
      ) {
      	config_pP->ip.port_s10 = (uint16_t)aint_s10;
    	  struct bstrList *list = NULL;

    	  /** S10: IPv4. */
          config_pP->ip.if_name_s10 = bfromcstr(if_name_s10);
          if(s10_mme_v4) {
              cidr = bfromcstr (s10_mme_v4);
              list = bsplit (cidr, '/');
              AssertFatal(2 == list->qty, "Bad CIDR address %s", bdata(cidr));
              address = list->entry[0];
              mask    = list->entry[1];
              IPV4_STR_ADDR_TO_INADDR (bdata(address), config_pP->ip.s10_mme_v4, "BAD IP ADDRESS FORMAT FOR S10 !\n");
              config_pP->ip.s10_mme_cidrv4 = atoi ((const char*)mask->data);
              bstrListDestroy(list);
              bdestroy_wrapper(&cidr);
              in_addr_var.s_addr = config_pP->ip.s10_mme_v4.s_addr;
              OAILOG_INFO (LOG_MME_APP, "Parsing configuration file found S10: %s/%d on %s\n",
                             inet_ntoa (in_addr_var), config_pP->ip.s10_mme_cidrv4, bdata(config_pP->ip.if_name_s10));
          }
          /** S10: IPv6. */
          if(s10_mme_v6){
          	/** S10. */
          	config_pP->ip.port_s10 = (uint16_t)aint_s10;
          	cidr = bfromcstr (s10_mme_v6);
          	list = bsplit (cidr, '/');
          	AssertFatal(2 == list->qty, "Bad CIDR address %s", bdata(cidr));
          	address = list->entry[0];
          	mask    = list->entry[1];
          	IPV6_STR_ADDR_TO_INADDR (bdata(address), config_pP->ip.s10_mme_v6, "BAD IPV6 ADDRESS FORMAT FOR S10-MME !\n");
          	//    	config_pP->ipv6.netmask_s1_mme = atoi ((const char*)mask->data);
          	bstrListDestroy(list);
          	//    	  memcpy(in6_addr_var.s6_addr, config_pP->ipv6.s1_mme.s6_addr, 16);
          	char strv6[16];
          	OAILOG_INFO (LOG_MME_APP, "Parsing configuration file found S10-MME: %s/%d on %s\n",
          			inet_ntop(AF_INET6, &in6_addr_var, strv6, 16), config_pP->ip.s10_mme_cidrv6, bdata(config_pP->ip.if_name_s10));
          	bdestroy_wrapper(&cidr);
          	//      	config_pP->ipv6.netmask_s11 = atoi ((const char*)mask->data);
          }
      }
    }
  }

  /** Optional MBMS Features. */
  setting_mce = config_setting_get_member (setting_mme, MME_CONFIG_STRING_MBMS);
  if (setting_mce != NULL) {

    /**
     * First initialize IP
     */
	config_pP->mbms.ip.mc_mce_v4.s_addr = INADDR_ANY;
	config_pP->mbms.ip.mc_mce_v6 = in6addr_any;
	config_pP->mbms.ip.port_sm = 2123;
	config_pP->mbms.ip.mc_mce_v4.s_addr = INADDR_ANY;
	config_pP->mbms.ip.mc_mce_v6 = in6addr_any;

	config_setting_lookup_string (setting_mce, MME_CONFIG_STRING_IPV4_ADDRESS_FOR_MC, (const char **)&mc_mce_v4);
	config_setting_lookup_string (setting_mce, MME_CONFIG_STRING_IPV6_ADDRESS_FOR_MC, (const char **)&mc_mce_v6);

	if (
		(
		/** MC. */
		config_setting_lookup_string (setting_mce, MME_CONFIG_STRING_INTERFACE_NAME_FOR_MC, (const char **)&if_name_mc) && (mc_mce_v4 || mc_mce_v6)
		&& config_setting_lookup_int (setting_mce, MME_CONFIG_STRING_MME_PORT_FOR_SM, &aint_mc)
		)
	) {
		config_pP->mbms.ip.port_sm = (uint16_t)aint_mc;
		struct bstrList *list = NULL;

		/** MC: IPv4. */
		config_pP->mbms.ip.if_name_mc = bfromcstr(if_name_mc);
		if(mc_mce_v4) {
			cidr = bfromcstr (mc_mce_v4);
			list = bsplit (cidr, '/');
			AssertFatal(2 == list->qty, "Bad CIDR address %s", bdata(cidr));
			address = list->entry[0];
			mask    = list->entry[1];
			IPV4_STR_ADDR_TO_INADDR (bdata(address), config_pP->mbms.ip.mc_mce_v4, "BAD IP ADDRESS FORMAT FOR MC-MME !\n");
			config_pP->mbms.ip.mc_mce_cidrv4 = atoi ((const char*)mask->data);
			bstrListDestroy(list);
			in_addr_var.s_addr = config_pP->mbms.ip.mc_mce_v4.s_addr;
			OAILOG_INFO (LOG_MME_APP, "Parsing configuration file found MC-MME: %s/%d on %s\n",
					inet_ntoa (in_addr_var), config_pP->mbms.ip.mc_mce_cidrv4, bdata(config_pP->mbms.ip.if_name_mc));
			bdestroy_wrapper(&cidr);
		}
		/** MC: IPv6. */
		if(mc_mce_v6) {
			cidr = bfromcstr (mc_mce_v6);
			list = bsplit (cidr, '/');
			AssertFatal(2 == list->qty, "Bad CIDR address %s", bdata(cidr));
			address = list->entry[0];
			mask    = list->entry[1];
			IPV6_STR_ADDR_TO_INADDR (bdata(address), config_pP->mbms.ip.mc_mce_v6, "BAD IPV6 ADDRESS FORMAT FOR MC-MME !\n");
			//    	config_pP->ipv6.netmask_s1_mme = atoi ((const char*)mask->data);
			bstrListDestroy(list);
			//    	memcpy(in6_addr_var.s6_addr, config_pP->ipv6.s1_mme.s6_addr, 16);

			char strv6[16];
			OAILOG_INFO (LOG_MME_APP, "Parsing configuration file found MC-MME: %s/%d on %s\n",
					inet_ntop(AF_INET6, &in6_addr_var, strv6, 16), config_pP->mbms.ip.mc_mce_cidrv6, bdata(config_pP->mbms.ip.if_name_mc));
			bdestroy_wrapper(&cidr);
		}
	}

    if ((config_setting_lookup_string (setting_mce, MME_CONFIG_STRING_MCC, &mcc))) {
      AssertFatal( 3 == strlen(mcc), "Bad MCE MCC length, it must be 3 digit ex: 001");
      char c[2] = { mcc[0], 0};
      config_pP->mbms.mce_plmn.mcc_digit1 = (uint8_t) atoi (c);
      c[0] = mcc[1];
      config_pP->mbms.mce_plmn.mcc_digit2 = (uint8_t) atoi (c);
      c[0] = mcc[2];
      config_pP->mbms.mce_plmn.mcc_digit3 = (uint8_t) atoi (c);
    }

    if ((config_setting_lookup_string (setting_mce, MME_CONFIG_STRING_MNC, &mnc))) {
      AssertFatal( (3 == strlen(mnc)) || (2 == strlen(mnc)) , "Bad MCE MNC length, it must be 3 digit ex: 001");
      char c[2] = { mnc[0], 0};
      config_pP->mbms.mce_plmn.mnc_digit1 = (uint8_t) atoi (c);
      c[0] = mnc[1];
      config_pP->mbms.mce_plmn.mnc_digit2 = (uint8_t) atoi (c);
      if (3 == strlen(mnc)) {
      	c[0] = mnc[2];
      	config_pP->mbms.mce_plmn.mnc_digit3 = (uint8_t) atoi (c);
      } else {
        config_pP->mbms.mce_plmn.mnc_digit3 = 0x0F;
      }
    }

    if ((config_setting_lookup_int (setting_mce, MME_CONFIG_MAX_MBMS_SERVICES, &aint))) {
      config_pP->mbms.max_mbms_services = (uint32_t) aint;
    }

    if ((config_setting_lookup_int (setting_mce, MME_CONFIG_MCE_ID, &aint))) {
      config_pP->mbms.mce_id = (uint16_t) aint;
    }

    if ((config_setting_lookup_int (setting_mce, MME_CONFIG_MBMS_SHORT_IDLE_SESSION_DUR_IN_SEC, &aint))) {
    	config_pP->mbms.mbms_short_idle_session_duration_in_sec = (uint8_t) aint;
    }

    if ((config_setting_lookup_int (setting_mce, MME_CONFIG_MBMS_MCCH_MSI_MCS, &aint))) {
      config_pP->mbms.mbms_mcch_msi_mcs = (uint8_t) aint;
			AssertFatal(config_pP->mbms.mbms_mcch_msi_mcs == 2 || config_pP->mbms.mbms_mcch_msi_mcs == 7
					|| config_pP->mbms.mbms_mcch_msi_mcs == 9 || config_pP->mbms.mbms_mcch_msi_mcs == 13,
					"Bad MCCH MSI %d", config_pP->mbms.mbms_mcch_msi_mcs);
    }

    if ((config_setting_lookup_int (setting_mce, MME_CONFIG_MBMS_MCCH_MODIFICATION_PERIOD_RF, &aint))) {
      config_pP->mbms.mbms_mcch_modification_period_rf = (uint16_t) aint;
    }

    AssertFatal(log2(config_pP->mbms.mbms_mcch_modification_period_rf) == 9
    		|| log2(config_pP->mbms.mbms_mcch_modification_period_rf) == 10,
    		"Only Pre-Release 14 MBMS MCCH Modification Period Rfs supported 512/1024. Current (%d).",
				config_pP->mbms.mbms_mcch_modification_period_rf);

    if ((config_setting_lookup_int (setting_mce, MME_CONFIG_MBMS_MCCH_REPETITION_PERIOD_RF, &aint))) {
      config_pP->mbms.mbms_mcch_repetition_period_rf = (uint16_t) aint;
    }

    AssertFatal(log2(config_pP->mbms.mbms_mcch_repetition_period_rf) >= 5
    		&& log2(config_pP->mbms.mbms_mcch_repetition_period_rf) <= 8,
    		"Only Pre-Release 14 MBMS MCCH Repetition Period Rfs supported 32..256. Current (%d).",
				config_pP->mbms.mbms_mcch_repetition_period_rf);

    /** Check the conditions on the MCCH modification & repetition periods. */
    AssertFatal(!(config_pP->mbms.mbms_mcch_modification_period_rf % config_pP->mbms.mbms_mcch_repetition_period_rf),
    		"MBMS MCCH Modification Period Rf (%d) should be an intiger multiple of MCCH Repetition Period RF (%d).",
				config_pP->mbms.mbms_mcch_modification_period_rf, config_pP->mbms.mbms_mcch_repetition_period_rf);

    AssertFatal((config_pP->mbms.mbms_mcch_modification_period_rf / config_pP->mbms.mbms_mcch_repetition_period_rf) > 1,
    		"MBMS MCCH Modification Period Rf (%d) should be larger than MCCH Repetition Period RF (%d).",
				config_pP->mbms.mbms_mcch_modification_period_rf, config_pP->mbms.mbms_mcch_repetition_period_rf);

    if ((config_setting_lookup_float(setting_mce, MME_CONFIG_MCH_MCS_ENB_FACTOR, &adouble))) {
      config_pP->mbms.mch_mcs_enb_factor = (double) adouble;
    }
    AssertFatal(config_pP->mbms.mch_mcs_enb_factor > 1, "MCH MCS eNB factor (%d) should >1.", config_pP->mbms.mch_mcs_enb_factor);

    if ((config_setting_lookup_float(setting_mce, MME_CONFIG_MBSFN_CSA_4_RF_THRESHOLD, &adouble))) {
    	config_pP->mbms.mbsfn_csa_4_rf_threshold = (double) adouble;
    }
    AssertFatal(config_pP->mbms.mbsfn_csa_4_rf_threshold > 1, "MBSFN 4RF Allocation Threshold (%d) should >1.", config_pP->mbms.mbsfn_csa_4_rf_threshold);

    /** MBMS SA configurations. */
    if ((config_setting_lookup_int (setting_mce, MME_CONFIG_MBMS_GLOBAL_SERVICE_AREA_TYPES, &aint))) {
     config_pP->mbms.mbms_global_service_area_types = (uint8_t) aint;
    }

    if ((config_setting_lookup_int (setting_mce, MME_CONFIG_MBMS_LOCAL_SERVICE_AREAS, &aint))) {
      config_pP->mbms.mbms_local_service_areas = (uint8_t) aint;
    }
    DevAssert(config_pP->mbms.mbms_local_service_areas < MME_CONFIG_MAX_LOCAL_MBMS_SERVICE_AREAS);

    if ((config_setting_lookup_int (setting_mce, MME_CONFIG_MBMS_LOCAL_SERVICE_AREA_TYPES, &aint))) {
	  config_pP->mbms.mbms_local_service_area_types = (uint8_t) aint;
    }

    if ((config_setting_lookup_int (setting_mce, MME_CONFIG_MBMS_LOCAL_SERVICE_AREA_SFD_DISTANCES_IN_M, &aint))) {
      config_pP->mbms.mbms_local_service_area_sfd_distance_in_m = (uint16_t) aint;
    }

    if ((config_setting_lookup_int (setting_mce, MME_CONFIG_MBSFN_SYNCH_AREA_ID, &aint))) {
      config_pP->mbms.mbsfn_synch_area_id = (uint8_t) aint;
    }

    /** MBMS eNB configurations. */
    if ((config_setting_lookup_int (setting_mce, MME_CONFIG_STRING_M2_MAX_ENB, &aint))) {
      config_pP->mbms.max_m2_enbs = (uint32_t) aint;
    }

    if ((config_setting_lookup_int (setting_mce, MME_CONFIG_MBMS_M2_ENB_BAND, &aint))) {
      config_pP->mbms.mbms_m2_enb_band = (enb_band_e) aint;
    }
    AssertFatal(get_enb_type(config_pP->mbms.mbms_m2_enb_band) != ENB_TYPE_NULL, "MBSFN band is invalid!");

    if ((config_setting_lookup_int (setting_mce, MME_CONFIG_MBMS_M2_ENB_TDD_UL_DL_SF_CONF, &aint))) {
      config_pP->mbms.mbms_m2_enb_tdd_ul_dl_sf_conf = (uint8_t) aint;
    }
    AssertFatal(config_pP->mbms.mbms_m2_enb_tdd_ul_dl_sf_conf >= 0 && config_pP->mbms.mbms_m2_enb_tdd_ul_dl_sf_conf <=6,
    		"MBMS TDD UL/DL Configuration (%d) is not in bounds [0,6].", config_pP->mbms.mbms_m2_enb_tdd_ul_dl_sf_conf);

    if ((config_setting_lookup_int (setting_mce, MME_CONFIG_MBMS_M2_ENB_BW, &aint))) {
      config_pP->mbms.mbms_m2_enb_bw = (enb_bw_e) aint;
    }
    AssertFatal(config_pP->mbms.mbms_m2_enb_tdd_ul_dl_sf_conf >= 0 && config_pP->mbms.mbms_m2_enb_tdd_ul_dl_sf_conf <=6,
    		"MBMS TDD UL/DL Configuration (%d) is not in bounds [0,6].", config_pP->mbms.mbms_m2_enb_tdd_ul_dl_sf_conf);

    /** MBMS Flags. */
    if ((config_setting_lookup_int (setting_mce, MME_CONFIG_MBMS_ENB_SCPTM, &aint))) {
      config_pP->mbms.mbms_enb_scptm = ((uint8_t) aint & 0x01);
    }
    AssertFatal(!config_pP->mbms.mbms_enb_scptm, "SC-PTM not supported right now.");

    if ((config_setting_lookup_int (setting_mce, MME_CONFIG_MBMS_RESOURCE_ALLOCATION_FULL, &aint))) {
      config_pP->mbms.mbms_resource_allocation_full = ((uint8_t) aint & 0x01);
    }
    AssertFatal(!config_pP->mbms.mbms_resource_allocation_full, "MBSFN Full Radio Frame allocation not supported currently.");

    if ((config_setting_lookup_int (setting_mce, MME_CONFIG_MBMS_GLOBAL_MBSFN_AREA_PER_LOCAL_GROUP, &aint))) {
      config_pP->mbms.mbms_global_mbsfn_area_per_local_group = ((uint8_t) aint & 0x01);
    }

    /** Check the MBMS Local and Global Service Area types. */
    int max_mbsfn_area_id = 0;
    if(config_pP->mbms.mbms_global_mbsfn_area_per_local_group) {
  		max_mbsfn_area_id = mce_config.mbms.mbms_global_service_area_types + (mce_config.mbms.mbms_local_service_areas) * (mce_config.mbms.mbms_local_service_area_types + mce_config.mbms.mbms_global_service_area_types);
    } else {
    	max_mbsfn_area_id = mce_config.mbms.mbms_global_service_area_types + (mce_config.mbms.mbms_local_service_areas * mce_config.mbms.mbms_local_service_area_types);
    }
    AssertFatal(max_mbsfn_area_id <= MAX_MBMSFN_AREAS, "MBMS Area Configuration exceeds bounds.");

    if ((config_setting_lookup_int (setting_mce, MME_CONFIG_MBMS_SUBFRAME_SLOT_HALF, &aint))) {
      config_pP->mbms.mbms_subframe_slot_half = ((uint8_t) aint & 0x01);
    }

    /**
     * Check for the case of local-global flag inactive, if, given the eNB configuration, any local/global MBMS areas are allowed.
     */
    uint8_t mbsfn_mcch_size = get_enb_subframe_size(get_enb_type(config_pP->mbms.mbms_m2_enb_band), config_pP->mbms.mbms_m2_enb_tdd_ul_dl_sf_conf);
    /**
     * If the local-global flag is set, we can configure arbitrarily local MBMS areas,
     * The MCCH subframes will be assigned sequentially.
     * If not, we need to check that we can reserve the given number of global MBMS areas.
     * We don't check for local.
     */
    if(!config_pP->mbms.mbms_global_mbsfn_area_per_local_group){
    	if(mbsfn_mcch_size < config_pP->mbms.mbms_global_service_area_types){
    		OAILOG_ERROR(LOG_MCE_APP, "Total (%d) MCCH MBSFN subframes cannot be lower than #global_mbms_areas (%d).\n",
    				mbsfn_mcch_size, config_pP->mbms.mbms_global_service_area_types);
    		DevAssert(0);
    	}
    	if(mbsfn_mcch_size == config_pP->mbms.mbms_global_service_area_types){
    		/** No local MBMS areas may exist. */
    		if(config_pP->mbms.mbms_local_service_areas){
    			OAILOG_ERROR(LOG_MCE_APP, "Total of (%d) MCCH MBSFN subframes are available. We cannot assign any local MBMS areas.\n", mbsfn_mcch_size);
    			DevAssert(0);
    		}
    	}
    }
  }
  OAILOG_SET_CONFIG(&config_pP->log_config);
  config_destroy (&cfg);
  return 0;
}


//------------------------------------------------------------------------------
static void mce_config_display (mce_config_t * config_pP)
{
  int                                     j;

  OAILOG_INFO (LOG_CONFIG, "==== EURECOM %s v%s ====\n", PACKAGE_NAME, PACKAGE_VERSION);
#if DEBUG_IS_ON
  OAILOG_DEBUG (LOG_CONFIG, "Built with CMAKE_BUILD_TYPE ................: %s\n", CMAKE_BUILD_TYPE);
  OAILOG_DEBUG (LOG_CONFIG, "Built with DISABLE_ITTI_DETECT_SUB_TASK_ID .: %d\n", DISABLE_ITTI_DETECT_SUB_TASK_ID);
  OAILOG_DEBUG (LOG_CONFIG, "Built with ITTI_TASK_STACK_SIZE ............: %d\n", ITTI_TASK_STACK_SIZE);
  OAILOG_DEBUG (LOG_CONFIG, "Built with ITTI_LITE .......................: %d\n", ITTI_LITE);
  OAILOG_DEBUG (LOG_CONFIG, "Built with LOG_OAI .........................: %d\n", LOG_OAI);
  OAILOG_DEBUG (LOG_CONFIG, "Built with LOG_OAI_CLEAN_HARD ..............: %d\n", LOG_OAI_CLEAN_HARD);
  OAILOG_DEBUG (LOG_CONFIG, "Built with MESSAGE_CHART_GENERATOR .........: %d\n", MESSAGE_CHART_GENERATOR);
  OAILOG_DEBUG (LOG_CONFIG, "Built with PACKAGE_NAME ....................: %s\n", PACKAGE_NAME);
  OAILOG_DEBUG (LOG_CONFIG, "Built with S1AP_DEBUG_LIST .................: %d\n", M2AP_DEBUG_LIST);
  OAILOG_DEBUG (LOG_CONFIG, "Built with SECU_DEBUG ......................: %d\n", SECU_DEBUG);
  OAILOG_DEBUG (LOG_CONFIG, "Built with SCTP_DUMP_LIST ..................: %d\n", SCTP_DUMP_LIST);
  OAILOG_DEBUG (LOG_CONFIG, "Built with TRACE_HASHTABLE .................: %d\n", TRACE_HASHTABLE);
  OAILOG_DEBUG (LOG_CONFIG, "Built with TRACE_3GPP_SPEC .................: %d\n", TRACE_3GPP_SPEC);
#endif
  OAILOG_INFO (LOG_CONFIG, "Configuration:\n");
  OAILOG_INFO (LOG_CONFIG, "- File .................................: %s\n", bdata(config_pP->config_file));
  OAILOG_INFO (LOG_CONFIG, "- Realm ................................: %s\n", bdata(config_pP->realm));
  OAILOG_INFO (LOG_CONFIG, "- Run mode .............................: %s\n", (RUN_MODE_BASIC == config_pP->run_mode) ? "BASIC":(RUN_MODE_SCENARIO_PLAYER == config_pP->run_mode) ? "SCENARIO_PLAYER":"UNKNOWN");
  OAILOG_INFO (LOG_CONFIG, "- Max M2 eNBs ..........................: %u\n", config_pP->mbms.max_m2_enbs);
  OAILOG_INFO (LOG_CONFIG, "- Max MBMS-Services ....................: %u\n", config_pP->mbms.max_mbms_services);
  OAILOG_INFO (LOG_CONFIG, "- Max MBMS-Global-Areas.................: %u\n", config_pP->mbms.mbms_global_service_area_types);
  OAILOG_INFO (LOG_CONFIG, "- Max MBMS-Local-Areas..................: %u\n", config_pP->mbms.mbms_local_service_areas);
  OAILOG_INFO (LOG_CONFIG, "- Max MBMS-Local-Area-Types.............: %u\n", config_pP->mbms.mbms_local_service_area_types);
  OAILOG_INFO (LOG_CONFIG, "- Location services via epc ............: %s\n", config_pP->eps_network_feature_support.location_services_via_epc == 0 ? "false" : "true");
  OAILOG_INFO (LOG_CONFIG, "- Extended service request .............: %s\n", config_pP->eps_network_feature_support.extended_service_request == 0 ? "false" : "true");
  OAILOG_INFO (LOG_CONFIG, "- Relative capa ........................: %u\n", config_pP->relative_capacity);
  OAILOG_INFO (LOG_CONFIG, "- Statistics timer .....................: %u (seconds)\n\n", config_pP->mme_statistic_timer);
  OAILOG_INFO (LOG_CONFIG, "- S1-MME:\n");
  OAILOG_INFO (LOG_CONFIG, "    port number ......: %d\n", config_pP->s1ap_config.port_number);
  OAILOG_INFO (LOG_CONFIG, "- IP:\n");
  // todo: print ipv6 addresses
  OAILOG_INFO (LOG_CONFIG, "    MC MME iface .....: %s\n", bdata(config_pP->mbms.ip.if_name_mc));
  OAILOG_INFO (LOG_CONFIG, "    MC MME Sm port ...: %d\n", config_pP->mbms.ip.port_sm);
  OAILOG_INFO (LOG_CONFIG, "    MC MME ip ........: %s\n", inet_ntoa (*((struct in_addr *)&config_pP->mbms.ip.mc_mce_v4)));
  OAILOG_INFO (LOG_CONFIG, "- ITTI:\n");
  OAILOG_INFO (LOG_CONFIG, "    queue size .......: %u (bytes)\n", config_pP->itti_config.queue_size);
  OAILOG_INFO (LOG_CONFIG, "    log file .........: %s\n", bdata(config_pP->itti_config.log_file));
  OAILOG_INFO (LOG_CONFIG, "- SCTP:\n");
  OAILOG_INFO (LOG_CONFIG, "    in streams .......: %u\n", config_pP->sctp_config.in_streams);
  OAILOG_INFO (LOG_CONFIG, "    out streams ......: %u\n", config_pP->sctp_config.out_streams);
  OAILOG_INFO (LOG_CONFIG, "- GUMMEIs (PLMN|MMEGI|MMEC):\n");
  for (j = 0; j < config_pP->gummei.nb; j++) {
    OAILOG_INFO (LOG_CONFIG, "            " PLMN_FMT "|%u|%u \n",
        PLMN_ARG(&config_pP->gummei.gummei[j].plmn), config_pP->gummei.gummei[j].mme_gid, config_pP->gummei.gummei[j].mme_code);
  }

  OAILOG_INFO (LOG_CONFIG, "- TAIs : (mcc.mnc:tac)\n");
  switch (config_pP->served_tai.list_type) {
  case TRACKING_AREA_IDENTITY_LIST_TYPE_ONE_PLMN_CONSECUTIVE_TACS:
    OAILOG_INFO (LOG_CONFIG, "- TAI list type one PLMN consecutive TACs\n");
    break;
  case TRACKING_AREA_IDENTITY_LIST_TYPE_ONE_PLMN_NON_CONSECUTIVE_TACS:
    OAILOG_INFO (LOG_CONFIG, "- TAI list type one PLMN non consecutive TACs\n");
    break;
  case TRACKING_AREA_IDENTITY_LIST_TYPE_MANY_PLMNS:
    OAILOG_INFO (LOG_CONFIG, "- TAI list type multiple PLMNs\n");
    break;
  }
  for (j = 0; j < config_pP->served_tai.nb_tai; j++) {
    if (config_pP->served_tai.plmn_mnc_len[j] == 2) {
      OAILOG_INFO (LOG_CONFIG, "            %3u.%3u:%u\n",
          config_pP->served_tai.plmn_mcc[j], config_pP->served_tai.plmn_mnc[j], config_pP->served_tai.tac[j]);
    } else {
      OAILOG_INFO (LOG_CONFIG, "            %3u.%03u:%u\n",
          config_pP->served_tai.plmn_mcc[j], config_pP->served_tai.plmn_mnc[j], config_pP->served_tai.tac[j]);
    }
  }

  OAILOG_INFO (LOG_CONFIG, "- NAS:\n");
  OAILOG_INFO (LOG_CONFIG, "    Prefered Integrity Algorithms .: EIA%d EIA%d EIA%d EIA%d (decreasing priority)\n",
      config_pP->nas_config.prefered_integrity_algorithm[0],
      config_pP->nas_config.prefered_integrity_algorithm[1],
      config_pP->nas_config.prefered_integrity_algorithm[2],
      config_pP->nas_config.prefered_integrity_algorithm[3]);
  OAILOG_INFO (LOG_CONFIG, "    Prefered Integrity Algorithms .: EEA%d EEA%d EEA%d EEA%d (decreasing priority)\n",
      config_pP->nas_config.prefered_ciphering_algorithm[0],
      config_pP->nas_config.prefered_ciphering_algorithm[1],
      config_pP->nas_config.prefered_ciphering_algorithm[2],
      config_pP->nas_config.prefered_ciphering_algorithm[3]);
  OAILOG_INFO (LOG_CONFIG, "    T3346 ....: %d min\n", config_pP->nas_config.t3346_sec);
  OAILOG_INFO (LOG_CONFIG, "    T3402 ....: %d min\n", config_pP->nas_config.t3402_min);
  OAILOG_INFO (LOG_CONFIG, "    T3412 ....: %d min\n", config_pP->nas_config.t3412_min);
  OAILOG_INFO (LOG_CONFIG, "    T3422 ....: %d sec\n", config_pP->nas_config.t3422_sec);
  OAILOG_INFO (LOG_CONFIG, "    T3450 ....: %d sec\n", config_pP->nas_config.t3450_sec);
  OAILOG_INFO (LOG_CONFIG, "    T3460 ....: %d sec\n", config_pP->nas_config.t3460_sec);
  OAILOG_INFO (LOG_CONFIG, "    T3470 ....: %d sec\n", config_pP->nas_config.t3470_sec);
  OAILOG_INFO (LOG_CONFIG, "    T3485 ....: %d sec\n", config_pP->nas_config.t3485_sec);
  OAILOG_INFO (LOG_CONFIG, "    T3486 ....: %d sec\n", config_pP->nas_config.t3486_sec);
  OAILOG_INFO (LOG_CONFIG, "    T3489 ....: %d sec\n", config_pP->nas_config.t3489_sec);
  OAILOG_INFO (LOG_CONFIG, "    T3470 ....: %d sec\n", config_pP->nas_config.t3470_sec);
  OAILOG_INFO (LOG_CONFIG, "    T3495 ....: %d sec\n", config_pP->nas_config.t3495_sec);
  OAILOG_INFO (LOG_CONFIG, "    NAS non standart features .:\n");
  OAILOG_INFO (LOG_CONFIG, "      Force TAU ...................: %s\n", (config_pP->nas_config.force_tau) ? "true":"false");
  OAILOG_INFO (LOG_CONFIG, "      Force reject SR .............: %s\n", (config_pP->nas_config.force_reject_sr) ? "true":"false");
  OAILOG_INFO (LOG_CONFIG, "      Disable Esm information .....: %s\n", (config_pP->nas_config.disable_esm_information) ? "true":"false");

  OAILOG_INFO (LOG_CONFIG, "- S6A:\n");
  OAILOG_INFO (LOG_CONFIG, "    conf file ........: %s\n", bdata(config_pP->s6a_config.conf_file));
  OAILOG_INFO (LOG_CONFIG, "- Logging:\n");
  OAILOG_INFO (LOG_CONFIG, "    Output ..............: %s\n", bdata(config_pP->log_config.output));
  OAILOG_INFO (LOG_CONFIG, "    Output thread safe ..: %s\n", (config_pP->log_config.is_output_thread_safe) ? "true":"false");
  OAILOG_INFO (LOG_CONFIG, "    Output with color ...: %s\n", (config_pP->log_config.color) ? "true":"false");
  OAILOG_INFO (LOG_CONFIG, "    UDP log level........: %s\n", OAILOG_LEVEL_INT2STR(config_pP->log_config.udp_log_level));
  OAILOG_INFO (LOG_CONFIG, "    GTPV2-C log level....: %s\n", OAILOG_LEVEL_INT2STR(config_pP->log_config.gtpv2c_log_level));
  OAILOG_INFO (LOG_CONFIG, "    SCTP log level.......: %s\n", OAILOG_LEVEL_INT2STR(config_pP->log_config.sctp_log_level));
  OAILOG_INFO (LOG_CONFIG, "    S1AP log level.......: %s\n", OAILOG_LEVEL_INT2STR(config_pP->log_config.s1ap_log_level));
  OAILOG_INFO (LOG_CONFIG, "    ASN1 Verbosity level : %d\n", config_pP->log_config.asn1_verbosity_level);
  OAILOG_INFO (LOG_CONFIG, "    NAS log level........: %s\n", OAILOG_LEVEL_INT2STR(config_pP->log_config.nas_log_level));
  OAILOG_INFO (LOG_CONFIG, "    MME_APP log level....: %s\n", OAILOG_LEVEL_INT2STR(config_pP->log_config.mme_app_log_level));
  OAILOG_INFO (LOG_CONFIG, "    MCE_APP log level....: %s\n", OAILOG_LEVEL_INT2STR(config_pP->log_config.mce_app_log_level));
  OAILOG_INFO (LOG_CONFIG, "    S10 log level........: %s\n", OAILOG_LEVEL_INT2STR(config_pP->log_config.s10_log_level));
  OAILOG_INFO (LOG_CONFIG, "    S11 log level........: %s\n", OAILOG_LEVEL_INT2STR(config_pP->log_config.s11_log_level));
  OAILOG_INFO (LOG_CONFIG, "    SM log level.........: %s\n", OAILOG_LEVEL_INT2STR(config_pP->log_config.sm_log_level));
  OAILOG_INFO (LOG_CONFIG, "    S6a log level........: %s\n", OAILOG_LEVEL_INT2STR(config_pP->log_config.s6a_log_level));
  OAILOG_INFO (LOG_CONFIG, "    UTIL log level.......: %s\n", OAILOG_LEVEL_INT2STR(config_pP->log_config.util_log_level));
  OAILOG_INFO (LOG_CONFIG, "    MSC log level........: %s (MeSsage Chart)\n", OAILOG_LEVEL_INT2STR(config_pP->log_config.msc_log_level));
  OAILOG_INFO (LOG_CONFIG, "    ITTI log level.......: %s (InTer-Task Interface)\n", OAILOG_LEVEL_INT2STR(config_pP->log_config.itti_log_level));
  OAILOG_INFO (LOG_CONFIG, "    XML log level........: %s (XML dump/load of messages)\n", OAILOG_LEVEL_INT2STR(config_pP->log_config.xml_log_level));
#if TRACE_XML
  if (RUN_MODE_SCENARIO_PLAYER == config_pP->run_mode) {
    OAILOG_INFO (LOG_CONFIG, "    MME SP log level.....: %s (MME scenario player)\n", OAILOG_LEVEL_INT2STR(config_pP->log_config.mme_scenario_player_log_level));
  }
#endif
}

//------------------------------------------------------------------------------
static void usage (char *target)
{
  OAI_FPRINTF_INFO( "==== EURECOM %s version: %s ====\n", PACKAGE_NAME, PACKAGE_VERSION);
  OAI_FPRINTF_INFO( "Please report any bug to: %s\n", PACKAGE_BUGREPORT);
  OAI_FPRINTF_INFO( "Usage: %s [options]\n", target);
  OAI_FPRINTF_INFO( "Available options:\n");
  OAI_FPRINTF_INFO( "-h      Print this help and return\n");
  OAI_FPRINTF_INFO( "-c<path>\n");
  OAI_FPRINTF_INFO( "        Set the configuration file for mme\n");
  OAI_FPRINTF_INFO( "        See template in UTILS/CONF\n");
#if TRACE_XML
  OAI_FPRINTF_INFO( "-s<path>\n");
  OAI_FPRINTF_INFO( "        Set the scenario file for mme scenario player\n");
#endif
  OAI_FPRINTF_INFO( "-V      Print %s version and return\n", PACKAGE_NAME);
  OAI_FPRINTF_INFO( "-v[1-2] Debug level:\n");
  OAI_FPRINTF_INFO( "            1 -> ASN1 XER printf on and ASN1 debug off\n");
  OAI_FPRINTF_INFO( "            2 -> ASN1 XER printf on and ASN1 debug on\n");
}

//------------------------------------------------------------------------------
int
mce_config_parse_opt_line (
  int argc,
  char *argv[],
  mce_config_t * config_pP)
{
  int                                     c;
  char                                   *config_file = NULL;

  mce_config_init (config_pP);

  /*
   * Parsing command line
   */
  while ((c = getopt (argc, argv, "c:hs:S:v:V")) != -1) {
    switch (c) {
    case 'c':{
        /*
         * Store the given configuration file. If no file is given,
         * * * * then the default values will be used.
         */
        config_pP->config_file = blk2bstr(optarg, strlen(optarg));
        OAI_FPRINTF_INFO ("%s mce_config.config_file %s\n", __FUNCTION__, bdata(config_pP->config_file));
      }
      break;

    case 'v':{
        config_pP->log_config.asn1_verbosity_level = atoi (optarg);
      }
      break;

    case 'V':{
      OAI_FPRINTF_INFO ("==== EURECOM %s v%s ====" "Please report any bug to: %s\n", PACKAGE_NAME, PACKAGE_VERSION, PACKAGE_BUGREPORT);
      }
      break;

#if TRACE_XML
    case 'S':
      config_pP->run_mode = RUN_MODE_SCENARIO_PLAYER;
      config_pP->scenario_player_config.scenario_file = blk2bstr(optarg, strlen(optarg));
      config_pP->scenario_player_config.stop_on_error = true;
      OAI_FPRINTF_INFO ("%s mce_config.scenario_player_config.scenario_file %s\n", __FUNCTION__, bdata(config_pP->scenario_player_config.scenario_file));
      break;
    case 's':
      config_pP->run_mode = RUN_MODE_SCENARIO_PLAYER;
      config_pP->scenario_player_config.scenario_file = blk2bstr(optarg, strlen(optarg));
      config_pP->scenario_player_config.stop_on_error = false;
      OAI_FPRINTF_INFO ("%s mce_config.scenario_player_config.scenario_file %s\n", __FUNCTION__, bdata(config_pP->scenario_player_config.scenario_file));
      break;
#else
    case 's':
    case 'S':
      OAI_FPRINTF_ERR ("Should have compiled mme executable with TRACE_XML set in CMakeLists.template\n");
      exit (0);
      break;
#endif

    case 'h':                  /* Fall through */
    default:
      OAI_FPRINTF_ERR ("Unknown command line option %c\n", c);
      usage (argv[0]);
      exit (0);
    }
  }

  if (!config_pP->config_file) {
    config_file = getenv("CONFIG_FILE");
    if (config_file) {
      config_pP->config_file            = bfromcstr(config_file);
    } else {
      OAILOG_ERROR (LOG_CONFIG, "No config file provided through arg -c, or env variable CONFIG_FILE, exiting\n");
      return RETURNerror;
    }
  }
  OAILOG_DEBUG (LOG_CONFIG, "Config file is %s\n", config_file);
  /*
   * Parse the configuration file using libconfig
   */
  if (mce_config_parse_file (config_pP) != 0) {
    return -1;
  }

  /*
   * Display the configuration
   */
  mce_config_display (config_pP);
  return 0;
}
