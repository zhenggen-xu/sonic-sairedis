#include "syncd.h"
#include "sairedis.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <set>

/*
 * NOTE: all those methods could be implemented inside SaiSwitch class so then
 * we could skip using switch_id in params and even they could be public then.
 */

sai_uint32_t SaiSwitch::saiGetPortCount()
{
    SWSS_LOG_ENTER();

    sai_attribute_t attr;

    attr.id = SAI_SWITCH_ATTR_PORT_NUMBER;

    sai_status_t status = sai_metadata_sai_switch_api->get_switch_attribute(m_switch_rid, 1, &attr);

    if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_THROW("failed to get port number: %s",
                sai_serialize_status(status).c_str());
    }

    SWSS_LOG_DEBUG("port count is %u", attr.value.u32);

    return attr.value.u32;
}

sai_object_id_t SaiSwitch::saiGetCpuId()
{
    SWSS_LOG_ENTER();

    sai_attribute_t attr;

    attr.id = SAI_SWITCH_ATTR_CPU_PORT;

    sai_status_t status = sai_metadata_sai_switch_api->get_switch_attribute(m_switch_rid, 1, &attr);

    if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_THROW("failed to get cpu port: %s",
                sai_serialize_status(status).c_str());
    }

    SWSS_LOG_DEBUG("cpu port RID %s",
            sai_serialize_object_id(attr.value.oid).c_str());

    m_cpu_rid = attr.value.oid;

    return attr.value.oid;
}

std::vector<sai_object_id_t> SaiSwitch::saiGetPortList()
{
    SWSS_LOG_ENTER();

    uint32_t portCount = saiGetPortCount();

    std::vector<sai_object_id_t> portList;

    portList.resize(portCount);

    sai_attribute_t attr;

    attr.id = SAI_SWITCH_ATTR_PORT_LIST;
    attr.value.objlist.count = portCount;
    attr.value.objlist.list = portList.data();

    // we assume port list is always returned in the same order
    sai_status_t status = sai_metadata_sai_switch_api->get_switch_attribute(m_switch_rid, 1, &attr);

    if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_THROW("failed to get port list: %s",
                sai_serialize_status(status).c_str());
    }

    portList.resize(attr.value.objlist.count);

    SWSS_LOG_DEBUG("number of ports: %zu", portList.resize());

    return portList;
}

std::unordered_map<sai_uint32_t, sai_object_id_t> SaiSwitch::saiGetHardwareLaneMap()
{
    SWSS_LOG_ENTER();

    std::unordered_map<sai_uint32_t, sai_object_id_t> map;

    const std::vector<sai_object_id_t> portList = saiGetPortList();

    /*
     * NOTE: currently we don't support port breakout
     * this will need to be addressed in future.
     */

    const int lanesPerPort = 4;

    for (const auto &port_rid : portList)
    {
        sai_uint32_t lanes[lanesPerPort];

        sai_attribute_t attr;

        attr.id = SAI_PORT_ATTR_HW_LANE_LIST;
        attr.value.u32list.count = lanesPerPort;
        attr.value.u32list.list = lanes;

        sai_status_t status = sai_metadata_sai_port_api->get_port_attribute(port_rid, 1, &attr);

        if (status != SAI_STATUS_SUCCESS)
        {
            SWSS_LOG_THROW("failed to get hardware lane list port RID %s: %s",
                    sai_serialize_object_id(port_rid).c_str(),
                    sai_serialize_status(status).c_str());
        }

        sai_int32_t laneCount = attr.value.u32list.count;

        for (int j = 0; j < laneCount; j++)
        {
            map[lanes[j]] = port_rid;
        }
    }

    return map;
}

sai_object_id_t SaiSwitch::saiGetDefaultTrapGroup()
{
    SWSS_LOG_ENTER();

    sai_attribute_t attr;

    attr.id = SAI_SWITCH_ATTR_DEFAULT_TRAP_GROUP;

    sai_status_t status = sai_metadata_sai_switch_api->get_switch_attribute(m_switch_rid, 1, &attr);

    if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_THROW("failed to get switch default trap group: %s",
                sai_serialize_status(status).c_str());
    }

    SWSS_LOG_DEBUG("default trap group RID %s",
            sai_serialize_object_id(attr.value.oid).c_str());

    m_default_trap_group_rid = attr.value.oid;

    return attr.value.oid;
}

