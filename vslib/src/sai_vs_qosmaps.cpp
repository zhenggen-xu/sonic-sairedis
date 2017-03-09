#include "sai_vs.h"

/**
 * @brief Create Qos Map
 *
 * @param[out] qos_map_id Qos Map Id
 * @param[in] attr_count number of attributes
 * @param[in] attr_list array of attributes
 *
 * @return SAI_STATUS_SUCCESS on success
 *          Failure status code on error
 */
sai_status_t vs_create_qos_map(
        _Out_ sai_object_id_t* qos_map_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_QOS_MAP,
            qos_map_id,
            switch_id,
            attr_count,
            attr_list,
            &vs_generic_create);
}

/**
 * @brief Remove Qos Map
 *
 * @param[in] qos_map_id Qos Map id to be removed.
 *
 * @return SAI_STATUS_SUCCESS on success
 *           Failure status code on error
 */
sai_status_t vs_remove_qos_map (
        _In_ sai_object_id_t qos_map_id)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_QOS_MAP,
            qos_map_id,
            &vs_generic_remove);
}

/**
 * @brief Set attributes for qos map
 *
 * @param[in] qos_map_id Qos Map Id
 * @param[in] attr attribute to set
 *
 * @return SAI_STATUS_SUCCESS on success
 *          Failure status code on error
 */
sai_status_t vs_set_qos_map_attribute(
        _In_ sai_object_id_t qos_map_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_QOS_MAP,
            qos_map_id,
            attr,
            &vs_generic_set);
}

/**
 * @brief Get attrbutes of qos map
 *
 * @param[in] qos_map_id map id
 * @param[in] attr_count number of attributes
 * @param[inout] attr_list array of attributes
 *
 * @return SAI_STATUS_SUCCESS on success
 *        Failure status code on error
 */
sai_status_t vs_get_qos_map_attribute(
        _In_ sai_object_id_t qos_map_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_QOS_MAP,
            qos_map_id,
            attr_count,
            attr_list,
            &vs_generic_get);
}

/**
 * Qos Map methods table retrieved with sai_api_query()
 */
const sai_qos_map_api_t vs_qos_map_api = {
    vs_create_qos_map,
    vs_remove_qos_map,
    vs_set_qos_map_attribute,
    vs_get_qos_map_attribute,
};
