#include "sai_redis.h"

/**
 * @brief Create an ACL table
 *
 * @param[out] acl_table_id The the ACL table id
 * @param[in] attr_count number of attributes
 * @param[in] switch_id Switch Object id
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_create_acl_table(
        _Out_ sai_object_id_t *acl_table_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_ACL_TABLE,
            acl_table_id,
            switch_id,
            attr_count,
            attr_list,
            &redis_generic_create);
}

/**
 * @brief Delete an ACL table
 *
 * @param[in] acl_table_id The ACL table id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_remove_acl_table(
        _In_ sai_object_id_t acl_table_id)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_ACL_TABLE,
            acl_table_id,
            &redis_generic_remove);
}

/**
 * @brief Set ACL table attribute
 *
 * @param[in] acl_table_id The ACL table id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_set_acl_table_attribute(
        _In_ sai_object_id_t acl_table_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_ACL_TABLE,
            acl_table_id,
            attr,
            &redis_generic_set);
}

/**
 * @brief Get ACL table attribute
 *
 * @param[in] acl_table_id ACL table id
 * @param[in] attr_count Number of attributes
 * @param[out] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_get_acl_table_attribute(
        _In_ sai_object_id_t acl_table_id,
        _In_ uint32_t attr_count,
        _Out_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_ACL_TABLE,
            acl_table_id,
            attr_count,
            attr_list,
            &redis_generic_get);
}

/**
 * @brief Create an ACL entry
 *
 * @param[out] acl_entry_id The ACL entry id
 * @param[in] switch_id The Switch Object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_create_acl_entry(
        _Out_ sai_object_id_t *acl_entry_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_ACL_ENTRY,
            acl_entry_id,
            switch_id,
            attr_count,
            attr_list,
            &redis_generic_create);
}

/**
 * @brief Delete an ACL entry
 *
 * @param[in] acl_entry_id The ACL entry id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_remove_acl_entry(
        _In_ sai_object_id_t acl_entry_id)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_ACL_ENTRY,
            acl_entry_id,
            &redis_generic_remove);
}

/**
 * @brief Set ACL entry attribute
 *
 * @param[in] acl_entry_id The ACL entry id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_set_acl_entry_attribute(
        _In_ sai_object_id_t acl_entry_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_ACL_ENTRY,
            acl_entry_id,
            attr,
            &redis_generic_set);
}

/**
 * @brief Get ACL entry attribute
 *
 * @param[in] acl_entry_id ACL entry id
 * @param[in] attr_count Number of attributes
 * @param[out] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_get_acl_entry_attribute(
        _In_ sai_object_id_t acl_entry_id,
        _In_ uint32_t attr_count,
        _Out_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_ACL_ENTRY,
            acl_entry_id,
            attr_count,
            attr_list,
            &redis_generic_get);
}

/**
 * @brief Create an ACL counter
 *
 * @param[out] acl_counter_id The ACL counter id
 * @param[out] switch_id The switch Object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_create_acl_counter(
        _Out_ sai_object_id_t *acl_counter_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_ACL_COUNTER,
            acl_counter_id,
            switch_id,
            attr_count,
            attr_list,
            &redis_generic_create);
}

/**
 * @brief Delete an ACL counter
 *
 * @param[in] acl_counter_id The ACL counter id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_remove_acl_counter(
        _In_ sai_object_id_t acl_counter_id)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_ACL_COUNTER,
            acl_counter_id,
            &redis_generic_remove);
}

/**
 * @brief Set ACL counter attribute
 *
 * @param[in] acl_counter_id The ACL counter id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_set_acl_counter_attribute(
        _In_ sai_object_id_t acl_counter_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_ACL_COUNTER,
            acl_counter_id,
            attr,
            &redis_generic_set);
}

/**
 * @brief Get ACL counter attribute
 *
 * @param[in] acl_counter_id ACL counter id
 * @param[in] attr_count Number of attributes
 * @param[out] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_get_acl_counter_attribute(
        _In_ sai_object_id_t acl_counter_id,
        _In_ uint32_t attr_count,
        _Out_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_ACL_COUNTER,
            acl_counter_id,
            attr_count,
            attr_list,
            &redis_generic_get);
}

/**
 * @brief Create an ACL Range
 *
 * @param[out] acl_range_id The ACL range id
 * @param[in] switch_id The Switch object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_create_acl_range(
        _Out_ sai_object_id_t *acl_range_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_ACL_RANGE,
            acl_range_id,
            switch_id,
            attr_count,
            attr_list,
            &redis_generic_create);
}

/**
 * @brief Remove an ACL Range
 *
 * @param[in] acl_range_id The ACL range id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_remove_acl_range(
        _In_ sai_object_id_t acl_range_id)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_ACL_RANGE,
            acl_range_id,
            &redis_generic_remove);
}

/**
 * @brief Set ACL range attribute
 *
 * @param[in] acl_range_id The ACL range id
 * @param[in] attr Attribute
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_set_acl_range_attribute(
        _In_ sai_object_id_t acl_range_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_ACL_RANGE,
            acl_range_id,
            attr,
            &redis_generic_set);
}

/**
 * @brief Get ACL range attribute
 *
 * @param[in] acl_range_id ACL range id
 * @param[in] attr_count Number of attributes
 * @param[out] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_get_acl_range_attribute(
        _In_ sai_object_id_t acl_range_id,
        _In_ uint32_t attr_count,
        _Out_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_ACL_RANGE,
            acl_range_id,
            attr_count,
            attr_list,
            &redis_generic_get);
}

/**
 * @brief Create an ACL Table Group
 *
 * @param[out] acl_table_group_id The ACL group id
 * @param[in] attr_count number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_create_acl_table_group(
        _Out_ sai_object_id_t *acl_table_group_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_ACL_TABLE_GROUP,
            acl_table_group_id,
            switch_id,
            attr_count,
            attr_list,
            &redis_generic_create);
}

/**
 * @brief Delete an ACL Group
 *
 * @param[in] acl_table_group_id The ACL group id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_remove_acl_table_group(
        _In_ sai_object_id_t acl_table_group_id)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_ACL_TABLE_GROUP,
            acl_table_group_id,
            &redis_generic_remove);
}

/**
 * @brief Set ACL table group attribute
 *
 * @param[in] acl_table_group_id The ACL table group id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_set_acl_table_group_attribute(
        _In_ sai_object_id_t acl_table_group_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_ACL_TABLE_GROUP,
            acl_table_group_id,
            attr,
            &redis_generic_set);
}

/**
 * @brief Get ACL table group attribute
 *
 * @param[in] acl_table_group_id ACL table group id
 * @param[in] attr_count Number of attributes
 * @param[out] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_get_acl_table_group_attribute(
        _In_ sai_object_id_t acl_table_group_id,
        _In_ uint32_t attr_count,
        _Out_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_ACL_TABLE_GROUP,
            acl_table_group_id,
            attr_count,
            attr_list,
            &redis_generic_get);
}

/**
 * @brief Create an ACL Table Group Member
 *
 * @param[out] acl_table_group_member_id The ACL table group member id
 * @param[in] attr_count number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_create_acl_table_group_member(
        _Out_ sai_object_id_t *acl_table_group_member_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER,
            acl_table_group_member_id,
            switch_id,
            attr_count,
            attr_list,
            &redis_generic_create);
}

/**
 * @brief Delete an ACL Group Member
 *
 * @param[in] acl_table_group_member_id The ACL table group member id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_remove_acl_table_group_member(
        _In_ sai_object_id_t acl_table_group_member_id)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER,
            acl_table_group_member_id,
            &redis_generic_remove);
}

/**
 * @brief Set ACL table group member attribute
 *
 * @param[in] acl_table_group_member_id The ACL table group member id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_set_acl_table_group_member_attribute(
        _In_ sai_object_id_t acl_table_group_member_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER,
            acl_table_group_member_id,
            attr,
            &redis_generic_set);
}

/**
 * @brief Get ACL table group member attribute
 *
 * @param[in] acl_table_group_id ACL table group member id
 * @param[in] attr_count Number of attributes
 * @param[out] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_get_acl_table_group_member_attribute(
        _In_ sai_object_id_t acl_table_group_member_id,
        _In_ uint32_t attr_count,
        _Out_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER,
            acl_table_group_member_id,
            attr_count,
            attr_list,
            &redis_generic_get);
}

/**
 * @brief Port methods table retrieved with sai_api_query()
 */
const sai_acl_api_t redis_acl_api = {
    redis_create_acl_table,
    redis_remove_acl_table,
    redis_set_acl_table_attribute,
    redis_get_acl_table_attribute,
    redis_create_acl_entry,
    redis_remove_acl_entry,
    redis_set_acl_entry_attribute,
    redis_get_acl_entry_attribute,
    redis_create_acl_counter,
    redis_remove_acl_counter,
    redis_set_acl_counter_attribute,
    redis_get_acl_counter_attribute,
    redis_create_acl_range,
    redis_remove_acl_range,
    redis_set_acl_range_attribute,
    redis_get_acl_range_attribute,
    redis_create_acl_table_group,
    redis_remove_acl_table_group,
    redis_set_acl_table_group_attribute,
    redis_get_acl_table_group_attribute,
    redis_create_acl_table_group_member,
    redis_remove_acl_table_group_member,
    redis_set_acl_table_group_member_attribute,
    redis_get_acl_table_group_member_attribute,
};
