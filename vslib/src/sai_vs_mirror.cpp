#include "sai_vs.h"

/**
 * @brief Create mirror session.
 *
 * @param[out] session_id Port mirror session id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Value of attributes
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *   error code is returned.
 */
sai_status_t vs_create_mirror_session(
        _Out_ sai_object_id_t *session_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_MIRROR_SESSION,
            session_id,
            switch_id,
            attr_count,
            attr_list,
            &vs_generic_create);
}

/**
 * @brief Remove mirror session.
 *
 * @param[in] session_id Port mirror session id
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *   error code is returned.
 */
sai_status_t vs_remove_mirror_session(
        _In_ sai_object_id_t session_id)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_MIRROR_SESSION,
            session_id,
            &vs_generic_remove);
}

/**
 * @brief Set mirror session attributes.
 *
 * @param[in] session_id Port mirror session id
 * @param[in] attr Value of attribute
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *   error code is returned.
 */
sai_status_t vs_set_mirror_session_attribute(
        _In_ sai_object_id_t session_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_MIRROR_SESSION,
            session_id,
            attr,
            &vs_generic_set);
}

/**
 * @brief Get mirror session attributes.
 *
 * @param[in] session_id Port mirror session id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Value of attribute
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *   error code is returned.
 */
sai_status_t vs_get_mirror_session_attribute(
        _In_ sai_object_id_t session_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_MIRROR_SESSION,
            session_id,
            attr_count,
            attr_list,
            &vs_generic_get);
}

/**
 * @brief MIRROR method table retrieved with sai_api_query()
 */
const sai_mirror_api_t vs_mirror_api = {
    vs_create_mirror_session,
    vs_remove_mirror_session,
    vs_set_mirror_session_attribute,
    vs_get_mirror_session_attribute,
};
