CREATE TABLE IF NOT EXISTS
    "rocpd_metadata" (
        "id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
        "tag" TEXT NOT NULL,
        "value" TEXT NOT NULL
    );

CREATE TABLE IF NOT EXISTS
    "rocpd_string" (
        "id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
        "string" TEXT NOT NULL UNIQUE ON CONFLICT IGNORE
    );

CREATE TABLE IF NOT EXISTS
    "_rocpd_node" (
        "id" INTEGER NOT NULL,
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
    "_rocpd_process" (
        "id" INTEGER NOT NULL,
        "node_id" INTEGER NOT NULL,
        "parent_pid" INTEGER,
        "init" BIGINT,
        "fini" BIGINT,
        "start" BIGINT,
        "end" BIGINT,
        "command" TEXT,
        "environment" JSONB DEFAULT "{}" NOT NULL,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (node_id) REFERENCES _rocpd_node (id),
        PRIMARY KEY (id, node_id)
    );

CREATE TABLE IF NOT EXISTS
    "_rocpd_thread" (
        "id" INTEGER NOT NULL,
        "node_id" INTEGER NOT NULL,
        "process_id" INTEGER NOT NULL,
        "name" TEXT,
        "start" BIGINT,
        "end" BIGINT,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (node_id) REFERENCES _rocpd_node (id),
        FOREIGN KEY (process_id) REFERENCES rocpd_process (id),
        PRIMARY KEY (id, process_id, node_id)
    );

CREATE TABLE IF NOT EXISTS
    "rocpd_agent" (
        "id" INTEGER NOT NULL,
        "node_id" INTEGER NOT NULL,
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
        FOREIGN KEY (node_id) REFERENCES _rocpd_node (id),
        PRIMARY KEY (id)
    );

CREATE TABLE IF NOT EXISTS
    "rocpd_queue" (
        "id" INTEGER NOT NULL,
        "node_id" INTEGER NOT NULL,
        "pid" INTEGER NOT NULL,
        "name" TEXT,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (node_id) REFERENCES _rocpd_node (id),
        PRIMARY KEY (id)
    );

CREATE TABLE IF NOT EXISTS
    "rocpd_stream" (
        "id" INTEGER NOT NULL,
        "node_id" INTEGER NOT NULL,
        "pid" INTEGER NOT NULL,
        "name" TEXT,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (node_id) REFERENCES _rocpd_node (id),
        PRIMARY KEY (id)
    );

-- Performance monitoring counters (PMC) descriptions
CREATE TABLE IF NOT EXISTS
    "rocpd_pmc" (
        "id" INTEGER NOT NULL,
        "target_arch" TEXT CHECK ("target_arch" IN ('CPU', 'GPU')),
        "agent_id" INTEGER,
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
        PRIMARY KEY (id, agent_id)
    );

CREATE TABLE IF NOT EXISTS
    "rocpd_code_object" (
        "id" INTEGER NOT NULL,
        "node_id" INTEGER NOT NULL,
        "agent_id" INTEGER,
        "uri" TEXT,
        "load_base" BIGINT,
        "load_size" BIGINT,
        "load_delta" BIGINT,
        "storage_type" TEXT CHECK ("storage_type" IN ('FILE', 'MEMORY')),
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (node_id) REFERENCES _rocpd_node (id),
        FOREIGN KEY (agent_id) REFERENCES rocpd_agent (id),
        PRIMARY KEY (id)
    );

CREATE TABLE IF NOT EXISTS
    "rocpd_kernel_symbol" (
        "id" INTEGER NOT NULL,
        "node_id" INTEGER NOT NULL,
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
        FOREIGN KEY (node_id) REFERENCES _rocpd_node (id),
        FOREIGN KEY (code_object_id) REFERENCES rocpd_code_object (id),
        PRIMARY KEY (id)
    );

-- Stores repetitive info for samples
CREATE TABLE IF NOT EXISTS
    "_rocpd_track" (
        "id" INTEGER NOT NULL,
        "node_id" INTEGER NOT NULL,
        "pid" INTEGER,
        "tid" INTEGER,
        "name_id" INTEGER NOT NULL,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (node_id) REFERENCES _rocpd_node (id),
        FOREIGN KEY (name_id) REFERENCES rocpd_string (id),
        PRIMARY KEY (id)
    );

-- Storage for a region, instant, and counter
CREATE TABLE IF NOT EXISTS
    "rocpd_event" (
        "id" INTEGER NOT NULL,
        "category_id" INTEGER,
        "correlation_id" INTEGER,
        "stack_id" INTEGER,
        "parent_stack_id" INTEGER,
        "args" JSONB DEFAULT "[]" NOT NULL, -- TODO this must be removed once rocpd_arg is setlled -- 
        "metrics" JSONB DEFAULT "{}" NOT NULL,
        "call_stack" JSONB DEFAULT "{}" NOT NULL,
        "line_info" JSONB DEFAULT "{}" NOT NULL,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (category_id) REFERENCES rocpd_string (id),
        PRIMARY KEY (id)
    );

-- stores arguments for events
CREATE TABLE IF NOT EXISTS
    "rocpd_arg" (
        "id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
        "event_id" INTEGER NOT NULL,
        "position" INTEGER NOT NULL,
        "type" TEXT NOT NULL,
        "name" TEXT NOT NULL,
        "value" TEXT, -- TODO: discuss make it value_id and integer, refer to string table -- 
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (event_id) REFERENCES rocpd_event (id)
    );

-- Region with a start/stop on the same thread (CPU)
CREATE TABLE IF NOT EXISTS
    "rocpd_pmc_event" (
        "id" INTEGER NOT NULL,
        "event_id" INTEGER,
        "pmc_id" INTEGER NOT NULL,
        "value" REAL DEFAULT 0.0,
        "extdata" JSONB DEFAULT "{}",
        FOREIGN KEY (pmc_id) REFERENCES rocpd_pmc (id),
        FOREIGN KEY (event_id) REFERENCES rocpd_event (id),
        PRIMARY KEY (id, event_id)
    );

-- Region with a start/stop on the same thread (CPU)
CREATE TABLE IF NOT EXISTS
    "_rocpd_region" (
        "id" INTEGER NOT NULL,
        "node_id" INTEGER NOT NULL,
        "pid" INTEGER NOT NULL,
        "tid" INTEGER NOT NULL,
        "start" BIGINT NOT NULL,
        "end" BIGINT NOT NULL,
        "name_id" INTEGER NOT NULL,
        "event_id" INTEGER,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (node_id) REFERENCES _rocpd_node (id),
        FOREIGN KEY (name_id) REFERENCES rocpd_string (id),
        FOREIGN KEY (event_id) REFERENCES rocpd_event (id),
        PRIMARY KEY (id)
    );

-- Instantaneous sample
CREATE TABLE IF NOT EXISTS
    "_rocpd_sample" (
        "id" INTEGER NOT NULL,
        "track_id" INTEGER NOT NULL,
        "timestamp" BIGINT NOT NULL,
        "event_id" INTEGER,
        "extdata" JSONB DEFAULT "{}" NOT NULL,
        FOREIGN KEY (track_id) REFERENCES _rocpd_track (id),
        FOREIGN KEY (event_id) REFERENCES rocpd_event (id),
        PRIMARY KEY (id)
    );

CREATE TABLE IF NOT EXISTS
    "_rocpd_kernel_dispatch" (
        "id" INTEGER NOT NULL,
        "node_id" INTEGER NOT NULL,
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
        FOREIGN KEY (node_id) REFERENCES _rocpd_node (id),
        FOREIGN KEY (agent_id) REFERENCES rocpd_agent (id),
        FOREIGN KEY (kernel_id) REFERENCES rocpd_kernel_symbol (id),
        FOREIGN KEY (queue_id) REFERENCES rocpd_queue (id),
        FOREIGN KEY (stream_id) REFERENCES rocpd_stream (id),
        FOREIGN KEY (region_name_id) REFERENCES rocpd_string (id),
        FOREIGN KEY (event_id) REFERENCES rocpd_event (id),
        PRIMARY KEY (id)
    );

CREATE TABLE IF NOT EXISTS
    "_rocpd_memory_copy" (
        "id" INTEGER NOT NULL,
        "node_id" INTEGER NOT NULL,
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
        FOREIGN KEY (node_id) REFERENCES _rocpd_node (id),
        FOREIGN KEY (name_id) REFERENCES rocpd_string (id),
        FOREIGN KEY (dst_agent_id) REFERENCES rocpd_agent (id),
        FOREIGN KEY (src_agent_id) REFERENCES rocpd_agent (id),
        FOREIGN KEY (stream_id) REFERENCES rocpd_stream (id),
        FOREIGN KEY (queue_id) REFERENCES rocpd_queue (id),
        FOREIGN KEY (region_name_id) REFERENCES rocpd_string (id),
        FOREIGN KEY (event_id) REFERENCES rocpd_event (id),
        PRIMARY KEY (id)
    );

-- Memory allocations (real memory, virtual memory, and scratch memory)
CREATE TABLE IF NOT EXISTS
    "_rocpd_memory_allocate" (
        "id" INTEGER PRIMARY KEY AUTOINCREMENT,
        "node_id" INTEGER NOT NULL,
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
        FOREIGN KEY (node_id) REFERENCES _rocpd_node (id),
        FOREIGN KEY (agent_id) REFERENCES rocpd_agent (id),
        FOREIGN KEY (stream_id) REFERENCES rocpd_stream (id),
        FOREIGN KEY (queue_id) REFERENCES rocpd_queue (id),
        FOREIGN KEY (event_id) REFERENCES rocpd_event (id)
    );

INSERT INTO
    "rocpd_metadata" (tag, value)
VALUES
    ("schema_version", "3");