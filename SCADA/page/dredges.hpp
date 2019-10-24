#pragma once

#include "timemachine.hpp"
#include "planet.hpp"
#include "plc.hpp"

namespace WarGrey::SCADA {
	private enum class DragView { PortSide, Starboard, Suctions, _ };

	private class DredgesPage : public WarGrey::SCADA::Planet, public WarGrey::SCADA::ITimeMachineListener {
	public:
		virtual ~DredgesPage() noexcept;
		DredgesPage(WarGrey::SCADA::DragView type, WarGrey::SCADA::PLCMaster* plc = nullptr);

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;

	public:
		void on_timestream(long long time_ms, size_t addr0, size_t addrn, uint8* data, size_t size, WarGrey::SCADA::Syslog* logger) override;

	public:
		bool can_select(IGraphlet* g) override;
		bool can_select_multiple() override;
		void on_focus(IGraphlet* g, bool yes) override;
		bool on_key(Windows::System::VirtualKey key, bool wargrey_keyboard) override;
		void on_tap_selected(IGraphlet* g, float x, float y) override;
		void on_gesture(std::list<Windows::Foundation::Numerics::float2>& anchors, float x, float y);

	public:
		WarGrey::SCADA::PLCMaster* get_plc_device();

	private:
		WarGrey::SCADA::PLCMaster* device;
		WarGrey::SCADA::PLCConfirmation* dashboard;
	};
}
