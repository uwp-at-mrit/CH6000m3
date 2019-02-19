#include "menu.hpp"
#include "credit.hpp"

#include "iotables/do_dredges.hpp"

#include "graphlet/device/draglet.hpp"
#include "graphlet/device/winchlet.hpp"
#include "graphlet/device/gantrylet.hpp"
#include "graphlet/device/compensatorlet.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::UI::Xaml::Controls;

private class GantryExecutor final : public IMenuCommand<GantryAction, Credit<Gantrylet, DredgesPosition>, PLCMaster*> {
public:
	GantryExecutor(dredges_diagnostics_f gd) : show_diagnostics(gd) {}

public:
	bool can_execute(GantryAction cmd, Credit<Gantrylet, DredgesPosition>* gantry, PLCMaster* plc, bool acc_executable) override {
		return (GantryAction::Diagnostics == cmd)
			|| (plc->connected() && plc->authorized());
	}

	void execute(GantryAction cmd, Credit<Gantrylet, DredgesPosition>* gantry, PLCMaster* plc) override { // DB300, starts from 1
		if (cmd == GantryAction::Diagnostics) {
			this->show_diagnostics(gantry->id, plc);
		} else {
			uint16 offset = 0U;
			uint16 index = 0U;

			switch (cmd) {
			case GantryAction::PushOut: offset = 0U; break;
			case GantryAction::PullIn:  offset = 1U; break;
			case GantryAction::Stop:    offset = 2U; break;
			}

			switch (gantry->id) {
			case DredgesPosition::psTrunnion:     index = 561U; break;
			case DredgesPosition::psIntermediate: index = 564U; break;
			case DredgesPosition::psDragHead:     index = 567U; break;
			case DredgesPosition::sbTrunnion:     index = 580U; break;
			case DredgesPosition::sbIntermediate: index = 583U; break;
			case DredgesPosition::sbDragHead:     index = 586U; break;
			}

			plc->send_command(index + offset);
		}
	}

private:
	dredges_diagnostics_f show_diagnostics;
};

private class CompensatorExecutor final : public IMenuCommand<WaveCompensatorAction, Credit<Compensatorlet, bool>, PLCMaster*> {
public:
	bool can_execute(WaveCompensatorAction cmd, Credit<Compensatorlet, bool>* wc, PLCMaster* plc, bool acc_executable) override {
		return plc->connected() && plc->authorized();
	}

	void execute(WaveCompensatorAction cmd, Credit<Compensatorlet, bool>* wc, PLCMaster* plc) override { // DB300, starts from 1
		uint16 offset = 0U;
		uint16 index = (wc->id ? 633U : 641U); // `wc->id` indicates PS or SB

		switch (cmd) {
		case WaveCompensatorAction::Charge:    offset = 0U; break;
		case WaveCompensatorAction::Discharge: offset = 1U; break;
		case WaveCompensatorAction::Stop:      offset = 2U; break;
		//case WaveCompensatorAction::Lock:      offset = 3U; break;
		//case WaveCompensatorAction::Unlock:    offset = 4U; break;
		}

		plc->send_command(index + offset);
	}
};

private class DragVisorExecutor final : public IMenuCommand<DragVisorAction, Credit<DragHeadlet, bool>, PLCMaster*> {
public:
	bool can_execute(DragVisorAction cmd, Credit<DragHeadlet, bool>* wc, PLCMaster* plc, bool acc_executable) override {
		return plc->connected() && plc->authorized();
	}

	void execute(DragVisorAction cmd, Credit<DragHeadlet, bool>* wc, PLCMaster* plc) override { // DB300, starts from 1
		uint16 offset = 0U;
		uint16 index = (wc->id ? 649U : 652U); // `wc->id` indicates PS or SB

		switch (cmd) {
		case DragVisorAction::Up:   offset = 0U; break;
		case DragVisorAction::Down: offset = 1U; break;
		case DragVisorAction::Stop: offset = 2U; break;
		}

		plc->send_command(index + offset);
	}
};