sai_object_id_t SaiSwitch::saiGetDefaultStpInstance()
{
    SWSS_LOG_ENTER();

    sai_attribute_t attr;

    attr.id = SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID;

    sai_status_t status = sai_metadata_sai_switch_api->get_switch_attribute(m_switch_rid, 1, &attr);

    if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_THROW("failed to get switch default stp instance: %s",
                sai_serialize_status(status).c_str());
    }

    SWSS_LOG_DEBUG("default stp instance RID %s",
            sai_serialize_object_id(attr.value.oid).c_str());

    m_default_stp_instance_rid = attr.value.oid;

    return attr.value.oid;
}

sai_object_id_t SaiSwitch::saiGetDefaultVirtualRouter()
{
    SWSS_LOG_ENTER();

    sai_attribute_t attr;

    attr.id = SAI_SWITCH_ATTR_DEFAULT_VIRTUAL_ROUTER_ID;

    sai_status_t status = sai_metadata_sai_switch_api->get_switch_attribute(m_switch_rid, 1, &attr);

    if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_THROW("failed to get switch virtual router id: %s",
                sai_serialize_status(status).c_str());
    }

    SWSS_LOG_DEBUG("default virtual router RID %s",
            sai_serialize_object_id(attr.value.oid).c_str());

    m_default_virtual_router_rid = attr.value.oid;

    return attr.value.oid;
}

std::string SaiSwitch::getRedisLanesKey()
{
    SWSS_LOG_ENTER();

    /*
     * Each switch will have it's own lanes in format LANES:oid:0xYYYYYYYY.
     *
     * NOTE: We are using switch VID here.
     */

    return std::string(LANES) + ":" + sai_serialize_object_id(m_switch_vid);
}

void SaiSwitch::redisClearLaneMap()
{
    SWSS_LOG_ENTER();

    auto key = getRedisLanesKey();

    g_redisClient->del(key);
}

std::unordered_map<sai_uint32_t, sai_object_id_t> SaiSwitch::redisGetLaneMap()
{
    SWSS_LOG_ENTER();

    auto key = getRedisLanesKey();

    auto hash = g_redisClient->hgetall(key);

    SWSS_LOG_DEBUG("previous lanes: %lu", hash.size());

    std::unordered_map<sai_uint32_t, sai_object_id_t> map;

    for (auto &kv: hash)
    {
        const std::string &str_key = kv.first;
        const std::string &str_value = kv.second;

        sai_uint32_t lane;
        sai_object_id_t portId;

        sai_deserialize_number(str_key, lane);

        sai_deserialize_object_id(str_value, portId);

        map[lane] = portId;
    }

    return map;
}

void SaiSwitch::redisSaveLaneMap(
        _In_ const std::unordered_map<sai_uint32_t, sai_object_id_t> &map)
{
    SWSS_LOG_ENTER();

    redisClearLaneMap();

    for (auto const &it: map)
    {
        sai_uint32_t lane = it.first;
        sai_object_id_t portId = it.second;

        std::string strLane = sai_serialize_number(lane);
        std::string strPortId = sai_serialize_object_id(portId);

        auto key = getRedisLanesKey();

        g_redisClient->hset(key, strLane, strPortId);
    }
}

std::vector<std::string> SaiSwitch::redisGetAsicStateKeys()
{
    SWSS_LOG_ENTER();

    return g_redisClient->keys(ASIC_STATE_TABLE + std::string(":*"));
}

std::unordered_map<sai_object_id_t, sai_object_id_t> SaiSwitch::redisGetObjectMap(
        _In_ const std::string &key)
{
    SWSS_LOG_ENTER();

    auto hash = g_redisClient->hgetall(key);

    std::unordered_map<sai_object_id_t, sai_object_id_t> map;

    for (auto &kv: hash)
    {
        const std::string &str_key = kv.first;
        const std::string &str_value = kv.second;

        sai_object_id_t objectIdKey;
        sai_object_id_t objectIdValue;

        sai_deserialize_object_id(str_key, objectIdKey);

        sai_deserialize_object_id(str_value, objectIdValue);

        map[objectIdKey] = objectIdValue;
    }

    return map;
}

