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

/*! \file mce_config.h
  \brief
  \author Lionel Gauthier
  \company Eurecom
  \email: lionel.gauthier@eurecom.fr
*/

#ifndef FILE_MME_CONFIG_SEEN
#define FILE_MME_CONFIG_SEEN
#include <arpa/inet.h>

#include "mme_default_values.h"
#include "3gpp_23.003.h"
#include "3gpp_29.274.h"
#include "common_dim.h"
#include "common_types_mbms.h"
#include "bstrlib.h"
#include "log.h"

#define MAX_GUMMEI                2
#define MAX_MBMS_SA				  8

#define MME_CONFIG_STRING_MME_CONFIG                     "MME"
#define MME_CONFIG_STRING_PID_DIRECTORY                  "PID_DIRECTORY"
#define MME_CONFIG_STRING_INSTANCE                       "INSTANCE"

// todo: what is run mode?
#define MME_CONFIG_STRING_RUN_MODE                       "RUN_MODE"
#define MME_CONFIG_STRING_RUN_MODE_TEST                  "TEST"

#define MME_CONFIG_STRING_REALM                          "REALM"
#define MME_CONFIG_STRING_S1_MAXENB                      "MAX_S1_ENB"
#define MME_CONFIG_STRING_MAXUE                          "MAX_UE"
#define MME_CONFIG_STRING_RELATIVE_CAPACITY              "RELATIVE_CAPACITY"
#define MME_CONFIG_STRING_STATISTIC_TIMER                "MME_STATISTIC_TIMER"
#define MME_CONFIG_STRING_MME_MOBILITY_COMPLETION_TIMER  "MME_MOBILITY_COMPLETION_TIMER"
#define MME_CONFIG_STRING_MME_S10_HANDOVER_COMPLETION_TIMER  "MME_S10_HANDOVER_COMPLETION_TIMER"

#define MME_CONFIG_STRING_EMERGENCY_ATTACH_SUPPORTED     "EMERGENCY_ATTACH_SUPPORTED"
#define MME_CONFIG_STRING_UNAUTHENTICATED_IMSI_SUPPORTED "UNAUTHENTICATED_IMSI_SUPPORTED"
#define MME_CONFIG_STRING_DUMMY_HANDOVER_FORWARDING_ENABLED "DUMMY_HANDOVER_FORWARDING_ENABLED"

#define EPS_NETWORK_FEATURE_SUPPORT_IMS_VOICE_OVER_PS_SESSION_IN_S1       "EPS_NETWORK_FEATURE_SUPPORT_IMS_VOICE_OVER_PS_SESSION_IN_S1"
#define EPS_NETWORK_FEATURE_SUPPORT_EMERGENCY_BEARER_SERVICES_IN_S1_MODE  "EPS_NETWORK_FEATURE_SUPPORT_EMERGENCY_BEARER_SERVICES_IN_S1_MODE"
#define EPS_NETWORK_FEATURE_SUPPORT_LOCATION_SERVICES_VIA_EPC             "EPS_NETWORK_FEATURE_SUPPORT_LOCATION_SERVICES_VIA_EPC"
#define EPS_NETWORK_FEATURE_SUPPORT_EXTENDED_SERVICE_REQUEST              "EPS_NETWORK_FEATURE_SUPPORT_EXTENDED_SERVICE_REQUEST"


#define MME_CONFIG_STRING_INTERTASK_INTERFACE_CONFIG     "INTERTASK_INTERFACE"
#define MME_CONFIG_STRING_INTERTASK_INTERFACE_QUEUE_SIZE "ITTI_QUEUE_SIZE"

#define MME_CONFIG_STRING_S6A_CONFIG                     "S6A"
#define MME_CONFIG_STRING_S6A_CONF_FILE_PATH             "S6A_CONF"
#define MME_CONFIG_STRING_S6A_HSS_HOSTNAME               "HSS_HOSTNAME"
#define MME_CONFIG_STRING_S6A_MME_HOSTNAME               "MME_HOSTNAME"

#define MME_CONFIG_STRING_SCTP_CONFIG                    "SCTP"
#define MME_CONFIG_STRING_SCTP_INSTREAMS                 "SCTP_INSTREAMS"
#define MME_CONFIG_STRING_SCTP_OUTSTREAMS                "SCTP_OUTSTREAMS"


