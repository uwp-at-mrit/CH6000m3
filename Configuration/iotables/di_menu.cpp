#include "iotables/di_menu.hpp"

#include "plc.hpp"
#include "menu.hpp"
#include "brushes.hxx"

using namespace WarGrey::SCADA;

using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Controls;

static void DI_condition(MenuFlyout^ menu, unsigned int idx, const uint8* db205, size_t idx205_p1) {
	static Brush^ menu_cmd_excuting_color = ref new SolidColorBrush(Colours::RoyalBlue->Color);
	static Brush^ menu_cmd_failure_color = ref new SolidColorBrush(Colours::Crimson->Color);
	static Brush^ menu_cmd_success_color = ref new SolidColorBrush(Colours::ForestGreen->Color);
	static Brush^ menu_cmd_ready_color = ref new SolidColorBrush(Colours::Cyan->Color);
	static Brush^ menu_cmd_expected_color = ref new SolidColorBrush(Colours::Green->Color);
	static Brush^ menu_cmd_foreground_color = ref new SolidColorBrush(Colours::Foreground->Color);

	if (DBX(db205, idx205_p1 - 1U)) {
		menu_set_foreground_color(menu, idx, menu_cmd_excuting_color);
	} else if (DBX(db205, idx205_p1 + 0U)) {
		menu_set_foreground_color(menu, idx, menu_cmd_failure_color);
	} else if (DBX(db205, idx205_p1 + 1U)) {
		menu_set_foreground_color(menu, idx, menu_cmd_success_color);
	} else if (DBX(db205, idx205_p1 + 2U)) {
		menu_set_foreground_color(menu, idx, menu_cmd_ready_color);
	} else if (DBX(db205, idx205_p1 + 3U)) {
		menu_set_foreground_color(menu, idx, menu_cmd_expected_color);
	} else {
		menu_set_foreground_color(menu, idx, menu_cmd_excuting_color);
	}
}

/*************************************************************************************************/
void WarGrey::SCADA::DI_condition_menu(MenuFlyout^ menu, unsigned int idx, const uint8* db205, size_t idx205_p1) {
}