std::unordered_map<sai_object_id_t, sai_object_id_t> SaiSwitch::redisGetVidToRidMap()
{
    SWSS_LOG_ENTER();

    return redisGetObjectMap(VIDTORID);
}

std::unordered_map<sai_object_id_t, sai_object_id_t> SaiSwitch::redisGetRidToVidMap()
{
    SWSS_LOG_ENTER();

    return redisGetObjectMap(RIDTOVID);
}

void SaiSwitch::helperCheckLaneMap()
{
    SWSS_LOG_ENTER();

    auto redisLaneMap = redisGetLaneMap();

    auto laneMap = saiGetHardwareLaneMap();

    if (redisLaneMap.size() == 0)
    {
        SWSS_LOG_INFO("no lanes defined in redis, seems like it is first syncd start");

        redisSaveLaneMap(laneMap);

        redisLaneMap = laneMap; // copy
    }

    if (laneMap.size() != redisLaneMap.size())
    {
        SWSS_LOG_THROW("lanes map size differ: %lu vs %lu", laneMap.size(), redisLaneMap.size());
    }

    for (auto kv: laneMap)
    {
        sai_uint32_t lane = kv.first;
        sai_object_id_t portId = kv.second;

        if (redisLaneMap.find(lane) == redisLaneMap.end())
        {
            SWSS_LOG_THROW("lane %u not found in redis", lane);
        }

        if (redisLaneMap[lane] != portId)
        {
            // if this happens, we need to remap VIDTORID and RIDTOVID
            SWSS_LOG_THROW("FIXME: lane port id differs: %s vs %s, port ids must be remapped",
                    sai_serialize_object_id(portId).c_str(),
                    sai_serialize_object_id(redisLaneMap[lane]).c_str());
        }
    }
}

std::string SaiSwitch::getRedisHiddenKey()
{
    SWSS_LOG_ENTER();

    /*
     * Each switch will have it's own hidden in format HIDDEN:oid:0xYYYYYYYY.
     *
     * NOTE: We are using switch VID here.
     */

    return std::string(HIDDEN) + ":" + sai_serialize_object_id(m_switch_vid);
}

sai_object_id_t SaiSwitch::redisGetDefaultVirtualRouterId()
{
    SWSS_LOG_ENTER();

    auto key = getRedisHiddenKey();

    auto redisVrId = g_redisClient->hget(key, DEFAULT_VIRTUAL_ROUTER_ID);

    if (redisVrId == NULL)
    {
        return SAI_NULL_OBJECT_ID;
    }

    sai_object_id_t vr_id;

    sai_deserialize_object_id(*redisVrId, vr_id);

    return vr_id;
}

sai_object_id_t SaiSwitch::redisGetDefaultTrapGroupId()
{
    SWSS_LOG_ENTER();

    auto key = getRedisHiddenKey();

    auto redisVrId = g_redisClient->hget(key, DEFAULT_TRAP_GROUP_ID);

    if (redisVrId == NULL)
    {
        return SAI_NULL_OBJECT_ID;
    }

    sai_object_id_t vr_id;

    sai_deserialize_object_id(*redisVrId, vr_id);

    return vr_id;
}

sai_object_id_t SaiSwitch::redisGetDefaultStpInstanceId()
{
    SWSS_LOG_ENTER();

    auto key = getRedisHiddenKey();

    auto redisStpId = g_redisClient->hget(key, DEFAULT_STP_INSTANCE_ID);

    if (redisStpId == NULL)
    {
        return SAI_NULL_OBJECT_ID;
    }

    sai_object_id_t stp_id;

    sai_deserialize_object_id(*redisStpId, stp_id);

    return stp_id;
}

sai_object_id_t SaiSwitch::redisGetCpuId()
{
    SWSS_LOG_ENTER();

    auto key = getRedisHiddenKey();

    auto redisCpuId = g_redisClient->hget(key, CPU_PORT_ID);

    if (redisCpuId == NULL)
    {
        return SAI_NULL_OBJECT_ID;
    }

    sai_object_id_t cpuId;

    sai_deserialize_object_id(*redisCpuId, cpuId);

    return cpuId;
}

