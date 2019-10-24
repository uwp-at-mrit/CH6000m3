#pragma once

#include "timemachine.hpp"
#include "planet.hpp"
#include "plc.hpp"

namespace WarGrey::SCADA {
	private class DraughtsPage : public WarGrey::SCADA::Planet, public WarGrey::SCADA::ITimeMachineListener {
	public:
		virtual ~DraughtsPage() noexcept;
		DraughtsPage(WarGrey::SCADA::PLCMaster* plc = nullptr);

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;

	public:
		void on_startover(long long departure_ms, long long destination_ms) override;
		void on_timestream(long long time_ms, size_t addr0, size_t addrn, uint8* data, size_t size, WarGrey::SCADA::Syslog* logger) override;
		
	public:
		bool can_select(IGraphlet* g) override;
		bool can_select_multiple() override;
		bool on_key(Windows::System::VirtualKey key, bool wargrey_keyboard) override;
		void on_focus(IGraphlet* g, bool yes_no) override;
		void on_tap_selected(IGraphlet* g, float local_x, float local_y) override;
		void on_gesture(std::list<Windows::Foundation::Numerics::float2>& points, float x, float y) override;

	private:
		WarGrey::SCADA::PLCMaster* device;
		WarGrey::SCADA::PLCConfirmation* dashboard;
		Windows::UI::Xaml::Controls::MenuFlyout^ overflow_op;
	};
}
