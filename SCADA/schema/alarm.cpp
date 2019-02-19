#include "alarm.hpp"

#include "dbsystem.hpp"
#include "dbtypes.hpp"

#include "dbmisc.hpp"

using namespace WarGrey::SCADA;

static const char* alarm_rowids[] = { "uuid" };

static TableColumnInfo alarm_columns[] = {
    { "uuid", SDT::Integer, nullptr, DB_PRIMARY_KEY | 0 | 0 },
    { "index", SDT::Integer, nullptr, 0 | DB_NOT_NULL | 0 },
    { "type", SDT::Integer, nullptr, 0 | 0 | 0 },
    { "alarmtime", SDT::Integer, nullptr, 0 | DB_NOT_NULL | 0 },
    { "fixedtime", SDT::Integer, nullptr, 0 | 0 | 0 },
};

/**************************************************************************************************/
Alarm_pk WarGrey::SCADA::alarm_identity(Alarm& self) {
    return self.uuid;
}

Alarm WarGrey::SCADA::make_alarm(std::optional<Integer> index, std::optional<Integer> type, std::optional<Integer> alarmtime, std::optional<Integer> fixedtime) {
    Alarm self;

    default_alarm(self, index, type, alarmtime, fixedtime);

    return self;
}

void WarGrey::SCADA::default_alarm(Alarm& self, std::optional<Integer> index, std::optional<Integer> type, std::optional<Integer> alarmtime, std::optional<Integer> fixedtime) {
    self.uuid = pk64_timestamp();
    if (index.has_value()) { self.index = index.value(); }
    self.type = ((type.has_value()) ? type.value() : 1000);
    if (alarmtime.has_value()) { self.alarmtime = alarmtime.value(); }
    self.fixedtime = ((fixedtime.has_value()) ? fixedtime.value() : 0);
}

void WarGrey::SCADA::refresh_alarm(Alarm& self) {
}

void WarGrey::SCADA::store_alarm(Alarm& self, IPreparedStatement* stmt) {
    stmt->bind_parameter(0U, self.uuid);
    stmt->bind_parameter(1U, self.index);
    stmt->bind_parameter(2U, self.type);
    stmt->bind_parameter(3U, self.alarmtime);
    stmt->bind_parameter(4U, self.fixedtime);
}

void WarGrey::SCADA::restore_alarm(Alarm& self, IPreparedStatement* stmt) {
    self.uuid = stmt->column_int64(0U);
    self.index = stmt->column_int64(1U);
    self.type = stmt->column_maybe_int64(2U);
    self.alarmtime = stmt->column_int64(3U);
    self.fixedtime = stmt->column_maybe_int64(4U);
}

/**************************************************************************************************/
void WarGrey::SCADA::create_alarm(IDBSystem* dbc, bool if_not_exists) {
    IVirtualSQL* vsql = dbc->make_sql_factory(alarm_columns);
    std::string sql = vsql->create_table("alarm", alarm_rowids, sizeof(alarm_rowids)/sizeof(char*), if_not_exists);

    dbc->exec(sql);
}

void WarGrey::SCADA::insert_alarm(IDBSystem* dbc, Alarm& self, bool replace) {
    insert_alarm(dbc, &self, 1, replace);
}

void WarGrey::SCADA::insert_alarm(IDBSystem* dbc, Alarm* selves, size_t count, bool replace) {
    IVirtualSQL* vsql = dbc->make_sql_factory(alarm_columns);
    std::string sql = vsql->insert_into("alarm", replace);
    IPreparedStatement* stmt = dbc->prepare(sql);

    if (stmt != nullptr) {
        for (size_t i = 0; i < count; i ++) {
            store_alarm(selves[i], stmt);

            dbc->exec(stmt);
            stmt->reset(true);
        }

        delete stmt;
    }
}

void WarGrey::SCADA::foreach_alarm(IDBSystem* dbc, IAlarmCursor* cursor, uint64 limit, uint64 offset, alarm order_by, bool asc) {
    IVirtualSQL* vsql = dbc->make_sql_factory(alarm_columns);
    const char* colname = ((order_by == alarm::_) ? nullptr : alarm_columns[static_cast<unsigned int>(order_by)].name);
    std::string sql = vsql->select_from("alarm", colname, asc, limit, offset);
    IPreparedStatement* stmt = dbc->prepare(sql);

    if (stmt != nullptr) {
        Alarm self;

        while(stmt->step()) {
            restore_alarm(self, stmt);
            if (!cursor->step(self, asc, dbc->last_errno())) break;
        }

        delete stmt;
    }

}

std::list<Alarm> WarGrey::SCADA::select_alarm(IDBSystem* dbc, uint64 limit, uint64 offset, alarm order_by, bool asc) {
    IVirtualSQL* vsql = dbc->make_sql_factory(alarm_columns);
    const char* colname = ((order_by == alarm::_) ? nullptr : alarm_columns[static_cast<unsigned int>(order_by)].name);
    std::string sql = vsql->select_from("alarm", colname, asc, limit, offset);
    IPreparedStatement* stmt = dbc->prepare(sql);
    std::list<Alarm> queries;

    if (stmt != nullptr) {
        Alarm self;

        while(stmt->step()) {
            restore_alarm(self, stmt);
            queries.push_back(self);
        }

        delete stmt;
    }

    return queries;
}

