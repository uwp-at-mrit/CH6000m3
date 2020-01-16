#pragma once

namespace WarGrey::SCADA {
	// DB205, starts from 1
	static unsigned int ps_ps_flush_details = 577U;
	static unsigned int ps_sb_flush_details = 585U;
	static unsigned int sb_ps_flush_details = 593U;
	static unsigned int sb_sb_flush_details = 601U;
	static unsigned int ps_2_flush_details = 609U;
	static unsigned int sb_2_flush_details = 617U;
	static unsigned int s2_ps_flush_details = 625U;
	static unsigned int s2_sb_flush_details = 633U;
	static unsigned int s2_2_flush_details = 641U;
	static unsigned int p2_2_flush_details = 649U;
	static unsigned int ps_h_flush_details = 657U;
	static unsigned int sb_h_flush_details = 665U;
	static unsigned int p2_h_flush_details = 673U;
	static unsigned int s2_h_flush_details = 681U;
	static unsigned int i2_2_flush_details = 1201U;

	static unsigned int ps_underwater_dredge_details = 697U;
	static unsigned int sb_underwater_dredge_details = 705U;
	static unsigned int ps_sb_underwater_dredge_details = 713U;
	static unsigned int ps_hopper_dredge_details = 721U;
	static unsigned int sb_hopper_dredge_details = 729U;
	static unsigned int ps_sb_hopper_dredge_details = 737U;
	static unsigned int sb_underwater_dredge_barge_details = 745U;
	static unsigned int sb_hopper_dredge_barge_details = 753U;

	static unsigned int ps_shore_discharge_details = 761U;
	static unsigned int sb_shore_discharge_details = 769U;
	static unsigned int ps_sb_shore_discharge_details = 777U;
	static unsigned int ps_rainbowing_details = 785U;
	static unsigned int sb_rainbowing_details = 793U;
	static unsigned int ps_sb_rainbowing_details = 801U;

	/*********************************************************************************************/
	void DI_condition_menu(Windows::UI::Xaml::Controls::MenuFlyout^ menu, unsigned int idx, const uint8* db205, size_t idx205_p1);
	void DI_binary_menu(Windows::UI::Xaml::Controls::MenuFlyout^ menu, unsigned int idx, const uint8* db, size_t idx_p1, size_t idx_offset);
	
	template<typename E>
	void DI_condition_menu(Windows::UI::Xaml::Controls::MenuFlyout^ menu, E e, const uint8* db205, size_t idx205_p1) {
		DI_condition_menu(menu, _I(e), db205, idx205_p1);
	}

	template<typename E>
	void DI_binary_menu(Windows::UI::Xaml::Controls::MenuFlyout^ menu, E e, const uint8* db, size_t idx_p1, size_t idx_offset) {
		DI_binary_menu(menu, _I(e), db, idx_p1, idx_offset);
	}

}
