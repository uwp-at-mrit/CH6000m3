#pragma once

#include "plc.hpp"
#include "satellite.hpp"

#include "decorator/decorator.hpp"

#include "graphlet/shapelet.hpp"
#include "graphlet/textlet.hpp"

namespace WarGrey::SCADA {
	private enum class PumpType { Hopper, Underwater, _ }; // `_` for gate flushing pump 

	private class GlandPumpDiagnostics : public WarGrey::SCADA::ICreditSatellite<unsigned int> {
	public:
		virtual ~GlandPumpDiagnostics() noexcept;
		GlandPumpDiagnostics(WarGrey::SCADA::PLCMaster* plc);

	public:
		void fill_extent(float* width, float* height) override;

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;

	public:
		bool can_select(IGraphlet* g) override;

	public:
		void set_pump(Platform::String^ name, WarGrey::SCADA::PumpType type, unsigned int detail_index);

	protected:
		void on_id_changed(unsigned int id);

	private:
		WarGrey::SCADA::PLCMaster* device;
		WarGrey::SCADA::PLCConfirmation* dashboard;
		WarGrey::SCADA::IPlanetDecorator* decorator;

	private:
		float title_height;

	private: // never delete these graphlets manually
		WarGrey::SCADA::Rectanglet* titlebar;
		WarGrey::SCADA::Labellet* title;
	};
}