void SaiSwitch::redisSetDefaultVirtualRouterId(
        _In_ sai_object_id_t vr_id)
{
    SWSS_LOG_ENTER();

    std::string strVrId = sai_serialize_object_id(vr_id);

    auto key = getRedisHiddenKey();

    g_redisClient->hset(key, DEFAULT_VIRTUAL_ROUTER_ID, strVrId);
}

void SaiSwitch::redisSetDefaultTrapGroup(
        _In_ sai_object_id_t vr_id)
{
    SWSS_LOG_ENTER();

    std::string strVrId = sai_serialize_object_id(vr_id);

    auto key = getRedisHiddenKey();

    g_redisClient->hset(key, DEFAULT_TRAP_GROUP_ID, strVrId);
}

void SaiSwitch::redisSetDefaultStpInstance(
        _In_ sai_object_id_t stp_id)
{
    SWSS_LOG_ENTER();

    std::string strStpId = sai_serialize_object_id(stp_id);

    auto key = getRedisHiddenKey();

    g_redisClient->hset(key, DEFAULT_STP_INSTANCE_ID, strStpId);
}

void SaiSwitch::redisCreateRidAndVidMapping(
        _In_ sai_object_id_t rid,
        _In_ sai_object_id_t vid)
{
    SWSS_LOG_ENTER();

    std::string strRid = sai_serialize_object_id(rid);
    std::string strVid = sai_serialize_object_id(vid);

    // TODO should we have different mapping per switch ?

    g_redisClient->hset(VIDTORID, strVid, strRid);
    g_redisClient->hset(RIDTOVID, strRid, strVid);

    SWSS_LOG_DEBUG("set VID 0x%lx and RID 0x%lx", vid, rid);
}

// TODO should we have different db's per each switch ?

void SaiSwitch::redisSetDummyAsicStateForRealObjectId(
        _In_ sai_object_id_t rid)
{
    SWSS_LOG_ENTER();

    sai_object_type_t objectType = sai_object_type_query(rid);

    if (objectType == SAI_OBJECT_TYPE_NULL)
    {
        SWSS_LOG_THROW("sai_object_type_query returned NULL type for RID: %s",
                sai_serialize_object_id(rid).c_str());
    }

    std::string strObjectType = sai_serialize_object_type(objectType);

    sai_object_id_t vid = redis_create_virtual_object_id(m_switch_vid, objectType);

    std::string strVid = sai_serialize_object_id(vid);

    std::string strKey = ASIC_STATE_TABLE + (":" + strObjectType + ":" + strVid);

    g_redisClient->hset(strKey, "NULL", "NULL");

    redisCreateRidAndVidMapping(rid, vid);
}

void SaiSwitch::helperCheckDefaultVirtualRouterId()
{
    SWSS_LOG_ENTER();

    sai_object_id_t vrId = saiGetDefaultVirtualRouter();

    sai_object_id_t redisVrId = redisGetDefaultVirtualRouterId();

    if (redisVrId == SAI_NULL_OBJECT_ID)
    {
        redisSetDummyAsicStateForRealObjectId(vrId);

        SWSS_LOG_INFO("redis default virtual router id is not defined yet");

        redisSetDefaultVirtualRouterId(vrId);

        redisVrId = vrId;
    }

    if (vrId != redisVrId)
    {
        // if this happens, we need to remap VIDTORID and RIDTOVID
        SWSS_LOG_THROW("FIXME: default virtual router id differs: %s vs %s, ids must be remapped",
                sai_serialize_object_id(vrId).c_str(),
                sai_serialize_object_id(redisVrId).c_str());
    }
}

void SaiSwitch::helperCheckDefaultTrapGroup()
{
    SWSS_LOG_ENTER();

    sai_object_id_t tgId = saiGetDefaultTrapGroup();

    sai_object_id_t redisTgId = redisGetDefaultTrapGroupId();

    if (redisTgId == SAI_NULL_OBJECT_ID)
    {
        redisSetDummyAsicStateForRealObjectId(tgId);

        SWSS_LOG_INFO("redis default trap group is not defined yet");

        redisSetDefaultTrapGroup(tgId);

        redisTgId = tgId;
    }

    if (tgId != redisTgId)
    {
        // if this happens, we need to remap VIDTORID and RIDTOVID
        SWSS_LOG_THROW("FIXME: default trap group id differs: %s vs %s, ids must be remapped",
                sai_serialize_object_id(tgId).c_str(),
                sai_serialize_object_id(redisTgId).c_str());
    }
}

