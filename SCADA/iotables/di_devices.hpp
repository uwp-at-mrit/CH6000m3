#pragma once

#include "graphlet/dashboard/alarmlet.hpp"
#include "graphlet/symbol/heaterlet.hpp"
#include "graphlet/device/tanklet.hpp"
#include "graphlet/buttonlet.hpp"

namespace WarGrey::SCADA {
	// DB4, starts from 1
	static unsigned int filter_01_status = 121U;
	static unsigned int filter_02_status = 127U;
	static unsigned int filter_10_status = 134U;

	static unsigned int master_tank_heater_feedback = 473U;
	static unsigned int master_tank_status = 118U;
	static unsigned int visor_tank_status = 130U;

	// DB205, starts from 1
	static unsigned int tank_heater_status = 361U;
	static unsigned int backoil_pressure_override_status = 1809U;
	static unsigned int backoil_pressure_too_low = 1810U;

	/************************************************************************************************/
	void DI_alarm(WarGrey::SCADA::Alarmlet* target, const uint8* db4, unsigned int idx_p1, WarGrey::SCADA::AlarmState hlstate = AlarmState::Alert);
	void DI_backoil_pressure_override(WarGrey::SCADA::Buttonlet* target, const uint8* db205, unsigned int idx_p1);
	void DI_tank_heater(WarGrey::SCADA::Heaterlet* target,
		const uint8* db4, unsigned int idx4_p1,
		const uint8* db205, unsigned int idx205_p1);

	void DI_visor_tank(WarGrey::SCADA::Tanklet* target, const uint8* db4, unsigned int idx_p1);

	bool DI_tank_level_low(const uint8* db4, unsigned int idx_p1);
	bool DI_tank_level_too_low(const uint8* db4, unsigned int idx_p1);

	template<typename E>
	void DI_master_tank(WarGrey::SCADA::StateTanklet<E>* target, const uint8* db4, unsigned int idx_p1) {
		if (DI_tank_level_low(db4, idx_p1)) {
			target->set_state(E::Low);
		} else if (DI_tank_level_too_low(db4, idx_p1)) {
			target->set_state(E::UltraLow);
		} else if (DBX(db4, idx_p1 + 1U)) {
			target->set_state(E::High);
		} else {
			target->set_state(E::Normal);
		}
	}
}
