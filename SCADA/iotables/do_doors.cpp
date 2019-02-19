#include "menu.hpp"
#include "credit.hpp"

#include "iotables/do_doors.hpp"

#include "graphlet/symbol/door/hopper_doorlet.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::UI::Xaml::Controls;

private class BottomDoorExecutor final : public IMenuCommand<DoorAction, Credit<HopperDoorlet, Door>, PLCMaster*> {
public:
	bool can_execute(DoorAction cmd, Credit<HopperDoorlet, Door>* door, PLCMaster* plc, bool acc_executable) override {
		return plc->connected() && plc->authorized();
	}

	void execute(DoorAction cmd, Credit<HopperDoorlet, Door>* door, PLCMaster* plc) override { // DB300, starts from 1
		uint16 offset = 0U;
		uint16 index = 0U;

		switch (cmd) {
		case DoorAction::Open:    offset = 0U; break;
		case DoorAction::Stop:    offset = 1U; break;
		case DoorAction::Close:   offset = 2U; break;
		case DoorAction::Disable: offset = 3U; break;
		}

		switch (door->id) {
		case Door::PS1: index = 337U; break;
		case Door::SB1: index = 341U; break;
		case Door::PS2: index = 345U; break;
		case Door::SB2: index = 349U; break;
		case Door::PS3: index = 353U; break;
		case Door::SB3: index = 357U; break;
		case Door::PS4: index = 361U; break;
		case Door::SB4: index = 365U; break;
		case Door::PS5: index = 369U; break;
		case Door::SB5: index = 373U; break;
		case Door::PS6: index = 377U; break;
		case Door::SB6: index = 381U; break;
		case Door::PS7: index = 385U; break;
		case Door::SB7: index = 389U; break;
		}

		plc->send_command(index + offset);
	}
};

private class BottomDoorGroupExecutor final : public IGroupMenuCommand<DoorsGroupAction, BottomDoorsGroup, PLCMaster*> {
public:
	bool can_execute(DoorsGroupAction cmd, BottomDoorsGroup group, PLCMaster* plc) override {
		return plc->connected() && plc->authorized();
	}

	void execute(DoorsGroupAction cmd, BottomDoorsGroup group, PLCMaster* plc) override { // DB300, starts from 1
		uint16 offset = 0U;
		uint16 index = 0U;

		switch (cmd) {
		case DoorsGroupAction::Open:  offset = 0U; break;
		case DoorsGroupAction::Stop:  offset = 1U; break;
		case DoorsGroupAction::Close: offset = 2U; break;
		}

		switch (group) {
		case BottomDoorsGroup::HDoor12: index = 417U; break;
		case BottomDoorsGroup::HDoor35: index = 420U; break;
		case BottomDoorsGroup::HDoor67: index = 423U; break;
		case BottomDoorsGroup::HDoor17: index = 426U; break;
		}

		plc->send_command(index + offset);
	}
};

private class UpperDoorExecutor final : public IMenuCommand<UpperDoorAction, Credit<UpperHopperDoorlet, Door>, PLCMaster*> {
public:
	bool can_execute(UpperDoorAction cmd, Credit<UpperHopperDoorlet, Door>* door, PLCMaster* plc, bool acc_executable) override {
		return plc->connected() && plc->authorized();
	}

	void execute(UpperDoorAction cmd, Credit<UpperHopperDoorlet, Door>* door, PLCMaster* plc) override { // DB300, starts from 1
		uint16 offset = 0U;
		uint16 index = 0U;
		uint16 pair = 0U;
		
		switch (cmd) {
		case UpperDoorAction::Open:    offset = 0U; break;
		case UpperDoorAction::Close:   offset = 1U; break;
		case UpperDoorAction::Stop:    offset = 2U; break;
		case UpperDoorAction::Disable: offset = 3U; break;
		case UpperDoorAction::Pair:    pair = 551U; break;
		}

		switch (door->id) {
		case Door::PS1: index = 433U; break;
		case Door::SB1: index = 437U; break;
		case Door::PS2: index = 441U; break;
		case Door::SB2: index = 445U; break;
		case Door::PS3: index = 449U; break;
		case Door::SB3: index = 453U; break;
		case Door::PS4: index = 457U; break;
		case Door::SB4: index = 461U; break;
		case Door::PS5: index = 465U; break;
		case Door::SB5: index = 469U; break;
		case Door::PS6: index = 473U; break;
		case Door::SB6: index = 477U; break;
		case Door::PS7: index = 481U; break;
		case Door::SB7: index = 485U; break;
		}

		plc->send_command((pair > 0U) ? pair : (index + offset));
	}
};

/*************************************************************************************************/
MenuFlyout^ WarGrey::SCADA::make_bottom_door_menu(PLCMaster* plc) {
	return make_menu<DoorAction, Credit<HopperDoorlet, Door>, PLCMaster*>(new BottomDoorExecutor(), plc);
}

MenuFlyout^ WarGrey::SCADA::make_bottom_doors_group_menu(BottomDoorsGroup group, PLCMaster* plc) {
	return make_group_menu<DoorsGroupAction, BottomDoorsGroup, PLCMaster*>(new BottomDoorGroupExecutor(), group, plc);
}

MenuFlyout^ WarGrey::SCADA::make_upper_door_menu(PLCMaster* plc) {
	return make_menu<UpperDoorAction, Credit<UpperHopperDoorlet, Door>, PLCMaster*>(new UpperDoorExecutor(), plc);
}

uint16 WarGrey::SCADA::DO_bottom_doors_special_command(BottomDoorCommand cmd) { // DB300, starts from 1
	uint16 index = 0U;

	switch (cmd) {
	case BottomDoorCommand::AutoLock:       index = 429U; break;
	case BottomDoorCommand::Locked:         index = 431U; break;
	case BottomDoorCommand::OpenDoorCheck:  index = 777U; break;
	case BottomDoorCommand::CloseDoorCheck: index = 778U; break;
	}

	return index;
}
