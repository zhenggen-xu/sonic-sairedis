#include "sai_redis.h"

/**
 * Routine Description:
 *    @brief Create next hop group
 *
 * Arguments:
 *    @param[out] next_hop_group_id - next hop group id
 *    @param[in] attr_count - number of attributes
 *    @param[in] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t redis_create_next_hop_group(
        _Out_ sai_object_id_t* next_hop_group_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_NEXT_HOP_GROUP,
            next_hop_group_id,
            switch_id,
            attr_count,
            attr_list,
            &redis_generic_create);
}

/**
 * Routine Description:
 *    @brief Remove next hop group
 *
 * Arguments:
 *    @param[in] next_hop_group_id - next hop group id
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t redis_remove_next_hop_group(
        _In_ sai_object_id_t next_hop_group_id)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_NEXT_HOP_GROUP,
            next_hop_group_id,
            &redis_generic_remove);
}

/**
 * Routine Description:
 *    @brief Set Next Hop Group attribute
 *
 * Arguments:
 *    @param[in] sai_object_id_t - next_hop_group_id
 *    @param[in] attr - attribute
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t redis_set_next_hop_group_attribute(
        _In_ sai_object_id_t next_hop_group_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_NEXT_HOP_GROUP,
            next_hop_group_id,
            attr,
            &redis_generic_set);
}

/**
 * Routine Description:
 *    @brief Get Next Hop Group attribute
 *
 * Arguments:
 *    @param[in] sai_object_id_t - next_hop_group_id
 *    @param[in] attr_count - number of attributes
 *    @param[inout] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t redis_get_next_hop_group_attribute(
        _In_ sai_object_id_t next_hop_group_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_NEXT_HOP_GROUP,
            next_hop_group_id,
            attr_count,
            attr_list,
            &redis_generic_get);
}

/**
 * @brief Create next hop group member
 *
 * @param[out] next_hop_group_member_id - next hop group member id
 * @param[in] attr_count - number of attributes
 * @param[in] attr_list - array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_create_next_hop_group_member(
        _Out_ sai_object_id_t* next_hop_group_member_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_NEXT_HOP_GROUP_MEMBER,
            next_hop_group_member_id,
            switch_id,
            attr_count,
            attr_list,
            &redis_generic_create);
}

/**
 * @brief Remove next hop group member
 *
 * @param[in] next_hop_group_member_id - next hop group member id
 *
 * @return SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_remove_next_hop_group_member(
        _In_ sai_object_id_t next_hop_group_member_id)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_NEXT_HOP_GROUP_MEMBER,
            next_hop_group_member_id,
            &redis_generic_remove);
}

/**
 * @brief Set Next Hop Group attribute
 *
 * @param[in] sai_object_id_t - next_hop_group_member_id
 * @param[in] attr - attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_set_next_hop_group_member_attribute(
        _In_ sai_object_id_t next_hop_group_member_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_NEXT_HOP_GROUP_MEMBER,
            next_hop_group_member_id,
            attr,
            &redis_generic_set);
}

/**
 * @brief Get Next Hop Group attribute
 *
 * @param[in] sai_object_id_t - next_hop_group_member_id
 * @param[in] attr_count - number of attributes
 * @param[inout] attr_list - array of attributes
 *
 * @return SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_get_next_hop_group_member_attribute(
        _In_ sai_object_id_t next_hop_group_member_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_NEXT_HOP_GROUP_MEMBER,
            next_hop_group_member_id,
            attr_count,
            attr_list,
            &redis_generic_get);
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

sai_status_t redis_bulk_object_create_next_hop_group_members(
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t object_count,
        _In_ const uint32_t *attr_count,
        _In_ const sai_attribute_t **attrs,
        _In_ sai_bulk_op_type_t type,
        _Out_ sai_object_id_t *object_id,
        _Out_ sai_status_t *object_statuses)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

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

sai_status_t redis_bulk_object_remove_next_hop_group_members(
        _In_ uint32_t object_count,
        _In_ const sai_object_id_t *object_id,
        _In_ sai_bulk_op_type_t type,
        _Out_ sai_status_t *object_statuses)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    SWSS_LOG_ERROR("not implemented");

    return SAI_STATUS_NOT_IMPLEMENTED;
}

/**
 * @brief Next Hop methods table retrieved with sai_api_query()
 */
const sai_next_hop_group_api_t redis_next_hop_group_api = {
    redis_create_next_hop_group,
    redis_remove_next_hop_group,
    redis_set_next_hop_group_attribute,
    redis_get_next_hop_group_attribute,
    redis_create_next_hop_group_member,
    redis_remove_next_hop_group_member,
    redis_set_next_hop_group_member_attribute,
    redis_get_next_hop_group_member_attribute,
    redis_bulk_object_create_next_hop_group_members,
    redis_bulk_object_remove_next_hop_group_members,
};
