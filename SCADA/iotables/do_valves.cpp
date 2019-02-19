#include "menu.hpp"

#include "iotables/do_valves.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::UI::Xaml::Controls;

template<typename Action, typename action_f>
private class ValveExecutor final : public IMenuCommand<Action, GateValvelet, PLCMaster*> {
public:
	ValveExecutor(action_f gva) : DO_action(gva) {}

public:
	bool can_execute(Action cmd, GateValvelet* valve, PLCMaster* plc, bool acc_executable) override {
		return plc->connected() && plc->authorized();
	}

	void execute(Action cmd, GateValvelet* valve, PLCMaster* plc) override {
		plc->send_command(this->DO_action(cmd, valve));
	}

private:
	action_f DO_action;
};

/*************************************************************************************************/
MenuFlyout^ WarGrey::SCADA::make_gate_valve_menu(gate_valve_action_f gva, PLCMaster* plc) {
	auto exe = new ValveExecutor<GateValveAction, gate_valve_action_f>(gva);

	return make_menu<GateValveAction, GateValvelet, PLCMaster*>(exe, plc);
}

MenuFlyout^ WarGrey::SCADA::make_butterfly_valve_menu(butterfly_valve_action_f bfva, PLCMaster* plc) {
	auto exe = new ValveExecutor<ButterflyValveAction, butterfly_valve_action_f>(bfva);

	return make_menu<ButterflyValveAction, GateValvelet, PLCMaster*>(exe, plc);
}
