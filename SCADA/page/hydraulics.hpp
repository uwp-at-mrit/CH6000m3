#pragma once

#include "timemachine.hpp"
#include "planet.hpp"
#include "plc.hpp"

#include "decorator/grid.hpp"

namespace WarGrey::SCADA {
	private class HydraulicsPage : public WarGrey::SCADA::Planet, public WarGrey::SCADA::ITimeMachineListener {
	public:
		virtual ~HydraulicsPage() noexcept;
		HydraulicsPage(WarGrey::SCADA::PLCMaster* plc = nullptr);

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;

	public:
		void on_timestream(long long time_ms, size_t addr0, size_t addrn, uint8* data, size_t size, WarGrey::SCADA::Syslog* logger) override;

	public:
		bool can_select(IGraphlet* g) override;
		bool can_select_multiple() override;
		void on_tap_selected(IGraphlet* g, float local_x, float local_y) override;
		void on_gesture(std::list<Windows::Foundation::Numerics::float2>& anchors, float x, float y);

	private:
		WarGrey::SCADA::PLCMaster* device;
		WarGrey::SCADA::PLCConfirmation* dashboard;
		Windows::UI::Xaml::Controls::MenuFlyout^ gbs_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ gps_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ gsb_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ gvisor_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ pump_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ heater_op;

	private:
		WarGrey::SCADA::GridDecorator* grid;
	};
}
