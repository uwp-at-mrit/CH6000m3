#include "menu.hpp"

#include "iotables/do_water_pumps.hpp"

#include "graphlet/symbol/pump/water_pumplet.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::UI::Xaml::Controls;

private enum class PrivateGroup { FlushCondition, _ };

/*************************************************************************************************/
template<typename OP>
static uint16 DO_water_pump_common_command(OP cmd) {
	uint16 index = 0U;

	switch (cmd) {
	case OP::S2_PS: index = 295U; break;
	case OP::S2_SB: index = 296U; break;
	case OP::S2_2:  index = 297U; break;
	case OP::P2_2:  index = 298U; break;
	case OP::S2_H:  index = 302U; break;
	case OP::P2_H:  index = 301U; break;
	case OP::I2_2:  index = 304U; break;
	}

	return index;
}

static uint16 DO_water_command(WaterPumpConditionAction cmd) {
	uint16 index = 0U;

	switch (cmd) {
	case WaterPumpConditionAction::PS_PS: index = 289U; break;
	case WaterPumpConditionAction::PS_SB: index = 290U; break;
	case WaterPumpConditionAction::PS_2:  index = 293U; break;
	case WaterPumpConditionAction::PS_H:  index = 299U; break;
	case WaterPumpConditionAction::SB_PS: index = 291U; break;
	case WaterPumpConditionAction::SB_SB: index = 292U; break;
	case WaterPumpConditionAction::SB_2:  index = 294U; break;
	case WaterPumpConditionAction::SB_H:  index = 300U; break;
	default: index = DO_water_pump_common_command(cmd);
	}

	return index;
}

/*************************************************************************************************/
private class PSWaterPumpExecutor final : public IMenuCommand<PSWaterPumpAction, WaterPumplet, PLCMaster*> {
public:
	bool can_execute(PSWaterPumpAction cmd, WaterPumplet* pump, PLCMaster* plc, bool acc_executable) override {
		return plc->connected() && plc->authorized();
	}

	void execute(PSWaterPumpAction cmd, WaterPumplet* pump, PLCMaster* plc) override {
		uint16 index = 0U;

		switch (cmd) {
		case PSWaterPumpAction::Prepare: index = 281U; break;
		case PSWaterPumpAction::Start:   index = 282U; break;
		case PSWaterPumpAction::Stop:    index = 283U; break;
		case PSWaterPumpAction::Reset:   index = DO_water_pump_reset_command(true); break;
		case PSWaterPumpAction::PS_PS:   index = DO_water_command(WaterPumpConditionAction::PS_PS); break;
		case PSWaterPumpAction::PS_SB:   index = DO_water_command(WaterPumpConditionAction::PS_SB); break;
		case PSWaterPumpAction::PS_2:    index = DO_water_command(WaterPumpConditionAction::PS_2); break;
		case PSWaterPumpAction::PS_H:    index = DO_water_command(WaterPumpConditionAction::PS_H); break;
		default: index = DO_water_pump_common_command(cmd);
		}

		plc->send_command(index);
	}
};

private class SBWaterPumpExecutor final : public IMenuCommand<SBWaterPumpAction, WaterPumplet, PLCMaster*> {
public:
	bool can_execute(SBWaterPumpAction cmd, WaterPumplet* pump, PLCMaster* plc, bool acc_executable) override {
		return plc->connected() && plc->authorized();
	}

	void execute(SBWaterPumpAction cmd, WaterPumplet* pump, PLCMaster* plc) override {
		uint16 index = 0U;

		switch (cmd) {
		case SBWaterPumpAction::Prepare: index = 285U; break;
		case SBWaterPumpAction::Start:   index = 286U; break;
		case SBWaterPumpAction::Stop:    index = 287U; break;
		case SBWaterPumpAction::Reset:   index = DO_water_pump_reset_command(false); break;
		case SBWaterPumpAction::SB_PS:   index = DO_water_command(WaterPumpConditionAction::SB_PS); break;
		case SBWaterPumpAction::SB_SB:   index = DO_water_command(WaterPumpConditionAction::SB_SB); break;
		case SBWaterPumpAction::SB_2:    index = DO_water_command(WaterPumpConditionAction::SB_2); break;
		case SBWaterPumpAction::SB_H:    index = DO_water_command(WaterPumpConditionAction::SB_H); break;
		default: index = DO_water_pump_common_command(cmd);
		}

		plc->send_command(index);
	}
};

private class WaterPumpConditionExecutor final : public IGroupMenuCommand<WaterPumpConditionAction, PrivateGroup, PLCMaster*> {
public:
	bool can_execute(WaterPumpConditionAction cmd, PrivateGroup _, PLCMaster* plc) override {
		return plc->connected() && plc->authorized();
	}

	void execute(WaterPumpConditionAction cmd, PrivateGroup pump, PLCMaster* plc) override {
		plc->send_command(DO_water_command(cmd));
	}
};

/*************************************************************************************************/
MenuFlyout^ WarGrey::SCADA::make_ps_water_pump_menu(PLCMaster* plc) {
	return make_menu<PSWaterPumpAction, WaterPumplet, PLCMaster*>(new PSWaterPumpExecutor(), plc);
}

MenuFlyout^ WarGrey::SCADA::make_sb_water_pump_menu(PLCMaster* plc) {
	return make_menu<SBWaterPumpAction, WaterPumplet, PLCMaster*>(new SBWaterPumpExecutor(), plc);
}

MenuFlyout^ WarGrey::SCADA::make_water_pump_condition_menu(WaterPumpConditionAction action, PLCMaster* plc) {
	return make_group_menu<WaterPumpConditionAction, PrivateGroup, PLCMaster*>(new WaterPumpConditionExecutor(),
		PrivateGroup::FlushCondition, action, action, plc);
}

unsigned int WarGrey::SCADA::DO_water_pump_reset_command(bool ps) {
	return (ps ? 284U : 288U);
}
