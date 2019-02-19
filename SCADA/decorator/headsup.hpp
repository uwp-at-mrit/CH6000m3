#pragma once

#include "planet.hpp"
#include "plc.hpp"

#include "graphlet/statuslet.hpp"

namespace WarGrey::SCADA {
	private class HeadsUpPlanet : public WarGrey::SCADA::IHeadUpPlanet {
	public:
		HeadsUpPlanet(WarGrey::SCADA::PLCMaster* plc = nullptr);

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void fill_margin(float* top = nullptr, float* right = nullptr, float* bottom = nullptr, float* left = nullptr) override;
		void reflow(float width, float height) override;

	public:
		void on_transfer(WarGrey::SCADA::IPlanet* from, WarGrey::SCADA::IPlanet* to) override;
		void on_tap_selected(WarGrey::SCADA::IGraphlet* g, float local_x, float local_y) override;

	public:
		bool can_select(WarGrey::SCADA::IGraphlet* g) override;
		bool can_select_multiple() override;

	private:
		WarGrey::SCADA::PLCMaster* device;

	private: // never deletes these graphlets mannually
		WarGrey::SCADA::Statusbarlet* statusbar;
		WarGrey::SCADA::Statuslinelet* statusline;
	};
}
