--
-- Useful views
--

-- Processes
CREATE VIEW IF NOT EXISTS
    "processes{{view_upid}}" AS
SELECT
    N.id AS nid,
    N.machine_id,
    N.system_name,
    N.hostname,
    N.release AS system_release,
    N.version AS system_version,
    P.guid,
    P.ppid,
    P.id AS pid,
    P.init,
    P.start,
    P.end,
    P.fini,
    P.command
FROM
    rocpd_info_process{{view_upid}} P
    INNER JOIN rocpd_info_node{{view_upid}} N ON N.id = P.nid AND N.guid = P.guid;

-- Threads
CREATE VIEW IF NOT EXISTS
    "threads{{view_upid}}" AS
SELECT
    N.id AS nid,
    N.machine_id,
    N.system_name,
    N.hostname,
    N.release AS system_release,
    N.version AS system_version,
    P.guid,
    P.ppid,
    P.id AS pid,
    T.id AS tid,
    T.start,
    T.end,
    T.name
FROM
    rocpd_info_thread{{view_upid}} T
    INNER JOIN rocpd_info_process{{view_upid}} P ON P.id = T.pid AND N.guid = T.guid
    INNER JOIN rocpd_info_node{{view_upid}} N ON N.id = T.nid AND N.guid = T.guid;

-- CPU regions
CREATE VIEW IF NOT EXISTS
    "regions{{view_upid}}" AS
SELECT
    R.id,
    R.guid,
    (
        SELECT
            string
        FROM
            rocpd_string{{view_upid}} RS
        WHERE
            RS.id = E.category_id AND RS.guid = E.guid
    ) AS category,
    S.string AS name,
    R.nid,
    R.pid,
    R.tid,
    R.start,
    R.end,
    (R.end - R.start) AS duration,
    R.event_id,
    E.stack_id,
    E.parent_stack_id,
    E.correlation_id AS corr_id,
    E.extdata,
    E.call_stack,
    E.line_info
FROM
    rocpd_region{{view_upid}} R
    INNER JOIN rocpd_event{{view_upid}} E ON E.id = R.event_id AND E.guid = R.guid
    INNER JOIN rocpd_string{{view_upid}} S ON S.id = R.name_id AND S.guid = R.guid;

--
-- Samples
CREATE VIEW IF NOT EXISTS
    "samples{{view_upid}}" AS
SELECT
    R.id,
    R.guid,
    (
        SELECT
            string
        FROM
            rocpd_string{{view_upid}} RS
        WHERE
            RS.id = E.category_id AND RS.guid = E.guid
    ) AS category,
    (
        SELECT
            string
        FROM
            rocpd_string{{view_upid}} RS
        WHERE
            RS.id = T.name_id AND RS.guid = T.guid
    ) AS name,
    T.nid,
    T.pid,
    T.tid,
    R.timestamp,
    E.stack_id AS stack_id,
    E.parent_stack_id AS parent_stack_id,
    E.correlation_id AS corr_id,
    E.extdata AS extdata,
    E.call_stack AS call_stack,
    E.line_info AS line_info
FROM
    rocpd_sample{{view_upid}} R
    INNER JOIN rocpd_track{{view_upid}} T ON T.id = R.track_id AND T.guid = R.guid
    INNER JOIN rocpd_event{{view_upid}} E ON E.id = R.event_id AND E.guid = R.guid;

--
-- Kernel information
CREATE VIEW
    "kernels{{view_upid}}" AS
SELECT
    K.id,
    K.guid,
    (
        SELECT
            string
        FROM
            rocpd_string{{view_upid}} RS
        WHERE
            RS.id = E.category_id AND RS.guid = E.guid
    ) AS category,
    R.string AS region,
    S.display_name AS name,
    K.nid,
    Q.pid,
    A.absolute_index AS agent_index,
    A.type_index AS gpu_index,
    S.code_object_id AS code_object_id,
    K.kernel_id,
    K.dispatch_id,
    K.stream_id,
    K.queue_id,
    Q.name AS queue,
    ST.name AS stream,
    K.start,
    K.end,
    (K.end - K.start) AS duration,
    K.grid_size_x AS grid_x,
    K.grid_size_y AS grid_y,
    K.grid_size_z AS grid_z,
    K.workgroup_size_x AS workgroup_x,
    K.workgroup_size_y AS workgroup_y,
    K.workgroup_size_z AS workgroup_z,
    K.group_segment_size AS lds_size,
    K.private_segment_size AS scratch_size,
    S.group_segment_size AS static_lds_size,
    S.private_segment_size AS static_scratch_size,
    E.parent_stack_id
FROM
    rocpd_kernel_dispatch{{view_upid}} K
    INNER JOIN rocpd_info_agent{{view_upid}} A ON A.id = K.agent_id AND A.guid = K.guid
    INNER JOIN rocpd_event{{view_upid}} E ON E.id = K.event_id AND E.guid = K.guid
    INNER JOIN rocpd_string{{view_upid}} R ON R.id = K.region_name_id AND R.guid = K.guid
    INNER JOIN rocpd_info_kernel_symbol{{view_upid}} S ON S.id = K.kernel_id AND S.guid = K.guid
    LEFT JOIN rocpd_info_stream{{view_upid}} ST ON ST.id = K.stream_id AND ST.guid = K.guid
    LEFT JOIN rocpd_info_queue{{view_upid}} Q ON Q.id = K.queue_id AND Q.guid = K.guid;

--
-- Performance Monitoring Counters (PMC)
CREATE VIEW IF NOT EXISTS
    "pmc_events{{view_upid}}" AS