#define MME_CONFIG_STRING_S1AP_CONFIG                    "S1AP"
#define MME_CONFIG_STRING_S1AP_OUTCOME_TIMER             "S1AP_OUTCOME_TIMER"
#define MME_CONFIG_STRING_S1AP_PORT                      "S1AP_PORT"

#define MME_CONFIG_STRING_GUMMEI_LIST                    "GUMMEI_LIST"
#define MME_CONFIG_STRING_MME_CODE                       "MME_CODE"
#define MME_CONFIG_STRING_MME_GID                        "MME_GID"
#define MME_CONFIG_STRING_TAI_LIST                       "TAI_LIST"
#define MME_CONFIG_STRING_MCC                            "MCC"
#define MME_CONFIG_STRING_MNC                            "MNC"
#define MME_CONFIG_STRING_NGHB_MME_IPV4_ADDR             "NGHB_MME_IPV4_ADDR"
/**
 * MBMS Configuration definitions
 */
#define MME_CONFIG_STRING_MBMS							 						 "MBMS"
#define MME_CONFIG_MAX_MBMS_SERVICES               	 	   "MAX_MBMS_SERVICES"
#define MME_CONFIG_MCE_ID						 		 								 "MCE_ID"
#define MME_CONFIG_MBMS_SHORT_IDLE_SESSION_DUR_IN_SEC    "MBMS_SHORT_IDLE_SESSION_DUR_IN_SEC"

#define MME_CONFIG_MBMS_MCCH_MSI_MCS														"MME_CONFIG_MBMS_MCCH_MSI_MCS"
#define MME_CONFIG_MBMS_MCCH_MODIFICATION_PERIOD_RF					  	"MME_CONFIG_MBMS_MCCH_MODIFICATION_PERIOD_RF"
#define MME_CONFIG_MBMS_MCCH_REPETITION_PERIOD_RF		 						"MME_CONFIG_MBMS_MCCH_REPETITION_PERIOD_RF"
#define MME_CONFIG_MCH_MCS_ENB_FACTOR													  "MME_CONFIG_MCH_MCS_ENB_FACTOR"
#define MME_CONFIG_MBSFN_CSA_4_RF_THRESHOLD											"MME_CONFIG_MBSFN_CSA_4_RF_THRESHOLD"

#define MME_CONFIG_MBMS_GLOBAL_SERVICE_AREA_TYPES		 						"MBMS_GLOBAL_SERVICE_AREAS"
#define MME_CONFIG_MBMS_LOCAL_SERVICE_AREAS			 	 							"MBMS_LOCAL_SERVICE_AREAS"
#define MME_CONFIG_MBMS_LOCAL_SERVICE_AREA_TYPES	 	 						"MBMS_LOCAL_SERVICE_AREA_TYPES"
#define MME_CONFIG_MBMS_LOCAL_SERVICE_AREA_SFD_DISTANCES_IN_M 	"MBMS_LOCAL_SERVICE_AREA_SFD_DISTANCES_IN_M"
#define MME_CONFIG_MBSFN_SYNCH_AREA_ID												  "MBSFN_SYNCH_AREA_ID"

#define MME_CONFIG_STRING_M2_MAX_ENB                     				"MAX_M2_ENB"
#define MME_CONFIG_MBMS_M2_ENB_BAND						 									"MBMS_M2_ENB_BAND"
#define MME_CONFIG_MBMS_M2_ENB_BW																"MBMS_M2_ENB_BW"
#define MME_CONFIG_MBMS_M2_ENB_TDD_UL_DL_SF_CONF 								"MBMS_M2_ENB_TDD_UL_DL_SF_CONF"

#define MME_CONFIG_MBMS_ENB_SCPTM						 							"MBMS_ENB_SCPTM"
#define MME_CONFIG_MBMS_RESOURCE_ALLOCATION_FULL		 			"MBMS_RESOURCE_ALLOCATION_FULL"
#define MME_CONFIG_MBMS_GLOBAL_MBSFN_AREA_PER_LOCAL_GROUP	"MBMS_GLOBAL_MBSFN_AREA_PER_LOCAL_GROUP"
#define MME_CONFIG_MBMS_SUBFRAME_SLOT_HALF				 				"MME_CONFIG_MBMS_SUBFRAME_SLOT_HALF"

