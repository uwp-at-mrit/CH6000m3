#include "plc.hpp"
#include "iotables/di_winches.hpp"

using namespace WarGrey::SCADA;

/*************************************************************************************************/
void WarGrey::SCADA::DI_winch(Winchlet* target
	, const uint8* db4, unsigned int feedback_p1, WinchLimits& limits
	, const uint8* db205, WinchDetails& details) {
	bool slack = DI_winch_slack(db4, &limits);
	bool suction = DI_winch_suction_limited(db4, &limits);
	bool saddle = DI_winch_saddle_limited(db4, &limits);
	bool cable_upper = DI_winch_top_limited(db4, &limits);
	bool soft_upper = DI_winch_soft_top_limited(db205, &details);

	target->set_remote_control(DI_winch_remote_control(db4, feedback_p1));

	if (cable_upper || soft_upper) {
		if (cable_upper && soft_upper) {
			target->set_state(WinchState::TopLimited);
		} else if (cable_upper) {
			target->set_state(WinchState::CableTopLimited);
		} else {
			target->set_state(WinchState::SoftTopLimited);
		}
	} else if (slack || suction || saddle) {
		if (suction) {
			target->set_state(slack, WinchState::SuctionSlack, WinchState::SuctionLimited);
		} else if (saddle) {
			target->set_state(slack, WinchState::SaddleSlack, WinchState::SaddleLimited);
		} else {
			target->set_state(WinchState::Slack);
		}
	} else {
		unsigned int status = details.status - 1U;
		bool can_windout = (DBX(db205, status + 4U));
		bool can_windup = (DBX(db205, status + 5U));
		bool fast = (details.draghead && DBX(db205, status + 7U));

		if (DI_winch_winding_out(db205, details.status)) {
			target->set_state(fast, WinchState::FastWindingOut, WinchState::WindingOut);
		} else if (DI_winch_winding_up(db205, details.status)) {
			target->set_state(fast, WinchState::FastWindingUp, WinchState::WindingUp);
		} else if (DI_winch_soft_bottom_limited(db205, &details)) {
			target->set_state(WinchState::SoftBottomLimited);
		} else if (can_windout && can_windup) {
			target->set_state(fast, WinchState::FastWindReady, WinchState::WindReady);
		} else if (can_windout) {
			target->set_state(fast, WinchState::FastWindOutReady, WinchState::WindOutReady);
		} else if (can_windup) {
			target->set_state(fast, WinchState::FastWindUpReady, WinchState::WindUpReady);
		} else {
			target->set_state(WinchState::Default);
		}

		// the rest are unused;
		//  target->set_state(DBX(db205, status + 2U), WinchState::Unlettable);
		//  target->set_state(DBX(db205, status + 3U), WinchState::Unpullable);
	}
}

void WarGrey::SCADA::DI_winch(Winchlet* shore_discharge_winch, const uint8* db205, unsigned int details_p1) {
	bool fast = DBX(db205, details_p1 + 3U);

	if (DBX(db205, details_p1 - 1U)) {
		shore_discharge_winch->set_state(fast, WinchState::FastWindingOut, WinchState::WindingOut);
	} else if (DBX(db205, details_p1 + 0U)) {
		shore_discharge_winch->set_state(fast, WinchState::FastWindingUp, WinchState::WindingUp);
	} else if (DBX(db205, details_p1 + 1U)) {
		shore_discharge_winch->set_state(WinchState::WindReady);
	} else {
		shore_discharge_winch->set_state(WinchState::Default);
	}
}

void WarGrey::SCADA::DI_winch(Winchlet* anchor_winch, const uint8* db4, unsigned int feedback_p1, const uint8* db205, unsigned int details_p1) {
	bool can_windout = (DBX(db205, details_p1 + 3U));
	bool can_windup = (DBX(db205, details_p1 + 4U));

	anchor_winch->set_remote_control(DI_winch_remote_control(db4, feedback_p1));

	if (DBX(db205, details_p1 - 1U)) {
		anchor_winch->set_state(WinchState::WindingOut);
	} else if (DBX(db205, details_p1 + 0U)) {
		anchor_winch->set_state(WinchState::WindingUp);
	} else if (DBX(db205, details_p1 + 1U)) {
		anchor_winch->set_state(WinchState::Unlettable);
	} else if (DBX(db205, details_p1 + 2U)) {
		anchor_winch->set_state(WinchState::Unpullable);
	} else if (can_windout && can_windup) {
		anchor_winch->set_state(WinchState::WindReady);
	} else if (can_windout) {
		anchor_winch->set_state(WinchState::WindOutReady);
	} else if (can_windup) {
		anchor_winch->set_state(WinchState::WindUpReady);
	} else {
		anchor_winch->set_state(WinchState::Default);
	}
}

