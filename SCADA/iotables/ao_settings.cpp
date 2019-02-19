#include "iotables/ao_settings.hpp"

using namespace WarGrey::SCADA;

unsigned int WarGrey::SCADA::AO_gantry_winch_trunnion_settings(GantryWinchTrunnionSettings setting, bool ps) { // DB20, DBD
	unsigned int idx = 0U;

	switch(setting) {
	case GantryWinchTrunnionSettings::GantryFlow:     idx = (ps ? 562U : 578U); break;
	case GantryWinchTrunnionSettings::PushFlow:       idx = (ps ? 566U : 582U); break;
	case GantryWinchTrunnionSettings::PullFlow:       idx = (ps ? 570U : 586U); break;
	case GantryWinchTrunnionSettings::GantryPressure: idx = (ps ? 574U : 590U); break;

	case GantryWinchTrunnionSettings::WinchFlow:      idx = (ps ? 506U : 510U); break;
	case GantryWinchTrunnionSettings::PushUpFlow:     idx = (ps ? 514U : 534U); break;
	case GantryWinchTrunnionSettings::PushOutFlow:    idx = (ps ? 518U : 538U); break;
	case GantryWinchTrunnionSettings::PullUpFlow:     idx = (ps ? 522U : 542U); break;
	case GantryWinchTrunnionSettings::PullOutFlow:    idx = (ps ? 526U : 546U); break;

	case GantryWinchTrunnionSettings::WinchPressure:  idx = (ps ? 554U : 558U); break;
	case GantryWinchTrunnionSettings::RemoteFlow:     idx = (ps ? 770U : 782U); break;
	case GantryWinchTrunnionSettings::RemotePressure: idx = (ps ? 774U : 786U); break;
	}

	return idx;
}

unsigned int WarGrey::SCADA::AO_gantry_winch_intermediate_settings(GantryWinchIntermediateSettings setting, bool ps) {
	unsigned int idx = 0U;

	switch (setting) {
	case GantryWinchIntermediateSettings::GantryFlow:     idx = (ps ? 474U : 490U); break;
	case GantryWinchIntermediateSettings::PushFlow:       idx = (ps ? 478U : 494U); break;
	case GantryWinchIntermediateSettings::PullFlow:       idx = (ps ? 482U : 498U); break;
	case GantryWinchIntermediateSettings::GantryPressure: idx = (ps ? 486U : 502U); break;

	case GantryWinchIntermediateSettings::WinchFlow:      idx = (ps ? 394U : 398U); break;
	case GantryWinchIntermediateSettings::PushUpFlow:     idx = (ps ? 402U : 434U); break;
	case GantryWinchIntermediateSettings::PushOutFlow:    idx = (ps ? 406U : 438U); break;
	case GantryWinchIntermediateSettings::PullUpFlow:     idx = (ps ? 410U : 442U); break;
	case GantryWinchIntermediateSettings::PullOutFlow:    idx = (ps ? 414U : 446U); break;

	case GantryWinchIntermediateSettings::DragOutFlow:    idx = (ps ? 418U : 450U); break;
	case GantryWinchIntermediateSettings::DragUpFlow:     idx = (ps ? 422U : 454U); break;
	case GantryWinchIntermediateSettings::SPWCUpFlow:     idx = (ps ? 426U : 458U); break;
	case GantryWinchIntermediateSettings::SPWCOutFlow:    idx = (ps ? 430U : 462U); break;

	case GantryWinchIntermediateSettings::WinchPressure:  idx = (ps ? 466U : 470U); break;
	}

	return idx;
}

