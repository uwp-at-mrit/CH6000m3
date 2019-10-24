#pragma once

namespace WarGrey::SCADA {
	// DB203, real data
	static unsigned int pump_C_pressure = 8U;
	static unsigned int pump_F_pressure = 9U;
	static unsigned int pump_D_pressure = 10U;
	static unsigned int pump_E_pressure = 11U;

	static unsigned int pump_A_pressure = 12U;
	static unsigned int pump_B_pressure = 13U;
	static unsigned int pump_G_pressure = 14U;
	static unsigned int pump_H_pressure = 15U;

	static unsigned int pump_I_pressure = 17U;
	static unsigned int pump_J_pressure = 16U;

	// DB204, DB5.DBD
	static unsigned int pump_C_flow = 48U;
	static unsigned int pump_F_flow = 80U;

	static unsigned int pump_A_flow = 32U;
	static unsigned int pump_B_flow = 40U;
	static unsigned int pump_G_flow = 72U;
	static unsigned int pump_H_flow = 64U;
}
