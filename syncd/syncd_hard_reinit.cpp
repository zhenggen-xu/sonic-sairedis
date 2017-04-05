#include <string>
#include <vector>
#include <unordered_map>

#include "syncd.h"

// TODO there will be needed extra care about switch
// since if we set switch attributes with oids then we need to process
// them first, but we need switch id to create them first, so we need
// to first create switch with mandatory or all non object id's and then
// DO "set" and process all
// TODO also handle pointers on switch
// mandatory on create or create only

/*
 * To support multiple switches here we need to refactor this to a class
 * similar to SaiSwitch, and then have separate vid/rid map and ecah class will
 * be recreating only one switch and handling that
 */

typedef std::unordered_map<std::string, std::string> StringHash;
typedef std::unordered_map<sai_object_id_t, sai_object_id_t> ObjectIdMap;

std::unordered_map<std::string, std::shared_ptr<SaiAttributeList>> g_attributesLists;

ObjectIdMap g_translated;

ObjectIdMap g_vidToRidMap;
ObjectIdMap g_ridToVidMap;

StringHash g_neighbors;
StringHash g_oids;
StringHash g_fdbs;
StringHash g_routes;

void processAttributesForOids(sai_object_type_t objectType, std::shared_ptr<SaiAttributeList> list);
void processOids();
void processFdbs();
void processNeighbors();
void processSwitchesStart();
void processSwitchesEnd();
void processRoutes(bool defaultOnly);

sai_object_type_t getObjectTypeFromAsicKey(
        _In_ const std::string &key);

sai_object_type_t getObjectTypeFromVid(
        _In_ sai_object_id_t object_vid)
{
    SWSS_LOG_ENTER();

    sai_object_type_t objectType = redis_sai_object_type_query(object_vid);

    if (objectType >= SAI_OBJECT_TYPE_MAX ||
            objectType == SAI_OBJECT_TYPE_NULL)
    {
        SWSS_LOG_THROW("invalid object type: %s on object id: %s",
                sai_serialize_object_type(objectType).c_str(),
                sai_serialize_object_id(object_vid).c_str());
    }

    return objectType;
}

std::shared_ptr<SaiAttributeList> redisGetAttributesFromAsicKey(
        _In_ const std::string &key)
{
    SWSS_LOG_ENTER();

    sai_object_type_t objectType = getObjectTypeFromAsicKey(key);

    std::vector<swss::FieldValueTuple> values;

    auto hash = g_redisClient->hgetall(key);

    for (auto &kv: hash)
    {
        const std::string &skey = kv.first;
        const std::string &svalue = kv.second;

        swss::FieldValueTuple fvt(skey, svalue);

        values.push_back(fvt);
    }

    return std::make_shared<SaiAttributeList>(objectType, values, false);
}

sai_object_type_t getObjectTypeFromAsicKey(
        _In_ const std::string &key)
{
    SWSS_LOG_ENTER();

    auto start = key.find_first_of(":") + 1;
    auto end = key.find(":", start);

    const std::string strObjectType = key.substr(start, end - start);

    sai_object_type_t objectType;
    sai_deserialize_object_type(strObjectType, objectType);

    if (objectType >= SAI_OBJECT_TYPE_MAX ||
        objectType == SAI_OBJECT_TYPE_NULL)
    {
        SWSS_LOG_THROW("invalid object type: %s on asic key: %s", 
                sai_serialize_object_type(objectType).c_str(), 
                key.c_str());
    }

    return objectType;
}

std::string getObjectIdFromAsicKey(
        _In_ const std::string &key)
{
    SWSS_LOG_ENTER();

    auto start = key.find_first_of(":") + 1;
    auto end = key.find(":", start);

    return key.substr(end + 1);
}

void redisClearVidToRidMap()
{
    SWSS_LOG_ENTER();

    g_redisClient->del(VIDTORID);
}

void redisClearRidToVidMap()
{
    SWSS_LOG_ENTER();

    g_redisClient->del(RIDTOVID);
}

