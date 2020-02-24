#pragma once

#include <list>
#include <optional>

#include "dbsystem.hpp"

namespace WarGrey::SCADA {
    typedef Integer Track_pk;

    private struct Track {
        Integer uuid;
        Integer group;
        Float x;
        Float y;
        Float z;
        Integer timestamp;
    };

    private class ITrackCursor abstract {
    public:
        virtual bool step(WarGrey::SCADA::Track& occurrence, bool asc, int code) = 0;
    };

    private enum class track { uuid, group, x, y, z, timestamp, _ };

    WarGrey::SCADA::Track_pk track_identity(WarGrey::SCADA::Track& self);

    WarGrey::SCADA::Track make_track(std::optional<Integer> group = std::nullopt, std::optional<Float> x = std::nullopt, std::optional<Float> y = std::nullopt, std::optional<Float> z = std::nullopt, std::optional<Integer> timestamp = std::nullopt);
    void default_track(WarGrey::SCADA::Track& self, std::optional<Integer> group = std::nullopt, std::optional<Float> x = std::nullopt, std::optional<Float> y = std::nullopt, std::optional<Float> z = std::nullopt, std::optional<Integer> timestamp = std::nullopt);
    void refresh_track(WarGrey::SCADA::Track& self);
    void store_track(WarGrey::SCADA::Track& self, WarGrey::SCADA::IPreparedStatement* stmt);
    void restore_track(WarGrey::SCADA::Track& self, WarGrey::SCADA::IPreparedStatement* stmt);

    void create_track(WarGrey::SCADA::IDBSystem* dbc, bool if_not_exists = true);
    void insert_track(WarGrey::SCADA::IDBSystem* dbc, Track& self, bool replace = false);
    void insert_track(WarGrey::SCADA::IDBSystem* dbc, Track* selves, size_t count, bool replace = false);
    void foreach_track(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::ITrackCursor* cursor, uint64 limit = 0U, uint64 offset = 0U, WarGrey::SCADA::track order_by = track::timestamp, bool asc = true);
    std::list<WarGrey::SCADA::Track> select_track(WarGrey::SCADA::IDBSystem* dbc, uint64 limit = 0U, uint64 offset = 0U, WarGrey::SCADA::track order_by = track::timestamp, bool asc = true);
    std::optional<WarGrey::SCADA::Track> seek_track(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::Track_pk where);
    void update_track(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::Track& self, bool refresh = true);
    void update_track(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::Track* selves, size_t count, bool refresh = true);
    void delete_track(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::Track_pk& where);
    void delete_track(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::Track_pk* wheres, size_t count);
    void drop_track(WarGrey::SCADA::IDBSystem* dbc);

    double track_average(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::track column = track::_, bool distinct = false);
    int64 track_count(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::track column = track::_, bool distinct = false);
    std::optional<double> track_max(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::track column = track::_, bool distinct = false);
    std::optional<double> track_min(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::track column = track::_, bool distinct = false);
    std::optional<double> track_sum(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::track column = track::_, bool distinct = false);

    template<size_t N>
    void insert_track(WarGrey::SCADA::IDBSystem* dbc, Track (&selves)[N], bool replace = false) {
        WarGrey::SCADA::insert_track(dbc, selves, N, replace);
    }

    template<size_t N>
    void update_track(WarGrey::SCADA::IDBSystem* dbc, Track (&selves)[N], bool refresh = true) {
        WarGrey::SCADA::update_track(dbc, selves, N, refresh);
    }

    template<size_t N>
    void delete_track(WarGrey::SCADA::IDBSystem* dbc, Track_pk (&wheres)[N]) {
        WarGrey::SCADA::delete_track(dbc, wheres, N);
    }

}