void WarGrey::SCADA::DI_winch(Winchlet* barge_winch
	, const uint8* db4, unsigned int feedback_p1, unsigned int limits_p1
	, const uint8* db205, unsigned int details_p1) {
	bool can_windout = (DBX(db205, details_p1 + 3U));
	bool can_windup = (DBX(db205, details_p1 + 4U));
	
	barge_winch->set_remote_control(DI_winch_remote_control(db4, feedback_p1));
	
	if (DBX(db4, limits_p1 - 1U)) {
		barge_winch->set_state(WinchState::CableTopLimited);
	} else if (DBX(db4, limits_p1 + 0U)) {
		barge_winch->set_state(WinchState::CableBottomLimited);
	} else if (DBX(db205, details_p1 - 1U)) {
		barge_winch->set_state(WinchState::WindingOut);
	} else if (DBX(db205, details_p1 + 0U)) {
		barge_winch->set_state(WinchState::WindingUp);
	} else if (DBX(db205, details_p1 + 1U)) {
		barge_winch->set_state(WinchState::Unlettable);
	} else if (DBX(db205, details_p1 + 2U)) {
		barge_winch->set_state(WinchState::Unpullable);
	} else if (can_windout && can_windup) {
		barge_winch->set_state(WinchState::WindReady);
	} else if (can_windout) {
		barge_winch->set_state(WinchState::WindOutReady);
	} else if (can_windup) {
		barge_winch->set_state(WinchState::WindUpReady);
	} else {
		barge_winch->set_state(WinchState::Default);
	}
}

/*************************************************************************************************/
void WarGrey::SCADA::DI_bolt(Boltlet* target, const uint8* db4, unsigned int feedback_p1) {
	target->set_state(DBX(db4, feedback_p1 - 1U), BoltState::SlidedIn, BoltState::SlidedOut);
}

void WarGrey::SCADA::DI_holdhoop(HoldHooplet* target, const uint8* db4, unsigned int feedback_p1) {
	target->set_state(DBX(db4, feedback_p1 - 1U), HoldHoopState::Held, HoldHoopState::Loose);
}

/*************************************************************************************************/
bool WarGrey::SCADA::DI_winch_remote_control(const uint8* db4, unsigned int feedback_p1) {
	return DBX(db4, feedback_p1 + 0U);
}

bool WarGrey::SCADA::DI_winch_locker_open(const uint8* db4, unsigned int feedback_p1) {
	return DBX(db4, feedback_p1 + 1U);
}

bool WarGrey::SCADA::DI_winch_slack(const uint8* db4, WinchLimits* limits) {
	return ((limits->slack > 0U) && DBX(db4, limits->slack - 1U));
}

bool WarGrey::SCADA::DI_winch_top_limited(const uint8* db4, WinchLimits* limits) {
	return DBX(db4, limits->upper - 1U);
}

bool WarGrey::SCADA::DI_winch_saddle_limited(const uint8* db4, WinchLimits* limits) {
	return DBX(db4, limits->saddle - 1U);
}

bool WarGrey::SCADA::DI_winch_suction_limited(const uint8* db4, WinchLimits* limits) {
	return ((limits->suction > 0U) && DBX(db4, limits->suction - 1U));
}

bool WarGrey::SCADA::DI_winch_soft_top_limited(const uint8* db205, WinchDetails* details) {
	return DBX(db205, details->soft_upper - 1U);
}

bool WarGrey::SCADA::DI_winch_soft_bottom_limited(const uint8* db205, WinchDetails* details) {
	return DBX(db205, details->soft_lower - 1U);
}

bool WarGrey::SCADA::DI_winch_constant_tension(const uint8* db205, unsigned int details_p1) {
	return DBX(db205, details_p1 + anchor_winch_details_offset);
}

bool WarGrey::SCADA::DI_winch_locked(const uint8* db205, unsigned int details_p1) {
	return DBX(db205, details_p1 + 7U);
}

bool WarGrey::SCADA::DI_winch_winding_out(const uint8* db205, unsigned int details_p1) {
	return DBX(db205, details_p1 - 1U);
}

bool WarGrey::SCADA::DI_winch_winding_up(const uint8* db205, unsigned int details_p1) {
	return DBX(db205, details_p1 + 0U);
}

bool WarGrey::SCADA::DI_winch_winding(const uint8* db205, unsigned int details_p1) {
	return DI_winch_winding_out(db205, details_p1) || DI_winch_winding_up(db205, details_p1);
}

bool WarGrey::SCADA::DI_bolt_moving(const uint8* db205, unsigned int details_p1) {
	return DBX(db205, details_p1 - 1U) || DBX(db205, details_p1 + 0U);
}

bool WarGrey::SCADA::DI_holdhoop_moving(const uint8* db205, unsigned int details_p1) {
	return DBX(db205, details_p1 - 1U) || DBX(db205, details_p1 + 0U);
}