std::optional<Alarm> WarGrey::SCADA::seek_alarm(IDBSystem* dbc, Alarm_pk where) {
    IVirtualSQL* vsql = dbc->make_sql_factory(alarm_columns);
    std::string sql = vsql->seek_from("alarm", alarm_rowids, sizeof(alarm_rowids)/sizeof(char*));
    IPreparedStatement* stmt = dbc->prepare(sql);
    std::optional<Alarm> query;

    if (stmt != nullptr) {
        Alarm self;

        stmt->bind_parameter(0U, where);

        if (stmt->step()) {
            restore_alarm(self, stmt);
            query = self;
        }

        delete stmt;
    }

    return query;
}

void WarGrey::SCADA::update_alarm(IDBSystem* dbc, Alarm& self, bool refresh) {
    update_alarm(dbc, &self, 1, refresh);
}

void WarGrey::SCADA::update_alarm(IDBSystem* dbc, Alarm* selves, size_t count, bool refresh) {
    IVirtualSQL* vsql = dbc->make_sql_factory(alarm_columns);
    std::string sql = vsql->update_set("alarm", alarm_rowids, sizeof(alarm_rowids)/sizeof(char*));
    IPreparedStatement* stmt = dbc->prepare(sql);

    if (stmt != nullptr) {
        for (size_t i = 0; i < count; i ++) {
            if (refresh) {
                refresh_alarm(selves[i]);
            }

            stmt->bind_parameter(4U, selves[i].uuid);

            stmt->bind_parameter(0U, selves[i].index);
            stmt->bind_parameter(1U, selves[i].type);
            stmt->bind_parameter(2U, selves[i].alarmtime);
            stmt->bind_parameter(3U, selves[i].fixedtime);

            dbc->exec(stmt);
            stmt->reset(true);
        }

        delete stmt;
    }
}

void WarGrey::SCADA::delete_alarm(IDBSystem* dbc, Alarm_pk& where) {
    delete_alarm(dbc, &where, 1);
}

void WarGrey::SCADA::delete_alarm(IDBSystem* dbc, Alarm_pk* wheres, size_t count) {
    IVirtualSQL* vsql = dbc->make_sql_factory(alarm_columns);
    std::string sql = vsql->delete_from("alarm", alarm_rowids, sizeof(alarm_rowids)/sizeof(char*));
    IPreparedStatement* stmt = dbc->prepare(sql);

    if (stmt != nullptr) {
        for (size_t i = 0; i < count; i ++) {
            stmt->bind_parameter(0U, wheres[i]);

            dbc->exec(stmt);
            stmt->reset(true);
        }

        delete stmt;
    }
}

void WarGrey::SCADA::drop_alarm(IDBSystem* dbc) {
    IVirtualSQL* vsql = dbc->make_sql_factory(alarm_columns);
    std::string sql = vsql->drop_table("alarm");

    dbc->exec(sql);
}

/**************************************************************************************************/
double WarGrey::SCADA::alarm_average(WarGrey::SCADA::IDBSystem* dbc, alarm column, bool distinct) {
    IVirtualSQL* vsql = dbc->make_sql_factory(alarm_columns);
    const char* colname = ((column == alarm::_) ? nullptr : alarm_columns[static_cast<unsigned int>(column)].name);

    return dbc->query_double(vsql->table_average("alarm", colname, distinct));
}

int64 WarGrey::SCADA::alarm_count(WarGrey::SCADA::IDBSystem* dbc, alarm column, bool distinct) {
    IVirtualSQL* vsql = dbc->make_sql_factory(alarm_columns);
    const char* colname = ((column == alarm::_) ? nullptr : alarm_columns[static_cast<unsigned int>(column)].name);

    return dbc->query_int64(vsql->table_count("alarm", colname, distinct));
}

std::optional<double> WarGrey::SCADA::alarm_max(WarGrey::SCADA::IDBSystem* dbc, alarm column, bool distinct) {
    IVirtualSQL* vsql = dbc->make_sql_factory(alarm_columns);
    const char* colname = ((column == alarm::_) ? nullptr : alarm_columns[static_cast<unsigned int>(column)].name);

    return dbc->query_maybe_double(vsql->table_max("alarm", colname, distinct));
}

std::optional<double> WarGrey::SCADA::alarm_min(WarGrey::SCADA::IDBSystem* dbc, alarm column, bool distinct) {
    IVirtualSQL* vsql = dbc->make_sql_factory(alarm_columns);
    const char* colname = ((column == alarm::_) ? nullptr : alarm_columns[static_cast<unsigned int>(column)].name);

    return dbc->query_maybe_double(vsql->table_min("alarm", colname, distinct));
}

std::optional<double> WarGrey::SCADA::alarm_sum(WarGrey::SCADA::IDBSystem* dbc, alarm column, bool distinct) {
    IVirtualSQL* vsql = dbc->make_sql_factory(alarm_columns);
    const char* colname = ((column == alarm::_) ? nullptr : alarm_columns[static_cast<unsigned int>(column)].name);

    return dbc->query_maybe_double(vsql->table_sum("alarm", colname, distinct));
}

