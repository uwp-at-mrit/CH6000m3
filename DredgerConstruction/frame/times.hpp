#pragma once

#include "planet.hpp"

#include "gps.hpp"
#include "mrit.hpp"

namespace WarGrey::SCADA {
	private class TimesFrame : public WarGrey::SCADA::Planet {
	public:
		virtual ~TimesFrame() noexcept;
		TimesFrame(float width, WarGrey::SCADA::MRMaster* plc = nullptr);

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;

	public:
		bool can_select(IGraphlet* g) override;

	private:
		WarGrey::SCADA::MRMaster* plc;
		WarGrey::SCADA::MRConfirmation* dashboard;
	};
}
