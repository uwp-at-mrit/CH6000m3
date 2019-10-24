#pragma once

#include "satellite.hpp"
#include "plc.hpp"

#include "graphlet/ui/textlet.hpp"

#include "graphlet/shapelet.hpp"

namespace WarGrey::SCADA {
	private class UnderwaterPumpMotorMetrics : public WarGrey::SCADA::ISatellite {
	public:
		virtual ~UnderwaterPumpMotorMetrics() noexcept;
		UnderwaterPumpMotorMetrics(WarGrey::SCADA::PLCMaster* plc);

	public:
		void fill_extent(float* width, float* height) override;

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;

	public:
		bool can_select(IGraphlet* g) override;

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
