#pragma once

#include "graphlet/textlet.hpp"
#include "graphlet/symbol/pump/hydraulic_pumplet.hpp"

namespace WarGrey::SCADA {
	// DB4, starts from 1
	static const unsigned int pump_A_feedback = 49U;
	static const unsigned int pump_B_feedback = 53U;
	static const unsigned int pump_G_feedback = 69U;
	static const unsigned int pump_H_feedback = 65U;

	static const unsigned int pump_C_feedback = 57U;
	static const unsigned int pump_F_feedback = 73U;
	static const unsigned int pump_D_feedback = 81U;
	static const unsigned int pump_E_feedback = 85U;

	static const unsigned int pump_Y_feedback = 101U;
	static const unsigned int pump_L_feedback = 93U;
	static const unsigned int pump_M_feedback = 97U;
	static const unsigned int pump_K_feedback = 89U;

	static const unsigned int pump_I_feedback = 61U;
	static const unsigned int pump_J_feedback = 77U;

	static const unsigned int console_ps_hydraulics_stop_button = 588U;
	static const unsigned int console_sb_hydraulics_stop_button = 652U;
	static const unsigned int sailing_hydraulics_stop_button = 669U;

	static const unsigned int pump_A_replace_C = 145U;
	static const unsigned int pump_C_replace_A = 146U;
	static const unsigned int pump_B_replace_C = 147U;
	static const unsigned int pump_C_replace_B = 148U;
	static const unsigned int pump_F_replace_C = 149U;
	static const unsigned int pump_C_replace_F = 150U;
	static const unsigned int pump_H_replace_F = 151U;
	static const unsigned int pump_F_replace_H = 152U;
	static const unsigned int pump_G_replace_F = 153U;
	static const unsigned int pump_F_replace_G = 154U;
	static const unsigned int pump_I_replace_J = 155U;
	static const unsigned int pump_J_replace_I = 156U;

	// DB205, starts from 1
	static const unsigned int pump_A_status = 9U;
	static const unsigned int pump_B_status = 17U;
	static const unsigned int pump_G_status = 57U;
	static const unsigned int pump_H_status = 65U;

	static const unsigned int pump_C_status = 25U;
	static const unsigned int pump_F_status = 49U;
	static const unsigned int pump_D_status = 33U;
	static const unsigned int pump_E_status = 41U;

	static const unsigned int pump_Y_status = 73U;
	static const unsigned int pump_L_status = 89U;
	static const unsigned int pump_M_status = 97U;
	static const unsigned int pump_K_status = 81U;

	static const unsigned int pump_I_status = 105U;
	static const unsigned int pump_J_status = 113U;

	/************************************************************************************************/
	void DI_hydraulic_pump_dimension(WarGrey::SCADA::Dimensionlet* target, const uint8* db4, size_t idx_p1);
	void DI_hydraulic_pump(WarGrey::SCADA::HydraulicPumplet* target, const uint8* db4, size_t idx4_p1, const uint8* db205, size_t idx205_p1);

	bool DI_hydraulic_pump_remote_control(const uint8* db4, size_t idx4_p1);
	bool DI_hydraulic_pump_running(const uint8* db4, size_t idx4_p1);
	bool DI_hydraulic_pump_broken(const uint8* db4, size_t idx4_p1);
	bool DI_hydraulic_pump_ready(const uint8* db205, size_t idx205_p1);
}
