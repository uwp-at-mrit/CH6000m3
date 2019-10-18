#pragma once

#include "satellite.hpp"
#include "ch6000m3/plc.hpp"

#include "graphlet/ui/textlet.hpp"

#include "graphlet/shapelet.hpp"

namespace WarGrey::SCADA {
	private class HopperPumpDiagnostics : public WarGrey::SCADA::ISatellite {
	public:
		virtual ~HopperPumpDiagnostics() noexcept;
		HopperPumpDiagnostics(WarGrey::SCADA::PLCMaster* plc);

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
