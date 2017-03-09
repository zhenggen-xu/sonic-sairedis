#include "sai_vs.h"

/**
 * Routine Description:
 *    @brief Create FDB entry
 *
 * Arguments:
 *    @param[in] fdb_entry - fdb entry
 *    @param[in] attr_count - number of attributes
 *    @param[in] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_create_fdb_entry(
        _In_ const sai_fdb_entry_t *fdb_entry,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_fdb_entry(
            fdb_entry,
            attr_count,
            attr_list,
            &vs_generic_create_fdb_entry);
}

/**
 * Routine Description:
 *    @brief Remove FDB entry
 *
 * Arguments:
 *    @param[in] fdb_entry - fdb entry
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_remove_fdb_entry(
        _In_ const sai_fdb_entry_t* fdb_entry)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_fdb_entry(
            fdb_entry,
            &vs_generic_remove_fdb_entry);
}

/**
 * Routine Description:
 *    @brief Set fdb entry attribute value
 *
 * Arguments:
 *    @param[in] fdb_entry - fdb entry
 *    @param[in] attr - attribute
 * * Return Values: *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_set_fdb_entry_attribute(
        _In_ const sai_fdb_entry_t* fdb_entry,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_fdb_entry(
            fdb_entry,
            attr,
            &vs_generic_set_fdb_entry);
}

/**
 * Routine Description:
 *    @brief Get fdb entry attribute value
 *
 * Arguments:
 *    @param[in] fdb_entry - fdb entry
 *    @param[in] attr_count - number of attributes
 *    @param[inout] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_get_fdb_entry_attribute(
        _In_ const sai_fdb_entry_t* fdb_entry,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_fdb_entry(
            fdb_entry,
            attr_count,
            attr_list,
            &vs_generic_get_fdb_entry);
}

/*
 * @brief Remove all FDB entries by attribute set in sai_fdb_flush_attr
 *
 * @param[in] switch_id Switch object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t vs_flush_fdb_entries(
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    SWSS_LOG_ERROR("not implemented");

    return SAI_STATUS_NOT_IMPLEMENTED;
}

/**
 * @brief FDB method table retrieved with sai_api_query()
 */
const sai_fdb_api_t vs_fdb_api = {
    vs_create_fdb_entry,
    vs_remove_fdb_entry,
    vs_set_fdb_entry_attribute,
    vs_get_fdb_entry_attribute,
    vs_flush_fdb_entries,
};
