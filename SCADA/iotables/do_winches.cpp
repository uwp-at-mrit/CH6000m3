#include "menu.hpp"
#include "credit.hpp"

#include "iotables/do_winches.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::UI::Xaml::Controls;

private class DredgingWinchExecutor final : public IMenuCommand<DredgingWinchAction, Credit<Winchlet, DredgesPosition>, PLCMaster*> {
public:
	DredgingWinchExecutor(dredges_diagnostics_f wd) : show_diagnostics(wd) {}

public:
	bool can_execute(DredgingWinchAction cmd, Credit<Winchlet, DredgesPosition>* winch, PLCMaster* plc, bool acc_executable) override {
		bool okay = true;

		if ((cmd == DredgingWinchAction::HighSpeed)
			&& (!((winch->id == DredgesPosition::psDragHead)
				|| (winch->id == DredgesPosition::sbDragHead)))) {
			okay = false;
		}

		return (DredgingWinchAction::Diagnostics == cmd)
			|| (okay && plc->connected() && plc->authorized());
	}

	void execute(DredgingWinchAction cmd, Credit<Winchlet, DredgesPosition>* winch, PLCMaster* plc) override { // DB300, starts from 1
		if (cmd == DredgingWinchAction::Diagnostics) {
			this->show_diagnostics(winch->id, plc);
		} else {
			uint16 offset = 0U;
			uint16 index = 0U;

			switch (cmd) {
			case DredgingWinchAction::WindUp:        offset = 0U; break;
			case DredgingWinchAction::WindOut:      offset = 1U; break;
			case DredgingWinchAction::Stop:      offset = 2U; break;
			case DredgingWinchAction::HighSpeed: offset = 3U; break;
			}

			switch (winch->id) {
			case DredgesPosition::psTrunnion:     index = 570U; break;
			case DredgesPosition::psIntermediate: index = 573U; break;
			case DredgesPosition::psDragHead:     index = 576U; break;
			case DredgesPosition::sbTrunnion:     index = 589U; break;
			case DredgesPosition::sbIntermediate: index = 592U; break;
			case DredgesPosition::sbDragHead:     index = 595U; break;
			}

			plc->send_command(index + offset);
		}
	}

private:
	dredges_diagnostics_f show_diagnostics;
};

private class AnchorWinchExecutor final : public IMenuCommand<AnchorWinchAction, Credit<Winchlet, ShipSlot>, PLCMaster*> {
public:
	bool can_execute(AnchorWinchAction cmd, Credit<Winchlet, ShipSlot>* winch, PLCMaster* plc, bool acc_executable) override {
		return (plc->connected() && plc->authorized());
	}

	void execute(AnchorWinchAction cmd, Credit<Winchlet, ShipSlot>* winch, PLCMaster* plc) override { // DB300, starts from 1
		uint16 offset = 0U;
		uint16 index = 0U;

		switch (cmd) {
		case AnchorWinchAction::WindUp:   offset = 0U; break;
		case AnchorWinchAction::WindOut:  offset = 1U; break;
		case AnchorWinchAction::Stop:     offset = 2U; break;
		case AnchorWinchAction::CTension: offset = 3U; break;
		}

		switch (winch->id) {
		case ShipSlot::BowWinch:   index = 921U; break;
		case ShipSlot::SternWinch: index = 925U; break;
		}

		if (index > 0) {
			plc->send_command(index + offset);
		}
	}
};

private class BargeWinchExecutor final : public IMenuCommand<BargeWinchAction, Credit<Winchlet, ShipSlot>, PLCMaster*> {
public:
	bool can_execute(BargeWinchAction cmd, Credit<Winchlet, ShipSlot>* winch, PLCMaster* plc, bool acc_executable) override {
		return (plc->connected() && plc->authorized());
	}

	void execute(BargeWinchAction cmd, Credit<Winchlet, ShipSlot>* winch, PLCMaster* plc) override { // DB300, starts from 1
		uint16 offset = 0U;
		uint16 index = 0U;

		switch (cmd) {
		case BargeWinchAction::WindUp:      offset = 0U; break;
		case BargeWinchAction::WindOut:     offset = 1U; break;
		case BargeWinchAction::Stop:        offset = 2U; break;
		}

		switch (winch->id) {
		case ShipSlot::BargeWinch: index = 929U; break;
		}

		if (index > 0) {
			plc->send_command(index + offset);
		}
	}
};

private class BargeCylinderExecutor final : public IMenuCommand<BargeCylinderAction, IGraphlet, PLCMaster*> {
public:
	bool can_execute(BargeCylinderAction cmd, IGraphlet* bolt, PLCMaster* plc, bool acc_executable) override {
		return (plc->connected() && plc->authorized());
	}

	void execute(BargeCylinderAction cmd, IGraphlet* bolt, PLCMaster* plc) override { // DB300, starts from 1
		uint16 index = 0U;
		
		switch (cmd) {
		case BargeCylinderAction::OpenBolt:  index = 932U; break;
		case BargeCylinderAction::CloseBolt: index = 933U; break;
		}

		if (index > 0) {
			plc->send_command(index);
		}
	}
};

