-- Enable foreign key support for cascading
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS
    "rocpd_metadata{{upid}}" (
        "id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
        "tag" TEXT NOT NULL,
        "value" TEXT NOT NULL
    );

CREATE TABLE IF NOT EXISTS
    "rocpd_string{{upid}}" (
        "id" INTEGER NOT NULL,
        "guid" TEXT NOT NULL,
        "string" TEXT NOT NULL UNIQUE ON CONFLICT ABORT,
        PRIMARY KEY (id)
    );

CREATE TABLE IF NOT EXISTS
    "rocpd_info_node{{upid}}" (
        "id" INTEGER NOT NULL,
        "guid" TEXT NOT NULL,
        "hash" BIGINT NOT NULL UNIQUE,
        "machine_id" TEXT NOT NULL UNIQUE,
        "system_name" TEXT,
        "hostname" TEXT,
        "release" TEXT,
        "version" TEXT,
        "hardware_name" TEXT,
        "domain_name" TEXT,
        PRIMARY KEY (id)
    );

CREATE TABLE IF NOT EXISTS
    "rocpd_info_process{{upid}}" (
        "id" INTEGER NOT NULL,
        "guid" TEXT NOT NULL,
        "nid" INTEGER NOT NULL,
        "ppid" INTEGER,
        "pid" INTEGER NOT NULL,
        "init" BIGINT,
        "fini" BIGINT,
        "start" BIGINT,
        "end" BIGINT,
        "command" TEXT,
        "environment" JSONB DEFAULT "{}" NOT NULL,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (nid) REFERENCES rocpd_info_node{{upid}} (id) ON UPDATE CASCADE,
        PRIMARY KEY (id)
    );