private class GantryGroupExecutor final : public IGroupMenuCommand<GantryAction, DredgesGroup, PLCMaster*> {
public:
	bool can_execute(GantryAction cmd, DredgesGroup group, PLCMaster* plc) override {
		return plc->connected() && plc->authorized();
	}

	void execute(GantryAction cmd, DredgesGroup group, PLCMaster* plc) override { // DB300, starts from 1
		uint16 offset = 0U;
		uint16 index = 0U;

		switch (cmd) {
		case GantryAction::PushOut: offset = 0U; break;
		case GantryAction::PullIn:  offset = 1U; break;
		case GantryAction::Stop:    offset = 2U; break;
		}

		switch (group) {
		case DredgesGroup::PSGantries: index = 601U; break;
		case DredgesGroup::SBGantries: index = 604U; break;
		}

		plc->send_command(index + offset);
	}
};

/*************************************************************************************************/
MenuFlyout^ WarGrey::SCADA::make_gantry_menu(dredges_diagnostics_f gd, PLCMaster* plc) {
	return make_menu<GantryAction, Credit<Gantrylet, DredgesPosition>, PLCMaster*>(new GantryExecutor(gd), plc);
}

MenuFlyout^ WarGrey::SCADA::make_wave_compensator_menu(PLCMaster* plc) {
	return make_menu<WaveCompensatorAction, Credit<Compensatorlet, bool>, PLCMaster*>(new CompensatorExecutor(), plc);
}

MenuFlyout^ WarGrey::SCADA::make_drag_visor_menu(PLCMaster* plc) {
	return make_menu<DragVisorAction, Credit<DragHeadlet, bool>, PLCMaster*>(new DragVisorExecutor(), plc);
}

MenuFlyout^ WarGrey::SCADA::make_gantry_group_menu(DredgesGroup group, PLCMaster* plc) {
	return make_group_menu<GantryAction, DredgesGroup, PLCMaster*>(new GantryGroupExecutor(), group, plc);
}

/*************************************************************************************************/
uint16 WarGrey::SCADA::DO_suction_command(SuctionCommand cmd, bool ps) {
	uint16 index = (ps ? 497U : 501U);
	uint16 offset = 0U;

	switch (cmd) {
	case SuctionCommand::Inflate: offset = 0U; break;
	case SuctionCommand::Deflate: offset = 1U; break;
	}

	return index + offset;
}

uint16 WarGrey::SCADA::DO_LMOD_command(LMODCommand cmd) {
	uint16 index = 0U;

	switch (cmd) {
	case LMODCommand::PSALMO: index = 883U; break;
	case LMODCommand::SBALMO: index = 887U; break;
	}

	return index;
}

uint16 WarGrey::SCADA::DO_gantry_virtual_action_command(DredgesPosition gid, GantryCommand cmd) {
	uint16 index = 0U;

	switch(gid) {
	case DredgesPosition::psTrunnion: index = (cmd == GantryCommand::VirtualUp ? 793U : 794U); break;
	case DredgesPosition::psIntermediate: index = (cmd == GantryCommand::VirtualUp ? 795U : 796U); break;
	case DredgesPosition::psDragHead: index = (cmd == GantryCommand::VirtualUp ? 797U : 798U); break;

	case DredgesPosition::sbTrunnion: index = (cmd == GantryCommand::VirtualUp ? 799U : 800U); break;
	case DredgesPosition::sbIntermediate: index = (cmd == GantryCommand::VirtualUp ? 801U : 802U); break;
	case DredgesPosition::sbDragHead: index = (cmd == GantryCommand::VirtualUp ? 803U : 804U); break;
	}

	return index;
}

uint16 WarGrey::SCADA::DO_use_long_sb_drag_command(bool yes) {
	return (yes ? 599U : 600U);
}
