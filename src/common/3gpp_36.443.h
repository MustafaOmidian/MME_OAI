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

/*! \file 3gpp_36.443.h
  \brief
  \author Dincer Beken
  \company Blackned GmbH
  \email: dbeken@blackned.de
*/

#ifndef FILE_3GPP_36_443_SEEN
#define FILE_3GPP_36_443_SEEN

// (36.413) 9.2.1.19 Bit Rate
// This IE indicates the number of bits delivered by E-UTRAN in UL or to E-UTRAN in DL within a period of time,
// divided by the duration of the period. It is used, for example, to indicate the maximum or guaranteed bit rate for a GBR
// bearer, or an aggregated maximum bit rate.

typedef uint64_t bit_rate_t;

//------------------------------------------------------------------------------
// 3.1 E-UTRAN Identifiers
//------------------------------------------------------------------------------



typedef uint16_t                 enb_mbms_m2ap_id_t;         /*!< \brief  Unique identity, referencing the MBMS-service-associated logical M2-connection within an eNB. */

typedef uint32_t                 mce_mbms_m2ap_id_t;         /*!< \brief  Unique identity, referencing the MBMS-service-associated logical M2-connection within an MCE. */

#endif /* FILE_3GPP_36_443_SEEN */