#define MME_CONFIG_STRING_NETWORK_INTERFACES_CONFIG      "NETWORK_INTERFACES"
#define MME_CONFIG_STRING_INTERFACE_NAME_FOR_S1_MME      "MME_INTERFACE_NAME_FOR_S1_MME"
#define MME_CONFIG_STRING_IPV4_ADDRESS_FOR_S1_MME        "MME_IPV4_ADDRESS_FOR_S1_MME"
#define MME_CONFIG_STRING_IPV6_ADDRESS_FOR_S1_MME        "MME_IPV6_ADDRESS_FOR_S1_MME"
#define MME_CONFIG_STRING_INTERFACE_NAME_FOR_S11         "MME_INTERFACE_NAME_FOR_S11"
#define MME_CONFIG_STRING_IPV4_ADDRESS_FOR_S11           "MME_IPV4_ADDRESS_FOR_S11"
#define MME_CONFIG_STRING_IPV6_ADDRESS_FOR_S11           "MME_IPV6_ADDRESS_FOR_S11"
#define MME_CONFIG_STRING_MME_PORT_FOR_S11               "MME_PORT_FOR_S11"
#define MME_CONFIG_STRING_INTERFACE_NAME_FOR_S10         "MME_INTERFACE_NAME_FOR_S10"
#define MME_CONFIG_STRING_IPV4_ADDRESS_FOR_S10           "MME_IPV4_ADDRESS_FOR_S10"
#define MME_CONFIG_STRING_IPV6_ADDRESS_FOR_S10           "MME_IPV6_ADDRESS_FOR_S10"
#define MME_CONFIG_STRING_MME_PORT_FOR_S10               "MME_PORT_FOR_S10"

#define MME_CONFIG_STRING_INTERFACE_NAME_FOR_MC          "MME_INTERFACE_NAME_FOR_MC"
#define MME_CONFIG_STRING_IPV4_ADDRESS_FOR_MC            "MME_IPV4_ADDRESS_FOR_MC"
#define MME_CONFIG_STRING_IPV6_ADDRESS_FOR_MC            "MME_IPV6_ADDRESS_FOR_MC"
#define MME_CONFIG_STRING_MME_PORT_FOR_SM                "MME_PORT_FOR_SM"


#define MME_CONFIG_STRING_NAS_CONFIG                     "NAS"
#define MME_CONFIG_STRING_NAS_SUPPORTED_INTEGRITY_ALGORITHM_LIST  "ORDERED_SUPPORTED_INTEGRITY_ALGORITHM_LIST"
#define MME_CONFIG_STRING_NAS_SUPPORTED_CIPHERING_ALGORITHM_LIST  "ORDERED_SUPPORTED_CIPHERING_ALGORITHM_LIST"

#define MME_CONFIG_STRING_NAS_T3346_TIMER                "T3346"
#define MME_CONFIG_STRING_NAS_T3402_TIMER                "T3402"
#define MME_CONFIG_STRING_NAS_T3412_TIMER                "T3412"
#define MME_CONFIG_STRING_NAS_T3422_TIMER                "T3422"
#define MME_CONFIG_STRING_NAS_T3450_TIMER                "T3450"
#define MME_CONFIG_STRING_NAS_T3460_TIMER                "T3460"
#define MME_CONFIG_STRING_NAS_T3470_TIMER                "T3470"
#define MME_CONFIG_STRING_NAS_T3485_TIMER                "T3485"
#define MME_CONFIG_STRING_NAS_T3486_TIMER                "T3486"
#define MME_CONFIG_STRING_NAS_T3489_TIMER                "T3489"
#define MME_CONFIG_STRING_NAS_T3495_TIMER                "T3495"

#define MME_CONFIG_STRING_NAS_DISABLE_ESM_INFORMATION_PROCEDURE    "DISABLE_ESM_INFORMATION_PROCEDURE"
#define MME_CONFIG_STRING_NAS_FORCE_PUSH_DEDICATED_BEARER "FORCE_PUSH_DEDICATED_BEARER"
#define MME_CONFIG_STRING_NAS_FORCE_TAU					  "NAS_FORCE_TAU"

