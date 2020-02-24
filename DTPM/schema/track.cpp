#include "track.hpp"

#include "dbsystem.hpp"
#include "dbtypes.hpp"

#include "dbmisc.hpp"

using namespace WarGrey::SCADA;

static const char* track_rowids[] = { "uuid" };

static TableColumnInfo track_columns[] = {
    { "uuid", SDT::Integer, nullptr, DB_PRIMARY_KEY | 0 | 0 },
    { "group", SDT::Integer, nullptr, 0 | DB_NOT_NULL | 0 },
    { "x", SDT::Float, nullptr, 0 | DB_NOT_NULL | 0 },
    { "y", SDT::Float, nullptr, 0 | DB_NOT_NULL | 0 },
    { "z", SDT::Float, nullptr, 0 | DB_NOT_NULL | 0 },
    { "timestamp", SDT::Integer, nullptr, 0 | DB_NOT_NULL | DB_UNIQUE },
};

/**************************************************************************************************/
Track_pk WarGrey::SCADA::track_identity(Track& self) {
    return self.uuid;
}

Track WarGrey::SCADA::make_track(std::optional<Integer> group, std::optional<Float> x, std::optional<Float> y, std::optional<Float> z, std::optional<Integer> timestamp) {
    Track self;

    default_track(self, group, x, y, z, timestamp);

    return self;
}

void WarGrey::SCADA::default_track(Track& self, std::optional<Integer> group, std::optional<Float> x, std::optional<Float> y, std::optional<Float> z, std::optional<Integer> timestamp) {
    self.uuid = pk64_timestamp();
    if (group.has_value()) { self.group = group.value(); }
    if (x.has_value()) { self.x = x.value(); }
    if (y.has_value()) { self.y = y.value(); }
    if (z.has_value()) { self.z = z.value(); }
    if (timestamp.has_value()) { self.timestamp = timestamp.value(); }
}

void WarGrey::SCADA::refresh_track(Track& self) {
}

void WarGrey::SCADA::store_track(Track& self, IPreparedStatement* stmt) {
    stmt->bind_parameter(0U, self.uuid);
    stmt->bind_parameter(1U, self.group);
    stmt->bind_parameter(2U, self.x);
    stmt->bind_parameter(3U, self.y);
    stmt->bind_parameter(4U, self.z);
    stmt->bind_parameter(5U, self.timestamp);
}

void WarGrey::SCADA::restore_track(Track& self, IPreparedStatement* stmt) {
    self.uuid = stmt->column_int64(0U);
    self.group = stmt->column_int64(1U);
    self.x = stmt->column_double(2U);
    self.y = stmt->column_double(3U);
    self.z = stmt->column_double(4U);
    self.timestamp = stmt->column_int64(5U);
}

/**************************************************************************************************/
void WarGrey::SCADA::create_track(IDBSystem* dbc, bool if_not_exists) {
    IVirtualSQL* vsql = dbc->make_sql_factory(track_columns);
    std::string sql = vsql->create_table("track", track_rowids, sizeof(track_rowids)/sizeof(char*), if_not_exists);

    dbc->exec(sql);
}

void WarGrey::SCADA::insert_track(IDBSystem* dbc, Track& self, bool replace) {
    insert_track(dbc, &self, 1, replace);
}

void WarGrey::SCADA::insert_track(IDBSystem* dbc, Track* selves, size_t count, bool replace) {
    IVirtualSQL* vsql = dbc->make_sql_factory(track_columns);
    std::string sql = vsql->insert_into("track", replace);
    IPreparedStatement* stmt = dbc->prepare(sql);

    if (stmt != nullptr) {
        for (size_t i = 0; i < count; i ++) {
            store_track(selves[i], stmt);

            dbc->exec(stmt);
            stmt->reset(true);
        }

        delete stmt;
    }
}

void WarGrey::SCADA::foreach_track(IDBSystem* dbc, ITrackCursor* cursor, uint64 limit, uint64 offset, track order_by, bool asc) {
    IVirtualSQL* vsql = dbc->make_sql_factory(track_columns);
    const char* colname = ((order_by == track::_) ? nullptr : track_columns[static_cast<unsigned int>(order_by)].name);
    std::string sql = vsql->select_from("track", colname, asc, limit, offset);
    IPreparedStatement* stmt = dbc->prepare(sql);

    if (stmt != nullptr) {
        Track self;

        while(stmt->step()) {
            restore_track(self, stmt);
            if (!cursor->step(self, asc, dbc->last_errno())) break;
        }

        delete stmt;
    }

}

std::list<Track> WarGrey::SCADA::select_track(IDBSystem* dbc, uint64 limit, uint64 offset, track order_by, bool asc) {
    IVirtualSQL* vsql = dbc->make_sql_factory(track_columns);
    const char* colname = ((order_by == track::_) ? nullptr : track_columns[static_cast<unsigned int>(order_by)].name);
    std::string sql = vsql->select_from("track", colname, asc, limit, offset);
    IPreparedStatement* stmt = dbc->prepare(sql);
    std::list<Track> queries;

    if (stmt != nullptr) {
        Track self;

        while(stmt->step()) {
            restore_track(self, stmt);
            queries.push_back(self);
        }

        delete stmt;
    }

    return queries;
}

