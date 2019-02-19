#include "earthwork.hpp"

#include "dbsystem.hpp"
#include "dbtypes.hpp"

#include "dbmisc.hpp"

using namespace WarGrey::SCADA;

static const char* earthwork_rowids[] = { "uuid" };

static TableColumnInfo earthwork_columns[] = {
    { "uuid", SDT::Integer, nullptr, DB_PRIMARY_KEY | 0 | 0 },
    { "product", SDT::Float, nullptr, 0 | DB_NOT_NULL | 0 },
    { "vessel", SDT::Float, nullptr, 0 | DB_NOT_NULL | 0 },
    { "hopper_height", SDT::Float, nullptr, 0 | DB_NOT_NULL | 0 },
    { "loading", SDT::Float, nullptr, 0 | DB_NOT_NULL | 0 },
    { "displacement", SDT::Float, nullptr, 0 | DB_NOT_NULL | 0 },
    { "timestamp", SDT::Integer, nullptr, 0 | DB_NOT_NULL | DB_UNIQUE },
};

/**************************************************************************************************/
EarthWork_pk WarGrey::SCADA::earthwork_identity(EarthWork& self) {
    return self.uuid;
}

EarthWork WarGrey::SCADA::make_earthwork(std::optional<Float> product, std::optional<Float> vessel, std::optional<Float> hopper_height, std::optional<Float> loading, std::optional<Float> displacement, std::optional<Integer> timestamp) {
    EarthWork self;

    default_earthwork(self, product, vessel, hopper_height, loading, displacement, timestamp);

    return self;
}

void WarGrey::SCADA::default_earthwork(EarthWork& self, std::optional<Float> product, std::optional<Float> vessel, std::optional<Float> hopper_height, std::optional<Float> loading, std::optional<Float> displacement, std::optional<Integer> timestamp) {
    self.uuid = pk64_timestamp();
    if (product.has_value()) { self.product = product.value(); }
    if (vessel.has_value()) { self.vessel = vessel.value(); }
    if (hopper_height.has_value()) { self.hopper_height = hopper_height.value(); }
    if (loading.has_value()) { self.loading = loading.value(); }
    if (displacement.has_value()) { self.displacement = displacement.value(); }
    if (timestamp.has_value()) { self.timestamp = timestamp.value(); }
}

void WarGrey::SCADA::refresh_earthwork(EarthWork& self) {
}

void WarGrey::SCADA::store_earthwork(EarthWork& self, IPreparedStatement* stmt) {
    stmt->bind_parameter(0U, self.uuid);
    stmt->bind_parameter(1U, self.product);
    stmt->bind_parameter(2U, self.vessel);
    stmt->bind_parameter(3U, self.hopper_height);
    stmt->bind_parameter(4U, self.loading);
    stmt->bind_parameter(5U, self.displacement);
    stmt->bind_parameter(6U, self.timestamp);
}

void WarGrey::SCADA::restore_earthwork(EarthWork& self, IPreparedStatement* stmt) {
    self.uuid = stmt->column_int64(0U);
    self.product = stmt->column_double(1U);
    self.vessel = stmt->column_double(2U);
    self.hopper_height = stmt->column_double(3U);
    self.loading = stmt->column_double(4U);
    self.displacement = stmt->column_double(5U);
    self.timestamp = stmt->column_int64(6U);
}

/**************************************************************************************************/
void WarGrey::SCADA::create_earthwork(IDBSystem* dbc, bool if_not_exists) {
    IVirtualSQL* vsql = dbc->make_sql_factory(earthwork_columns);
    std::string sql = vsql->create_table("earthwork", earthwork_rowids, sizeof(earthwork_rowids)/sizeof(char*), if_not_exists);

    dbc->exec(sql);
}

void WarGrey::SCADA::insert_earthwork(IDBSystem* dbc, EarthWork& self, bool replace) {
    insert_earthwork(dbc, &self, 1, replace);
}

void WarGrey::SCADA::insert_earthwork(IDBSystem* dbc, EarthWork* selves, size_t count, bool replace) {
    IVirtualSQL* vsql = dbc->make_sql_factory(earthwork_columns);
    std::string sql = vsql->insert_into("earthwork", replace);
    IPreparedStatement* stmt = dbc->prepare(sql);

    if (stmt != nullptr) {
        for (size_t i = 0; i < count; i ++) {
            store_earthwork(selves[i], stmt);

            dbc->exec(stmt);
            stmt->reset(true);
        }

        delete stmt;
    }
}

void WarGrey::SCADA::foreach_earthwork(IDBSystem* dbc, IEarthWorkCursor* cursor, uint64 limit, uint64 offset, earthwork order_by, bool asc) {
    IVirtualSQL* vsql = dbc->make_sql_factory(earthwork_columns);
    const char* colname = ((order_by == earthwork::_) ? nullptr : earthwork_columns[static_cast<unsigned int>(order_by)].name);
    std::string sql = vsql->select_from("earthwork", colname, asc, limit, offset);
    IPreparedStatement* stmt = dbc->prepare(sql);

    if (stmt != nullptr) {
        EarthWork self;

        while(stmt->step()) {
            restore_earthwork(self, stmt);
            if (!cursor->step(self, asc, dbc->last_errno())) break;
        }

        delete stmt;
    }

}

