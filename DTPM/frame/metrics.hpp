#pragma once

#include "planet.hpp"

#include "graphlet/ui/metricslet.hpp"

#include "gps.hpp"
#include "mrit.hpp"

namespace WarGrey::DTPM {
	private class MetricsFrame : public WarGrey::SCADA::Planet {
	public:
		virtual ~MetricsFrame() noexcept;
		MetricsFrame(float width, unsigned int slot_count, WarGrey::SCADA::MRMaster* plc = nullptr);

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;

	public:
		bool can_select(WarGrey::SCADA::IGraphlet* g) override;

	private:
		WarGrey::SCADA::MRMaster* plc;
		WarGrey::DTPM::GPSReceiver* dashboard;
	};

	private class DredgeMetricslet : public WarGrey::DTPM::Metricslet {
	public:
		DredgeMetricslet(Platform::String^ name, float width, WarGrey::SCADA::GraphletAnchor anchor, size_t slot_count = 0U);

	public:
		unsigned int capacity() override;
		Platform::String^ label_ref(unsigned int idx) override;
		Microsoft::Graphics::Canvas::Brushes::CanvasSolidColorBrush^ label_color_ref(unsigned int idx) override;
	};
}
