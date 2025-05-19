CREATE VIEW IF NOT EXISTS
    "rocpd_metadata{{view_upid}}" AS
SELECT
    *
FROM
    "rocpd_metadata{{upid}}";

CREATE VIEW IF NOT EXISTS
    "rocpd_string{{view_upid}}" AS
SELECT
    *
FROM
    "rocpd_string{{upid}}";

CREATE VIEW IF NOT EXISTS
    "rocpd_info_node{{view_upid}}" AS
SELECT
    *
FROM
    "rocpd_info_node{{upid}}";

CREATE VIEW IF NOT EXISTS
    "rocpd_info_process{{view_upid}}" AS
SELECT
    *
FROM
    "rocpd_info_process{{upid}}";

CREATE VIEW IF NOT EXISTS
    "rocpd_info_thread{{view_upid}}" AS
SELECT
    *
FROM
    "rocpd_info_thread{{upid}}";

CREATE VIEW IF NOT EXISTS
    "rocpd_info_agent{{view_upid}}" AS
SELECT
    *
FROM
    "rocpd_info_agent{{upid}}";

CREATE VIEW IF NOT EXISTS
    "rocpd_info_queue{{view_upid}}" AS
SELECT
    *
FROM
    "rocpd_info_queue{{upid}}";

CREATE VIEW IF NOT EXISTS
    "rocpd_info_stream{{view_upid}}" AS
SELECT
    *
FROM
    "rocpd_info_stream{{upid}}";

CREATE VIEW IF NOT EXISTS
    "rocpd_info_pmc{{view_upid}}" AS
SELECT
    *
FROM
    "rocpd_info_pmc{{upid}}";

CREATE VIEW IF NOT EXISTS
    "rocpd_info_code_object{{view_upid}}" AS
SELECT
    *
FROM
    "rocpd_info_code_object{{upid}}";

CREATE VIEW IF NOT EXISTS
    "rocpd_info_kernel_symbol{{view_upid}}" AS
SELECT
    *
FROM
    "rocpd_info_kernel_symbol{{upid}}";

CREATE VIEW IF NOT EXISTS
    "rocpd_track{{view_upid}}" AS
SELECT
    *
FROM
    "rocpd_track{{upid}}";

CREATE VIEW IF NOT EXISTS
    "rocpd_event{{view_upid}}" AS
SELECT
    *
FROM
    "rocpd_event{{upid}}";

CREATE VIEW IF NOT EXISTS
    "rocpd_arg{{view_upid}}" AS
SELECT
    *
FROM
    "rocpd_arg{{upid}}";

CREATE VIEW IF NOT EXISTS
    "rocpd_pmc_event{{view_upid}}" AS
SELECT
    *
FROM
    "rocpd_pmc_event{{upid}}";

CREATE VIEW IF NOT EXISTS
    "rocpd_region{{view_upid}}" AS
SELECT
    *
FROM
    "rocpd_region{{upid}}";

CREATE VIEW IF NOT EXISTS
    "rocpd_sample{{view_upid}}" AS
SELECT
    *
FROM
    "rocpd_sample{{upid}}";

CREATE VIEW IF NOT EXISTS
    "rocpd_kernel_dispatch{{view_upid}}" AS
SELECT
    *
FROM
    "rocpd_kernel_dispatch{{upid}}";

CREATE VIEW IF NOT EXISTS
    "rocpd_memory_copy{{view_upid}}" AS
SELECT
    *
FROM
    "rocpd_memory_copy{{upid}}";

CREATE VIEW IF NOT EXISTS
    "rocpd_memory_allocate{{view_upid}}" AS
SELECT
    *
FROM
    "rocpd_memory_allocate{{upid}}";