CREATE TABLE IF NOT EXISTS
    "rocpd_info_thread{{upid}}" (
        "id" INTEGER NOT NULL,
        "guid" TEXT NOT NULL,
        "nid" INTEGER NOT NULL,
        "ppid" INTEGER,
        "pid" INTEGER NOT NULL,
        "tid" INTEGER NOT NULL,
        "name" TEXT,
        "start" BIGINT,
        "end" BIGINT,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (nid) REFERENCES rocpd_info_node{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (pid) REFERENCES rocpd_info_process{{upid}} (id) ON UPDATE CASCADE,
        PRIMARY KEY (id)
    );

CREATE TABLE IF NOT EXISTS
    "rocpd_info_agent{{upid}}" (
        "id" INTEGER NOT NULL,
        "guid" TEXT NOT NULL,
        "nid" INTEGER NOT NULL,
        "pid" INTEGER NOT NULL,
        "type" TEXT CHECK ("type" IN ('CPU', 'GPU')),
        "absolute_index" INTEGER,
        "logical_index" INTEGER,
        "type_index" INTEGER,
        "uuid" INTEGER,
        "name" TEXT,
        "model_name" TEXT,
        "vendor_name" TEXT,
        "product_name" TEXT,
        "user_name" TEXT,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (nid) REFERENCES rocpd_info_node{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (pid) REFERENCES rocpd_info_process{{upid}} (id) ON UPDATE CASCADE,
        PRIMARY KEY (id)
    );

CREATE TABLE IF NOT EXISTS
    "rocpd_info_queue{{upid}}" (
        "id" INTEGER NOT NULL,
        "guid" TEXT NOT NULL,
        "nid" INTEGER NOT NULL,
        "pid" INTEGER NOT NULL,
        "name" TEXT,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (nid) REFERENCES rocpd_info_node{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (pid) REFERENCES rocpd_info_process{{upid}} (id) ON UPDATE CASCADE,
        PRIMARY KEY (id)
    );

CREATE TABLE IF NOT EXISTS
    "rocpd_info_stream{{upid}}" (
        "id" INTEGER NOT NULL,
        "guid" TEXT NOT NULL,
        "nid" INTEGER NOT NULL,
        "pid" INTEGER NOT NULL,
        "name" TEXT,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (nid) REFERENCES rocpd_info_node{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (pid) REFERENCES rocpd_info_process{{upid}} (id) ON UPDATE CASCADE,
        PRIMARY KEY (id)
    );

-- 2993533, 2269219937, 2993533
-- 2993533, 2269219937, 2993533
-- Performance monitoring counters (PMC) descriptions
CREATE TABLE IF NOT EXISTS
    "rocpd_info_pmc{{upid}}" (
        "id" INTEGER NOT NULL,
        "guid" TEXT NOT NULL,
        "nid" INTEGER NOT NULL,
        "pid" INTEGER NOT NULL,
        "agent_id" INTEGER,
        "target_arch" TEXT CHECK ("target_arch" IN ('CPU', 'GPU')),
        "event_code" INT,
        "instance_id" INTEGER,
        "name" TEXT NOT NULL,
        "symbol" TEXT NOT NULL,
        "description" TEXT,
        "long_description" TEXT DEFAULT "",
        "component" TEXT,
        "units" TEXT DEFAULT "",
        "value_type" TEXT CHECK ("value_type" IN ('ABS', 'ACCUM', 'RELATIVE')),
        "block" TEXT,
        "expression" TEXT,
        "is_constant" INTEGER,
        "is_derived" INTEGER,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (nid) REFERENCES rocpd_info_node{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (pid) REFERENCES rocpd_info_process{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (agent_id) REFERENCES rocpd_info_agent{{upid}} (id) ON UPDATE CASCADE,
        PRIMARY KEY (id)
    );

CREATE TABLE IF NOT EXISTS
    "rocpd_info_code_object{{upid}}" (
        "id" INTEGER NOT NULL,
        "guid" TEXT NOT NULL,
        "nid" INTEGER NOT NULL,
        "pid" INTEGER NOT NULL,
        "agent_id" INTEGER,
        "uri" TEXT,
        "load_base" BIGINT,
        "load_size" BIGINT,
        "load_delta" BIGINT,
        "storage_type" TEXT CHECK ("storage_type" IN ('FILE', 'MEMORY')),
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (nid) REFERENCES rocpd_info_node{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (pid) REFERENCES rocpd_info_process{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (agent_id) REFERENCES rocpd_info_agent{{upid}} (id) ON UPDATE CASCADE,
        PRIMARY KEY (id)
    );

CREATE TABLE IF NOT EXISTS
    "rocpd_info_kernel_symbol{{upid}}" (
        "id" INTEGER NOT NULL,
        "guid" TEXT NOT NULL,
        "nid" INTEGER NOT NULL,
        "pid" INTEGER NOT NULL,
        "code_object_id" INTEGER NOT NULL,
        "kernel_name" TEXT,
        "display_name" TEXT,
        "kernel_object" INTEGER,
        "kernarg_segment_size" INTEGER,
        "kernarg_segment_alignment" INTEGER,
        "group_segment_size" INTEGER,
        "private_segment_size" INTEGER,
        "sgpr_count" INTEGER,
        "arch_vgpr_count" INTEGER,
        "accum_vgpr_count" INTEGER,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (nid) REFERENCES rocpd_info_node{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (pid) REFERENCES rocpd_info_process{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (code_object_id) REFERENCES rocpd_info_code_object{{upid}} (id) ON UPDATE CASCADE,
        PRIMARY KEY (id)
    );

-- Stores repetitive info for samples
CREATE TABLE IF NOT EXISTS
    "rocpd_track{{upid}}" (
        "id" INTEGER NOT NULL,
        "guid" TEXT NOT NULL,
        "nid" INTEGER NOT NULL,
        "pid" INTEGER,
        "tid" INTEGER,
        "name_id" INTEGER,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (nid) REFERENCES rocpd_info_node{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (pid) REFERENCES rocpd_info_process{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (name_id) REFERENCES rocpd_string{{upid}} (id) ON UPDATE CASCADE,
        PRIMARY KEY (id)
    );

-- Storage for a region, instant, and counter
CREATE TABLE IF NOT EXISTS
    "rocpd_event{{upid}}" (
        "id" INTEGER NOT NULL,
        "guid" TEXT NOT NULL,
        "category_id" INTEGER,
        "stack_id" INTEGER,
        "parent_stack_id" INTEGER,
        "correlation_id" INTEGER,
        "call_stack" JSONB DEFAULT "{}" NOT NULL,
        "line_info" JSONB DEFAULT "{}" NOT NULL,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (category_id) REFERENCES rocpd_string{{upid}} (id) ON UPDATE CASCADE,
        PRIMARY KEY (id)
    );

-- stores arguments for events
CREATE TABLE IF NOT EXISTS
    "rocpd_arg{{upid}}" (
        "id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
        "guid" TEXT NOT NULL,
        "event_id" INTEGER NOT NULL,
        "position" INTEGER NOT NULL,
        "type" TEXT NOT NULL,
        "name" TEXT NOT NULL,
        "value" TEXT, -- TODO: discuss make it value_id and integer, refer to string table --
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (event_id) REFERENCES rocpd_event{{upid}} (id) ON UPDATE CASCADE
    );

-- Region with a start/stop on the same thread (CPU)
CREATE TABLE IF NOT EXISTS
    "rocpd_pmc_event{{upid}}" (
        "id" INTEGER NOT NULL,
        "guid" TEXT NOT NULL,
        "event_id" INTEGER,
        "pmc_id" INTEGER NOT NULL,
        "value" REAL DEFAULT 0.0,
        "extdata" JSONB DEFAULT "{}",
        FOREIGN KEY (pmc_id) REFERENCES rocpd_info_pmc{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (event_id) REFERENCES rocpd_event{{upid}} (id) ON UPDATE CASCADE,
        PRIMARY KEY (id)
    );

-- Region with a start/stop on the same thread (CPU)
CREATE TABLE IF NOT EXISTS
    "rocpd_region{{upid}}" (
        "id" INTEGER NOT NULL,
        "guid" TEXT NOT NULL,
        "nid" INTEGER NOT NULL,
        "pid" INTEGER NOT NULL,
        "tid" INTEGER NOT NULL,
        "start" BIGINT NOT NULL,
        "end" BIGINT NOT NULL,
        "name_id" INTEGER NOT NULL,
        "event_id" INTEGER,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (nid) REFERENCES rocpd_info_node{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (pid) REFERENCES rocpd_info_process{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (tid) REFERENCES rocpd_info_thread{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (name_id) REFERENCES rocpd_string{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (event_id) REFERENCES rocpd_event{{upid}} (id) ON UPDATE CASCADE,
        PRIMARY KEY (id)
    );

-- Instantaneous sample
CREATE TABLE IF NOT EXISTS
    "rocpd_sample{{upid}}" (
        "id" INTEGER NOT NULL,
        "guid" TEXT NOT NULL,
        "track_id" INTEGER NOT NULL,
        "timestamp" BIGINT NOT NULL,
        "event_id" INTEGER,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (track_id) REFERENCES rocpd_track{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (event_id) REFERENCES rocpd_event{{upid}} (id) ON UPDATE CASCADE,
        PRIMARY KEY (id)
    );

CREATE TABLE IF NOT EXISTS
    "rocpd_kernel_dispatch{{upid}}" (
        "id" INTEGER NOT NULL,
        "guid" TEXT NOT NULL,
        "nid" INTEGER NOT NULL,
        "pid" INTEGER NOT NULL,
        "tid" INTEGER,
        "agent_id" INTEGER NOT NULL,
        "kernel_id" INTEGER NOT NULL,
        "dispatch_id" INTEGER NOT NULL,
        "queue_id" INTEGER NOT NULL,
        "stream_id" INTEGER NOT NULL,
        "start" BIGINT NOT NULL,
        "end" BIGINT NOT NULL,
        "private_segment_size" INTEGER,
        "group_segment_size" INTEGER,
        "workgroup_size_x" INTEGER NOT NULL,
        "workgroup_size_y" INTEGER NOT NULL,
        "workgroup_size_z" INTEGER NOT NULL,
        "grid_size_x" INTEGER NOT NULL,
        "grid_size_y" INTEGER NOT NULL,
        "grid_size_z" INTEGER NOT NULL,
        "region_name_id" INTEGER,
        "event_id" INTEGER,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (nid) REFERENCES rocpd_info_node{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (pid) REFERENCES rocpd_info_process{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (tid) REFERENCES rocpd_info_thread{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (agent_id) REFERENCES rocpd_info_agent{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (kernel_id) REFERENCES rocpd_info_kernel_symbol{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (queue_id) REFERENCES rocpd_info_queue{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (stream_id) REFERENCES rocpd_info_stream{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (region_name_id) REFERENCES rocpd_string{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (event_id) REFERENCES rocpd_event{{upid}} (id) ON UPDATE CASCADE,
        PRIMARY KEY (id)
    );

CREATE TABLE IF NOT EXISTS
    "rocpd_memory_copy{{upid}}" (
        "id" INTEGER NOT NULL,
        "guid" TEXT NOT NULL,
        "nid" INTEGER NOT NULL,
        "pid" INTEGER NOT NULL,
        "tid" INTEGER,
        "start" BIGINT NOT NULL,
        "end" BIGINT NOT NULL,
        "name_id" INTEGER NOT NULL,
        "dst_agent_id" INTEGER,
        "dst_address" INTEGER,
        "src_agent_id" INTEGER,
        "src_address" INTEGER,
        "size" INTEGER NOT NULL,
        "queue_id" INTEGER,
        "stream_id" INTEGER,
        "region_name_id" INTEGER,
        "event_id" INTEGER,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (nid) REFERENCES rocpd_info_node{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (pid) REFERENCES rocpd_info_process{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (tid) REFERENCES rocpd_info_thread{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (name_id) REFERENCES rocpd_string{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (dst_agent_id) REFERENCES rocpd_info_agent{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (src_agent_id) REFERENCES rocpd_info_agent{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (stream_id) REFERENCES rocpd_info_stream{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (queue_id) REFERENCES rocpd_info_queue{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (region_name_id) REFERENCES rocpd_string{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (event_id) REFERENCES rocpd_event{{upid}} (id) ON UPDATE CASCADE,
        PRIMARY KEY (id)
    );

-- Memory allocations (real memory, virtual memory, and scratch memory)
CREATE TABLE IF NOT EXISTS
    "rocpd_memory_allocate{{upid}}" (
        "id" INTEGER PRIMARY KEY AUTOINCREMENT,
        "guid" TEXT NOT NULL,
        "nid" INTEGER NOT NULL,
        "pid" INTEGER NOT NULL,
        "tid" INTEGER,
        "agent_id" INTEGER,
        "type" TEXT CHECK ("type" IN ('ALLOC', 'FREE', 'REALLOC', 'RECLAIM')),
        "level" TEXT CHECK ("level" IN ('REAL', 'VIRTUAL', 'SCRATCH')),
        "start" BIGINT NOT NULL,
        "end" BIGINT NOT NULL,
        "address" INTEGER,
        "size" INTEGER NOT NULL,
        "queue_id" INTEGER,
        "stream_id" INTEGER,
        "event_id" INTEGER,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (nid) REFERENCES rocpd_info_node{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (pid) REFERENCES rocpd_info_process{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (tid) REFERENCES rocpd_info_thread{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (agent_id) REFERENCES rocpd_info_agent{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (stream_id) REFERENCES rocpd_info_stream{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (queue_id) REFERENCES rocpd_info_queue{{upid}} (id) ON UPDATE CASCADE,
        FOREIGN KEY (event_id) REFERENCES rocpd_event{{upid}} (id) ON UPDATE CASCADE
    );

INSERT INTO
    "rocpd_metadata{{upid}}" (tag, value)
VALUES
    ("schema_version", "3"),
    -- ("upid", "{{upid}}"),
    -- foreign key tables (just base names)
    ("fktable{{upid}}", "rocpd_string"),
    ("fktable{{upid}}", "rocpd_info_node"),
    ("fktable{{upid}}", "rocpd_info_process"),
    ("fktable{{upid}}", "rocpd_info_thread"),
    ("fktable{{upid}}", "rocpd_info_agent"),
    ("fktable{{upid}}", "rocpd_info_queue"),
    ("fktable{{upid}}", "rocpd_info_stream"),
    ("fktable{{upid}}", "rocpd_info_pmc"),
    ("fktable{{upid}}", "rocpd_info_code_object"),
    ("fktable{{upid}}", "rocpd_info_kernel_symbol"),
    ("fktable{{upid}}", "rocpd_track"),
    ("fktable{{upid}}", "rocpd_event");
