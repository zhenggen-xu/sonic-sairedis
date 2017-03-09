#include "sai_vs.h"

/**
 * Routine Description:
 *    @brief Create Route
 *
 * Arguments:
 *    @param[in] route_entry - route entry
 *    @param[in] attr_count - number of attributes
 *    @param[in] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 *
 * Note: IP prefix/mask expected in Network Byte Order.
 *
 */
sai_status_t vs_create_route(
        _In_ const sai_route_entry_t* route_entry,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_route_entry(
            route_entry,
            attr_count,
            attr_list,
            &vs_generic_create_route_entry);
}

/**
 * Routine Description:
 *    @brief Remove Route
 *
 * Arguments:
 *    @param[in] route_entry - route entry
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 *
 * Note: IP prefix/mask expected in Network Byte Order.
 */
sai_status_t vs_remove_route(
        _In_ const sai_route_entry_t* route_entry)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_route_entry(
            route_entry,
            &vs_generic_remove_route_entry);
}

/**
 * Routine Description:
 *    @brief Set route attribute value
 *
 * Arguments:
 *    @param[in] route_entry - route entry
 *    @param[in] attr - attribute
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_set_route_attribute(
        _In_ const sai_route_entry_t* route_entry,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_route_entry(
            route_entry,
            attr,
            &vs_generic_set_route_entry);
}

/**
 * Routine Description:
 *    @brief Get route attribute value
 *
 * Arguments:
 *    @param[in] route_entry - route entry
 *    @param[in] attr_count - number of attributes
 *    @param[inout] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_get_route_attribute(
        _In_ const sai_route_entry_t* route_entry,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_route_entry(
            route_entry,
            attr_count,
            attr_list,
            &vs_generic_get_route_entry);
}

/**
 * @brief Router entry methods table retrieved with sai_api_query()
 */
const sai_route_api_t vs_route_api = {
    vs_create_route,
    vs_remove_route,
    vs_set_route_attribute,
    vs_get_route_attribute,
};