void redisSetVidAndRidMap(
        _In_ const std::unordered_map<sai_object_id_t, sai_object_id_t> &map)
{
    SWSS_LOG_ENTER();

    /*
     * TODO clear can be done after recreating all switches unless vid/rid map
     * will be per switch.
     *
     * This needs to be addressed when we want to support multiple switches.
     */

    redisClearVidToRidMap();
    redisClearRidToVidMap();

    for (auto &kv: map)
    {
        std::string strVid = sai_serialize_object_id(kv.first);
        std::string strRid = sai_serialize_object_id(kv.second);

        g_redisClient->hset(VIDTORID, strVid, strRid);
        g_redisClient->hset(RIDTOVID, strRid, strVid);
    }
}

void checkAllIds()
{
    SWSS_LOG_ENTER();

    for (auto &kv: g_translated)
    {
        auto it = g_vidToRidMap.find(kv.first);

        if (it == g_vidToRidMap.end())
        {
            SWSS_LOG_THROW("failed to find vid %s in previous map", 
                    sai_serialize_object_id(kv.first).c_str());
        }

        g_vidToRidMap.erase(it);
    }

    size_t size = g_vidToRidMap.size();

    if (size != 0)
    {

        for (auto &kv: g_vidToRidMap)
        {
            sai_object_type_t objectType = getObjectTypeFromVid(kv.first);

            SWSS_LOG_ERROR("vid not translated: %s, object type: %s",
                    sai_serialize_object_id(kv.first).c_str(),
                    sai_serialize_object_type(objectType).c_str());
        }

        SWSS_LOG_THROW("vid to rid map is not empty (%zu) after translation", size);
    }

    redisSetVidAndRidMap(g_translated);
}

void saiRemoveDefaultVlanMembers()
{
    SWSS_LOG_ENTER();

    // TODO this method needs to be revisited
    // after asic init, all ports are vlan 1 members
    // we will remove them to not complicate reinit
    // if user want to add vlan 1, then it needs to be
    // done explicitly.

    const auto& portList = saiGetPortList();

    // TODO we should query for actual list size not port size
    std::vector<sai_object_id_t> vlanMemberList;

    vlanMemberList.resize(portList.size());

    sai_attribute_t attr;

    attr.id = SAI_VLAN_ATTR_MEMBER_LIST;

    attr.value.objlist.count = (uint32_t)vlanMemberList.size();
    attr.value.objlist.list = vlanMemberList.data();

    sai_status_t status = sai_vlan_api->get_vlan_attribute(DEFAULT_VLAN_NUMBER, 1, &attr);

    if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_THROW("Failed to obtain vlan %d members", DEFAULT_VLAN_NUMBER);
    }

    vlanMemberList.resize(attr.value.objlist.count);

    for (auto &vm: vlanMemberList)
    {
        status = sai_vlan_api->remove_vlan_member(vm);

        if (status != SAI_STATUS_SUCCESS)
        {
            SWSS_LOG_THROW("Failed to remove vlan member 0x%lx from vlan %d", vm, DEFAULT_VLAN_NUMBER);
        }
    }
}

void hardReinit()
{
    SWSS_LOG_ENTER();

    SWSS_LOG_TIMER("hard reinit");

    saiRemoveDefaultVlanMembers();

    /*
     * Repopulate asic view from redis db after hard asic initialize.
     */

    /*
     * To support multiple switchies this needs to be refactores since we need
     * a class with hard reinit, per switch
     */

    g_vidToRidMap = redisGetVidToRidMap();
    g_ridToVidMap = redisGetRidToVidMap();

    std::vector<std::string> asicStateKeys = redisGetAsicStateKeys();

    for (auto &key: asicStateKeys)
    {
        /*
         * TODO if key will be meta_kay anyway we could use deserialize here.
         */

        sai_object_type_t objectType = getObjectTypeFromAsicKey(key);
        const std::string &strObjectId = getObjectIdFromAsicKey(key);

        auto info = sai_all_object_type_infos[objectType];

        switch (objectType)
        {
            case SAI_OBJECT_TYPE_ROUTE_ENTRY:
                g_routes[strObjectId] = key;
                break;

            case SAI_OBJECT_TYPE_FDB_ENTRY:
                g_fdbs[strObjectId] = key;
                break;

            case SAI_OBJECT_TYPE_NEIGHBOR_ENTRY:
                g_neighbors[strObjectId] = key;
                break;

            case SAI_OBJECT_TYPE_SWITCH:
                g_oids[strObjectId] = key;
                break;

            default:

                if (info->isnonobjectid)
                {
                    SWSS_LOG_THROW("passing non object id %s as generic object", info->objecttypename);
                }

                g_oids[strObjectId] = key;
                break;
        }

        g_attributesLists[key] = redisGetAttributesFromAsicKey(key);
    }

    processSwitchesStart();
    processFdbs();
    processNeighbors();
    processOids();
    processSwitchesEnd();
    processRoutes(true);
    processRoutes(false);

    checkAllIds();
}

