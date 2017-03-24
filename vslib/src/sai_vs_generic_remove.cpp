#include "sai_vs.h"
#include "sai_vs_state.h"
#include "sai_vs_switch_BCM56850.h"

sai_status_t internal_vs_generic_remove(
        _In_ sai_object_type_t object_type,
        _In_ const std::string &serialized_object_id,
        _In_ sai_object_id_t switch_id)
{
    SWSS_LOG_ENTER();

    auto & objectHash = g_switch_state_map.at(switch_id)->objectHash;

    auto it = objectHash.find(serialized_object_id);

    if (it == objectHash.end())
    {
        SWSS_LOG_ERROR("Remove failed, object not found, object type: %d: id: %s", object_type, serialized_object_id.c_str());

        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    objectHash.erase(it);

    SWSS_LOG_DEBUG("Remove succeeded, object type: %d, id: %s", object_type, serialized_object_id.c_str());

    return SAI_STATUS_SUCCESS;
}

sai_status_t vs_generic_remove(
        _In_ sai_object_type_t object_type,
        _In_ sai_object_id_t object_id)
{
    SWSS_LOG_ENTER();

    std::string str_object_id = sai_serialize_object_id(object_id);

    sai_object_id_t switch_id = sai_switch_id_query(object_id);

    sai_status_t status = internal_vs_generic_remove(
            object_type,
            str_object_id,
            switch_id);

    if (object_type == SAI_OBJECT_TYPE_SWITCH &&
            status == SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_NOTICE("removing switch id %s", sai_serialize_object_id(object_id).c_str());

        vs_free_real_object_id(object_id);

        uninit_switch_BCM56850(object_id);
    }

    return status;
}

sai_status_t vs_generic_remove_fdb_entry(
        _In_ const sai_fdb_entry_t* fdb_entry)
{
    SWSS_LOG_ENTER();

    std::string str_fdb_entry = sai_serialize_fdb_entry(*fdb_entry);

    return internal_vs_generic_remove(
            SAI_OBJECT_TYPE_FDB_ENTRY,
            str_fdb_entry,
            fdb_entry->switch_id);
}

sai_status_t vs_generic_remove_neighbor_entry(
        _In_ const sai_neighbor_entry_t* neighbor_entry)
{
    SWSS_LOG_ENTER();

    std::string str_neighbor_entry = sai_serialize_neighbor_entry(*neighbor_entry);

    return internal_vs_generic_remove(
            SAI_OBJECT_TYPE_NEIGHBOR_ENTRY,
            str_neighbor_entry,
            neighbor_entry->switch_id);
}

sai_status_t vs_generic_remove_route_entry(
        _In_ const sai_route_entry_t* route_entry)
{
    SWSS_LOG_ENTER();

    std::string str_route_entry = sai_serialize_route_entry(*route_entry);

    return internal_vs_generic_remove(
            SAI_OBJECT_TYPE_ROUTE_ENTRY,
            str_route_entry,
            route_entry->switch_id);
}