#define MME_CONFIG_STRING_ASN1_VERBOSITY                 "ASN1_VERBOSITY"
#define MME_CONFIG_STRING_ASN1_VERBOSITY_NONE            "none"
#define MME_CONFIG_STRING_ASN1_VERBOSITY_ANNOYING        "annoying"
#define MME_CONFIG_STRING_ASN1_VERBOSITY_INFO            "info"
#define SGW_CONFIG_STRING_SGW_IP_ADDRESS_FOR_S11    	   "SGW_IP_ADDRESS_FOR_S11"

#define MME_CONFIG_STRING_WRR_LIST_SELECTION             "WRR_LIST_SELECTION"
#define MME_CONFIG_STRING_PEER_MME_IP_ADDRESS_FOR_S10  	 "MME_IP_ADDRESS_FOR_S10"

///** MME S10 List --> todo: later FULL WRR : Finding MME via eNB. */
//#define MME_CONFIG_STRING_MME_LIST_SELECTION             "MME_LIST_SELECTION"

#define MME_CONFIG_STRING_ID                             "ID"

typedef enum {
   RUN_MODE_BASIC,
   RUN_MODE_SCENARIO_PLAYER
} run_mode_t;

// todo: run mode?
//typedef enum {
//  RUN_MODE_TEST = 0,
//  RUN_MODE_OTHER
//} run_mode_t;