std::optional<Track> WarGrey::SCADA::seek_track(IDBSystem* dbc, Track_pk where) {
    IVirtualSQL* vsql = dbc->make_sql_factory(track_columns);
    std::string sql = vsql->seek_from("track", track_rowids, sizeof(track_rowids)/sizeof(char*));
    IPreparedStatement* stmt = dbc->prepare(sql);
    std::optional<Track> query;

    if (stmt != nullptr) {
        Track self;

        stmt->bind_parameter(0U, where);

        if (stmt->step()) {
            restore_track(self, stmt);
            query = self;
        }

        delete stmt;
    }

    return query;
}

void WarGrey::SCADA::update_track(IDBSystem* dbc, Track& self, bool refresh) {
    update_track(dbc, &self, 1, refresh);
}

void WarGrey::SCADA::update_track(IDBSystem* dbc, Track* selves, size_t count, bool refresh) {
    IVirtualSQL* vsql = dbc->make_sql_factory(track_columns);
    std::string sql = vsql->update_set("track", track_rowids, sizeof(track_rowids)/sizeof(char*));
    IPreparedStatement* stmt = dbc->prepare(sql);

    if (stmt != nullptr) {
        for (size_t i = 0; i < count; i ++) {
            if (refresh) {
                refresh_track(selves[i]);
            }

            stmt->bind_parameter(5U, selves[i].uuid);

            stmt->bind_parameter(0U, selves[i].group);
            stmt->bind_parameter(1U, selves[i].x);
            stmt->bind_parameter(2U, selves[i].y);
            stmt->bind_parameter(3U, selves[i].z);
            stmt->bind_parameter(4U, selves[i].timestamp);

            dbc->exec(stmt);
            stmt->reset(true);
        }

        delete stmt;
    }
}

void WarGrey::SCADA::delete_track(IDBSystem* dbc, Track_pk& where) {
    delete_track(dbc, &where, 1);
}

void WarGrey::SCADA::delete_track(IDBSystem* dbc, Track_pk* wheres, size_t count) {
    IVirtualSQL* vsql = dbc->make_sql_factory(track_columns);
    std::string sql = vsql->delete_from("track", track_rowids, sizeof(track_rowids)/sizeof(char*));
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

void WarGrey::SCADA::drop_track(IDBSystem* dbc) {
    IVirtualSQL* vsql = dbc->make_sql_factory(track_columns);
    std::string sql = vsql->drop_table("track");

    dbc->exec(sql);
}

/**************************************************************************************************/
double WarGrey::SCADA::track_average(WarGrey::SCADA::IDBSystem* dbc, track column, bool distinct) {
    IVirtualSQL* vsql = dbc->make_sql_factory(track_columns);
    const char* colname = ((column == track::_) ? nullptr : track_columns[static_cast<unsigned int>(column)].name);

    return dbc->query_double(vsql->table_average("track", colname, distinct));
}

int64 WarGrey::SCADA::track_count(WarGrey::SCADA::IDBSystem* dbc, track column, bool distinct) {
    IVirtualSQL* vsql = dbc->make_sql_factory(track_columns);
    const char* colname = ((column == track::_) ? nullptr : track_columns[static_cast<unsigned int>(column)].name);

    return dbc->query_int64(vsql->table_count("track", colname, distinct));
}

std::optional<double> WarGrey::SCADA::track_max(WarGrey::SCADA::IDBSystem* dbc, track column, bool distinct) {
    IVirtualSQL* vsql = dbc->make_sql_factory(track_columns);
    const char* colname = ((column == track::_) ? nullptr : track_columns[static_cast<unsigned int>(column)].name);

    return dbc->query_maybe_double(vsql->table_max("track", colname, distinct));
}

std::optional<double> WarGrey::SCADA::track_min(WarGrey::SCADA::IDBSystem* dbc, track column, bool distinct) {
    IVirtualSQL* vsql = dbc->make_sql_factory(track_columns);
    const char* colname = ((column == track::_) ? nullptr : track_columns[static_cast<unsigned int>(column)].name);

    return dbc->query_maybe_double(vsql->table_min("track", colname, distinct));
}

std::optional<double> WarGrey::SCADA::track_sum(WarGrey::SCADA::IDBSystem* dbc, track column, bool distinct) {
    IVirtualSQL* vsql = dbc->make_sql_factory(track_columns);
    const char* colname = ((column == track::_) ? nullptr : track_columns[static_cast<unsigned int>(column)].name);

    return dbc->query_maybe_double(vsql->table_sum("track", colname, distinct));
}

