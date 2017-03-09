#include "sai_redis.h"

/**
 * @brief Create host interface trap group
 *
 * @param[out] hostif_trap_group_id Host interface trap group id
 * @param[in] switch_id Switch object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t  redis_create_hostif_trap_group(
        _Out_ sai_object_id_t *hostif_trap_group_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP,
            hostif_trap_group_id,
            switch_id,
            attr_count,
            attr_list,
            &redis_generic_create);
}

/**
 * @brief Remove host interface trap group
 *
 * @param[in] hostif_trap_group_id Host interface trap group id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t  redis_remove_hostif_trap_group(
        _In_ sai_object_id_t hostif_trap_group_id)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP,
            hostif_trap_group_id,
            &redis_generic_remove);
}

/**
 * @brief Set host interface trap group attribute value.
 *
 * @param[in] hostif_trap_group_id Host interface trap group id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t  redis_set_hostif_trap_group_attribute(
        _In_ sai_object_id_t hostif_trap_group_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP,
            hostif_trap_group_id,
            attr,
            &redis_generic_set);
}

/**
 * @brief get host interface trap group attribute value.
 *
 * @param[in] hostif_trap_group_id Host interface trap group id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_get_hostif_trap_group_attribute(
        _In_ sai_object_id_t hostif_trap_group_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP,
            hostif_trap_group_id,
            attr_count,
            attr_list,
            &redis_generic_get);
}

/**
 * @brief Create host interface trap
 *
 * @param[out] hostif_trap_id Host interface trap id
 * @param[in] switch_id Switch object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t  redis_create_hostif_trap(
        _Out_ sai_object_id_t *hostif_trap_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_HOSTIF_TRAP,
            hostif_trap_id,
            switch_id,
            attr_count,
            attr_list,
            &redis_generic_create);
}

/**
 * @brief Remove host interface trap
 *
 * @param[in] hostif_trap_id Host interface trap id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t  redis_remove_hostif_trap(
        _In_ sai_object_id_t hostif_trap_id)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_HOSTIF_TRAP,
            hostif_trap_id,
            &redis_generic_remove);
}

/**
 * @brief Set trap attribute value.
 *
 * @param[in] hostif_trap_id Host interface trap id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_set_hostif_trap_attribute(
        _In_ sai_object_id_t hostif_trap_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_HOSTIF_TRAP,
            hostif_trap_id,
            attr,
            &redis_generic_set);
}

/**
 * @brief Get trap attribute value.
 *
 * @param[in] hostif_trap_id Host interface trap id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_get_hostif_trap_attribute(
        _In_ sai_object_id_t hostif_trap_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_HOSTIF_TRAP,
            hostif_trap_id,
            attr_count,
            attr_list,
            &redis_generic_get);
}

/**
 * @brief Create host interface user defined trap
 *
 * @param[out] hostif_user_defined_trap_id Host interface user defined trap id
 * @param[in] switch_id Switch object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t  redis_create_hostif_user_defined_trap(
        _Out_ sai_object_id_t *hostif_user_defined_trap_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_HOSTIF_USER_DEFINED_TRAP,
            hostif_user_defined_trap_id,
            switch_id,
            attr_count,
            attr_list,
            &redis_generic_create);
}

/**
 * @brief Remove host interface user defined trap
 *
 * @param[in] hostif_user_defined_trap_id Host interface user defined trap id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t  redis_remove_hostif_user_defined_trap(
        _In_ sai_object_id_t hostif_user_defined_trap_id)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_HOSTIF_USER_DEFINED_TRAP,
            hostif_user_defined_trap_id,
            &redis_generic_remove);
}

/**
 * @brief Set user defined trap attribute value.
 *
 * @param[in] hostif_user_defined_trap_id Host interface user defined trap id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_set_hostif_user_defined_trap_attribute(
        _In_ sai_object_id_t hostif_user_defined_trap_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_HOSTIF_USER_DEFINED_TRAP,
            hostif_user_defined_trap_id,
            attr,
            &redis_generic_set);
}

/**
 * @brief Get user defined trap attribute value.
 *
 * @param[in] hostif_user_defined_trap_id Host interface user defined trap id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_get_hostif_user_defined_trap_attribute(
        _In_ sai_object_id_t hostif_user_defined_trap_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_HOSTIF_USER_DEFINED_TRAP,
            hostif_user_defined_trap_id,
            attr_count,
            attr_list,
            &redis_generic_get);
}

/**
 * @brief Create host interface
 *
 * @param[out] hif_id Host interface id
 * @param[in] switch_id Switch object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Aarray of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_create_hostif(
        _Out_ sai_object_id_t *hif_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_HOSTIF,
            hif_id,
            switch_id,
            attr_count,
            attr_list,
            &redis_generic_create);
}

/**
 * @brief Remove host interface
 *
 * @param[in] hif_id Host interface id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_remove_hostif(
        _In_ sai_object_id_t hif_id)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_HOSTIF,
            hif_id,
            &redis_generic_remove);
}

/**
 * @brief Set host interface attribute
 *
 * @param[in] hif_id Host interface id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_set_hostif_attribute(
        _In_ sai_object_id_t hif_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_HOSTIF,
            hif_id,
            attr,
            &redis_generic_set);
}

/**
 * @brief Get host interface attribute
 *
 * @param[in] hif_id Host interface id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_get_hostif_attribute(
        _In_ sai_object_id_t hif_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_HOSTIF,
            hif_id,
            attr_count,
            attr_list,
            &redis_generic_get);
}

/**
 * @brief Create host interface table entry
 *
 * @param[out] hif_table_entry Host interface table entry
 * @param[in] switch_id Switch object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Aarray of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_create_hostif_table_entry(
        _Out_ sai_object_id_t *hif_table_entry,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_HOSTIF_TABLE_ENTRY,
            hif_table_entry,
            switch_id,
            attr_count,
            attr_list,
            &redis_generic_create);
}

/**
 * @brief Remove host interface table entry
 *
 * @param[in] hif_table_entry - host interface table entry
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_remove_hostif_table_entry(
        _In_ sai_object_id_t hif_table_entry)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_HOSTIF_TABLE_ENTRY,
            hif_table_entry,
            &redis_generic_remove);
}

/**
 * @brief Set host interface table entry attribute
 *
 * @param[in] hif_table_entry - host interface table entry
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_set_hostif_table_entry_attribute(
        _In_ sai_object_id_t hif_table_entry,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_HOSTIF_TABLE_ENTRY,
            hif_table_entry,
            attr,
            &redis_generic_set);
}

/**
 * @brief Get host interface table entry attribute
 *
 * @param[in] hif_table_entry - host interface table entry
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_get_hostif_table_entry_attribute(
        _In_ sai_object_id_t hif_table_entry,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_HOSTIF_TABLE_ENTRY,
            hif_table_entry,
            attr_count,
            attr_list,
            &redis_generic_get);
}

/**
 * @brief Hostif receive function
 *
 * @param[in] hif_id Host interface id
 * @param[out] buffer Packet buffer
 * @param[inout] buffer_size Allocated buffer size [in], actual packet size in bytes [out]
 * @param[inout] attr_count Allocated list size [in], number of attributes [out]
 * @param[out] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success #SAI_STATUS_BUFFER_OVERFLOW if
 * buffer_size is insufficient, and buffer_size will be filled with required
 * size. Or if attr_count is insufficient, and attr_count will be filled with
 * required count. Failure status code on error
 */
