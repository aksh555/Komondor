/* Komondor IEEE 802.11ax Simulator
 *
 * Copyright (c) 2017, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007

 * Copyright (C) 2007 Free Software Foundation, Inc. <http://fsf.org/>
 * Everyone is permitted to copy and distribute verbatim copies
 * of this license document, but changing it is not allowed.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 * -----------------------------------------------------------------
 *
 * Author  : Sergio Barrachina-Muñoz and Francesc Wilhelmi
 * Created : 2016-12-05
 * Updated : $Date: 2017/03/20 10:32:36 $
 *           $Revision: 1.0 $
 *
 * -----------------------------------------------------------------
 * File description:
 *
 * - This file contains the methods related to "spatial reuse" operations
 */

#include <stddef.h>
#include <math.h>
#include <iostream>

#include "../list_of_macros.h"
#include "auxiliary_methods.h"

/*
 * CheckPacketOrigin(): checks whether the received notification is an intra or a inter-BSS frame
 **/
int CheckPacketOrigin(Notification notification, int bss_color) {

	int type_of_packet;

	// Check if the packet is an intra or an inter-BSS frame
//	if ( (notification.tx_info.bss_color == bss_color &&
//			notification.tx_info.spatial_reuse_group == spatial_reuse_group) ||
//		 (notification.tx_info.bss_color != bss_color &&
//			notification.tx_info.spatial_reuse_group == spatial_reuse_group) ||
//		 (notification.tx_info.bss_color == bss_color &&
//			notification.tx_info.spatial_reuse_group == 0) ) {
	if ( notification.tx_info.bss_color == bss_color && bss_color > 0) {

		type_of_packet = INTRA_BSS_FRAME;

	} else {

		type_of_packet = INTER_BSS_FRAME;

	}

	return type_of_packet;

}

/*
 * CheckPacketOrigin(): checks whether an inter-BSS frame belongs to the same SRG or not
 **/
int CheckSrg(Notification notification, int spatial_reuse_group) {

	int same_srg_flag;

	// Check if the packet is an intra or an inter-BSS frame
	if ( spatial_reuse_group > 0 &&
			notification.tx_info.spatial_reuse_group == spatial_reuse_group) {

		same_srg_flag = TRUE;

	} else {

		same_srg_flag = FALSE;

	}

	return same_srg_flag;

}

/*
 * CheckOBSSPDConstraints(): checks if the proposed obss_pd_level is valid, according to
 * the constraints indicated in the IEEE 802.11ax amendment
 **/
int CheckObssPdConstraints(double current_obss_pd, double obss_pd_min, double obss_pd_max,
		double tx_power_ref, double tx_power) {

	double obss_pd_ref_dbm = std::max(ConvertPower(PW_TO_DBM, obss_pd_min),
			std::min(ConvertPower(PW_TO_DBM, obss_pd_max), ConvertPower(PW_TO_DBM, obss_pd_min)
			+ (ConvertPower(PW_TO_DBM, tx_power_ref) - ConvertPower(PW_TO_DBM,tx_power))));

//	printf("obss_pd_ref = %f (%f pW)\n", obss_pd_ref, ConvertPower(DBM_TO_PW, obss_pd_ref));
//	printf("  - obss_pd_min = %f (%f pW)\n", ConvertPower(PW_TO_DBM, obss_pd_min), obss_pd_min);
//	printf("  - obss_pd_max = %f (%f pW)\n", ConvertPower(PW_TO_DBM, obss_pd_max), obss_pd_max);
//	printf("  - tx_power_ref = %f (%f pW)\n", ConvertPower(PW_TO_DBM, tx_power_ref), tx_power_ref);
//	printf("  - tx_power = %f (%f pW)\n", ConvertPower(PW_TO_DBM,tx_power), tx_power);

	if( ConvertPower(PW_TO_DBM, current_obss_pd) <=  obss_pd_ref_dbm ) {
		//printf("The OBSS_PD level is appropriate!\n");
		return 1;
	} else {
		//printf("ALERT! The OBSS_PD level is NOT appropriate!\n");
		return 0;
	}

}

/*
 * CheckSrgObssPdConstraints(): checks if the proposed srg_obss_pd_level is valid, according to
 * the constraints indicated in the IEEE 802.11ax amendment
 **/
int CheckSrgObssPdConstraints(double current_srg_obss_pd,
		double srg_obss_pd_min, double srg_obss_pd_max,
		double srg_obss_pd_min_offset, double srg_obss_pd_max_offset) {

	if( (ConvertPower(PW_TO_DBM, srg_obss_pd_min_offset) - 82) <= -62 &&
			(ConvertPower(PW_TO_DBM, srg_obss_pd_min_offset) - 82) >= -82 &&
			ConvertPower(PW_TO_DBM, srg_obss_pd_min_offset) <= ConvertPower(PW_TO_DBM, srg_obss_pd_max_offset) &&
			(ConvertPower(PW_TO_DBM, srg_obss_pd_max_offset) - 82) <= -62 &&
			ConvertPower(PW_TO_DBM, current_srg_obss_pd) >= ConvertPower(PW_TO_DBM, srg_obss_pd_min) &&
			ConvertPower(PW_TO_DBM, current_srg_obss_pd) <= ConvertPower(PW_TO_DBM, srg_obss_pd_max)) {
		//printf("The SRG_OBSS_PD level is appropriate!\n");
		return 1;
	} else {
		//printf("ALERT! The SRG_OBSS_PD level is NOT appropriate!\n");
		return 0;
	}

}

/*
 * CheckNonSrgObssPdConstraints(): checks if the proposed non_srg_obss_pd_level is valid, according to
 * the constraints indicated in the IEEE 802.11ax amendment
 **/
int CheckNonSrgObssPdConstraints(double current_non_srg_obss_pd,
		double non_srg_obss_pd_min, double non_srg_obss_pd_max,
		double non_srg_obss_pd_max_offset, double srg_obss_pd_max_offset) {

	if( non_srg_obss_pd_max_offset <= srg_obss_pd_max_offset &&
			ConvertPower(PW_TO_DBM, non_srg_obss_pd_max_offset) -82 <= -62 &&
			current_non_srg_obss_pd >= non_srg_obss_pd_min &&
			current_non_srg_obss_pd <= non_srg_obss_pd_max ) {
		//printf("The NON_SRG_OBSS_PD level is appropriate!\n");
		return 1;
	} else {
		//printf("ALERT! The NON_SRG_OBSS_PD level is NOT appropriate!\n");
		return 0;
	}

}

/*
 * CheckPowerConstraints():
 **/
int CheckPowerConstraints(double current_obss_pd, double obss_pd_min, double obss_pd_max) {

	if (current_obss_pd <= obss_pd_min) {
		// Unconstrained
		return 1;
	} else {
		// Apply constraint
		return 0;
	}

}