template<typename FUN>
bool shouldSkipCreation(
        _In_ sai_object_id_t vid,
        _In_ sai_object_id_t& rid,
        _In_ bool& createObject,
        _In_ FUN fun)
{
    SWSS_LOG_ENTER();

    auto it = g_vidToRidMap.find(vid);

    if (it == g_vidToRidMap.end())
    {
        SWSS_LOG_THROW("failed to find VID %s in VIDTORID map", 
                sai_serialize_object_id(vid).c_str());
    }

    if (fun(it->second))
    {
        rid = it->second;

        createObject = false;

        return true;
    }

    return false;
}

void trapGroupWorkaround(
        _In_ sai_object_id_t vid,
        _In_ sai_object_id_t& rid,
        _In_ bool& createObject,
        _In_ uint32_t attrCount,
        _In_ const sai_attribute_t* attrList)
{
    SWSS_LOG_ENTER();

    if (createObject)
    {
        createObject = false; // force to "SET" left attributes
    }
    else
    {
        // default trap group
        return;
    }

    sai_object_type_t objectType = SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP;

    SWSS_LOG_INFO("creating trap group and setting attributes 1 by 1 as workaround");

    const sai_attribute_t* queue_attr = sai_metadata_get_attr_by_id(SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE, attrCount, attrList);

    if (queue_attr == NULL)
    {
        SWSS_LOG_THROW("missing QUEUE attribute on TRAP_GROUP creation even if it's not MANDATORY");
    }

    auto info = sai_all_object_type_infos[objectType];

    if (info == NULL)
    {
        SWSS_LOG_THROW("create function is not defined for object type %s", 
                sai_serialize_object_type(objectType).c_str());
    }

    sai_object_meta_key meta_key;

    meta_key.object_type = objectType;

    sai_status_t status = info->(&meta_key, switch_rid, 1, queue_attr);

    rid = meta_key.object_ket.key.object_id;

    if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_THROW("failed to create TRAP_GROUP %s", 
                sai_serialize_status(status).c_str());
    }

    SWSS_LOG_DEBUG("created TRAP_GROUP (%s), processed VID %s to RID %s", 
            sai_serialize_object_type(objectType).c_str(),
            sai_serialize_object_id(vid).c_str(),
            sai_serialize_object_id(rid).c_str();
}

void listFailedAttributes(
        _In_ sai_object_type_t objectType,
        _In_ uint32_t attrCount,
        _In_ const sai_attribute_t* attrList)
{
    SWSS_LOG_ENTER();

    for (uint32_t idx = 0; idx < attrCount; idx++)
    {
        const sai_attribute_t *attr = &attrList[idx];

        auto meta = sai_metadata_get_attr_metadata(objectType, attr->id);

        if (meta == NULL)
        {
            SWSS_LOG_ERROR("failed to get atribute metadata %s %d", 
                    sai_serialize_object_type(objectType).c_str(),
                    attr->id);
        }

        SWSS_LOG_ERROR("%s = %s", meta->attridname, sai_serialize_attr_value(*meta, *attr).c_str());
    }
}

