#include "menu.hpp"

#include "iotables/do_devices.hpp"

#include "graphlet/primitive.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::UI::Xaml::Controls;

private class OverflowExecutor final : public IMenuCommand<OverflowAction, IGraphlet, PLCMaster*> {
public:
	bool can_execute(OverflowAction cmd, IGraphlet* overflow, PLCMaster* plc, bool acc_executable) override {
		return plc->connected() && plc->authorized();
	}

	void execute(OverflowAction cmd, IGraphlet* overflow, PLCMaster* plc) override { // DB300, starts from 1
		uint16 index = 0U;

		switch (cmd) {
		case OverflowAction::Up:     index = 857U; break;
		case OverflowAction::Down:   index = 858U; break;
		case OverflowAction::Stop:   index = 859U; break;
		case OverflowAction::MoveTo: index = overflow_pipe_move_to_target_height; break;
		case OverflowAction::Auto:   index = 868U; break;
		}

		plc->send_command(index);
	}
};

private class TankHeaterExecutor final : public IMenuCommand<TankHeaterAction, IGraphlet, PLCMaster*> {
public:
	bool can_execute(TankHeaterAction cmd, IGraphlet* heater, PLCMaster* plc, bool acc_executable) override {
		return plc->connected() && plc->authorized();
	}

	void execute(TankHeaterAction cmd, IGraphlet* heater, PLCMaster* plc) override { // DB300, starts from 1
		uint16 index = 0U;

		switch (cmd) {
		case TankHeaterAction::Start:  index = 665U; break;
		case TankHeaterAction::Stop:   index = 666U; break;
		case TankHeaterAction::Reset:  index = 667U; break;
		case TankHeaterAction::Auto:   index = 668U; break;
		}

		plc->send_command(index);
	}
};

/*************************************************************************************************/
MenuFlyout^ WarGrey::SCADA::make_overflow_menu(PLCMaster* plc) {
	return make_menu<OverflowAction, IGraphlet, PLCMaster*>(new OverflowExecutor(), plc);
}

MenuFlyout^ WarGrey::SCADA::make_tank_heater_menu(PLCMaster* plc) {
	return make_menu<TankHeaterAction, IGraphlet, PLCMaster*>(new TankHeaterExecutor(), plc);
}
