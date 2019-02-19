#pragma once

#include "satellite.hpp"
#include "plc.hpp"

#include "graphlet/shapelet.hpp"
#include "graphlet/textlet.hpp"

namespace WarGrey::SCADA {
	private class WaterPumpDiagnostics : public WarGrey::SCADA::ISatellite {
	public:
		virtual ~WaterPumpDiagnostics() noexcept;
		WaterPumpDiagnostics(WarGrey::SCADA::PLCMaster* plc);

	public:
		void fill_extent(float* width, float* height) override;

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;

	public:
		bool can_select(IGraphlet* g) override;
		void on_tap_selected(IGraphlet* g, float local_x, float local_y) override;

	private:
		WarGrey::SCADA::PLCMaster* device;
		WarGrey::SCADA::PLCConfirmation* ps_dashboard;
		WarGrey::SCADA::PLCConfirmation* sb_dashboard;

	private:
		float title_height;
		float vgapsize;

	private: // never delete these graphlets manually
		WarGrey::SCADA::Rectanglet* titlebar;
		WarGrey::SCADA::Labellet* title;
	};
}
