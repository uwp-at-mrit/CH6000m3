#pragma once

#include "timemachine.hpp"
#include "planet.hpp"
#include "plc.hpp"

#include "decorator/grid.hpp"

namespace WarGrey::SCADA {
	private class GlandsPage : public WarGrey::SCADA::Planet, public WarGrey::SCADA::ITimeMachineListener {
	public:
		virtual ~GlandsPage() noexcept;
		GlandsPage(WarGrey::SCADA::PLCMaster* plc = nullptr);

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;
		void update(long long count, long long interval, long long uptime) override;

	public:
		void on_timestream(long long time_ms, size_t addr0, size_t addrn, uint8* data, size_t size, WarGrey::SCADA::Syslog* logger) override;

	public:
		bool can_select(IGraphlet* g) override;
		void on_focus(IGraphlet* g, bool yes_no) override;
		bool on_key(Windows::System::VirtualKey key, bool wargrey_keyboard) override;
		void on_tap_selected(IGraphlet* g, float x, float y) override;

	private:
		WarGrey::SCADA::PLCMaster* device;
		WarGrey::SCADA::PLCConfirmation* dashboard;
		Windows::UI::Xaml::Controls::MenuFlyout^ pump_op;

	private:
		WarGrey::SCADA::GridDecorator* grid;
	};
}
