#pragma once

#include "timemachine.hpp"
#include "planet.hpp"
#include "mrit.hpp"

namespace WarGrey::SCADA {
	private class DragsFrame : public WarGrey::SCADA::Planet, public WarGrey::SCADA::ITimeMachineListener {
	public:
		virtual ~DragsFrame() noexcept;
		DragsFrame(WarGrey::SCADA::MRMaster* plc = nullptr);

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;

	public:
		void on_timestream(long long time_ms, size_t addr0, size_t addrn, uint8* data, size_t size, WarGrey::SCADA::Syslog* logger) override;

	public:
		bool can_select(WarGrey::SCADA::IGraphlet* g) override;

	private:
		WarGrey::SCADA::MRConfirmation* dashboard;
	};
}
