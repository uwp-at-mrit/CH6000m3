#pragma once

#include <list>
#include <optional>

#include "dbsystem.hpp"

namespace WarGrey::SCADA {
    typedef Integer EarthWork_pk;

    private struct EarthWork {
        Integer uuid;
        Float product;
        Float vessel;
        Float hopper_height;
        Float loading;
        Float displacement;
        Integer timestamp;
    };

	private class IEarthWorkCursor abstract {
    public:
        virtual bool step(WarGrey::SCADA::EarthWork& occurrence, bool asc, int code) = 0;
    };

    private enum class earthwork { uuid, product, vessel, hopper_height, loading, displacement, timestamp, _ };

    WarGrey::SCADA::EarthWork_pk earthwork_identity(WarGrey::SCADA::EarthWork& self);

    WarGrey::SCADA::EarthWork make_earthwork(std::optional<Float> product = std::nullopt, std::optional<Float> vessel = std::nullopt, std::optional<Float> hopper_height = std::nullopt, std::optional<Float> loading = std::nullopt, std::optional<Float> displacement = std::nullopt, std::optional<Integer> timestamp = std::nullopt);
    void default_earthwork(WarGrey::SCADA::EarthWork& self, std::optional<Float> product = std::nullopt, std::optional<Float> vessel = std::nullopt, std::optional<Float> hopper_height = std::nullopt, std::optional<Float> loading = std::nullopt, std::optional<Float> displacement = std::nullopt, std::optional<Integer> timestamp = std::nullopt);
    void refresh_earthwork(WarGrey::SCADA::EarthWork& self);
    void store_earthwork(WarGrey::SCADA::EarthWork& self, WarGrey::SCADA::IPreparedStatement* stmt);
    void restore_earthwork(WarGrey::SCADA::EarthWork& self, WarGrey::SCADA::IPreparedStatement* stmt);

    void create_earthwork(WarGrey::SCADA::IDBSystem* dbc, bool if_not_exists = true);
    void insert_earthwork(WarGrey::SCADA::IDBSystem* dbc, EarthWork& self, bool replace = false);
    void insert_earthwork(WarGrey::SCADA::IDBSystem* dbc, EarthWork* selves, size_t count, bool replace = false);
    void foreach_earthwork(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::IEarthWorkCursor* cursor, uint64 limit = 0U, uint64 offset = 0U, WarGrey::SCADA::earthwork order_by = earthwork::timestamp, bool asc = true);
    std::list<WarGrey::SCADA::EarthWork> select_earthwork(WarGrey::SCADA::IDBSystem* dbc, uint64 limit = 0U, uint64 offset = 0U, WarGrey::SCADA::earthwork order_by = earthwork::timestamp, bool asc = true);
    std::optional<WarGrey::SCADA::EarthWork> seek_earthwork(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::EarthWork_pk where);
    void update_earthwork(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::EarthWork& self, bool refresh = true);
    void update_earthwork(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::EarthWork* selves, size_t count, bool refresh = true);
    void delete_earthwork(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::EarthWork_pk& where);
    void delete_earthwork(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::EarthWork_pk* wheres, size_t count);
    void drop_earthwork(WarGrey::SCADA::IDBSystem* dbc);

    double earthwork_average(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::earthwork column = earthwork::_, bool distinct = false);
    int64 earthwork_count(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::earthwork column = earthwork::_, bool distinct = false);
    std::optional<double> earthwork_max(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::earthwork column = earthwork::_, bool distinct = false);
    std::optional<double> earthwork_min(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::earthwork column = earthwork::_, bool distinct = false);
    std::optional<double> earthwork_sum(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::earthwork column = earthwork::_, bool distinct = false);

    template<size_t N>
    void insert_earthwork(WarGrey::SCADA::IDBSystem* dbc, EarthWork (&selves)[N], bool replace = false) {
        WarGrey::SCADA::insert_earthwork(dbc, selves, N, replace);
    }

    template<size_t N>
    void update_earthwork(WarGrey::SCADA::IDBSystem* dbc, EarthWork (&selves)[N], bool refresh = true) {
        WarGrey::SCADA::update_earthwork(dbc, selves, N, refresh);
    }

    template<size_t N>
    void delete_earthwork(WarGrey::SCADA::IDBSystem* dbc, EarthWork_pk (&wheres)[N]) {
        WarGrey::SCADA::delete_earthwork(dbc, wheres, N);
    }

}