sai_object_id_t processSingleVid(
        _In_ sai_object_id_t vid)
{
    SWSS_LOG_ENTER();

    if (vid == SAI_NULL_OBJECT_ID)
    {
        SWSS_LOG_DEBUG("processed VID 0 to RID 0");

        return SAI_NULL_OBJECT_ID;
    }

    auto it = g_translated.find(vid);

    if (it != g_translated.end())
    {
        /*
         * This object was already processed, just return real object id.
         */

        SWSS_LOG_DEBUG("processed VID %s to RID %s", 
                sai_serialize_object_id(vid).c_str(),
                sai_serialize_object_id(it->second).c_str());

        return it->second;
    }

    sai_object_id_t rid;
    sai_object_type_t objectType = getObjectTypeFromVid(vid);

    std::string strVid = sai_serialize_object_id(vid);

    auto oit = g_oids.find(strVid);

    if (oit == g_oids.end())
    {
        SWSS_LOG_THROW("failed to find VID %s in OIDs map", strVid.c_str());
    }

    std::string asicKey = oit->second;;

    std::shared_ptr<SaiAttributeList> list = g_attributesLists[asicKey];

    processAttributesForOids(objectType, list); // recursion

    sai_attribute_t *attrList = list->get_attr_list();

    uint32_t attrCount = list->get_attr_count();

    bool createObject = true;

    /*
     * TODO optimze this should skip creation. We should have some stuff inside
     * SaiSwitch.
     */

    if (objectType == SAI_OBJECT_TYPE_VIRTUAL_ROUTER)
    {
        if (shouldSkipCreation(vid, rid, createObject, [](sai_object_id_t id) { return id == redisGetDefaultVirtualRouterId(); }))
        {
            SWSS_LOG_INFO("default virtual router will not be created, processed VID 0x%lx to RID 0x%lx", vid, rid);
        }
    }
    else if (objectType == SAI_OBJECT_TYPE_STP)
    {
        if (shouldSkipCreation(vid, rid, createObject, [](sai_object_id_t id) { return id == redisGetDefaultStpInstanceId(); }))
        {
            SWSS_LOG_INFO("default stp instance will not be created, processed VID 0x%lx to RID 0x%lx", vid, rid);
        }
    }
    else if (objectType == SAI_OBJECT_TYPE_QUEUE)
    {
        if (shouldSkipCreation(vid, rid, createObject, [&](sai_object_id_t queueId) { return g_defaultQueuesRids.find(queueId) != g_defaultQueuesRids.end(); }))
        {
            SWSS_LOG_DEBUG("default queue will not be created, processed VID 0x%lx to RID 0x%lx", vid, rid);
        }
    }
    else if (objectType == SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP)
    {
        if (shouldSkipCreation(vid, rid, createObject, [&](sai_object_id_t pgId) { return g_defaultPriorityGroupsRids.find(pgId) != g_defaultPriorityGroupsRids.end(); }))
        {
            SWSS_LOG_DEBUG("default priority group will not be created, processed VID 0x%lx to RID 0x%lx", vid, rid);
        }
    }
    else if (objectType == SAI_OBJECT_TYPE_SCHEDULER_GROUP)
    {
        if (shouldSkipCreation(vid, rid, createObject, [&](sai_object_id_t sgId) { return g_defaultSchedulerGroupsRids.find(sgId) != g_defaultSchedulerGroupsRids.end(); }))
        {
            SWSS_LOG_DEBUG("default scheduler group will not be created, processed VID 0x%lx to RID 0x%lx", vid, rid);
        }
    }
    else if (objectType == SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP)
    {
        if (shouldSkipCreation(vid, rid, createObject, [](sai_object_id_t id) { return id == redisGetDefaultTrapGroupId(); }))
        {
            SWSS_LOG_INFO("default trap group will not be created, processed VID 0x%lx to RID 0x%lx", vid, rid);
        }

        trapGroupWorkaround(vid, rid, createObject, attrCount, attrList);
    }
    else if (objectType == SAI_OBJECT_TYPE_PORT)
    {
        if (shouldSkipCreation(vid, rid, createObject, [](sai_object_id_t) { return true; }))
        {
            SWSS_LOG_INFO("port will not be created, processed VID 0x%lx to RID 0x%lx", vid, rid);
        }
    }

    if (createObject)
    {
        create_fn create = common_create[objectType];

        if (create == NULL)
        {
            SWSS_LOG_THROW("create function is not defined for object type %s", 
                    sai_serialize_object_type(objectType).c_str());
        }

        sai_status_t status = create(&rid, attrCount, attrList);

        if (status != SAI_STATUS_SUCCESS)
        {
            listFailedAttributes(objectType, attrCount, attrList);

            SWSS_LOG_THROW("failed to create object %s: %s", 
                    sai_serialize_object_type(objectType).c_str(), 
                    sai_serialize_status(status).c_str());
        }

        SWSS_LOG_DEBUG("created object of type %x, processed VID 0x%lx to RID 0x%lx", objectType, vid, rid);
    }
    else
    {
        SWSS_LOG_DEBUG("setting attributes on object of type %x, processed VID 0x%lx to RID 0x%lx", objectType, vid, rid);

        set_attribute_fn set = common_set_attribute[objectType];

        for (uint32_t idx = 0; idx < attrCount; idx++)
        {
            sai_attribute_t *attr = &attrList[idx];

            sai_status_t status = set(rid, attr);

            if (status != SAI_STATUS_SUCCESS)
            {
                auto meta = sai_metadata_get_attr_metadata(objectType, attr->id);

                if (meta == NULL)
                {
                    SWSS_LOG_ERROR("failed to get atribute metadata %d %d", objectType, attr->id);
                    exit_and_notify(EXIT_FAILURE);
                }

                SWSS_LOG_ERROR(
                        "failed to set %s value %s: %s",
                        meta->attridname,
                        sai_serialize_attr_value(*meta, *attr).c_str(),
                        sai_serialize_status(status).c_str());

                exit_and_notify(EXIT_FAILURE);
            }
        }
    }

    g_translated[vid] = rid;

    return rid;
}

