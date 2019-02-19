#include "plc.hpp"
#include "iotables/di_dredges.hpp"

using namespace WarGrey::SCADA;

template<class G>
static void _DI_gantry(G* target, const uint8* db4, unsigned int idx4, const uint8* db205, unsigned int idx205) {
	if (DI_gantry_pushed_out(db4, idx4)) {
		target->set_state(GantryState::PushedOut);
	} else if (DI_gantry_pulled_in(db4, idx4)) {
		target->set_state(GantryState::PulledIn);
	} else if (DBX(db205, idx205 - 1U)) {
		target->set_state(GantryState::PushingOut);
	} else if (DBX(db205, idx205 + 0U)) {
		target->set_state(GantryState::PullingIn);
	} else {
		target->set_state(GantryState::Default);
	}
}

/*************************************************************************************************/
void WarGrey::SCADA::DI_gantry(Gantrylet* target, const uint8* db4, unsigned int idx4_p1, const uint8* db205, unsigned int idx205_p1) {
	_DI_gantry(target, db4, idx4_p1, db205, idx205_p1);
}

void WarGrey::SCADA::DI_gantry(GantrySymbollet* target, const uint8* db4, unsigned int idx4_p1, const uint8* db205, unsigned int idx205_p1) {
	_DI_gantry(target, db4, idx4_p1, db205, idx205_p1);
}

bool WarGrey::SCADA::DI_gantry_pushed_out(const uint8* db4, unsigned int idx4_p1) {
	return DBX(db4, idx4_p1 - 1U);
}

bool WarGrey::SCADA::DI_gantry_pulled_in(const uint8* db4, unsigned int idx4_p1) {
	return DBX(db4, idx4_p1 + 0U);
}

bool WarGrey::SCADA::DI_dredges_scene_control(const uint8* db4, unsigned int idx4_p1) {
	return DBX(db4, idx4_p1 - 1U);
}

bool WarGrey::SCADA::DI_dredges_remote_control(const uint8* db4, unsigned int idx4_p1) {
	return DBX(db4, idx4_p1 + 0U);
}

bool WarGrey::SCADA::DI_dredges_winch_allowed(const uint8* db4, unsigned int idx4_p1) {
	return DBX(db4, idx4_p1 + 1U);
}

bool WarGrey::SCADA::DI_dredges_gantry_allowed(const uint8* db4, unsigned int idx4_p1) {
	return DBX(db4, idx4_p1 + 2U);
}

bool WarGrey::SCADA::DI_dredges_emergence_stop(const uint8* db4, unsigned int idx4_p1) {
	return DBX(db4, idx4_p1 + 6U);
}

void WarGrey::SCADA::DI_suction_buttons(Buttonlet* intarget, Buttonlet* detarget, const uint8* db205, unsigned int idx_p1) {
	if (DI_suction_inflating(db205, idx_p1)) {
		intarget->set_state(ButtonState::Executing);
	} else if (DBX(db205, idx_p1 + 1U)) {
		intarget->set_state(ButtonState::Failed);
	} else if (DBX(db205, idx_p1 + 3U)) {
		intarget->set_state(ButtonState::Ready);
	} else {
		intarget->set_state(ButtonState::Default);
	}

	if (DI_suction_deflating(db205, idx_p1)) {
		detarget->set_state(ButtonState::Executing);
	} else if (DBX(db205, idx_p1 + 2U)) {
		detarget->set_state(ButtonState::Failed);
	} else if (DBX(db205, idx_p1 + 4U)) {
		detarget->set_state(ButtonState::Ready);
	} else {
		detarget->set_state(ButtonState::Default);
	}
}

bool WarGrey::SCADA::DI_suction_inflating(const uint8* db205, unsigned int idx_p1) {
	return DBX(db205, idx_p1 - 1U);
}

bool WarGrey::SCADA::DI_suction_deflating(const uint8* db205, unsigned int idx_p1) {
	return DBX(db205, idx_p1 + 0U);
}

void WarGrey::SCADA::DI_boolean_button(Buttonlet* target, const uint8* db205, unsigned idx_p1) {
	target->set_state(DBX(db205, idx_p1 - 1U), ButtonState::Executing, ButtonState::Default);
}

bool WarGrey::SCADA::DI_long_sb_drag(const uint8* db205) {
	return DBX(db205, 1417U - 1U);
}