private class ShoreWinchExecutor final : public IMenuCommand<ShoreWinchAction, Credit<Winchlet, ShipSlot>, PLCMaster*> {
public:
	bool can_execute(ShoreWinchAction cmd, Credit<Winchlet, ShipSlot>* winch, PLCMaster* plc, bool acc_executable) override {
		return (plc->connected() && plc->authorized());
	}

	void execute(ShoreWinchAction cmd, Credit<Winchlet, ShipSlot>* winch, PLCMaster* plc) override { // DB300, starts from 1
		uint16 offset = 0U;
		uint16 index = 0U;

		switch (cmd) {
		case ShoreWinchAction::WindUp:    offset = 1U; break;
		case ShoreWinchAction::WindOut:   offset = 0U; break;
		case ShoreWinchAction::HighSpeed: offset = 2U; break;
		}

		switch (winch->id) {
		case ShipSlot::ShoreWinch: index = 623U; break;
		}

		if (index > 0) {
			plc->send_command(index + offset);
		}
	}
};


private class ShoreCylinderExecutor final : public IMenuCommand<ShoreCylinderAction, IGraphlet, PLCMaster*> {
public:
	bool can_execute(ShoreCylinderAction cmd, IGraphlet* bolt, PLCMaster* plc, bool acc_executable) override {
		return (plc->connected() && plc->authorized());
	}

	void execute(ShoreCylinderAction cmd, IGraphlet* bolt, PLCMaster* plc) override { // DB300, starts from 1
		uint16 index = 0U;
		
		switch (cmd) {
		case ShoreCylinderAction::Hold:   index = 627U; break;
		case ShoreCylinderAction::Loose:  index = 626U; break;
		case ShoreCylinderAction::Bolt:   index = 629U; break;
		case ShoreCylinderAction::Unbolt: index = 630U; break;
		}

		if (index > 0) {
			plc->send_command(index);
		}
	}
};

/*************************************************************************************************/
MenuFlyout^ WarGrey::SCADA::make_dredging_winch_menu(dredges_diagnostics_f wd, PLCMaster* plc) {
	return make_menu<DredgingWinchAction, Credit<Winchlet, DredgesPosition>, PLCMaster*>(new DredgingWinchExecutor(wd), plc);
}

MenuFlyout^ WarGrey::SCADA::make_anchor_winch_menu(PLCMaster* plc) {
	return make_menu<AnchorWinchAction, Credit<Winchlet, ShipSlot>, PLCMaster*>(new AnchorWinchExecutor(), plc);
}

MenuFlyout^ WarGrey::SCADA::make_barge_winch_menu(PLCMaster* plc) {
	return make_menu<BargeWinchAction, Credit<Winchlet, ShipSlot>, PLCMaster*>(new BargeWinchExecutor(), plc);
}

MenuFlyout^ WarGrey::SCADA::make_barge_cylinder_menu(PLCMaster* plc) {
	return make_menu<BargeCylinderAction, IGraphlet, PLCMaster*>(new BargeCylinderExecutor(), plc);
}

MenuFlyout^ WarGrey::SCADA::make_shore_winch_menu(PLCMaster* plc) {
	return make_menu<ShoreWinchAction, Credit<Winchlet, ShipSlot>, PLCMaster*>(new ShoreWinchExecutor(), plc);
}

MenuFlyout^ WarGrey::SCADA::make_shore_cylinder_menu(PLCMaster* plc) {
	return make_menu<ShoreCylinderAction, IGraphlet, PLCMaster*>(new ShoreCylinderExecutor(), plc);
}

/*************************************************************************************************/
uint16 WarGrey::SCADA::DO_winch_override_command(DredgesPosition id) {
	uint16 index = 0U;

	switch (id) {
	case DredgesPosition::psTrunnion:     index = 617U; break;
	case DredgesPosition::psIntermediate: index = 618U; break;
	case DredgesPosition::psDragHead:     index = 619U; break;
	case DredgesPosition::sbTrunnion:     index = 620U; break;
	case DredgesPosition::sbIntermediate: index = 621U; break;
	case DredgesPosition::sbDragHead:     index = 622U; break;
	}

	return index;
}

uint16 WarGrey::SCADA::DO_winch_upper_check_command(DredgesPosition id) {
	uint16 index = 0U;

	switch (id) {
	case DredgesPosition::psTrunnion:     index = 823U; break;
	case DredgesPosition::psIntermediate: index = 824U; break;
	case DredgesPosition::psDragHead:     index = 825U; break;
	case DredgesPosition::sbTrunnion:     index = 826U; break;
	case DredgesPosition::sbIntermediate: index = 827U; break;
	case DredgesPosition::sbDragHead:     index = 828U; break;
	}

	return index;
}

uint16 WarGrey::SCADA::DO_winch_saddle_check_command(DredgesPosition id) {
	uint16 index = 0U;

	switch (id) {
	case DredgesPosition::psTrunnion:     index = 809U; break;
	case DredgesPosition::psIntermediate: index = 810U; break;
	case DredgesPosition::psDragHead:     index = 811U; break;
	case DredgesPosition::sbTrunnion:     index = 812U; break;
	case DredgesPosition::sbIntermediate: index = 813U; break;
	case DredgesPosition::sbDragHead:     index = 814U; break;
	}

	return index;
}