sai_status_t redis_recv_hostif_packet(
        _In_ sai_object_id_t hif_id,
        _Out_ void *buffer,
        _Inout_ sai_size_t *buffer_size,
        _Inout_ uint32_t *attr_count,
        _Out_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    SWSS_LOG_ERROR("not implemented");

    return SAI_STATUS_NOT_IMPLEMENTED;
}

/**
 * @brief Hostif send function
 *
 * @param[in] hif_id Host interface id.
 * When sending through FD channel, fill SAI_OBJECT_TYPE_HOST_INTERFACE object, of type #SAI_HOSTIF_TYPE_FD.
 * When sending through CB channel, fill Switch Object ID, SAI_OBJECT_TYPE_SWITCH.
 * @param[in] buffer Packet buffer
 * @param[in] buffer size Packet size in bytes
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_send_hostif_packet(
        _In_ sai_object_id_t hif_id,
        _In_ void *buffer,
        _In_ sai_size_t buffer_size,
        _In_ uint32_t attr_count,
        _In_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    SWSS_LOG_ERROR("not implemented");

    return SAI_STATUS_NOT_IMPLEMENTED;
}

/**
 * @brief hostif methods table retrieved with sai_api_query()
 */
const sai_hostif_api_t redis_hostif_api = {
    redis_create_hostif,
    redis_remove_hostif,
    redis_set_hostif_attribute,
    redis_get_hostif_attribute,
    redis_create_hostif_table_entry,
    redis_remove_hostif_table_entry,
    redis_set_hostif_table_entry_attribute,
    redis_get_hostif_table_entry_attribute,
    redis_create_hostif_trap_group,
    redis_remove_hostif_trap_group,
    redis_set_hostif_trap_group_attribute,
    redis_get_hostif_trap_group_attribute,
    redis_create_hostif_trap,
    redis_remove_hostif_trap,
    redis_set_hostif_trap_attribute,
    redis_get_hostif_trap_attribute,
    redis_create_hostif_user_defined_trap,
    redis_remove_hostif_user_defined_trap,
    redis_set_hostif_user_defined_trap_attribute,
    redis_get_hostif_user_defined_trap_attribute,
    redis_recv_hostif_packet,
    redis_send_hostif_packet,
};
