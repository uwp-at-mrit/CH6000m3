#pragma once

#include "universe.hxx"
#include "mrit.hpp"

namespace WarGrey::DTPM {
	private ref class UniverseWidget : public WarGrey::SCADA::UniverseDisplay {
	internal:
		UniverseWidget(Windows::UI::Xaml::Controls::SplitView^ frame, WarGrey::SCADA::UniverseDisplay^ master,
			WarGrey::SCADA::MRMaster* plc);

	protected:
		void construct(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason) override;

	private:
		Windows::UI::Xaml::Controls::SplitView^ frame;
		WarGrey::SCADA::UniverseDisplay^ master;

	private:
		WarGrey::SCADA::MRMaster* plc;
	};
}
