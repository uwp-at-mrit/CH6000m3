#pragma once

#include "graphlet/device/winchlet.hpp"

#include "graphlet/cylinder/boltlet.hpp"
#include "graphlet/cylinder/holdhooplet.hpp"

namespace WarGrey::SCADA {
	private struct WinchLimits {
	public:
		WinchLimits(unsigned int upper, unsigned int saddle, unsigned int slack = 0U, unsigned int suction = 0U)
			: upper(upper), saddle(saddle), suction(suction), slack(slack) {}

	public:
		unsigned int upper;
		unsigned int saddle;
		unsigned int suction;
		unsigned int slack;
	};

	private struct WinchDetails {
	public:
		WinchDetails(bool draghead, unsigned int status, unsigned int soft_upper, unsigned int soft_lower
			, unsigned int override, unsigned int upper_check, unsigned int saddle_check)
			: draghead(draghead), status(status), soft_upper(soft_upper), soft_lower(soft_lower)
			, override(override), upper_check(upper_check), saddle_check(saddle_check) {}

	public:
		unsigned int status;
		unsigned int soft_upper;
		unsigned int soft_lower;
		unsigned int override;
		unsigned int upper_check;
		unsigned int saddle_check;
		bool draghead;
	};

	// DB4, starts from 1
	static WarGrey::SCADA::WinchLimits winch_ps_trunnion_limits = WarGrey::SCADA::WinchLimits(321U, 322U, 323U, 324U);
	static WarGrey::SCADA::WinchLimits winch_ps_intermediate_limits = WarGrey::SCADA::WinchLimits(332U, 354U);
	static WarGrey::SCADA::WinchLimits winch_ps_draghead_limits = WarGrey::SCADA::WinchLimits(361U, 362U);
	static unsigned int winch_ps_trunnion_feedback = 169U;
	static unsigned int winch_ps_intermediate_feedback = 177U;
	static unsigned int winch_ps_draghead_feedback = 185U;

	static WarGrey::SCADA::WinchLimits winch_sb_trunnion_limits = WarGrey::SCADA::WinchLimits(337U, 338U, 339U, 340U);
	static WarGrey::SCADA::WinchLimits winch_sb_intermediate_limits = WarGrey::SCADA::WinchLimits(348U, 386U);
	static WarGrey::SCADA::WinchLimits winch_sb_draghead_limits = WarGrey::SCADA::WinchLimits(393U, 394U);
	static unsigned int winch_sb_trunnion_feedback = 193U;
	static unsigned int winch_sb_intermediate_feedback = 201U;
	static unsigned int winch_sb_draghead_feedback = 209U;

	static unsigned int bow_anchor_winch_feedback = 225U;
	static unsigned int stern_anchor_winch_feedback = 233U;

	static unsigned int shore_discharge_bolt_feedback = 165U;
	static unsigned int shore_discharge_holdhoop_feedback = 167U;

	static unsigned int barge_winch_feedback = 217U;
	static unsigned int barge_winch_limits = 377U;

	// DB205, starts from 1
	static WarGrey::SCADA::WinchDetails winch_ps_trunnion_details = WarGrey::SCADA::WinchDetails(false, 1353U, 1793U, 1817U, 2033U, 2009U, 2025U);
	static WarGrey::SCADA::WinchDetails winch_ps_intermediate_details = WarGrey::SCADA::WinchDetails(false, 1361U, 1795U, 1818U, 2034U, 2010U, 2026U);
	static WarGrey::SCADA::WinchDetails winch_ps_draghead_details = WarGrey::SCADA::WinchDetails(true, 1369U, 1797U, 1819U, 2035U, 2011U, 2027U);
	
	static WarGrey::SCADA::WinchDetails winch_sb_trunnion_details = WarGrey::SCADA::WinchDetails(false, 1377U, 1799U, 1820U, 2036U, 2012U, 2028U);
	static WarGrey::SCADA::WinchDetails winch_sb_intermediate_details = WarGrey::SCADA::WinchDetails(false, 1385U, 1801U, 1821U, 2037U, 2013U, 2029U);
	static WarGrey::SCADA::WinchDetails winch_sb_draghead_details = WarGrey::SCADA::WinchDetails(true, 1393U, 1803U, 1822U, 2038U, 2014U, 2030U);

	static unsigned int barge_winch_override = 2039U;
	static unsigned int barge_winch_details = 2801U;

	static unsigned int shore_discharge_winch_details = 1497U;

	static unsigned int bow_anchor_winch_details = 2785U;
	static unsigned int stern_anchor_winch_details = 2793U;
	
	/************************************************************************************************/
	void DI_winch(WarGrey::SCADA::Winchlet* target,
		const uint8* db4, unsigned int feedback_p1, WarGrey::SCADA::WinchLimits& limits,
		const uint8* db205, WarGrey::SCADA::WinchDetails& details);

	void DI_winch(WarGrey::SCADA::Winchlet* shore_discharge_winch, const uint8* db205, unsigned int details_p1);
	void DI_winch(WarGrey::SCADA::Winchlet* anchor_winch, const uint8* db4, unsigned int feedback_p1, const uint8* db205, unsigned int details_p1);
	void DI_winch(WarGrey::SCADA::Winchlet* barge_winch,
		const uint8* db4, unsigned int feedback_p1, unsigned int limits_p1,
		const uint8* db205, unsigned int details_p1);

	void DI_bolt(WarGrey::SCADA::Boltlet* target, const uint8* db4, unsigned int feedback_p1);
	void DI_holdhoop(WarGrey::SCADA::HoldHooplet* target, const uint8* db4, unsigned int feedback_p1);

	bool DI_winch_remote_control(const uint8* db4, unsigned int feedback_p1);
	bool DI_winch_locker_open(const uint8* db4, unsigned int feedback_p1);
	bool DI_winch_constant_tension(const uint8* db205, unsigned int details_p1);
	bool DI_winch_locked(const uint8* db205, unsigned int details_p1);

	bool DI_winch_slack(const uint8* db4, WarGrey::SCADA::WinchLimits* limits);
	bool DI_winch_upper_limited(const uint8* db4, WarGrey::SCADA::WinchLimits* limits);
	bool DI_winch_saddle_limited(const uint8* db4, WarGrey::SCADA::WinchLimits* limits);
	bool DI_winch_suction_limited(const uint8* db4, WarGrey::SCADA::WinchLimits* limits);
	bool DI_winch_soft_upper_limited(const uint8* db205, WarGrey::SCADA::WinchDetails* details);
	bool DI_winch_soft_lower_limited(const uint8* db205, WarGrey::SCADA::WinchDetails* details);
}
