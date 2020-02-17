#pragma once

#include "universe.hxx"
#include "plc.hpp"

namespace WarGrey::SCADA {
	float widget_evaluate_height();

	private ref class UniverseWidget : public WarGrey::SCADA::UniverseDisplay {
	internal:
		UniverseWidget(Windows::UI::Xaml::Controls::SplitView^ frame,
			WarGrey::SCADA::UniverseDisplay^ master, WarGrey::SCADA::PLCMaster* plc);

	protected:
		void construct(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason) override;

	private:
		Windows::UI::Xaml::Controls::SplitView^ frame;
		WarGrey::SCADA::UniverseDisplay^ master;
		WarGrey::SCADA::PLCMaster* plc;
	};
}