unsigned int WarGrey::SCADA::AO_gantry_winch_draghead_settings(GantryWinchDragHeadSettings setting, bool ps) {
	unsigned int idx = 0U;

	switch (setting) {
	case GantryWinchDragHeadSettings::GantryFlow:      idx = (ps ? 362U : 378U); break;
	case GantryWinchDragHeadSettings::PushFlow:        idx = (ps ? 366U : 382U); break;
	case GantryWinchDragHeadSettings::PullFlow:        idx = (ps ? 370U : 386U); break;
	case GantryWinchDragHeadSettings::GantryPressure:  idx = (ps ? 374U : 390U); break;

	case GantryWinchDragHeadSettings::WinchFlow:       idx = (ps ? 266U : 270U); break;
	case GantryWinchDragHeadSettings::PushUpFlow:      idx = (ps ? 274U : 314U); break;
	case GantryWinchDragHeadSettings::PushOutFlow:     idx = (ps ? 278U : 318U); break;
	case GantryWinchDragHeadSettings::PullUpFlow:      idx = (ps ? 282U : 322U); break;
	case GantryWinchDragHeadSettings::PullOutFlow:     idx = (ps ? 286U : 326U); break;

	case GantryWinchDragHeadSettings::DragOutFlow:     idx = (ps ? 290U : 330U); break;
	case GantryWinchDragHeadSettings::DragUpFlow:      idx = (ps ? 294U : 334U); break;
	case GantryWinchDragHeadSettings::SPWCFastUpFlow:  idx = (ps ? 298U : 338U); break;
	case GantryWinchDragHeadSettings::SPWCFastOutFlow: idx = (ps ? 302U : 342U); break;
	case GantryWinchDragHeadSettings::SPWCSlowUpFlow:  idx = (ps ? 306U : 346U); break;
	case GantryWinchDragHeadSettings::SPWCSlowOutFlow: idx = (ps ? 310U : 350U); break;

	case GantryWinchDragHeadSettings::WinchPressure:   idx = (ps ? 354U : 358U); break;
	}

	return idx;
}

unsigned int WarGrey::SCADA::AO_drag_pipes_settings(DragPipesSettings setting, bool ps) { // DB20, DBD
	unsigned int idx = 0U;

	switch (setting) {
	case DragPipesSettings::ForearmDegrees:  idx = (ps ? 610U : 626U); break;
	case DragPipesSettings::BackarmDegrees:  idx = (ps ? 614U : 630U); break;
	case DragPipesSettings::ForeBackDegrees: idx = (ps ? 618U : 634U); break;
	case DragPipesSettings::CableLength:     idx = (ps ? 622U : 638U); break;
	}

	return idx;
}

unsigned int WarGrey::SCADA::AO_doors_settings(DoorsSettings setting, bool ps) { // DB20, DBD
	unsigned int idx = 0U;

	switch (setting) {
	case DoorsSettings::UpperFlow:     idx = (ps ? 706U : 710U); break;
	case DoorsSettings::UpperPressure: idx = (ps ? 714U : 718U); break;
	case DoorsSettings::BottomFlow:    idx = (ps ? 690U : 694U); break;
	case DoorsSettings::OpenPressure:  idx = (ps ? 674U : 682U); break;
	case DoorsSettings::ClosePressure: idx = (ps ? 678U : 686U); break;
	}

	return idx;
}

unsigned int WarGrey::SCADA::AO_shore_discharge_settings(ShoreDischargeSettings setting, bool placeholder_for_useless_args) { // DB20, DBD
	unsigned int idx = 0U;

	switch (setting) {
	case ShoreDischargeSettings::NormalFlow:       idx = 738U; break;
	case ShoreDischargeSettings::FastFlow:         idx = 742U; break;
	case ShoreDischargeSettings::WinchPressure:    idx = 746U; break;
	case ShoreDischargeSettings::HoldhoopFlow:     idx = 754U; break;
	case ShoreDischargeSettings::HoldhoopPressure: idx = 758U; break;
	case ShoreDischargeSettings::BoltFlow:         idx = 762U; break;
	case ShoreDischargeSettings::BoltPressure:     idx = 766U; break;
	}

	return idx;
}

unsigned int WarGrey::SCADA::AO_other_settings(OtherSettings setting, bool placeholder_for_useless_args) { // DB20, DBD
	unsigned int idx = 0U;

	switch (setting) {
	case OtherSettings::BargeFlow:        idx = 242U; break;
	case OtherSettings::BargePressure:    idx = 246U; break;
	case OtherSettings::OverflowFlow:     idx = 722U; break;
	case OtherSettings::OverflowPressure: idx = 726U; break;
	case OtherSettings::ChargeDensity:    idx = 150U; break;
	case OtherSettings::DischargeDensity: idx = 154U; break;
	}

	return idx;
}
