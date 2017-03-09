#include "sai_vs.h"
#include "sai_vs_state.h"

bool g_switch_initialized = false;

sai_status_t vs_initialize_switch()
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    if (g_switch_initialized)
    {
        SWSS_LOG_ERROR("already initialized");

        return SAI_STATUS_FAILURE;
    }

    sai_status_t status = init_switch();

    if (status == SAI_STATUS_SUCCESS)
    {
        g_switch_initialized = true;
    }

    return status;
}

void vs_shutdown_switch(
        _In_ bool warm_restart_hint)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    if (!g_switch_initialized)
    {
        SWSS_LOG_ERROR("not initialized");

        return;
    }

    g_switch_initialized = false;
}

sai_status_t vs_create_switch(
        _Out_ sai_object_id_t* switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_SWITCH,
            switch_id,
            SAI_NULL_OBJECT_ID, // no switch id since we create switch
            attr_count,
            attr_list,
            &vs_generic_create);
}

sai_status_t vs_remove_switch(
        _In_ sai_object_id_t switch_id)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_SWITCH,
            switch_id,
            &vs_generic_remove);
}

sai_status_t vs_set_switch_attribute(
        _In_ sai_object_id_t switch_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_SWITCH,
            switch_id,
            attr,
            &vs_generic_set);
}

sai_status_t vs_get_switch_attribute(
        _In_ sai_object_id_t switch_id,
        _In_ sai_uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::recursive_mutex> lock(g_recursive_mutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_SWITCH,
            switch_id,
            attr_count,
            attr_list,
            &vs_generic_get);
}

const sai_switch_api_t vs_switch_api = {
    vs_create_switch,
    vs_remove_switch,
    vs_set_switch_attribute,
    vs_get_switch_attribute,
};
