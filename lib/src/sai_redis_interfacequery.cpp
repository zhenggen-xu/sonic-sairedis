#include <string.h>
#include "sai_redis.h"
#include "sairedis.h"

std::mutex g_apimutex;

service_method_table_t g_services;
bool                   g_apiInitialized = false;

std::shared_ptr<swss::DBConnector>          g_db;
std::shared_ptr<swss::DBConnector>          g_dbNtf;
std::shared_ptr<swss::ProducerTable>        g_asicState;
std::shared_ptr<swss::ConsumerTable>        g_redisGetConsumer;
std::shared_ptr<swss::NotificationConsumer> g_redisNotifications;
std::shared_ptr<swss::RedisClient>          g_redisClient;

sai_status_t sai_api_initialize(
        _In_ uint64_t flags,
        _In_ const service_method_table_t* services)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    if (g_apiInitialized)
    {
        SWSS_LOG_ERROR("api already initialized");

        return SAI_STATUS_FAILURE;
    }

    if ((NULL == services) || (NULL == services->profile_get_next_value) || (NULL == services->profile_get_value))
    {
        SWSS_LOG_ERROR("Invalid services handle passed to SAI API initialize");

        return SAI_STATUS_INVALID_PARAMETER;
    }

    memcpy(&g_services, services, sizeof(g_services));

    g_db                 = std::make_shared<swss::DBConnector>(ASIC_DB, swss::DBConnector::DEFAULT_UNIXSOCKET, 0);
    g_dbNtf              = std::make_shared<swss::DBConnector>(ASIC_DB, swss::DBConnector::DEFAULT_UNIXSOCKET, 0);
    g_asicState          = std::make_shared<swss::ProducerTable>(g_db.get(), ASIC_STATE_TABLE);
    g_redisGetConsumer   = std::make_shared<swss::ConsumerTable>(g_db.get(), "GETRESPONSE");
    g_redisNotifications = std::make_shared<swss::NotificationConsumer>(g_dbNtf.get(), "NOTIFICATIONS");
    g_redisClient        = std::make_shared<swss::RedisClient>(g_db.get());

    /*
     * Initialize metatada database.
     */

    meta_init_db();

    /*
     * Reset used switch ids
     */

    redis_clear_switch_ids();

    g_apiInitialized = true;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_api_uninitialize(void)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    if (!g_apiInitialized)
    {
        SWSS_LOG_ERROR("api not initialized");

        return SAI_STATUS_FAILURE;
    }

    g_apiInitialized = false;

    return SAI_STATUS_SUCCESS;
}

// FIXME: Currently per API log level cannot be set
sai_status_t sai_log_set(
        _In_ sai_api_t sai_api_id,
        _In_ sai_log_level_t log_level)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    SWSS_LOG_ERROR("not implemented");

    return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t sai_api_query(
        _In_ sai_api_t sai_api_id,
        _Out_ void** api_method_table)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    if (api_method_table == NULL)
    {
        SWSS_LOG_ERROR("NULL method table passed to SAI API initialize");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!g_apiInitialized)
    {
        SWSS_LOG_ERROR("SAI API not initialized before calling API query");
        return SAI_STATUS_UNINITIALIZED;
    }

    switch (sai_api_id)
    {
        case SAI_API_BUFFER:
            *(const sai_buffer_api_t**)api_method_table = &redis_buffer_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_HASH:
            *(const sai_hash_api_t**)api_method_table = &redis_hash_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_SWITCH:
            *(const sai_switch_api_t**)api_method_table = &redis_switch_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_PORT:
            *(const sai_port_api_t**)api_method_table = &redis_port_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_FDB:
            *(const sai_fdb_api_t**)api_method_table = &redis_fdb_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_VLAN:
            *(const sai_vlan_api_t**)api_method_table = &redis_vlan_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_WRED:
            *(const sai_wred_api_t**)api_method_table = &redis_wred_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_VIRTUAL_ROUTER:
            *(const sai_virtual_router_api_t**)api_method_table = &redis_router_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_ROUTE:
            *(const sai_route_api_t**)api_method_table = &redis_route_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_NEXT_HOP:
            *(const sai_next_hop_api_t**)api_method_table = &redis_next_hop_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_NEXT_HOP_GROUP:
            *(const sai_next_hop_group_api_t**)api_method_table = &redis_next_hop_group_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_ROUTER_INTERFACE:
            *(const sai_router_interface_api_t**)api_method_table = &redis_router_interface_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_NEIGHBOR:
            *(const sai_neighbor_api_t**)api_method_table = &redis_neighbor_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_ACL:
            *(const sai_acl_api_t**)api_method_table = &redis_acl_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_HOSTIF:
            *(const sai_hostif_api_t**)api_method_table = &redis_hostif_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_POLICER:
            *(const sai_policer_api_t**)api_method_table = &redis_policer_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_QOS_MAP:
            *(const sai_qos_map_api_t**)api_method_table = &redis_qos_map_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_QUEUE:
            *(const sai_queue_api_t**)api_method_table = &redis_queue_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_SCHEDULER:
            *(const sai_scheduler_api_t**)api_method_table = &redis_scheduler_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_SCHEDULER_GROUP:
            *(const sai_scheduler_group_api_t**)api_method_table = &redis_scheduler_group_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_MIRROR:
            *(const sai_mirror_api_t**)api_method_table = &redis_mirror_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_UDF:
            *(const sai_udf_api_t**)api_method_table = &redis_udf_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_SAMPLEPACKET:
            *(const sai_samplepacket_api_t**)api_method_table = &redis_samplepacket_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_STP:
            *(const sai_stp_api_t**)api_method_table = &redis_stp_api;
            return SAI_STATUS_NOT_IMPLEMENTED;

        case SAI_API_LAG:
            *(const sai_lag_api_t**)api_method_table = &redis_lag_api;
            return SAI_STATUS_SUCCESS;

        case SAI_API_TUNNEL:
            *(const sai_tunnel_api_t**)api_method_table = &redis_tunnel_api;
            return SAI_STATUS_SUCCESS;

        default:
            SWSS_LOG_ERROR("Invalid API type %d", sai_api_id);
            return SAI_STATUS_INVALID_PARAMETER;
    }
}