std::list<EarthWork> WarGrey::SCADA::select_earthwork(IDBSystem* dbc, uint64 limit, uint64 offset, earthwork order_by, bool asc) {
    IVirtualSQL* vsql = dbc->make_sql_factory(earthwork_columns);
    const char* colname = ((order_by == earthwork::_) ? nullptr : earthwork_columns[static_cast<unsigned int>(order_by)].name);
    std::string sql = vsql->select_from("earthwork", colname, asc, limit, offset);
    IPreparedStatement* stmt = dbc->prepare(sql);
    std::list<EarthWork> queries;

    if (stmt != nullptr) {
        EarthWork self;

        while(stmt->step()) {
            restore_earthwork(self, stmt);
            queries.push_back(self);
        }

        delete stmt;
    }

    return queries;
}

std::optional<EarthWork> WarGrey::SCADA::seek_earthwork(IDBSystem* dbc, EarthWork_pk where) {
    IVirtualSQL* vsql = dbc->make_sql_factory(earthwork_columns);
    std::string sql = vsql->seek_from("earthwork", earthwork_rowids, sizeof(earthwork_rowids)/sizeof(char*));
    IPreparedStatement* stmt = dbc->prepare(sql);
    std::optional<EarthWork> query;

    if (stmt != nullptr) {
        EarthWork self;

        stmt->bind_parameter(0U, where);

        if (stmt->step()) {
            restore_earthwork(self, stmt);
            query = self;
        }

        delete stmt;
    }

    return query;
}

void WarGrey::SCADA::update_earthwork(IDBSystem* dbc, EarthWork& self, bool refresh) {
    update_earthwork(dbc, &self, 1);
}

void WarGrey::SCADA::update_earthwork(IDBSystem* dbc, EarthWork* selves, size_t count, bool refresh) {
    IVirtualSQL* vsql = dbc->make_sql_factory(earthwork_columns);
    std::string sql = vsql->update_set("earthwork", earthwork_rowids, sizeof(earthwork_rowids)/sizeof(char*));
    IPreparedStatement* stmt = dbc->prepare(sql);

    if (stmt != nullptr) {
        for (size_t i = 0; i < count; i ++) {
            if (refresh) {
                refresh_earthwork(selves[i]);
            }

            stmt->bind_parameter(6U, selves[i].uuid);

            stmt->bind_parameter(0U, selves[i].product);
            stmt->bind_parameter(1U, selves[i].vessel);
            stmt->bind_parameter(2U, selves[i].hopper_height);
            stmt->bind_parameter(3U, selves[i].loading);
            stmt->bind_parameter(4U, selves[i].displacement);
            stmt->bind_parameter(5U, selves[i].timestamp);

            dbc->exec(stmt);
            stmt->reset(true);
        }

        delete stmt;
    }
}

void WarGrey::SCADA::delete_earthwork(IDBSystem* dbc, EarthWork_pk& where) {
    delete_earthwork(dbc, &where, 1);
}

void WarGrey::SCADA::delete_earthwork(IDBSystem* dbc, EarthWork_pk* wheres, size_t count) {
    IVirtualSQL* vsql = dbc->make_sql_factory(earthwork_columns);
    std::string sql = vsql->delete_from("earthwork", earthwork_rowids, sizeof(earthwork_rowids)/sizeof(char*));
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

void WarGrey::SCADA::drop_earthwork(IDBSystem* dbc) {
    IVirtualSQL* vsql = dbc->make_sql_factory(earthwork_columns);
    std::string sql = vsql->drop_table("earthwork");

    dbc->exec(sql);
}

/**************************************************************************************************/
double WarGrey::SCADA::earthwork_average(WarGrey::SCADA::IDBSystem* dbc, earthwork column, bool distinct) {
    IVirtualSQL* vsql = dbc->make_sql_factory(earthwork_columns);
    const char* colname = ((column == earthwork::_) ? nullptr : earthwork_columns[static_cast<unsigned int>(column)].name);

    return dbc->query_double(vsql->table_average("earthwork", colname, distinct));
}

int64 WarGrey::SCADA::earthwork_count(WarGrey::SCADA::IDBSystem* dbc, earthwork column, bool distinct) {
    IVirtualSQL* vsql = dbc->make_sql_factory(earthwork_columns);
    const char* colname = ((column == earthwork::_) ? nullptr : earthwork_columns[static_cast<unsigned int>(column)].name);

    return dbc->query_int64(vsql->table_count("earthwork", colname, distinct));
}

std::optional<double> WarGrey::SCADA::earthwork_max(WarGrey::SCADA::IDBSystem* dbc, earthwork column, bool distinct) {
    IVirtualSQL* vsql = dbc->make_sql_factory(earthwork_columns);
    const char* colname = ((column == earthwork::_) ? nullptr : earthwork_columns[static_cast<unsigned int>(column)].name);

    return dbc->query_maybe_double(vsql->table_max("earthwork", colname, distinct));
}

std::optional<double> WarGrey::SCADA::earthwork_min(WarGrey::SCADA::IDBSystem* dbc, earthwork column, bool distinct) {
    IVirtualSQL* vsql = dbc->make_sql_factory(earthwork_columns);
    const char* colname = ((column == earthwork::_) ? nullptr : earthwork_columns[static_cast<unsigned int>(column)].name);

    return dbc->query_maybe_double(vsql->table_min("earthwork", colname, distinct));
}

std::optional<double> WarGrey::SCADA::earthwork_sum(WarGrey::SCADA::IDBSystem* dbc, earthwork column, bool distinct) {
    IVirtualSQL* vsql = dbc->make_sql_factory(earthwork_columns);
    const char* colname = ((column == earthwork::_) ? nullptr : earthwork_columns[static_cast<unsigned int>(column)].name);

    return dbc->query_maybe_double(vsql->table_sum("earthwork", colname, distinct));
}