SELECT
    PMC_E.id,
    E.id AS event_id,
    (
        SELECT
            string
        FROM
            rocpd_string{{view_upid}} RS
        WHERE
            RS.id = E.category_id AND RS.guid = E.guid
    ) AS category,
    (
        SELECT
            display_name
        FROM
            rocpd_info_kernel_symbol{{view_upid}} KS
        WHERE
            KS.id = K.kernel_id AND KS.guid = K.guid
    ) AS name,
    K.nid,
    K.dispatch_id,
    K.start,
    K.end,
    (K.end - K.start) AS duration,
    PMC_I.name AS counter_name,
    PMC_E.value AS counter_value
FROM
    rocpd_pmc_event{{view_upid}} PMC_E
    INNER JOIN rocpd_info_pmc{{view_upid}} PMC_I ON PMC_I.id = PMC_E.pmc_id AND PMC_I.guid = PMC_E.guid
    INNER JOIN rocpd_event{{view_upid}} E ON E.id = PMC_E.event_id AND E.guid = PMC_E.guid
    INNER JOIN rocpd_kernel_dispatch{{view_upid}} K ON K.event_id = PMC_E.event_id AND K.guid = PMC_E.guid;

-- events with arguments ---
CREATE VIEW IF NOT EXISTS
    "events_args{{view_upid}}" AS
SELECT
    E.id AS event_id,
    (
        SELECT
            string
        FROM
            rocpd_string{{view_upid}} RS
        WHERE
            RS.id = E.category_id AND RS.guid = E.guid
    ) AS category,
    E.stack_id,
    E.parent_stack_id,
    E.correlation_id,
    A.position AS arg_position,
    A.type AS arg_type,
    A.name AS arg_name,
    A.value AS arg_value,
    E.call_stack,
    E.line_info,
    A.extdata
FROM
    rocpd_event{{view_upid}} E
    INNER JOIN rocpd_arg{{view_upid}} A ON A.event_id = E.id AND A.guid = E.guid;

-- list of astream arguments enriched by the corresponding stream descriptions
CREATE VIEW IF NOT EXISTS
    "stream_args{{view_upid}}" AS
SELECT
    A.id AS argument_id,
    A.event_id AS event_id,
    A.position AS arg_position,
    A.type AS arg_type,
    A.value AS arg_value,
    JSON_EXTRACT(A.extdata, '$.stream_id') AS stream_id,
    S.nid,
    S.pid,
    S.name AS stream_name,
    S.extdata AS extdata
FROM
    rocpd_arg{{view_upid}} A
    INNER JOIN rocpd_info_stream{{view_upid}} S ON JSON_EXTRACT(A.extdata, '$.stream_id') = S.id AND A.guid = S.guid
WHERE
    A.name = 'stream';

--
--
CREATE VIEW IF NOT EXISTS
    "memory_copies{{view_upid}}" AS
SELECT
    M.id,
    M.guid,
    (
        SELECT
            string
        FROM
            rocpd_string{{view_upid}} RS
        WHERE
            RS.id = E.category_id AND RS.guid = E.guid
    ) AS category,
    M.nid,
    M.pid,
    M.tid,
    M.start,
    M.end,
    (M.end - M.start) AS duration,
    S.string AS name,
    R.string AS region_name,
    M.stream_id,
    M.queue_id,
    ST.name AS stream_name,
    Q.name AS queue_name,
    M.size,
    dst_agent.name AS dst_device,
    dst_agent.absolute_index AS dst_agent_index,
    M.dst_address,
    src_agent.name AS src_device,
    src_agent.absolute_index AS src_agent_index,
    M.src_address,
    E.parent_stack_id
FROM
    rocpd_memory_copy{{view_upid}} M
    INNER JOIN rocpd_string{{view_upid}} S ON S.id = M.name_id AND S.guid = M.guid
    LEFT JOIN rocpd_string{{view_upid}} R ON R.id = M.region_name_id AND R.guid = M.guid
    INNER JOIN rocpd_info_agent{{view_upid}} dst_agent ON dst_agent.id = M.dst_agent_id AND dst_agent.guid = M.guid
    INNER JOIN rocpd_info_agent{{view_upid}} src_agent ON src_agent.id = M.src_agent_id AND src_agent.guid = M.guid
    LEFT JOIN rocpd_info_queue{{view_upid}} Q ON Q.id = M.queue_id AND Q.guid = M.guid
    LEFT JOIN rocpd_info_stream{{view_upid}} ST ON ST.id = M.stream_id AND ST.guid = M.guid
    INNER JOIN rocpd_event{{view_upid}} E ON E.id = M.event_id AND E.guid = M.guid;

--
--
CREATE VIEW IF NOT EXISTS
    "memory_allocations{{view_upid}}" AS
SELECT
    M.id,
    M.guid,
    (
        SELECT
            string
        FROM
            rocpd_string{{view_upid}} RS
        WHERE
            RS.id = E.category_id AND RS.guid = E.guid
    ) AS category,
    M.nid,
    M.pid,
    M.tid,
    M.start,
    M.end,
    (M.end - M.start) AS duration,
    M.type,
    M.level,
    A.name AS agent_name,
    A.absolute_index AS agent_index,
    M.address,
    M.size,
    M.queue_id,
    Q.name AS queue_name,
    M.stream_id,
    ST.name AS stream_name,
    E.parent_stack_id
FROM
    rocpd_memory_allocate{{view_upid}} M
    LEFT JOIN rocpd_info_agent{{view_upid}} A ON M.agent_id = A.id AND M.guid = A.guid
    LEFT JOIN rocpd_info_queue{{view_upid}} Q ON Q.id = M.queue_id AND Q.guid = M.guid
    LEFT JOIN rocpd_info_stream{{view_upid}} ST ON ST.id = M.stream_id AND ST.guid = M.guid
    INNER JOIN rocpd_event{{view_upid}} E ON E.id = M.event_id AND E.guid = M.guid;