void SaiSwitch::helperCheckDefaultStpInstance()
{
    SWSS_LOG_ENTER();

    sai_object_id_t stpId = saiGetDefaultStpInstance();

    sai_object_id_t redisStpId = redisGetDefaultStpInstanceId();

    if (redisStpId == SAI_NULL_OBJECT_ID)
    {
        redisSetDummyAsicStateForRealObjectId(stpId);

        SWSS_LOG_INFO("redis default stp instance id is not defined yet");

        redisSetDefaultStpInstance(stpId);

        redisStpId = stpId;
    }

    if (stpId != redisStpId)
    {
        // if this happens, we need to remap VIDTORID and RIDTOVID
        SWSS_LOG_THROW("FIXME: default stp instance id differs: %s vs %s, ids must be remapped",
                sai_serialize_object_id(stpId).c_str(),
                sai_serialize_object_id(redisStpId).c_str());
    }
}

void SaiSwitch::redisSetCpuId(
        _In_ sai_object_id_t cpuId)
{
    SWSS_LOG_ENTER();

    std::string strCpuId = sai_serialize_object_id(cpuId);

    auto key = getRedisHiddenKey();

    g_redisClient->hset(key, CPU_PORT_ID, strCpuId);
}

void SaiSwitch::helperCheckCpuId()
{
    SWSS_LOG_ENTER();

    sai_object_id_t cpuId = saiGetCpuId();

    sai_object_id_t redisCpuId = redisGetCpuId();

    if (redisCpuId == SAI_NULL_OBJECT_ID)
    {
        redisSetDummyAsicStateForRealObjectId(cpuId);

        SWSS_LOG_INFO("redis cpu id is not defined yet");

        redisSetCpuId(cpuId);

        redisCpuId = cpuId;
    }

    if (cpuId != redisCpuId)
    {
        // if this happens, we need to remap VIDTORID and RIDTOVID
        SWSS_LOG_THROW("FIXME: cpu id differs: %s vs %s, ids must be remapped",
                sai_serialize_object_id(cpuId).c_str(),
                sai_serialize_object_id(redisCpuId).c_str());
    }
}

void SaiSwitch::redisCreateDummyEntryInAsicView(
        _In_ sai_object_id_t objectId)
{
    SWSS_LOG_ENTER();

    sai_object_id_t vid = translate_rid_to_vid(objectId);

    sai_object_type_t objectType = sai_object_type_query(objectId);

    if (objectType == SAI_OBJECT_TYPE_NULL)
    {
        SWSS_LOG_THROW("sai_object_type_query returned NULL type for RID: %s",
                sai_serialize_object_id(objectId).c_str());
    }

    std::string strObjectType = sai_serialize_object_type(objectType);

    std::string strVid = sai_serialize_object_id(vid);

    std::string strKey = ASIC_STATE_TABLE + (":" + strObjectType + ":" + strVid);

    g_redisClient->hset(strKey, "NULL", "NULL");
}

void SaiSwitch::helperCheckPortIds()
{
    SWSS_LOG_ENTER();

    auto laneMap = saiGetHardwareLaneMap();

    for (auto kv: laneMap)
    {
        sai_object_id_t port_rid = kv.second;

        /*
         * NOTE: Translate will create entry if missing, we assume here that
         * port numbers didn't changed during restarts.
         */

        redisCreateDummyEntryInAsicView(port_rid);

        m_default_ports_rids.insert(port_rid);
    }
}

// TODO vlan 1 is now object id, we could extract it
void SaiSwitch::helperCheckVlanId()
{
    SWSS_LOG_ENTER();

    // TODO create vlan entry only when its hard restart
    //
    // TODO get that from switch

    sai_object_type_t objectType = SAI_OBJECT_TYPE_VLAN;

    std::string strObjectType = sai_serialize_object_type(objectType);

    sai_vlan_id_t vlanId = 1;

    std::string strVlanId = sai_serialize_vlan_id(vlanId);

    std::string strKey = ASIC_STATE_TABLE + (":" + strObjectType + ":" + strVlanId);

    g_redisClient->hset(strKey, "NULL", "NULL");
}

