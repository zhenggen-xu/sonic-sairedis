#include "sai_vs.h"

/**
 * Routine Description:
 *    @brief Create next hop
 *
 * Arguments:
 *    @param[out] next_hop_id - next hop id
 *    @param[in] attr_count - number of attributes
 *    @param[in] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 *
 * Note: IP address expected in Network Byte Order.
 */
sai_status_t vs_create_next_hop(
        _Out_ sai_object_id_t* next_hop_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_NEXT_HOP,
            next_hop_id,
            switch_id,
            attr_count,
            attr_list,
            &vs_generic_create);
}

/**
 * Routine Description:
 *    @brief Remove next hop
 *
 * Arguments:
 *    @param[in] next_hop_id - next hop id
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_remove_next_hop(
        _In_ sai_object_id_t next_hop_id)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_NEXT_HOP,
            next_hop_id,
            &vs_generic_remove);
}

/**
 * Routine Description:
 *    @brief Set Next Hop attribute
 *
 * Arguments:
 *    @param[in] next_hop_id - next hop id
 *    @param[in] attr - attribute
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_set_next_hop_attribute(
        _In_ sai_object_id_t next_hop_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_NEXT_HOP,
            next_hop_id,
            attr,
            &vs_generic_set);
}

/**
 * Routine Description:
 *    @brief Get Next Hop attribute
 *
 * Arguments:
 *    @param[in] next_hop_id - next hop id
 *    @param[in] attr_count - number of attributes
 *    @param[inout] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_get_next_hop_attribute(
        _In_ sai_object_id_t next_hop_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_NEXT_HOP,
            next_hop_id,
            attr_count,
            attr_list,
            &vs_generic_get);
}

/**
 * @brief Next Hop methods table retrieved with sai_api_query()
 */
const sai_next_hop_api_t vs_next_hop_api = {
    vs_create_next_hop,
    vs_remove_next_hop,
    vs_set_next_hop_attribute,
    vs_get_next_hop_attribute,
};
