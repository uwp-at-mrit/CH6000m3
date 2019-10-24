#pragma once

#include "planet.hpp"

#include "gps.hpp"
#include "mrit.hpp"

namespace WarGrey::SCADA {
	private class MetricsFrame : public WarGrey::SCADA::Planet {
	public:
		virtual ~MetricsFrame() noexcept;
		MetricsFrame(float width, unsigned int slot_count,
			WarGrey::SCADA::MRMaster* plc = nullptr,
			WarGrey::SCADA::GPS* gps1 = nullptr, WarGrey::SCADA::GPS* gps2 = nullptr,
			WarGrey::SCADA::GPS* gyro = nullptr);

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;

	public:
		bool can_select(IGraphlet* g) override;

	private:
		WarGrey::SCADA::MRMaster* plc;
		WarGrey::SCADA::GPSReceiver* dashboard;
	};
}