void processAttributesForOids(sai_object_type_t objectType, std::shared_ptr<SaiAttributeList> list)
{
    SWSS_LOG_ENTER();

    SWSS_LOG_DEBUG("processing list for object type %d", objectType);

    sai_attribute_t *attrList = list->get_attr_list();

    uint32_t attrCount = list->get_attr_count();

    for (uint32_t idx = 0; idx < attrCount; idx++)
    {
        sai_attribute_t &attr = attrList[idx];

        auto meta = sai_metadata_get_attr_metadata(objectType, attr.id);

        if (meta == NULL)
        {
            SWSS_LOG_ERROR("unable to get metadata for object type %x, attribute %x", objectType, attr.id);
            exit_and_notify(EXIT_FAILURE);
        }

        uint32_t count = 0;
        sai_object_id_t *objectIdList;

        // TODO add enable flag
        switch (meta->attrvaluetype)
        {
            case SAI_ATTR_VALUE_TYPE_OBJECT_ID:
                count = 1;
                objectIdList = &attr.value.oid;
                break;

            case SAI_ATTR_VALUE_TYPE_OBJECT_LIST:
                count = attr.value.objlist.count;
                objectIdList = attr.value.objlist.list;
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_ID:
                count = 1;
                objectIdList = &attr.value.aclfield.data.oid;
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_LIST:
                count = attr.value.aclfield.data.objlist.count;
                objectIdList = attr.value.aclfield.data.objlist.list;
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_ID:
                count = 1;
                objectIdList = &attr.value.aclaction.parameter.oid;
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_LIST:
                count = attr.value.aclaction.parameter.objlist.count;
                objectIdList = attr.value.aclaction.parameter.objlist.list;
                break;

            default:
                continue;
        }

        // attribute contains object id's, they need to be translated
        // some of them could be already translated

        for (uint32_t j = 0; j < count; j++)
        {
            sai_object_id_t vid = objectIdList[j];

            sai_object_id_t rid = processSingleVid(vid);

            objectIdList[j] = rid;
        }
    }
}

sai_object_id_t getObjectIdFromString(const std::string &strObjectId)
{
    SWSS_LOG_ENTER();

    sai_object_id_t objectId;
    sai_deserialize_object_id(strObjectId, objectId);

    return objectId;
}