sai_uint32_t SaiSwitch::saiGetPortNumberOfQueues(
        _In_ sai_object_id_t port_rid)
{
    SWSS_LOG_ENTER();

    sai_attribute_t attr;

    attr.id = SAI_PORT_ATTR_QOS_NUMBER_OF_QUEUES;

    sai_status_t status = sai_metadata_sai_port_api->get_port_attribute(port_rid, 1, &attr);

    if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_THROW("failed to get port RID %s number of queues: %s",
                sai_serialize_object_id(port_rid),
                sai_serialize_status(status).c_str());
    }

    SWSS_LOG_DEBUG("port RID %s queues number: %u",
            sai_serialize_object_id(port_rid).c_str(),
            attr.value.u32);

    return attr.value.u32;
}

std::vector<sai_object_id_t> SaiSwitch::saiGetPortQueues(
        _In_ sai_object_id_t port_rid)
{
    SWSS_LOG_ENTER();

    uint32_t queueCount = saiGetPortNumberOfQueues(port_rid);

    std::vector<sai_object_id_t> queueList;

    if (queueCount == 0)
    {
        return queueList;
    }

    queueList.resize(queueCount);

    sai_attribute_t attr;

    attr.id = SAI_PORT_ATTR_QOS_QUEUE_LIST;
    attr.value.objlist.count = queueCount;
    attr.value.objlist.list = queueList.data();

    sai_status_t status = sai_metadata_sai_port_api->get_port_attribute(port_rid, 1, &attr);

    if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_THROW("failed to get port RID %s queue list: %s",
                sai_serialize_object_id(port_rid),
                sai_serialize_status(status).c_str());
    }

    queueList.resize(attr.value.objlist.count);

    return queueList;
}

void SaiSwitch::helperCheckQueuesIds()
{
    SWSS_LOG_ENTER();

    std::vector<sai_object_id_t> ports = saiGetPortList();

    for (const auto& port_rid: ports)
    {
        SWSS_LOG_DEBUG("getting queues for port RID: %s",
                sai_serialize_object_id(port_rid).c_str());

        std::vector<sai_object_id_t> queues = saiGetPortQueues(port_rid);

        /*
         * We have queues.
         */

        for (const auto& queue_rid: queues)
        {
            /*
             * NOTE: Create entry in asic view if missing we assume here that
             * queue numbers will not be changed during restarts.
             */

            redisCreateDummyEntryInAsicView(queue_rid);

            m_default_queues_rids.insert(queue_rid);
        }
    }
}

sai_uint32_t SaiSwitch::saiGetPortNumberOfPriorityGroups(
        _In_ sai_object_id_t port_rid)
{
    SWSS_LOG_ENTER();

    sai_attribute_t attr;

    attr.id = SAI_PORT_ATTR_NUMBER_OF_INGRESS_PRIORITY_GROUPS;

    sai_status_t status = sai_metadata_sai_port_api->get_port_attribute(port_rid, 1, &attr);

    if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_THROW("failed to get port RID %s number of priority groups: %s",
                sai_serialize_object_id(port_rid),
                sai_serialize_status(status).c_str());
    }

    SWSS_LOG_DEBUG("port RID %s priority groups number: %u",
            sai_serialize_object_id(port_rid).c_str(),
            attr.value.u32);

    return attr.value.u32;
}

std::vector<sai_object_id_t> SaiSwitch::saiGetPortPriorityGroups(
        _In_ sai_object_id_t port_rid)
{
    SWSS_LOG_ENTER();

    uint32_t pgCount = saiGetPortNumberOfPriorityGroups(port_rid);

    std::vector<sai_object_id_t> pgList;

    if (pgCount == 0)
    {
        return pgList;
    }

    pgList.resize(pgCount);

    sai_attribute_t attr;

    attr.id = SAI_PORT_ATTR_INGRESS_PRIORITY_GROUP_LIST;
    attr.value.objlist.count = pgCount;
    attr.value.objlist.list = pgList.data();

    sai_status_t status = sai_metadata_sai_port_api->get_port_attribute(port_rid, 1, &attr);

    if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_THROW("failed to get port RID %s priority groups list: %s",
                sai_serialize_object_id(port_rid),
                sai_serialize_status(status).c_str());
    }

    pgList.resize(attr.value.objlist.count);

    return pgList;
}

