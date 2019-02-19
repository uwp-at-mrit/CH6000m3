#pragma once

#include "timemachine.hpp"
#include "planet.hpp"
#include "plc.hpp"

namespace WarGrey::SCADA {
	private class HopperDoorsPage : public WarGrey::SCADA::Planet, public WarGrey::SCADA::ITimeMachineListener {
	public:
		virtual ~HopperDoorsPage() noexcept;
		HopperDoorsPage(WarGrey::SCADA::PLCMaster* plc = nullptr);

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;

	public:
		void on_timestream(long long time_ms, size_t addr0, size_t addrn, uint8* data, size_t size, WarGrey::SCADA::Syslog* logger) override;

	public:
		bool can_select(IGraphlet* g) override;
		bool can_select_multiple() override;
		void on_tap_selected(IGraphlet* g, float x, float y) override;
		void on_gesture(std::list<Windows::Foundation::Numerics::float2>& anchors, float x, float y) override;

	private:
		WarGrey::SCADA::PLCMaster* device;
		WarGrey::SCADA::PLCConfirmation* dashboard;
		Windows::UI::Xaml::Controls::MenuFlyout^ door_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ gdoors12_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ gdoors35_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ gdoors67_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ gdoors17_op;
	};
}
