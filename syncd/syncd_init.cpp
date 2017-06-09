#include <map>
#include "syncd.h"

create_fn           common_create[SAI_OBJECT_TYPE_MAX];
remove_fn           common_remove[SAI_OBJECT_TYPE_MAX];
set_attribute_fn    common_set_attribute[SAI_OBJECT_TYPE_MAX];
get_attribute_fn    common_get_attribute[SAI_OBJECT_TYPE_MAX];

extern const std::map<std::string, sai_api_t> saiApiMap = {
    { "SAI_API_SWITCH", SAI_API_SWITCH },
    { "SAI_API_PORT", SAI_API_PORT },
    { "SAI_API_FDB", SAI_API_FDB },
    { "SAI_API_VLAN", SAI_API_VLAN },
    { "SAI_API_VIRTUAL_ROUTER", SAI_API_VIRTUAL_ROUTER },
    { "SAI_API_ROUTE", SAI_API_ROUTE },
    { "SAI_API_NEXT_HOP", SAI_API_NEXT_HOP },
    { "SAI_API_NEXT_HOP_GROUP", SAI_API_NEXT_HOP_GROUP },
    { "SAI_API_ROUTER_INTERFACE", SAI_API_ROUTER_INTERFACE },
    { "SAI_API_NEIGHBOR", SAI_API_NEIGHBOR },
    { "SAI_API_ACL", SAI_API_ACL },
    { "SAI_API_HOSTIF", SAI_API_HOSTIF },
    { "SAI_API_MIRROR", SAI_API_MIRROR },
    { "SAI_API_SAMPLEPACKET", SAI_API_SAMPLEPACKET },
    { "SAI_API_STP", SAI_API_STP },
    { "SAI_API_LAG", SAI_API_LAG },
    { "SAI_API_POLICER", SAI_API_POLICER },
    { "SAI_API_WRED", SAI_API_WRED },
    { "SAI_API_QOS_MAP", SAI_API_QOS_MAP },
    { "SAI_API_QUEUE", SAI_API_QUEUE },
    { "SAI_API_SCHEDULER", SAI_API_SCHEDULER },
    { "SAI_API_SCHEDULER_GROUP", SAI_API_SCHEDULER_GROUP },
    { "SAI_API_BUFFER", SAI_API_BUFFER },
    { "SAI_API_HASH", SAI_API_HASH },
    { "SAI_API_UDF", SAI_API_UDF },
    { "SAI_API_TUNNEL", SAI_API_TUNNEL },
};

sai_acl_api_t                *sai_acl_api;
sai_buffer_api_t             *sai_buffer_api;
sai_fdb_api_t                *sai_fdb_api;
sai_hash_api_t               *sai_hash_api;
sai_hostif_api_t             *sai_hostif_api;
sai_lag_api_t                *sai_lag_api;
sai_mirror_api_t             *sai_mirror_api;
sai_neighbor_api_t           *sai_neighbor_api;
sai_next_hop_api_t           *sai_next_hop_api;
sai_next_hop_group_api_t     *sai_next_hop_group_api;
sai_policer_api_t            *sai_policer_api;
sai_port_api_t               *sai_port_api;
sai_qos_map_api_t            *sai_qos_map_api;
sai_queue_api_t              *sai_queue_api;
sai_route_api_t              *sai_route_api;
sai_router_interface_api_t   *sai_router_interface_api;
sai_samplepacket_api_t       *sai_samplepacket_api;
sai_scheduler_api_t          *sai_scheduler_api;
sai_scheduler_group_api_t    *sai_scheduler_group_api;
sai_stp_api_t                *sai_stp_api;
sai_switch_api_t             *sai_switch_api;
sai_tunnel_api_t             *sai_tunnel_api;
sai_udf_api_t                *sai_udf_api;
sai_virtual_router_api_t     *sai_router_api;
sai_vlan_api_t               *sai_vlan_api;
sai_wred_api_t               *sai_wred_api;

void initialize_common_api_pointers()
{
    SWSS_LOG_ENTER();

    // TODO populate using metadata

    common_create[SAI_OBJECT_TYPE_NULL] = NULL;
}

void populate_sai_apis()
{
    SWSS_LOG_ENTER();

    // TODO do query using metadata object infos

    sai_api_query(SAI_API_ACL, (void**)&sai_acl_api);
    sai_api_query(SAI_API_BUFFER, (void**)&sai_buffer_api);
    sai_api_query(SAI_API_FDB, (void**)&sai_fdb_api);
    sai_api_query(SAI_API_HASH, (void**)&sai_hash_api);
    sai_api_query(SAI_API_HOSTIF, (void**)&sai_hostif_api);
    sai_api_query(SAI_API_LAG, (void**)&sai_lag_api);
    sai_api_query(SAI_API_MIRROR, (void**)&sai_mirror_api);
    sai_api_query(SAI_API_NEIGHBOR, (void**)&sai_neighbor_api);
    sai_api_query(SAI_API_NEXT_HOP, (void**)&sai_next_hop_api);
    sai_api_query(SAI_API_NEXT_HOP_GROUP, (void**)&sai_next_hop_group_api);
    sai_api_query(SAI_API_POLICER, (void**)&sai_policer_api);
    sai_api_query(SAI_API_PORT, (void**)&sai_port_api);
    sai_api_query(SAI_API_QOS_MAP, (void**)&sai_qos_map_api);
    sai_api_query(SAI_API_QUEUE, (void**)&sai_queue_api);
    sai_api_query(SAI_API_ROUTE, (void**)&sai_route_api);
    sai_api_query(SAI_API_ROUTER_INTERFACE, (void**)&sai_router_interface_api);
    sai_api_query(SAI_API_SAMPLEPACKET, (void**)&sai_samplepacket_api);
    sai_api_query(SAI_API_SCHEDULER, (void**)&sai_scheduler_api);
    sai_api_query(SAI_API_SCHEDULER_GROUP, (void**)&sai_scheduler_group_api);
    sai_api_query(SAI_API_STP, (void**)&sai_stp_api);
    sai_api_query(SAI_API_SWITCH, (void**)&sai_switch_api);
    sai_api_query(SAI_API_TUNNEL, (void**)&sai_tunnel_api);
    sai_api_query(SAI_API_UDF, (void**)&sai_udf_api);
    sai_api_query(SAI_API_VIRTUAL_ROUTER, (void**)&sai_router_api);
    sai_api_query(SAI_API_VLAN, (void**)&sai_vlan_api);
    sai_api_query(SAI_API_WRED, (void**)&sai_wred_api);
}