void SaiSwitch::helperCheckPriorityGroupsIds()
{
    SWSS_LOG_ENTER();

    std::vector<sai_object_id_t> ports = saiGetPortList();

    for (const auto& port_rid: ports)
    {
        SWSS_LOG_DEBUG("getting priority groups for port RID: %s",
                sai_serialize_object_id(port_rid).c_str());

        std::vector<sai_object_id_t> pgs = saiGetPortPriorityGroups(port_rid);

        for (const auto& pg_rid: pgs)
        {
            /*
             * NOTE: Create entry in asic view if missing we assume here that
             * PG numbers will not be changed during restarts.
             */

            redisCreateDummyEntryInAsicView(pg_rid);

            m_default_priority_groups_rids.insert(pg_rid);
        }
    }
}

uint32_t SaiSwitch::saiGetMaxNumberOfChildsPerSchedulerGroup()
{
    SWSS_LOG_ENTER();

    sai_attribute_t attr;

    attr.id = SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_CHILDS_PER_SCHEDULER_GROUP;

    sai_status_t status = sai_metadata_sai_switch_api->get_switch_attribute(m_switch_rid, 1, &attr);

    if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_THROW("faled to obtain SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_CHILDS_PER_SCHEDULER_GROUP for switch RID %s: %s",
                sai_serialize_object_id(m_switch_rid).c_str(),
                sai_serialize_status(status).c_str());
    }

    return attr.value.u32;
}

uint32_t SaiSwitch::saiGetQosNumberOfSchedulerGroups(
        _In_ sai_object_id_t port_rid)
{
    SWSS_LOG_ENTER();

    sai_attribute_t attr;

    attr.id = SAI_PORT_ATTR_QOS_NUMBER_OF_SCHEDULER_GROUPS;

    sai_status_t status = sai_metadata_sai_port_api->get_port_attribute(port_rid, 1, &attr);

    if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_THROW("Failed to get number of scheduler groups for port RID %s: %s",
                sai_serialize_object_id(port_rid).c_str(),
                sai_serialize_status(status).c_str());
    }

    /*
     * Total groups list on the port.
     */

    return attr.value.u32;
}

std::vector<sai_object_id_t> SaiSwitch::saiGetSchedulerGroupList(
        _In_ sai_object_id_t port_rid)
{
    SWSS_LOG_ENTER();

    uint32_t groupsCount = saiGetQosNumberOfSchedulerGroups(port_rid);

    std::vector<sai_object_id_t> groups;

    groups.resize(groupsCount);

    sai_attribute_t attr;

    attr.id = SAI_PORT_ATTR_QOS_SCHEDULER_GROUP_LIST;
    attr.value.objlist.list = groups.data();
    attr.value.objlist.count = (uint32_t)groups.size();

    sai_status_t status = sai_metadata_sai_port_api->get_port_attribute(port_rid, 1, &attr);

    if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_THROW("Failed to get scheduler group list for port RID %s: %s",
                sai_serialize_object_id(port_rid).c_str(),
                sai_serialize_status(status).c_str());
    }

    groups.resize(attr.value.objlist.count);

    SWSS_LOG_DEBUG("got %zu scheduler groups on port RID %s",
            groups.size(),
            sai_serialize_object_id(port_rid).c_str());

    return groups;
}

uint32_t SaiSwitch::saiGetSchedulerGroupChildCount(
        _In_ sai_object_id_t sg_rid)
{
    SWSS_LOG_ENTER();

    sai_attribute_t attr;

    attr.id = SAI_SCHEDULER_GROUP_ATTR_CHILD_COUNT; // queues + sched groups

    sai_status_t status = sai_metadata_sai_scheduler_group_api->get_scheduler_group_attribute(sg_rid, 1, &attr);

    if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_THROW("Failed to get child count for scheduler group RID %s: %s",
                sai_serialize_object_id(sg_rid).c_str(),
                sai_serialize_status(status).c_str());
    }

    return attr.value.u32;
}

