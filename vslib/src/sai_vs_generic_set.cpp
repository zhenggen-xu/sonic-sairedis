#include "sai_vs.h"
#include "sai_vs_state.h"

sai_status_t internal_vs_generic_set(
        _In_ sai_object_type_t object_type,
        _In_ const std::string &serialized_object_id,
        _In_ sai_object_id_t switch_id,
        _In_ const sai_attribute_t *attr)
{
    SWSS_LOG_ENTER();

    auto & objectHash = g_switch_state_map.at(switch_id)->objectHash;

    auto it = objectHash.find(serialized_object_id);

    if (it == objectHash.end())
    {
        SWSS_LOG_ERROR("Set failed, object not found, object type: %s: id: %s",
                sai_serialize_object_type(object_type).c_str(),
                serialized_object_id.c_str());

        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    std::vector<swss::FieldValueTuple> values = SaiAttributeList::serialize_attr_list(
            object_type,
            1,
            attr,
            false);

    AttrHash &attrHash = it->second;

    const std::string &str_attr_id = fvField(values[0]);
    const std::string &str_attr_value = fvValue(values[0]);

    // sai set have only one attribute
    attrHash[str_attr_id] = str_attr_value;

    SWSS_LOG_DEBUG("Set succeeded, object type: %s, id: %s",
                sai_serialize_object_type(object_type).c_str(),
                serialized_object_id.c_str());

    return SAI_STATUS_SUCCESS;
}

sai_status_t vs_generic_set(
        _In_ sai_object_type_t object_type,
        _In_ sai_object_id_t object_id,
        _In_ const sai_attribute_t *attr)
{
    SWSS_LOG_ENTER();

    std::string str_object_id = sai_serialize_object_id(object_id);

    sai_object_id_t switch_id = sai_switch_id_query(object_id);

    return internal_vs_generic_set(
            object_type,
            str_object_id,
            switch_id,
            attr);
}

sai_status_t vs_generic_set_fdb_entry(
        _In_ const sai_fdb_entry_t *fdb_entry,
        _In_ const sai_attribute_t *attr)
{
    SWSS_LOG_ENTER();

    std::string str_fdb_entry = sai_serialize_fdb_entry(*fdb_entry);

    return internal_vs_generic_set(
            SAI_OBJECT_TYPE_FDB_ENTRY,
            str_fdb_entry,
            fdb_entry->switch_id,
            attr);
}

sai_status_t vs_generic_set_neighbor_entry(
        _In_ const sai_neighbor_entry_t* neighbor_entry,
        _In_ const sai_attribute_t *attr)
{
    SWSS_LOG_ENTER();

    std::string str_neighbor_entry = sai_serialize_neighbor_entry(*neighbor_entry);

    return internal_vs_generic_set(
            SAI_OBJECT_TYPE_NEIGHBOR_ENTRY,
            str_neighbor_entry,
            neighbor_entry->switch_id,
            attr);
}

sai_status_t vs_generic_set_route_entry(
        _In_ const sai_route_entry_t* route_entry,
        _In_ const sai_attribute_t *attr)
{
    SWSS_LOG_ENTER();

    std::string str_route_entry = sai_serialize_route_entry(*route_entry);

    return internal_vs_generic_set(
            SAI_OBJECT_TYPE_ROUTE_ENTRY,
            str_route_entry,
            route_entry->switch_id,
            attr);
}
