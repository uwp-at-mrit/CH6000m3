#pragma once

#include "graphlet/ui/metricslet.hpp"

#include "asn/der.hpp"

#include "mrit.hpp"

namespace WarGrey::DTPM {
	private enum class TP : unsigned int {
		DredgingStart, DredgingEnd,

		_
	};

	private class Timepoint : public WarGrey::GYDM::IASNSequence {
	public:
		Timepoint();
		Timepoint(const uint8* basn, size_t* offse = nullptr);

	public:
		void set(WarGrey::DTPM::TP field, long long value);
		long long ref(WarGrey::DTPM::TP field);

	protected:
		size_t field_payload_span(size_t idx) override;
		size_t fill_field(size_t idx, uint8* octets, size_t offset);
		void extract_field(size_t idx, const uint8* basn, size_t* offset);

	private:
		long long seconds[_N(TP)];
	};

	private class TimeMetrics : public WarGrey::DTPM::IMetricsProvider {
	public:
		TimeMetrics(WarGrey::SCADA::MRMaster* plc);

	public:
		unsigned int capacity() override;

	public:
		Platform::String^ label_ref(unsigned int idx) override;
		WarGrey::DTPM::MetricValue value_ref(unsigned int idx) override;

	public:
		Microsoft::Graphics::Canvas::Brushes::CanvasSolidColorBrush^ label_color_ref(unsigned int idx) override;

	private:
		~TimeMetrics() noexcept {};

	private:
		class Provider;
		WarGrey::DTPM::TimeMetrics::Provider* provider;
	};
}
