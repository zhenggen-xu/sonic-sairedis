#include "sai_vs.h"

/**
 * Routine Description:
 *    @brief Create UDF
 *
 * Arguments:
 *    @param[out] udf_id - UDF id
 *    @param[in] attr_count - number of attributes
 *    @param[in] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 *
 */
sai_status_t vs_create_udf(
        _Out_ sai_object_id_t* udf_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_UDF,
            udf_id,
            switch_id,
            attr_count,
            attr_list,
            &vs_generic_create);
}

/**
 * Routine Description:
 *    @brief Remove UDF
 *
 * Arguments:
 *    @param[in] udf_id - UDF id
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_remove_udf(
        _In_ sai_object_id_t udf_id)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_UDF,
            udf_id,
            &vs_generic_remove);
}

/**
 * Routine Description:
 *    @brief Set UDF attribute
 *
 * Arguments:
 *    @param[in] udf_id - UDF id
 *    @param[in] attr - attribute
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_set_udf_attribute(
        _In_ sai_object_id_t udf_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_UDF,
            udf_id,
            attr,
            &vs_generic_set);
}

/**
 * Routine Description:
 *    @brief Get UDF attribute value
 *
 * Arguments:
 *    @param[in] udf_id - UDF id
 *    @param[in] attr_count - number of attributes
 *    @param[inout] attrs - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_get_udf_attribute(
        _In_ sai_object_id_t udf_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_UDF,
            udf_id,
            attr_count,
            attr_list,
            &vs_generic_get);
}

/**
 * Routine Description:
 *    @brief Create UDF match
 *
 * Arguments:
 *    @param[out] udf_match_id - UDF match id
 *    @param[in] attr_count - number of attributes
 *    @param[in] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 *
 */
sai_status_t vs_create_udf_match(
        _Out_ sai_object_id_t* udf_match_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_UDF_MATCH,
            udf_match_id,
            switch_id,
            attr_count,
            attr_list,
            &vs_generic_create);
}

/**
 * Routine Description:
 *    @brief Remove UDF match
 *
 * Arguments:
 *    @param[in] udf_match_id - UDF match id
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_remove_udf_match(
        _In_ sai_object_id_t udf_match_id)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_UDF_MATCH,
            udf_match_id,
            &vs_generic_remove);
}

/**
 * Routine Description:
 *    @brief Set UDF match attribute
 *
 * Arguments:
 *    @param[in] udf_match_id - UDF match id
 *    @param[in] attr - attribute
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_set_udf_match_attribute(
        _In_ sai_object_id_t udf_match_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_UDF_MATCH,
            udf_match_id,
            attr,
            &vs_generic_set);
}

/**
 * Routine Description:
 *    @brief Get UDF match attribute value
 *
 * Arguments:
 *    @param[in] udf_match_id - UDF match id
 *    @param[in] attr_count - number of attributes
 *    @param[inout] attrs - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_get_udf_match_attribute(
        _In_ sai_object_id_t udf_match_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_UDF_MATCH,
            udf_match_id,
            attr_count,
            attr_list,
            &vs_generic_get);
}

/**
 * Routine Description:
 *    @brief Create UDF group
 *
 * Arguments:
 *    @param[out] udf_group_id - UDF group id
 *    @param[in] attr_count - number of attributes
 *    @param[in] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 *
 */
sai_status_t vs_create_udf_group(
        _Out_ sai_object_id_t* udf_group_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_UDF_GROUP,
            udf_group_id,
            switch_id,
            attr_count,
            attr_list,
            &vs_generic_create);
}

/**
 * Routine Description:
 *    @brief Remove UDF group
 *
 * Arguments:
 *    @param[in] udf_group_id - UDF group id
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_remove_udf_group(
        _In_ sai_object_id_t udf_group_id)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_UDF_GROUP,
            udf_group_id,
            &vs_generic_remove);
}

/**
 * Routine Description:
 *    @brief Set UDF group attribute
 *
 * Arguments:
 *    @param[in] udf_group_id - UDF group id
 *    @param[in] attr - attribute
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_set_udf_group_attribute(
        _In_ sai_object_id_t udf_group_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_UDF_GROUP,
            udf_group_id,
            attr,
            &vs_generic_set);
}

/**
 * Routine Description:
 *    @brief Get UDF group attribute value
 *
 * Arguments:
 *    @param[in] udf_group_id - UDF group id
 *    @param[in] attr_count - number of attributes
 *    @param[inout] attrs - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t vs_get_udf_group_attribute(
        _In_ sai_object_id_t udf_group_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_UDF_GROUP,
            udf_group_id,
            attr_count,
            attr_list,
            &vs_generic_get);
}

/**
 * @brief UDF methods, retrieved via sai_api_query()
 */
const sai_udf_api_t vs_udf_api = {
    vs_create_udf,
    vs_remove_udf,
    vs_set_udf_attribute,
    vs_get_udf_attribute,

    vs_create_udf_match,
    vs_remove_udf_match,
    vs_set_udf_match_attribute,
    vs_get_udf_match_attribute,

    vs_create_udf_group,
    vs_remove_udf_group,
    vs_set_udf_group_attribute,
    vs_get_udf_group_attribute,
};
