#include "iotables/di_menu.hpp"

#include "plc.hpp"
#include "menu.hpp"
#include "system.hpp"
#include "brushes.hxx"

using namespace WarGrey::SCADA;

using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Controls;

/*************************************************************************************************/
static Brush^ menu_cmd_excuting_color = nullptr;
static Brush^ menu_cmd_failure_color = nullptr;
static Brush^ menu_cmd_success_color = nullptr;
static Brush^ menu_cmd_ready_color = nullptr;
static Brush^ menu_cmd_expected_color = nullptr;
static Brush^ menu_cmd_foreground_color = nullptr;

static void menu_style_initialize() {
	if (menu_cmd_excuting_color == nullptr) {
		menu_cmd_excuting_color = ref new SolidColorBrush(Colours::RoyalBlue->Color);
		menu_cmd_failure_color = ref new SolidColorBrush(Colours::Crimson->Color);
		menu_cmd_success_color = ref new SolidColorBrush(Colours::ForestGreen->Color);
		menu_cmd_ready_color = ref new SolidColorBrush(Colours::Azure->Color);
		menu_cmd_expected_color = ref new SolidColorBrush(Colours::Green->Color);
		menu_cmd_foreground_color = ref new SolidColorBrush(Colours::Foreground->Color);
	}
}

/*************************************************************************************************/
void WarGrey::SCADA::DI_condition_menu(MenuFlyout^ menu, unsigned int idx, const uint8* db205, size_t idx205_p1) {
	if (ui_thread_accessed()) {
		menu_style_initialize();

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
			menu_set_foreground_color(menu, idx, menu_cmd_foreground_color);
		}
	} else {
		ui_thread_run_async([=]() { DI_condition_menu(menu, idx, db205, idx205_p1); });
	}
}

void WarGrey::SCADA::DI_binary_menu(MenuFlyout^ menu, unsigned int idx, const uint8* db, size_t idx_p1, size_t idx_offset) {
	if (ui_thread_accessed()) {
		menu_style_initialize();

		if (menu->IsOpen) {
			if (DBX(db, idx_p1 + idx_offset)) {
				menu_set_foreground_color(menu, idx, menu_cmd_success_color);
			} else {
				menu_set_foreground_color(menu, idx, menu_cmd_foreground_color);
			}
		}
	} else {
		ui_thread_run_async([=]() { DI_binary_menu(menu, idx, db, idx_p1, idx_offset); });
	}
}
