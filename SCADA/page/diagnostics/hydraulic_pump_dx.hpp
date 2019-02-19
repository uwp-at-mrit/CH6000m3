#pragma once

#include "satellite.hpp"
#include "plc.hpp"

#include "iotables/do_dredges.hpp"

#include "graphlet/shapelet.hpp"
#include "graphlet/textlet.hpp"

namespace WarGrey::SCADA {
	private enum class HPDX { PS, SB, Visor, Other, _ };

	private class HydraulicPumpDiagnostics : public WarGrey::SCADA::ICreditSatellite<unsigned int> {
	public:
		virtual ~HydraulicPumpDiagnostics() noexcept;
		HydraulicPumpDiagnostics(WarGrey::SCADA::PLCMaster* plc);

	public:
		void fill_extent(float* width, float* height) override;

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;

	public:
		void set_pump(Platform::String^ id, HPDX group, unsigned int details);

	protected:
		void on_id_changed(unsigned int id) override;

	private:
		WarGrey::SCADA::PLCMaster* device;
		WarGrey::SCADA::PLCConfirmation* dashboard;

	private:
		float title_height;
		float vgapsize;

	private: // never delete these graphlets manually
		WarGrey::SCADA::Rectanglet* titlebar;
		WarGrey::SCADA::Labellet* title;
	};
}
