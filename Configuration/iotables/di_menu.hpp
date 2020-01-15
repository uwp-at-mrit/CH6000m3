#pragma once

#include "graphlet/symbol/pump/water_pumplet.hpp"
#include "graphlet/ui/buttonlet.hpp"

namespace WarGrey::SCADA {
	// DB205, starts from 1
	static unsigned int ps_ps_details = 577U;
	static unsigned int ps_sb_details = 585U;
	static unsigned int sb_ps_details = 593U;
	static unsigned int sb_sb_details = 601U;
	static unsigned int ps_2_details = 609U;
	static unsigned int sb_2_details = 617U;
	static unsigned int s2_ps_details = 625U;
	static unsigned int s2_sb_details = 633U;
	static unsigned int s2_2_details = 641U;
	static unsigned int p2_2_details = 649U;
	static unsigned int ps_h_details = 657U;
	static unsigned int sb_h_details = 665U;
	static unsigned int p2_h_details = 673U;
	static unsigned int s2_h_details = 681U;
	static unsigned int i2_2_details = 1201U;

	/*********************************************************************************************/
	void DI_condition_menu(Windows::UI::Xaml::Controls::MenuFlyout^ menu, unsigned int idx, const uint8* db205, size_t idx205_p1);
}