void processOids()
{
    for (auto &kv: g_oids)
    {
        const std::string &strObjectId = kv.first;

        sai_object_id_t vidObjectId = getObjectIdFromString(strObjectId);

        processSingleVid(vidObjectId);
    }
}

void processFdbs()
{
    SWSS_LOG_ENTER();

    // TODO process objects inside struct

    for (auto &kv: g_fdbs)
    {
        const std::string &strFdbEntry = kv.first;
        const std::string &asicKey = kv.second;

        sai_fdb_entry_t fdbEntry;
        sai_deserialize_fdb_entry(strFdbEntry, fdbEntry);

        std::shared_ptr<SaiAttributeList> list = g_attributesLists[asicKey];

        processAttributesForOids(SAI_OBJECT_TYPE_FDB_ENTRY, list);

        sai_attribute_t *attrList = list->get_attr_list();

        uint32_t attrCount = list->get_attr_count();

        sai_status_t status = sai_fdb_api->create_fdb_entry(&fdbEntry, attrCount, attrList);

        if (status != SAI_STATUS_SUCCESS)
        {
            SWSS_LOG_ERROR("failed to create_fdb_entry: %d", status);

            exit_and_notify(EXIT_FAILURE);
        }
    }
}

void processNeighbors()
{
    SWSS_LOG_ENTER();

    // TODO process objects inside struct
    for (auto &kv: g_neighbors)
    {
        const std::string &strNeighborEntry = kv.first;
        const std::string &asicKey = kv.second;

        sai_neighbor_entry_t neighborEntry;
        sai_deserialize_neighbor_entry(strNeighborEntry, neighborEntry);

        neighborEntry.rif_id = processSingleVid(neighborEntry.rif_id);

        std::shared_ptr<SaiAttributeList> list = g_attributesLists[asicKey];

        processAttributesForOids(SAI_OBJECT_TYPE_NEIGHBOR_ENTRY, list);

        sai_attribute_t *attrList = list->get_attr_list();

        uint32_t attrCount = list->get_attr_count();

        sai_status_t status = sai_neighbor_api->create_neighbor_entry(&neighborEntry, attrCount, attrList);

        if (status != SAI_STATUS_SUCCESS)
        {
            SWSS_LOG_ERROR("failed to create_neighbor_entry: %d", status);

            exit_and_notify(EXIT_FAILURE);
        }
    }
}

void processRoutes(bool defaultOnly)
{
    SWSS_LOG_ENTER();

    SWSS_LOG_TIMER("apply routes");

    // TODO process objects inside struct
    for (auto &kv: g_routes)
    {
        const std::string &strRouteEntry = kv.first;
        const std::string &asicKey = kv.second;

        bool isDefault = strRouteEntry.find("/0") != std::string::npos;

        if (defaultOnly ^ isDefault)
        {
            continue;
        }

        sai_route_entry_t routeEntry;
        sai_deserialize_route_entry(strRouteEntry, routeEntry);

        // TODO do this in automatic way usign stricts, it will impact speed
        routeEntry.vr_id = processSingleVid(routeEntry.vr_id);

        std::shared_ptr<SaiAttributeList> list = g_attributesLists[asicKey];

        processAttributesForOids(SAI_OBJECT_TYPE_ROUTE_ENTRY, list);

        sai_attribute_t *attrList = list->get_attr_list();

        uint32_t attrCount = list->get_attr_count();

        sai_status_t status = sai_route_api->create_route_entry(&routeEntry, attrCount, attrList);

        if (status != SAI_STATUS_SUCCESS)
        {
            SWSS_LOG_ERROR(
                    "failed to create ROUTE %s: %s",
                    sai_serialize_route_entry(routeEntry).c_str(),
                    sai_serialize_status(status).c_str());

            listFailedAttributes(SAI_OBJECT_TYPE_ROUTE_ENTRY, attrCount, attrList);

            exit_and_notify(EXIT_FAILURE);
        }
    }
}

// TODO at the end we need to repopulate switch attributes
// since on create we need only use the ones without object id's
