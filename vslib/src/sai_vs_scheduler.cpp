#include "sai_vs.h"

/**
 * @brief Create Scheduler Profile
 *
 * @param[out] scheduler_id Scheduler id
 * @param[in] attr_count number of attributes
 * @param[in] attr_list array of attributes
 *
 * @return SAI_STATUS_SUCCESS on success
 *          Failure status code on error
 */
sai_status_t vs_create_scheduler_profile(
        _Out_ sai_object_id_t *scheduler_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_SCHEDULER,
            scheduler_id,
            switch_id,
            attr_count,
            attr_list,
            &vs_generic_create);
}

/**
 * @brief Remove Scheduler profile
 *
 * @param[in] scheduler_id Scheduler id
 *
 * @return SAI_STATUS_SUCCESS on success
 *          Failure status code on error
 */
sai_status_t vs_remove_scheduler_profile(
        _In_ sai_object_id_t scheduler_id)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_SCHEDULER,
            scheduler_id,
            &vs_generic_remove);
}

/**
 * @brief Set Scheduler Attribute
 *
 * @param[in] scheduler_id Scheduler id
 * @param[in] attr attribute to set
 *
 * @return SAI_STATUS_SUCCESS on success
 *          Failure status code on error
 */
sai_status_t vs_set_scheduler_attribute(
        _In_ sai_object_id_t scheduler_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_SCHEDULER,
            scheduler_id,
            attr,
            &vs_generic_set);
}

/**
 * @brief Get Scheduler attribute
 *
 * @param[in] scheduler_id - scheduler id
 * @param[in] attr_count - number of attributes
 * @param[inout] attr_list - array of attributes
 *
 * @return SAI_STATUS_SUCCESS on success
 *        Failure status code on error
 */

sai_status_t vs_get_scheduler_attribute(
        _In_ sai_object_id_t scheduler_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_SCHEDULER,
            scheduler_id,
            attr_count,
            attr_list,
            &vs_generic_get);
}

/**
 * @brief Scheduler methods table retrieved with sai_api_query()
 */
const sai_scheduler_api_t vs_scheduler_api = {
    vs_create_scheduler_profile,
    vs_remove_scheduler_profile,
    vs_set_scheduler_attribute,
    vs_get_scheduler_attribute,
};