typedef struct mce_config_s {
  /* Reader/writer lock for this configuration */
  pthread_rwlock_t rw_lock;

  unsigned int instance;
  bstring config_file;
  bstring pid_dir;
  bstring realm;

  run_mode_t  run_mode;

  uint32_t max_s1_enbs;
  uint32_t max_ues;

  uint8_t relative_capacity;

  uint32_t mme_statistic_timer;
  uint32_t mme_mobility_completion_timer;
  uint32_t mme_s10_handover_completion_timer;

  uint8_t unauthenticated_imsi_supported;
  uint8_t dummy_handover_forwarding_enabled;

  struct {
    uint8_t ims_voice_over_ps_session_in_s1;
    uint8_t emergency_bearer_services_in_s1_mode;
    uint8_t location_services_via_epc;
    uint8_t extended_service_request;
  } eps_network_feature_support;

  struct {
    int      nb;
    gummei_t gummei[MAX_GUMMEI];
  } gummei;

  /** MBMS Parameters. */
  struct {
		plmn_t   		mce_plmn;                                        /*!< \brief  GUMMEI               */
		uint16_t 		mce_id;
		uint32_t 		max_mbms_services;
		uint8_t 		mbms_short_idle_session_duration_in_sec;
		/** MCCH values. */
		uint8_t 		mbms_mcch_msi_mcs;
		uint16_t 		mbms_mcch_modification_period_rf;
		uint16_t 		mbms_mcch_repetition_period_rf;
		/** MBMS Service Area configurations. */
		uint8_t  		mbms_global_service_area_types; 	/**< Offset 0. */
		uint8_t  		mbms_local_service_areas; 	/**< Mod. */
		uint8_t  		mbms_local_service_area_types; 	/**< Offset 1. */
		uint16_t 		mbms_local_service_area_sfd_distance_in_m;
		double	 		mch_mcs_enb_factor;
		uint8_t  		mbsfn_synch_area_id;
		double   		mbsfn_csa_4_rf_threshold;

		/** Possible eNB configurations. */
		uint32_t 		max_m2_enbs;
		enb_band_e  mbms_m2_enb_band;
		enb_bw_e	  mbms_m2_enb_bw;
		uint8_t  		mbms_m2_enb_tdd_ul_dl_sf_conf;
		/** Flags. */
		uint8_t  	mbms_enb_scptm:1;
		uint8_t  	mbms_resource_allocation_full:1;
		uint8_t  	mbms_global_mbsfn_area_per_local_group:1;
		uint8_t  	mbms_subframe_slot_half:1;

		/** Interface Configuration. */
			/** Sm/M2AP */
		struct {
				bstring    if_name_mc;
				struct in_addr  mc_mce_v4;
				struct in6_addr mc_mce_v6;
				int        mc_mce_cidrv4;
				int        mc_mce_cidrv6;
				uint16_t   port_sm;
		}ip;
  } mbms;

#define TRACKING_AREA_IDENTITY_LIST_TYPE_ONE_PLMN_NON_CONSECUTIVE_TACS 0x00
#define TRACKING_AREA_IDENTITY_LIST_TYPE_ONE_PLMN_CONSECUTIVE_TACS     0x01
#define TRACKING_AREA_IDENTITY_LIST_TYPE_MANY_PLMNS                    0x02
  struct {
    uint8_t   list_type;
    uint8_t   nb_tai;
    uint16_t *plmn_mcc;
    uint16_t *plmn_mnc;
    uint16_t *plmn_mnc_len;
    uint16_t *tac;
  } served_tai;

  struct {
    uint16_t in_streams;
    uint16_t out_streams;
  } sctp_config;

  struct {
    uint16_t port_number;
    uint8_t  outcome_drop_timer_sec;
  } s1ap_config;

  struct {
    bstring    if_name_s1_mme;
    struct in_addr  s1_mme_v4;
    struct in6_addr s1_mme_v6;
    int        s1_mme_cidrv4;
    int        s1_mme_cidrv6;

    bstring    if_name_s11;
    struct in_addr 	s11_mme_v4;
    struct in6_addr s11_mme_v6;
    int        s11_mme_cidrv4;
    int        s11_mme_cidrv6;
    uint16_t   port_s11;

    bstring    if_name_s10;
    struct in_addr  s10_mme_v4;
    struct in6_addr s10_mme_v6;
    int        s10_mme_cidrv4;
    int        s10_mme_cidrv6;
    uint16_t   port_s10;

  } ip;

  struct {
    bstring conf_file;
    bstring hss_host_name;
    bstring mme_host_name;
  } s6a_config;

  struct {
    uint32_t  queue_size;
    bstring   log_file;
  } itti_config;

  struct {
    uint8_t  prefered_integrity_algorithm[8];
    uint8_t  prefered_ciphering_algorithm[8];
    uint32_t t3346_sec;
    uint32_t t3402_min;
    uint32_t t3412_min;
    uint32_t t3422_sec;
    uint32_t t3450_sec;
    uint32_t t3460_sec;
    uint32_t t3470_sec;
    uint32_t t3485_sec;
    uint32_t t3486_sec;
    uint32_t t3489_sec;
    uint32_t t3495_sec;

    // non standart features
    bool     force_tau;
    bool     force_reject_sr;
    bool     disable_esm_information;
  } nas_config;

  struct {
    int nb_service_entries;
//#define MME_CONFIG_MAX_SGW 16
#define MME_CONFIG_MAX_SERVICE 128
    bstring        			 service_id[MME_CONFIG_MAX_SERVICE];
    interface_type_t 		 interface_type[MME_CONFIG_MAX_SERVICE];
    union{
    	struct sockaddr_in   v4; //   service_ip_addr[MME_CONFIG_MAX_SERVICE]; /**< Just allocating sockaddr was not enough. */
    	struct sockaddr_in6  v6; //; /**< Just allocating sockaddr was not enough. */
    }sockaddr[MME_CONFIG_MAX_SERVICE];
    /** MME entries. */
  } e_dns_emulation;

#if TRACE_XML
  struct {
    bstring scenario_file;
    bool    stop_on_error;
  } scenario_player_config;
#endif

  log_config_t log_config;
} mce_config_t;

extern mce_config_t mce_config;

mbms_service_area_id_t mce_app_check_mbms_sa_exists(const plmn_t * target_plmn, const mbms_service_area_t * mbms_service_area);

int mce_config_find_mnc_length(const char mcc_digit1P,
                               const char mcc_digit2P,
                               const char mcc_digit3P,
                               const char mnc_digit1P,
                               const char mnc_digit2P,
                               const char mnc_digit3P);
int mce_config_parse_opt_line(int argc, char *argv[], mce_config_t *mce_config);

void mce_config_exit (void);

#define mce_config_read_lock(mMEcONFIG)  pthread_rwlock_rdlock(&(mMEcONFIG)->rw_lock)
#define mce_config_write_lock(mMEcONFIG) pthread_rwlock_wrlock(&(mMEcONFIG)->rw_lock)
#define mce_config_unlock(mMEcONFIG)     pthread_rwlock_unlock(&(mMEcONFIG)->rw_lock)

#endif /* FILE_MME_CONFIG_SEEN */