std::vector<sai_object_id_t> SaiSwitch::saiGetSchedulerGroupChildList(
        _In_ sai_object_id_t sg_rid)
{
    SWSS_LOG_ENTER();

    uint32_t childCount = saiGetSchedulerGroupChildCount(sg_rid);

    SWSS_LOG_DEBUG("child count for SG RID %s is %u",
            sai_serialize_object_id(sg_rid).c_str(),
            childCount);

    std::vector<sai_object_id_t> childs;

    childs.resize(childCount);

    sai_attribute_t attr;

    attr.id = SAI_SCHEDULER_GROUP_ATTR_CHILD_LIST;
    attr.value.objlist.list = childs.data();
    attr.value.objlist.count = (uint32_t)childs.size();

    sai_status_t status = sai_metadata_sai_scheduler_group_api->get_scheduler_group_attribute(sg_rid, 1, &attr);

    if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_THROW("Failed to get child list for scheduler group RID %s: %s",
                sai_serialize_object_id(sg_rid).c_str(),
                sai_serialize_status(status).c_str());
    }

    childs.resize(attr.value.objlist.count);

    SWSS_LOG_DEBUG("got %zu childs on scheduler group RID %s",
            childs.size(),
            sai_serialize_object_id(sg_rid).c_str());

    return childs; // scheduler groups + queues
}

void SaiSwitch::helperCheckSchedulerGroupsIds()
{
    SWSS_LOG_ENTER();

    for (const auto& port_rid: saiGetPortList())
    {
        /*
         * Each group can contain next scheduler group or queue.
         */

        for (const auto& sg_rid: saiGetSchedulerGroupList(port_rid))
        {
            /*
             * NOTE: Create entry in asic view if missing we assume here that
             * SchedulerGroups numbers will not be changed during restarts.
             */

            redisCreateDummyEntryInAsicView(sg_rid);

            m_default_scheduler_groups_rids.insert(sg_rid);
        }
    }
}

SaiSwitch::SaiSwitch(
        _In_ sai_object_id_t switch_vid)
{
    m_switch_id = switch_id;

    // TODO ! we need RID !

    helperCheckLaneMap();

    helperCheckCpuId();

    helperCheckDefaultVirtualRouterId();

    helperCheckDefaultTrapGroup();

    // currently STP is disabled since not all vendors supports that
    //helperCheckDefaultStpInstance();

    helperCheckVlanId();

    helperCheckPortIds();

    helperCheckQueuesIds();

    helperCheckPriorityGroupsIds();

    helperCheckSchedulerGroupsIds();

    // TODO we need to get SAI_SWITCH_ATTR_SWITCH_HARDWARE_INFO
}



// if this is worm start then we assume some switches exists
// already, we can get them from asic state table
// and run those checks,
// if no switches exists, we can create them first
// TODO - lets only choose CREATE_ONLY and MANDATORY_ON_CREATE
// atttributes, and rest could be jsut set in hard reinit
//
// but before we need helper functions to run
// we can also put this inside class
void onSyncdStart(bool warmStart)
{
    // it may happen that after initialize we will receive
    // some port notifications with port'ids that are not in
    // redis db yet, so after checking VIDTORID map there will
    // be entries and translate_vid_to_rid will generate new
    // id's for ports, this may cause race condition so we need
    // to use a lock here to prevent that

    std::lock_guard<std::mutex> lock(g_mutex);

    // TODO this needs to be check on warm start and on hard reinit
    // so we need to get list of all switches somehow, since after
    // restart that list will not be populated
    // we could use redis or bulk api query to do that
    // and check some get operations etc

    // TODO and on restart we need to populate switch map
    // and initialize them both, if they exists ?
    // if they don't exist, then hard reinit, but they will exist
    // anyway and hard reinit will create them then ?
    // how we will know if it's in asic ?

    SWSS_LOG_ENTER();

    SWSS_LOG_TIMER("on syncd start");

    // helpers

    // TODO add SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID
    // also support this values inside virtual switch
    // and what will happen if user will moify existing profile values?

    if (warmStart)
    {
        SWSS_LOG_NOTICE("skipping hard reinit since WARM start was performed");
        return;
    }

    SWSS_LOG_NOTICE("performing hard reinit since COLD start was performed");

    hardReinit();
}
