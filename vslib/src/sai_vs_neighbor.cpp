#include "sai_vs.h"

/**
 * Routine Description:
 *    @brief Create neighbor entry
 *
 * Arguments:
 *    @param[in] neighbor_entry - neighbor entry
 *    @param[in] attr_count - number of attributes
 *    @param[in] attrs - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 *
 * Note: IP address expected in Network Byte Order.
 */
sai_status_t vs_create_neighbor_entry(
        _In_ const sai_neighbor_entry_t* neighbor_entry,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_neighbor_entry(
            neighbor_entry,
            attr_count,
            attr_list,
            &vs_generic_create_neighbor_entry);
}

/**
 * Routine Description:
 *    @brief Remove neighbor entry
 *
 * Arguments:
 *    @param[in] neighbor_entry - neighbor entry
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 *
 * Note: IP address expected in Network Byte Order.
 */
sai_status_t vs_remove_neighbor_entry(
        _In_ const sai_neighbor_entry_t* neighbor_entry)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_neighbor_entry(
            neighbor_entry,
            &vs_generic_remove_neighbor_entry);
}

/**
 * Routine Description:
 *    @brief Set neighbor attribute value
 *
 * Arguments:
 *    @param[in] neighbor_entry - neighbor entry
 *    @param[in] attr - attribute
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_set_neighbor_attribute(
        _In_ const sai_neighbor_entry_t* neighbor_entry,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_neighbor_entry(
            neighbor_entry,
            attr,
            &vs_generic_set_neighbor_entry);
}

/**
 * Routine Description:
 *    @brief Get neighbor attribute value
 *
 * Arguments:
 *    @param[in] neighbor_entry - neighbor entry
 *    @param[in] attr_count - number of attributes
 *    @param[inout] attrs - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_get_neighbor_attribute(
        _In_ const sai_neighbor_entry_t* neighbor_entry,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_neighbor_entry(
            neighbor_entry,
            attr_count,
            attr_list,
            &vs_generic_get_neighbor_entry);
}

/**
 * @brief Remove all neighbor entries
 *
 * @param[in] switch_id Switch id
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t vs_remove_all_neighbor_entries(
        _In_ sai_object_id_t switch_id)
{
    SWSS_LOG_ENTER();

    SWSS_LOG_ERROR("not implemented");

    return SAI_STATUS_NOT_IMPLEMENTED;
}

/**
 * @brief neighbor table methods, retrieved via sai_api_query()
 */
const sai_neighbor_api_t vs_neighbor_api = {
    vs_create_neighbor_entry,
    vs_remove_neighbor_entry,
    vs_set_neighbor_attribute,
    vs_get_neighbor_attribute,
    vs_remove_all_neighbor_entries,
};

