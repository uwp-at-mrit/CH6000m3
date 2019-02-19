#pragma once

#include <list>
#include <optional>

#include "dbsystem.hpp"

namespace WarGrey::SCADA {
    typedef Integer Alarm_pk;

    private struct Alarm {
        Integer uuid;
        Integer index;
        std::optional<Integer> type;
        Integer alarmtime;
        std::optional<Integer> fixedtime;
    };

    private class IAlarmCursor abstract {
    public:
        virtual bool step(WarGrey::SCADA::Alarm& occurrence, bool asc, int code) = 0;
    };

    private enum class alarm { uuid, index, type, alarmtime, fixedtime, _ };

    WarGrey::SCADA::Alarm_pk alarm_identity(WarGrey::SCADA::Alarm& self);

    WarGrey::SCADA::Alarm make_alarm(std::optional<Integer> index = std::nullopt, std::optional<Integer> type = std::nullopt, std::optional<Integer> alarmtime = std::nullopt, std::optional<Integer> fixedtime = std::nullopt);
    void default_alarm(WarGrey::SCADA::Alarm& self, std::optional<Integer> index = std::nullopt, std::optional<Integer> type = std::nullopt, std::optional<Integer> alarmtime = std::nullopt, std::optional<Integer> fixedtime = std::nullopt);
    void refresh_alarm(WarGrey::SCADA::Alarm& self);
    void store_alarm(WarGrey::SCADA::Alarm& self, WarGrey::SCADA::IPreparedStatement* stmt);
    void restore_alarm(WarGrey::SCADA::Alarm& self, WarGrey::SCADA::IPreparedStatement* stmt);

    void create_alarm(WarGrey::SCADA::IDBSystem* dbc, bool if_not_exists = true);
    void insert_alarm(WarGrey::SCADA::IDBSystem* dbc, Alarm& self, bool replace = false);
    void insert_alarm(WarGrey::SCADA::IDBSystem* dbc, Alarm* selves, size_t count, bool replace = false);
    void foreach_alarm(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::IAlarmCursor* cursor, uint64 limit = 0U, uint64 offset = 0U, WarGrey::SCADA::alarm order_by = alarm::alarmtime, bool asc = true);
    std::list<WarGrey::SCADA::Alarm> select_alarm(WarGrey::SCADA::IDBSystem* dbc, uint64 limit = 0U, uint64 offset = 0U, WarGrey::SCADA::alarm order_by = alarm::alarmtime, bool asc = true);
    std::optional<WarGrey::SCADA::Alarm> seek_alarm(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::Alarm_pk where);
    void update_alarm(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::Alarm& self, bool refresh = true);
    void update_alarm(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::Alarm* selves, size_t count, bool refresh = true);
    void delete_alarm(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::Alarm_pk& where);
    void delete_alarm(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::Alarm_pk* wheres, size_t count);
    void drop_alarm(WarGrey::SCADA::IDBSystem* dbc);

    double alarm_average(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::alarm column = alarm::_, bool distinct = false);
    int64 alarm_count(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::alarm column = alarm::_, bool distinct = false);
    std::optional<double> alarm_max(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::alarm column = alarm::_, bool distinct = false);
    std::optional<double> alarm_min(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::alarm column = alarm::_, bool distinct = false);
    std::optional<double> alarm_sum(WarGrey::SCADA::IDBSystem* dbc, WarGrey::SCADA::alarm column = alarm::_, bool distinct = false);

    template<size_t N>
    void insert_alarm(WarGrey::SCADA::IDBSystem* dbc, Alarm (&selves)[N], bool replace = false) {
        WarGrey::SCADA::insert_alarm(dbc, selves, N, replace);
    }

    template<size_t N>
    void update_alarm(WarGrey::SCADA::IDBSystem* dbc, Alarm (&selves)[N], bool refresh = true) {
        WarGrey::SCADA::update_alarm(dbc, selves, N, refresh);
    }

    template<size_t N>
    void delete_alarm(WarGrey::SCADA::IDBSystem* dbc, Alarm_pk (&wheres)[N]) {
        WarGrey::SCADA::delete_alarm(dbc, wheres, N);
    }

}
