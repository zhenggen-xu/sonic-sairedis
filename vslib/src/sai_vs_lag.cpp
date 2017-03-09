#include "sai_vs.h"

/**
 * Routine Description:
 *    @brief Create LAG
 *
 * Arguments:
 *    @param[out] lag_id - LAG id
 *    @param[in] attr_count - number of attributes
 *    @param[in] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_create_lag(
        _Out_ sai_object_id_t* lag_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_LAG,
            lag_id,
            switch_id,
            attr_count,
            attr_list,
            &vs_generic_create);
}

/**
 * Routine Description:
 *    @brief Remove LAG
 *
 * Arguments:
 *    @param[in] lag_id - LAG id
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_remove_lag(
        _In_ sai_object_id_t lag_id)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_LAG,
            lag_id,
            &vs_generic_remove);
}

/**
 * Routine Description:
 *    @brief Set LAG attribute
 *
 * Arguments:
 *    @param[in] lag_id - LAG id
 *    @param[in] attr - Structure containing ID and value to be set
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_set_lag_attribute(
        _In_ sai_object_id_t lag_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_LAG,
            lag_id,
            attr,
            &vs_generic_set);
}

/**
 * Routine Description:
 *    @brief Get LAG attribute
 *
 * Arguments:
 *    @param[in] lag_id - LAG id
 *    @param[in] attr_count - number of attributes
 *    @param[inout] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_get_lag_attribute(
        _In_ sai_object_id_t lag_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_LAG,
            lag_id,
            attr_count,
            attr_list,
            &vs_generic_get);
}

/**
 * Routine Description:
 *    @brief Create LAG member
 *
 * Arguments:
 *    @param[out] lag_member_id - LAG member id
 *    @param[in] attr_count - number of attributes
 *    @param[in] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_create_lag_member(
        _Out_ sai_object_id_t* lag_member_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_LAG_MEMBER,
            lag_member_id,
            switch_id,
            attr_count,
            attr_list,
            &vs_generic_create);
}

/**
 * Routine Description:
 *    @brief Remove LAG member
 *
 * Arguments:
 *    @param[in] lag_member_id - lag member id
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_remove_lag_member(
        _In_ sai_object_id_t lag_member_id)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_LAG_MEMBER,
            lag_member_id,
            &vs_generic_remove);
}

/**
 * Routine Description:
 *    @brief Set LAG member attribute
 *
 * Arguments:
 *    @param[in] lag_member_id - LAG member id
 *    @param[in] attr - attribute
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_set_lag_member_attribute(
        _In_ sai_object_id_t lag_member_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_LAG_MEMBER,
            lag_member_id,
            attr,
            &vs_generic_set);
}

/**
 * Routine Description:
 *    @brief Get LAG attribute
 *
 * Arguments:
 *    @param[in] lag_member_id - LAG member id
 *    @param[in] attr_count - number of attributes
 *    @param[inout] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_get_lag_member_attribute(
        _In_ sai_object_id_t lag_member_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_LAG_MEMBER,
            lag_member_id,
            attr_count,
            attr_list,
            &vs_generic_get);
}

/**
 * @brief Bulk objects creation.
 *
 * @param[in] switch_id SAI Switch object id
 * @param[in] object_count Number of objects to create
 * @param[in] attr_count List of attr_count. Caller passes the number
 *    of attribute for each object to create.
 * @param[in] attrs List of attributes for every object.
 * @param[in] type bulk operation type.
 *
 * @param[out] object_id List of object ids returned
 * @param[out] object_statuses List of status for every object. Caller needs to allocate the buffer.
 *
 * @return #SAI_STATUS_SUCCESS on success when all objects are created or #SAI_STATUS_FAILURE when
 * any of the objects fails to create. When there is failure, Caller is expected to go through the
 * list of returned statuses to find out which fails and which succeeds.
 */

sai_status_t vs_bulk_object_create_lag(
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t object_count,
        _In_ const uint32_t *attr_count,
        _In_ const sai_attribute_t **attrs,
        _In_ sai_bulk_op_type_t type,
        _Out_ sai_object_id_t *object_id,
        _Out_ sai_status_t *object_statuses)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    SWSS_LOG_ERROR("not implemented");

    return SAI_STATUS_NOT_IMPLEMENTED;
}

/**
 * @brief Bulk objects removal.
 *
 * @param[in] object_count Number of objects to create
 * @param[in] object_id List of object ids
 * @param[in] type bulk operation type.
 * @param[out] object_statuses List of status for every object. Caller needs to allocate the buffer.
 *
 * @return #SAI_STATUS_SUCCESS on success when all objects are removed or #SAI_STATUS_FAILURE when
 * any of the objects fails to remove. When there is failure, Caller is expected to go through the
 * list of returned statuses to find out which fails and which succeeds.
 */

sai_status_t vs_bulk_object_remove_lag(
        _In_ uint32_t object_count,
        _In_ const sai_object_id_t *object_id,
        _In_ sai_bulk_op_type_t type,
        _Out_ sai_status_t *object_statuses)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    SWSS_LOG_ERROR("not implemented");

    return SAI_STATUS_NOT_IMPLEMENTED;
}

/**
 * @brief LAG methods table retrieved with sai_api_query()
 */
const sai_lag_api_t vs_lag_api = {
    vs_create_lag,
    vs_remove_lag,
    vs_set_lag_attribute,
    vs_get_lag_attribute,
    vs_create_lag_member,
    vs_remove_lag_member,
    vs_set_lag_member_attribute,
    vs_get_lag_member_attribute,
    vs_bulk_object_create_lag,
    vs_bulk_object_remove_lag,
};
