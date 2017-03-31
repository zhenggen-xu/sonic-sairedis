#include "syncd.h"
#include "sairedis.h"
#include "swss/tokenize.h"

#include <iostream>
#include <map>

std::mutex g_mutex;

std::shared_ptr<swss::RedisClient>          g_redisClient;
std::shared_ptr<swss::ProducerTable>        getResponse;
std::shared_ptr<swss::NotificationProducer> notifications;

std::map<std::string, std::string> gProfileMap;

/*
 * By default we are in APPLY mode.
 */
volatile bool g_asicInitViewMode = false;

struct cmdOptions
{
    int countersThreadIntervalInSeconds;
    bool diagShell;
    bool useTempView;
    int startType;
    bool disableCountersThread;
    std::string profileMapFile;
#ifdef SAITHRIFT
    bool run_rpc_server;
    std::string portMapFile;
#endif // SAITHRIFT
    ~cmdOptions() {}
};

cmdOptions options;

bool inline isInitViewMode()
{
    SWSS_LOG_ENTER();

    return g_asicInitViewMode && options.useTempView;
}

bool g_veryFirstRun = false;

void exit_and_notify(
        _In_ int status)
{
    SWSS_LOG_ENTER();

    try
    {
        if (notifications != NULL)
        {
            std::vector<swss::FieldValueTuple> entry;

            SWSS_LOG_NOTICE("sending switch_shutdown_request notification to OA");

            notifications->send("switch_shutdown_request", "", entry);

            SWSS_LOG_NOTICE("notification send successfull");
        }
    }
    catch(const std::exception &e)
    {
        SWSS_LOG_ERROR("Runtime error: %s", e.what());
    }
    catch(...)
    {
        SWSS_LOG_ERROR("Unknown runtime error");
    }

    exit(status);
}

void sai_diag_shell(
        _In_ sai_object_id_t switch_id)
{
    SWSS_LOG_ENTER();

    sai_status_t status;

    /*
     * This is currently blocking API on broadcom, it will block untill we exit
     * shell.
     */

    while (true)
    {
        sai_attribute_t attr;
        attr.id = SAI_SWITCH_ATTR_SWITCH_SHELL_ENABLE;
        attr.value.booldata = true;

        status = sai_metadata_sai_switch_api->set_switch_attribute(switch_id, &attr);

        if (status != SAI_STATUS_SUCCESS)
        {
            SWSS_LOG_ERROR("Failed to enable switch shell %d", status);
            return;
        }

        sleep(1);
    }
}

/*
 * Defined bit position on sairedis VID where object type and switch id is
 * located.
 */

#define OT_POSITION     48
#define SWID_POSITION   56

/*
 * NOTE: those redis functions could go to librediscommon etc so syncd could
 * link against it, so we don't have to duplicate code.
 */

sai_object_id_t redis_construct_object_id(
        _In_ sai_object_type_t object_type,
        _In_ int switch_index,
        _In_ uint64_t real_id)
{
    SWSS_LOG_ENTER();

    return (sai_object_id_t)(((uint64_t)switch_index << SWID_POSITION) | ((uint64_t)object_type << OT_POSITION) | real_id);
}

sai_object_type_t redis_sai_object_type_query(
        _In_ sai_object_id_t object_id)
{
    SWSS_LOG_ENTER();

    if (object_id == SAI_NULL_OBJECT_ID)
    {
        return SAI_OBJECT_TYPE_NULL;
    }

    sai_object_type_t ot = (sai_object_type_t)((object_id >> OT_POSITION) & 0xFF);

    /*
     * TODO: Later on use metadata utils.
     */

    if (ot == SAI_OBJECT_TYPE_NULL || ot >= SAI_OBJECT_TYPE_MAX)
    {
        SWSS_LOG_THROW("invalid object id 0x%lx", object_id);
    }

    return ot;
}

int redis_get_switch_id_index(
        _In_ sai_object_id_t switch_id)
{
    SWSS_LOG_ENTER();

    sai_object_type_t switch_object_type = redis_sai_object_type_query(switch_id);

    if (switch_object_type == SAI_OBJECT_TYPE_SWITCH)
    {
        return (int)((switch_id >> SWID_POSITION) & 0xFF);
    }

    SWSS_LOG_THROW("object type of switch %s is %s, should be SWITCH",
            sai_serialize_object_id(switch_id).c_str(),
            sai_serialize_object_type(switch_object_type).c_str());
}

sai_object_id_t redis_sai_switch_id_query(
        _In_ sai_object_id_t oid)
{
    SWSS_LOG_ENTER();

    if (oid == SAI_NULL_OBJECT_ID)
    {
        return oid;
    }

    sai_object_type_t object_type = sai_object_type_query(oid);

    if (object_type == SAI_OBJECT_TYPE_NULL)
    {
        SWSS_LOG_THROW("invalid object type of oid 0x%lx", oid);
    }

    if (object_type == SAI_OBJECT_TYPE_SWITCH)
    {
        return oid;
    }

    /*
     * Each VID contains switch index at constant position.
     *
     * We extract this index from VID and we create switch ID (VID) for
     * specific object. We can do this for each object.
     */

    int sw_index = (int)((oid >> SWID_POSITION) & 0xFF);

    sai_object_id_t switch_id = redis_construct_object_id(SAI_OBJECT_TYPE_SWITCH, sw_index, sw_index);

    return switch_id;
}

sai_object_id_t redis_create_virtual_object_id(
        _In_ sai_object_type_t object_type,
        _In_ sai_object_id_t switch_id)
{
    SWSS_LOG_ENTER();

    /*
     * NOTE: switch ID is VID switch ID from sairedis.
     */

    /*
     * Check if object type is in valid range.
     *
     * TODO: Later on we can use metadata utils.
     */

    if (object_type == SAI_OBJECT_TYPE_NULL || object_type >= SAI_OBJECT_TYPE_MAX)
    {
        SWSS_LOG_THROW("invalid object type: %s", sai_serialize_object_type(object_type).c_str());
    }

    /*
     * Switch id is deterministic and it comes from sairedis so make check here
     * that we will not use this for createing switch VIDs.
     */

    if (object_type == SAI_OBJECT_TYPE_SWITCH)
    {
        SWSS_LOG_THROW("this function should not be used to create VID for switch id");
    }

    uint64_t virtual_id = g_redisClient->incr(VIDCOUNTER);

    int switch_index =  redis_get_switch_id_index(switch_id);

    sai_object_id_t vid = redis_construct_object_id(object_type, switch_index, virtual_id);

    SWSS_LOG_DEBUG("created virtual object id 0x%lx for object type %d", vid, object_type);

    return vid;
}

std::unordered_map<sai_object_id_t, sai_object_id_t> local_rid_to_vid;
std::unordered_map<sai_object_id_t, sai_object_id_t> local_vid_to_rid;

void save_rid_and_vid_to_local(
        _In_ sai_object_id_t rid,
        _In_ sai_object_id_t vid)
{
    SWSS_LOG_ENTER();

    local_rid_to_vid[rid] = vid;
    local_vid_to_rid[vid] = rid;
}

void remove_rid_and_vid_from_local(
        _In_ sai_object_id_t rid,
        _In_ sai_object_id_t vid)
{
    SWSS_LOG_ENTER();

    local_rid_to_vid.erase(rid);
    local_vid_to_rid.erase(vid);
}

/*
 * This method will create VID for actual RID retrived from device when doing
 * GET api and snooping while in init view mode.
 *
 * This function should not be used to create VID for SWITCH object type.
 */
sai_object_id_t translate_rid_to_vid(
        _In_ sai_object_id_t rid,
        _In_ sai_object_id_t switch_id)
{
    SWSS_LOG_ENTER();

    /*
     * NOTE: switch_id here is Virtual ID of switch for which we need
     * create VID for given RID.
     */

    if (rid == SAI_NULL_OBJECT_ID)
    {
        SWSS_LOG_DEBUG("translated RID null to VID null");

        return SAI_NULL_OBJECT_ID;
    }

    auto it = local_rid_to_vid.find(rid);

    if (it != local_rid_to_vid.end())
    {
        return it->second;
    }

    sai_object_id_t vid;

    std::string str_rid = sai_serialize_object_id(rid);

    auto pvid = g_redisClient->hget(RIDTOVID, str_rid);

    if (pvid != NULL)
    {
        /*
         * Object exists.
         */

        std::string str_vid = *pvid;

        sai_deserialize_object_id(str_vid, vid);

        SWSS_LOG_DEBUG("translated RID 0x%lx to VID 0x%lx", rid, vid);

        return vid;
    }

    SWSS_LOG_INFO("spotted new RID 0x%lx", rid);

    sai_object_type_t object_type = sai_object_type_query(rid);

    if (object_type == SAI_OBJECT_TYPE_NULL)
    {
        SWSS_LOG_THROW("sai_object_type_query returned NULL type for RID 0x%lx", rid);
    }

    if (object_type == SAI_OBJECT_TYPE_SWITCH)
    {
        SWSS_LOG_THROW("RID 0x%lx is switch object, bug!");
    }

    vid = redis_create_virtual_object_id(object_type, switch_id);

    SWSS_LOG_DEBUG("translated RID 0x%lx to VID 0x%lx", rid, vid);

    std::string str_vid = sai_serialize_object_id(vid);

    /*
     * TODO: This must be ATOMIC.
     */

    g_redisClient->hset(RIDTOVID, str_rid, str_vid);
    g_redisClient->hset(VIDTORID, str_vid, str_rid);

    save_rid_and_vid_to_local(rid, vid);

    return vid;
}

void translate_list_rid_to_vid(
        _In_ sai_object_list_t &element,
        _In_ sai_object_id_t switch_id)
{
    SWSS_LOG_ENTER();

    for (uint32_t i = 0; i < element.count; i++)
    {
        element.list[i] = translate_rid_to_vid(element.list[i], switch_id);
    }
}

/*
 * This method is required to translate RID to VIDs when we are doing snoop for
 * new ID's in init view mode, on in apply view mode when we are executing GET
 * api, and new object RIDs were spotted the we will create new VIDs for those
 * objects and we will put them to redis db.
 */
void translate_rid_to_vid_list(
        _In_ sai_object_type_t object_type,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ sai_attribute_t *attr_list)
{
    SWSS_LOG_ENTER();

    /*
     * We receive real id's here, if they are new then create new VIDs for them
     * and put in db, if entry exists in db, use it.
     *
     * NOTE: switch_id is VID of switch on which those RIDs are probided.
     */

    for (uint32_t i = 0; i < attr_count; i++)
    {
        sai_attribute_t &attr = attr_list[i];

        auto meta = sai_metadata_get_attr_metadata(object_type, attr.id);

        if (meta == NULL)
        {
            SWSS_LOG_THROW("unable to get metadata for object type %x, attribute %d", object_type, attr.id);
        }

        /*
         * TODO: Many times we do switch for list of attributes to perform some
         * operation on each oid from that attribute, we should provide clever
         * way via sai metadata utils to get that.
         */

        switch (meta->attrvaluetype)
        {
            case SAI_ATTR_VALUE_TYPE_OBJECT_ID:
                attr.value.oid = translate_rid_to_vid(attr.value.oid, switch_id);
                break;

            case SAI_ATTR_VALUE_TYPE_OBJECT_LIST:
                translate_list_rid_to_vid(attr.value.objlist, switch_id);
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_ID:
                if (attr.value.aclfield.enable)
                    attr.value.aclfield.data.oid = translate_rid_to_vid(attr.value.aclfield.data.oid, switch_id);
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_LIST:
                if (attr.value.aclfield.enable)
                    translate_list_rid_to_vid(attr.value.aclfield.data.objlist, switch_id);
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_ID:
                if (attr.value.aclaction.enable)
                    attr.value.aclaction.parameter.oid = translate_rid_to_vid(attr.value.aclaction.parameter.oid, switch_id);
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_LIST:
                if (attr.value.aclaction.enable)
                    translate_list_rid_to_vid(attr.value.aclaction.parameter.objlist, switch_id);
                break;

            default:

                /*
                 * If in futre new attribute with object id will be added this
                 * will make sure that we will need to add handler here.
                 */

                /*
                 * TODO: Add metadata field for this.
                 */

                if (meta->allowedobjecttypeslength > 0)
                {
                    SWSS_LOG_THROW("attribute %s is object id, but not processed, FIXME");
                }

                break;
        }
    }
}

/*
 * NOTE: We could have in metadata utils option to execute function on each
 * object on oid like this.  Problem is that we can't then add extra
 * parameters.
 */

sai_object_id_t translate_vid_to_rid(
        _In_ sai_object_id_t vid)
{
    SWSS_LOG_ENTER();

    if (vid == SAI_NULL_OBJECT_ID)
    {
        SWSS_LOG_DEBUG("translated RID null to VID null");

        return SAI_NULL_OBJECT_ID;
    }

    auto it = local_vid_to_rid.find(vid);

    if (it != local_vid_to_rid.end())
    {
        return it->second;
    }

    std::string str_vid = sai_serialize_object_id(vid);

    std::string str_rid;

    auto prid = g_redisClient->hget(VIDTORID, str_vid);

    if (prid == NULL)
    {
        if (isInitViewMode())
        {
            /*
             * If user created object that is object id, then it should not
             * query attributes of this object in init view mode, because he
             * knows all attributes passed to that object.
             */

            SWSS_LOG_THROW("can't get RID in init view mode - don't query created objects");
        }

        SWSS_LOG_THROW("unable to get RID for VID: 0x%lx", vid);
    }

    str_rid = *prid;

    sai_object_id_t rid;

    sai_deserialize_object_id(str_rid, rid);

    /*
     * We got this RID from redis db, so put it also to local db so it will be
     * faster to retrive it late on.
     */

    local_vid_to_rid[vid] = rid;

    SWSS_LOG_DEBUG("translated VID 0x%lx to RID 0x%lx", vid, rid);

    return rid;
}

void translate_list_vid_to_rid(
        _In_ sai_object_list_t &element)
{
    for (uint32_t i = 0; i < element.count; i++)
    {
        element.list[i] = translate_vid_to_rid(element.list[i]);
    }
}

void translate_vid_to_rid_list(
        _In_ sai_object_type_t object_type,
        _In_ uint32_t attr_count,
        _In_ sai_attribute_t *attr_list)
{
    SWSS_LOG_ENTER();

    /*
     * All id's received from sairedis should be virtual, so lets translate
     * them to real id's before we execute actual api.
     */

    for (uint32_t i = 0; i < attr_count; i++)
    {
        sai_attribute_t &attr = attr_list[i];

        auto meta = sai_metadata_get_attr_metadata(object_type, attr.id);

        if (meta == NULL)
        {
            SWSS_LOG_THROW("unable to get metadata for object type %x, attribute %d", object_type, attr.id);
        }

        switch (meta->attrvaluetype)
        {
            case SAI_ATTR_VALUE_TYPE_OBJECT_ID:
                attr.value.oid = translate_vid_to_rid(attr.value.oid);
                break;

            case SAI_ATTR_VALUE_TYPE_OBJECT_LIST:
                translate_list_vid_to_rid(attr.value.objlist);
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_ID:
                if (attr.value.aclfield.enable)
                    attr.value.aclfield.data.oid = translate_vid_to_rid(attr.value.aclfield.data.oid);
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_LIST:
                if (attr.value.aclfield.enable)
                    translate_list_vid_to_rid(attr.value.aclfield.data.objlist);
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_ID:
                if (attr.value.aclaction.enable)
                    attr.value.aclaction.parameter.oid = translate_vid_to_rid(attr.value.aclaction.parameter.oid);
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_LIST:
                if (attr.value.aclaction.enable)
                    translate_list_vid_to_rid(attr.value.aclaction.parameter.objlist);
                break;

            default:

                /*
                 * If in futre new attribute with object id will be added this
                 * will make sure that we will need to add handler here.
                 */

                /*
                 * TODO: Add metadata field for this.
                 */

                if (meta->allowedobjecttypeslength > 0)
                {
                    SWSS_LOG_THROW("attribute %s is object id, but not processed, FIXME");
                }

                break;
        }
    }
}

void snoop_get_attr(
        _In_ sai_object_type_t object_type,
        _In_ const std::string &str_object_id,
        _In_ const std::string &attr_id,
        _In_ const std::string &attr_value)
{
    SWSS_LOG_ENTER();

    /*
     * Note: str_object_type + ":" + str_object_id is meta_key we can us that
     * here later on.
     */

    std::string str_object_type = sai_serialize_object_type(object_type);

    std::string key = TEMP_PREFIX + (ASIC_STATE_TABLE + (":" + str_object_type + ":" + str_object_id));

    SWSS_LOG_DEBUG("%s", key.c_str());

    g_redisClient->hset(key, attr_id, attr_value);
}

void snoop_get_oid(
        _In_ sai_object_id_t vid)
{
    SWSS_LOG_ENTER();

    if (vid == SAI_NULL_OBJECT_ID)
    {
        /*
         * If snooped ois is NULL then we don't need take any action.
         */

        return;
    }

    /*
     * We need use redis version of object type query here since we are
     * operating on VID value, and syncd is compiled agains real SAI
     * implementation which has diffrent function sai_object_type_query.
     */

    sai_object_type_t object_type = redis_sai_object_type_query(vid);

    std::string str_vid = sai_serialize_object_id(vid);

    snoop_get_attr(object_type, str_vid, "NULL", "NULL");
}

void snoop_get_oid_list(
        _In_ const sai_object_list_t &list)
{
    SWSS_LOG_ENTER();

    for (uint32_t i = 0; i < list.count; i++)
    {
        snoop_get_oid(list.list[i]);
    }
}

void snoop_get_attr_value(
        _In_ const std::string &str_object_id,
        _In_ const sai_attr_metadata_t *meta,
        _In_ const sai_attribute_t &attr)
{
    SWSS_LOG_ENTER();

    std::string value = sai_serialize_attr_value(*meta, attr);

    SWSS_LOG_NOTICE("%s:%s", meta->attridname, value.c_str());

    snoop_get_attr(meta->objecttype, str_object_id, meta->attridname, value);
}

void snoop_get_response(
        _In_ sai_object_type_t object_type,
        _In_ const std::string &str_object_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    SWSS_LOG_ENTER();

    /*
     * NOTE: this method is operating on VIDs, all RIDs were translated outside
     * this method.
     */

    /*
     * Vlan (including vlan 1) will need to be put into TEMP view this should
     * also be valid for all objects that were queried.
     */

    for (uint32_t idx = 0; idx < attr_count; ++idx)
    {
        const sai_attribute_t &attr = attr_list[idx];

        auto meta = sai_metadata_get_attr_metadata(object_type, attr.id);

        if (meta == NULL)
        {
            SWSS_LOG_THROW("unable to get metadata for object type %d, attribute %d", object_type, attr.id);
        }

        /*
         * We should snoop oid values even if they are readonly we just note in
         * temp view that those objects exist on switch.
         */

        switch (meta->attrvaluetype)
        {
            case SAI_ATTR_VALUE_TYPE_OBJECT_ID:
                snoop_get_oid(attr.value.oid);
                break;

            case SAI_ATTR_VALUE_TYPE_OBJECT_LIST:
                snoop_get_oid_list(attr.value.objlist);
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_ID:
                if (attr.value.aclfield.enable)
                    snoop_get_oid(attr.value.aclfield.data.oid);
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_LIST:
                if (attr.value.aclfield.enable)
                    snoop_get_oid_list(attr.value.aclfield.data.objlist);
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_ID:
                if (attr.value.aclaction.enable)
                    snoop_get_oid(attr.value.aclaction.parameter.oid);
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_LIST:
                if (attr.value.aclaction.enable)
                    snoop_get_oid_list(attr.value.aclaction.parameter.objlist);
                break;

            default:

                /*
                 * If in futre new attribute with object id will be added this
                 * will make sure that we will need to add handler here.
                 */

                /*
                 * TODO: Add metadata field for this.
                 */

                if (meta->allowedobjecttypeslength > 0)
                {
                    SWSS_LOG_THROW("attribute %s is object id, but not processed, FIXME");
                }

                break;
        }

        if (HAS_FLAG_READ_ONLY(meta->flags))
        {
            /*
             * If value is read only, we skip it, since after syncd restart we
             * won't be able to set/create it anyway.
             */

            continue;
        }

        if (meta->objecttype == SAI_OBJECT_TYPE_PORT &&
                meta->attrid == SAI_PORT_ATTR_HW_LANE_LIST)
        {
            /*
             * Skip port lanes for now since we don't create ports.
             */

            SWSS_LOG_INFO("skipping %s for %s", meta->attridname, str_object_id.c_str());
            continue;
        }

        /*
         * Put non readonly, and non oid attribute value to temp view.
         */

        snoop_get_attr_value(str_object_id, meta, attr);
    }
}

void internal_syncd_get_send(
        _In_ sai_object_type_t object_type,
        _In_ const std::string &str_object_id,
        _In_ sai_status_t status,
        _In_ uint32_t attr_count,
        _In_ sai_attribute_t *attr_list)
{
    SWSS_LOG_ENTER();

    std::vector<swss::FieldValueTuple> entry;

    if (status == SAI_STATUS_SUCCESS)
    {
        translate_rid_to_vid_list(object_type, attr_count, attr_list);

        /*
         * Normal serialization + translate RID to VID.
         */

        entry = SaiAttributeList::serialize_attr_list(
                object_type,
                attr_count,
                attr_list,
                false);

        if (isInitViewMode())
        {
            /*
             * All oid values here are VIDs.
             */

            snoop_get_response(object_type, str_object_id, attr_count, attr_list);
        }

        /*
         * TODO: When we are doing GET in non init view mode, maybe we could
         * snoop data also, since we will put this data anyway when we will do
         * view compare. We would need to fix snoop_get_response since
         * currently this method is writing only to TEMP view.
         */
    }
    else if (status == SAI_STATUS_BUFFER_OVERFLOW)
    {
        /*
         * In this case we got correct values for list, but list was too small
         * so serialize only count without list itself, sairedis will need to
         * take this into account when deseralzie.
         *
         * If there was a list somewhere, count will be changed to actual value
         * different attributes can have different lists, many of them may
         * serialize only count, and will need to support that on the receiver.
         */

        entry = SaiAttributeList::serialize_attr_list(
                object_type,
                attr_count,
                attr_list,
                true);
    }
    else
    {
        /*
         * Some other error, don't send attributes at all.
         */
    }

    std::string str_status = sai_serialize_status(status);

    SWSS_LOG_INFO("sending response for GET api with status: %s", str_status.c_str());

    /*
     * Since we have only one get at a time, we don't have to serialize object
     * type and object id, only get status is required to be returned.  Get
     * response will not put any data to table, only queue is used.
     */

    getResponse->set(str_status, entry, "getresponse");

    SWSS_LOG_INFO("response for GET api was send");
}

const char* profile_get_value(
        _In_ sai_switch_profile_id_t profile_id,
        _In_ const char* variable)
{
    SWSS_LOG_ENTER();

    if (variable == NULL)
    {
        SWSS_LOG_WARN("variable is null");
        return NULL;
    }

    auto it = gProfileMap.find(variable);

    if (it == gProfileMap.end())
    {
        SWSS_LOG_INFO("%s: NULL", variable);
        return NULL;
    }

    SWSS_LOG_INFO("%s: %s", variable, it->second.c_str());

    return it->second.c_str();
}

std::map<std::string, std::string>::iterator gProfileIter = gProfileMap.begin();

int profile_get_next_value(
        _In_ sai_switch_profile_id_t profile_id,
        _Out_ const char** variable,
        _Out_ const char** value)
{
    SWSS_LOG_ENTER();

    if (value == NULL)
    {
        SWSS_LOG_INFO("resetting profile map iterator");

        gProfileIter = gProfileMap.begin();
        return 0;
    }

    if (variable == NULL)
    {
        SWSS_LOG_WARN("variable is null");
        return -1;
    }

    if (gProfileIter == gProfileMap.end())
    {
        SWSS_LOG_INFO("iterator reached end");
        return -1;
    }

    *variable = gProfileIter->first.c_str();
    *value = gProfileIter->second.c_str();

    SWSS_LOG_INFO("key: %s:%s", *variable, *value);

    gProfileIter++;

    return 0;
}

service_method_table_t test_services = {
    profile_get_value,
    profile_get_next_value
};

sai_status_t handle_generic(
        _In_ sai_object_type_t object_type,
        _In_ const std::string &str_object_id,
        _In_ sai_common_api_t api,
        _In_ uint32_t attr_count,
        _In_ sai_attribute_t *attr_list)
{
    SWSS_LOG_ENTER();

    // TODO we need special case for SWITCH

    // TODO switch

    sai_object_id_t switch_id = SAI_NULL_OBJECT_ID; // TODO populate !

    sai_object_id_t object_id;
    sai_deserialize_object_id(str_object_id, object_id); // TODO could deserialize to meta key

    SWSS_LOG_DEBUG("calling %s for %s",
            sai_serialize_common_api(api).c_str(),
            sai_serialize_object_type(object_type).c_str());

    sai_object_meta_key_t meta_key;

    meta_key.objecttype = object_type;
    meta_key.objectkey.key.object_id = object_id;

    // TODO we need to do translate vid/rid except for create

    /*
     * TODO: use metadata utils.
     */

    auto info = sai_all_object_type_infos[object_type];

    if (info->isnonobjectid)
    {
        SWSS_LOG_THROW("passing non object id %s as generic object", info->objecttypename);
    }

    switch (api)
    {
        case SAI_COMMON_API_CREATE:

            {
                sai_status_t status = info->create(&meta_key, switch_id, attr_count, attr_list);

                if (status == SAI_STATUS_SUCCESS)
                {
                    sai_object_id_t real_object_id = meta_key.objectkey.key.object_id;

                    // object was created so new object id was generated
                    // we need to save virtual id's to redis db

                    std::string str_vid = sai_serialize_object_id(object_id);
                    std::string str_rid = sai_serialize_object_id(real_object_id);

                    g_redisClient->hset(VIDTORID, str_vid, str_rid);
                    g_redisClient->hset(RIDTOVID, str_rid, str_vid);

                    save_rid_and_vid_to_local(real_object_id, object_id);

                    SWSS_LOG_INFO("saved VID %s to RID %s", str_vid.c_str(), str_rid.c_str());
                }

                return status;
            }

        case SAI_COMMON_API_REMOVE:

            {
                sai_object_id_t rid = translate_vid_to_rid(object_id);

                meta_key.objectkey.key.object_id = rid;

                sai_status_t status = info->remove(&meta_key);

                if (status == SAI_STATUS_SUCCESS)
                {
                    std::string str_vid = sai_serialize_object_id(object_id);
                    std::string str_rid = sai_serialize_object_id(rid);

                    g_redisClient->hdel(VIDTORID, str_vid);
                    g_redisClient->hdel(RIDTOVID, str_rid);

                    remove_rid_and_vid_from_local(rid, object_id);

                    // TODO on remove switch there should be extra action
                    // all local obejcts and redis object sshould be removed on remove switch
                }

                return status;
            }

        case SAI_COMMON_API_SET:

            {
                sai_object_id_t rid = translate_vid_to_rid(object_id);

                meta_key.objectkey.key.object_id = rid;

                return info->set(&meta_key, attr_list);
            }

        case SAI_COMMON_API_GET:

            {
                sai_object_id_t rid = translate_vid_to_rid(object_id);

                meta_key.objectkey.key.object_id = rid;

                return info->get(&meta_key, attr_count, attr_list);
            }

        default:

            SWSS_LOG_THROW("common api (%s) is not implemented", sai_serialize_common_api(api).c_str());
    }
}

/*
 * TODO: If we add deserialize auto generated from metadata then we can have
 * one function for non object id objects that will iterate through structures
 * automatically.
 */

sai_status_t handle_fdb(
        _In_ const std::string &str_object_id,
        _In_ sai_common_api_t api,
        _In_ uint32_t attr_count,
        _In_ sai_attribute_t *attr_list)
{
    SWSS_LOG_ENTER();

    sai_fdb_entry_t fdb_entry;
    sai_deserialize_fdb_entry(str_object_id, fdb_entry);

    fdb_entry.switch_id = translate_vid_to_rid(fdb_entry.switch_id);
    fdb_entry.bridge_id = translate_vid_to_rid(fdb_entry.bridge_id);

    switch (api)
    {
        case SAI_COMMON_API_CREATE:
            return sai_metadata_sai_fdb_api->create_fdb_entry(&fdb_entry, attr_count, attr_list);

        case SAI_COMMON_API_REMOVE:
            return sai_metadata_sai_fdb_api->remove_fdb_entry(&fdb_entry);

        case SAI_COMMON_API_SET:
            return sai_metadata_sai_fdb_api->set_fdb_entry_attribute(&fdb_entry, attr_list);

        case SAI_COMMON_API_GET:
            return sai_metadata_sai_fdb_api->get_fdb_entry_attribute(&fdb_entry, attr_count, attr_list);

        default:
            SWSS_LOG_THROW("fdb other apis not implemented");
    }
}

sai_status_t handle_neighbor(
        _In_ const std::string &str_object_id,
        _In_ sai_common_api_t api,
        _In_ uint32_t attr_count,
        _In_ sai_attribute_t *attr_list)
{
    SWSS_LOG_ENTER();

    sai_neighbor_entry_t neighbor_entry;
    sai_deserialize_neighbor_entry(str_object_id, neighbor_entry);

    neighbor_entry.switch_id = translate_vid_to_rid(neighbor_entry.switch_id);
    neighbor_entry.rif_id = translate_vid_to_rid(neighbor_entry.rif_id);

    switch (api)
    {
        case SAI_COMMON_API_CREATE:
            return sai_metadata_sai_neighbor_api->create_neighbor_entry(&neighbor_entry, attr_count, attr_list);

        case SAI_COMMON_API_REMOVE:
            return sai_metadata_sai_neighbor_api->remove_neighbor_entry(&neighbor_entry);

        case SAI_COMMON_API_SET:
            return sai_metadata_sai_neighbor_api->set_neighbor_entry_attribute(&neighbor_entry, attr_list);

        case SAI_COMMON_API_GET:
            return sai_metadata_sai_neighbor_api->get_neighbor_entry_attribute(&neighbor_entry, attr_count, attr_list);

        default:
            SWSS_LOG_THROW("neighbor other apis not implemented");
    }
}

sai_status_t handle_route(
        _In_ const std::string &str_object_id,
        _In_ sai_common_api_t api,
        _In_ uint32_t attr_count,
        _In_ sai_attribute_t *attr_list)
{
    SWSS_LOG_ENTER();

    sai_route_entry_t route_entry;
    sai_deserialize_route_entry(str_object_id, route_entry);

    route_entry.switch_id = translate_vid_to_rid(route_entry.switch_id);
    route_entry.vr_id = translate_vid_to_rid(route_entry.vr_id);

    switch (api)
    {
        case SAI_COMMON_API_CREATE:
            return sai_metadata_sai_route_api->create_route_entry(&route_entry, attr_count, attr_list);

        case SAI_COMMON_API_REMOVE:
            return sai_metadata_sai_route_api->remove_route_entry(&route_entry);

        case SAI_COMMON_API_SET:
            return sai_metadata_sai_route_api->set_route_entry_attribute(&route_entry, attr_list);

        case SAI_COMMON_API_GET:
            return sai_metadata_sai_route_api->get_route_entry_attribute(&route_entry, attr_count, attr_list);

        default:
            SWSS_LOG_THROW("route other apis not implemented");
    }
}

void sendNotifyResponse(
        _In_ sai_status_t status)
{
    SWSS_LOG_ENTER();

    std::string str_status = sai_serialize_status(status);

    std::vector<swss::FieldValueTuple> entry;

    SWSS_LOG_NOTICE("sending response: %s", str_status.c_str());

    getResponse->set(str_status, entry, "notify");
}

void clearTempView()
{
    SWSS_LOG_ENTER();

    SWSS_LOG_NOTICE("clearing current TEMP VIEW");

    SWSS_LOG_TIMER("clear temp view");

    std::string pattern = TEMP_PREFIX + (ASIC_STATE_TABLE + std::string(":*"));

    /*
     * TODO this must be ATOMIC, and could use lua script.
     *
     * We need to expose api to execute user lua script not only predefined.
     */

    for (const auto &key: g_redisClient->keys(pattern))
    {
        g_redisClient->del(key);
    }
}

sai_status_t notifySyncd(
        _In_ const std::string& op)
{
    SWSS_LOG_ENTER();

    if (!options.useTempView)
    {
        SWSS_LOG_NOTICE("received %s, ignored since TEMP VIEW is not used, returning success", op.c_str());

        sendNotifyResponse(SAI_STATUS_SUCCESS);

        return SAI_STATUS_SUCCESS;
    }

    if (g_veryFirstRun)
    {
        SWSS_LOG_NOTICE("very first run is TRUE, op = %s", op.c_str());

        /*
         * On the very first start of syncd, "compile" view is directly applied
         * on device, since it will make it easier to switch to new asic state
         * later on when we restart orch agent.
         */

        if (op == SYNCD_INIT_VIEW)
        {
            /*
             * On first start we just do "apply" directly on asic so we set
             * init to false instead of true.
             */

            g_asicInitViewMode = false;

            /*
             * We need to clear current temp view to make space for new one.
             */

            clearTempView();
        }
        else if (op == SYNCD_APPLY_VIEW)
        {
            g_veryFirstRun = false;

            g_asicInitViewMode = false;

            SWSS_LOG_NOTICE("setting very first run to FALSE, op = %s", op.c_str());
        }
        else
        {
            SWSS_LOG_THROW("unknown operation: %s", op.c_str());
        }

        sendNotifyResponse(SAI_STATUS_SUCCESS);

        return SAI_STATUS_SUCCESS;
    }

    if (op == SYNCD_INIT_VIEW)
    {
        if (g_asicInitViewMode)
        {
            SWSS_LOG_WARN("syncd is already in asic INIT VIEW mode, but received init again, orchagent restarted before apply?");
        }

        g_asicInitViewMode = true;

        clearTempView();

        /*
         * TODO: Currently as WARN to be easier to spoot, later should be NOTICE.
         */

        SWSS_LOG_WARN("syncd switched to INIT VIEW mode, all op will be saved to TEMP view");

        sendNotifyResponse(SAI_STATUS_SUCCESS);
    }
    else if (op == SYNCD_APPLY_VIEW)
    {
        g_asicInitViewMode = false;

        /*
         * TODO: Currently as WARN to be easier to spoot, later should be NOTICE.
         */

        SWSS_LOG_WARN("syncd received APPLY VIEW, will translate");

        sai_status_t status = syncdApplyView();

        sendNotifyResponse(status);

        if (status == SAI_STATUS_SUCCESS)
        {
            /*
             * We succesfully applied new view, VID mapping could change, so we
             * need to clear local db, and all new VIDs will be queried using
             * redis.
             */

            local_rid_to_vid.clear();
            local_vid_to_rid.clear();
        }
    }
    else
    {
        SWSS_LOG_ERROR("unknown operation: %s", op.c_str());

        sendNotifyResponse(SAI_STATUS_NOT_IMPLEMENTED);

        SWSS_LOG_THROW("notify syncd %s operation failed", op.c_str());
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t processEventInInitViewMode(
        _In_ sai_object_type_t object_type,
        _In_ const std::string &str_object_id,
        _In_ sai_common_api_t api,
        _In_ uint32_t attr_count,
        _In_ sai_attribute_t *attr_list)
{
    SWSS_LOG_ENTER();

    /* 
     * Since attributes are not checked, it may happen that user will send some
     * invalid VID in object id/list in attribute, metadata should handle that,
     * but if that happen, this id will be treated as "new" object instead of
     * existing one.
     */

    /*
     * TODO: use metadata utils.
     */

    auto info = sai_all_object_type_infos[object_type];

    if (api == SAI_COMMON_API_CREATE)
    {
        if (info->isnonobjectid)
        {
            // we assume create of those non object id object types will succeed
        }
        else
        {
            // case SAI_OBJECT_TYPE_SWITCH: // TODO for this should be special ? in redis ?
            // TODO for switch this will not work ! damn !
            // // BLOCKER
            //
            // TODO we have 2 options here, we match existing switch (even if multiple)
            // by SAI_SWITCH_ATTR_SWITCH_HARDWARE_INFO, if they are different
            // then we need here to create new switch
            // thats why we can keep track for all switches
            // and then we can do comparison and GET will work then

            // BLOCKER
            //
            // also this will be a problem if orhagent will create the same switch
            // with the same hardware info but with different order
            // since switch id is deterministic, then VID of both switches will not match
            //
            // first we can have INFO = "A" swid 0x00170000
            // te                INFO = "B" swid 0x01170001
            //
            // // how to deal with that ?, just throw in that case ? - so far yes !

            // first we can have INFO = "B" swid 0x00170000
            // te                INFO = "!" swid 0x01170001

            sai_object_id_t object_id;
            sai_deserialize_object_id(str_object_id, object_id);

            // object ID here is actual VID returned from redis during creation
            // this is floating VID in init view mode

            SWSS_LOG_DEBUG("generic create (init view) for %s, floating VID: %s",
                    sai_serialize_object_type(object_type).c_str(),
                    sai_serialize_object_id(object_id).c_str());

            SWSS_LOG_THROW("CREATE not supportred yet, FIXME");
        }

        return SAI_STATUS_SUCCESS;
    }

    if (api == SAI_COMMON_API_GET)
    {
        sai_status_t status;

        if (info->isnonobjectid)
        {
            // those object's are user created, so if user created ROUTE
            // he passed some attributes, there is no sense to support GET
            // since user explicitly know what attributes were set, similar
            // for other object types here
            //

            SWSS_LOG_ERROR("get is not supported on %s in init view mode", sai_serialize_object_type(object_type).c_str());

            status = SAI_STATUS_NOT_SUPPORTED;
        }
        else
        {
            sai_object_id_t object_id;
            sai_deserialize_object_id(str_object_id, object_id);

            SWSS_LOG_DEBUG("generic get (init view) for object type %s", sai_serialize_object_type(object_type).c_str());

            // object must exists, we can't call GET on created object in init view mode
            // get here can be called on existing objects like default trap group to
            // get some vendor specific values

            // TODO switch must exist to do GET !

            sai_object_id_t rid = translate_vid_to_rid(object_id);

            sai_object_meta_key_t meta_key;

            meta_key.objecttype = object_type;
            meta_key.objectkey.key.object_id = rid;

            status = info->get(&meta_key, attr_count, attr_list);
        }

        internal_syncd_get_send(object_type, str_object_id, status, attr_count, attr_list);

        return status;
    }

    if (object_type == SAI_OBJECT_TYPE_SWITCH && api == SAI_COMMON_API_REMOVE)
    {
        SWSS_LOG_ERROR("remove switch is not supported yet! FIXME");
        exit_and_notify(EXIT_FAILURE);

        // special care needs to be taken to clear all this switch id's from all db's
        // TODO make this proper comment
    }

    // TODO remove of the switch should be also a case here

    // we assume that SET and REMOVE succeeded, no real asic operations
    // this is only DB operation in TEMP asic view

    if (api == SAI_COMMON_API_SET)
    {
        return SAI_STATUS_SUCCESS;
    }

    if (api == SAI_COMMON_API_REMOVE)
    {
        return SAI_STATUS_SUCCESS;
    }

    /*
     * We don't support bulk API's in init view mode yet.
     */

    SWSS_LOG_ERROR("api %d is not supported in init view mode", api);
    exit_and_notify(EXIT_FAILURE);
}

sai_status_t handle_bulk_route(
        _In_ const std::vector<std::string> &object_ids,
        _In_ sai_common_api_t api,
        _In_ const std::vector<std::shared_ptr<SaiAttributeList>> &attributes)
{
    SWSS_LOG_ENTER();

    /*
     * Since we don't have asic support yet for bulk api, just execute one by
     * one.
     */

    for (size_t idx = 0; idx < object_ids.size(); ++idx)
    {
        sai_status_t status = SAI_STATUS_FAILURE;

        auto &list = attributes[idx];

        sai_attribute_t *attr_list = list->get_attr_list();
        uint32_t attr_count = list->get_attr_count();

        if (api == (sai_common_api_t)SAI_COMMON_API_BULK_SET)
        {
            status = handle_route(object_ids[idx], SAI_COMMON_API_SET, attr_count, attr_list);
        }
        else
        {
            SWSS_LOG_ERROR("api %d is not supported in bulk route", api);
            exit_and_notify(EXIT_FAILURE);
        }

        if (status != SAI_STATUS_SUCCESS)
        {
            return status;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t processEvent(
        _In_ swss::ConsumerTable &consumer)
{
    std::lock_guard<std::mutex> lock(g_mutex);

    SWSS_LOG_ENTER();

    swss::KeyOpFieldsValuesTuple kco;

    if (isInitViewMode())
    {
        /*
         * In init mode we put all data to TEMP view and we snoop.  We need to
         * specify temporary view prefis in consumer since consumer puts data
         * to redis db.
         */

        consumer.pop(kco, TEMP_PREFIX);
    }
    else
    {
        consumer.pop(kco);
    }

    const std::string &key = kfvKey(kco);
    const std::string &op = kfvOp(kco);

    /*
     * TODO: Key is serialized meta_key, we could use deserialize
     * to extract it here.
     */

    const std::string &str_object_type = key.substr(0, key.find(":"));
    const std::string &str_object_id = key.substr(key.find(":") + 1);

    SWSS_LOG_INFO("key: %s op: %s", key.c_str(), op.c_str());

    sai_common_api_t api = SAI_COMMON_API_MAX;

    if (op == "create")
    {
        api = SAI_COMMON_API_CREATE;
    }
    else if (op == "remove")
    {
        api = SAI_COMMON_API_REMOVE;
    }
    else if (op == "set")
    {
        api = SAI_COMMON_API_SET;
    }
    else if (op == "get")
    {
        api = SAI_COMMON_API_GET;
    }
    else if (op == "notify")
    {
        return notifySyncd(key);
    }
    else
    {
        SWSS_LOG_THROW("api %s is not implemented", op.c_str());
    }

    std::string str_object_type = key.substr(0, key.find(":"));
    std::string str_object_id = key.substr(key.find(":") + 1);

    sai_object_type_t object_type;
    sai_deserialize_object_type(str_object_type, object_type);

    /*
     * TODO: use metadata utils
     */

    if (object_type == SAI_OBJECT_TYPE_NULL || object_type >= SAI_OBJECT_TYPE_MAX)
    {
        SWSS_LOG_THROW("undefined object type %s", sai_serialize_object_type(object_type).c_str());
    }

    const std::vector<swss::FieldValueTuple> &values = kfvFieldsValues(kco);

    SaiAttributeList list(object_type, values, false);
    
    /*
     * Attribute list can't be const since we will use it to translate VID to
     * RID inplace.
     */

    sai_attribute_t *attr_list = list.get_attr_list();
    uint32_t attr_count = list.get_attr_count();

===========
    // TODO should this be before init view mode or after ?

    if (object_type == SAI_OBJECT_TYPE_SWITCH && (api == SAI_COMMON_API_CREATE || api == SAI_COMMON_API_SET))
    {
        check_notifications_pointers(attr_count, attr_list);
    }

    if (isInitViewMode())
    {
        return processEventInInitViewMode(object_type, str_object_id, api, attr_count, attr_list);
    }

    if (api != SAI_COMMON_API_GET)
    {
        translate_vid_to_rid_list(object_type, attr_count, attr_list);
    }

    auto info = sai_all_object_type_infos[object_type];

    sai_status_t status;

    switch (object_type)
    {
        case SAI_OBJECT_TYPE_FDB_ENTRY:
            status = handle_fdb(str_object_id, api, attr_count, attr_list);
            break;

        case SAI_OBJECT_TYPE_NEIGHBOR_ENTRY:
            status = handle_neighbor(str_object_id, api, attr_count, attr_list);
            break;

        case SAI_OBJECT_TYPE_ROUTE_ENTRY:
            status = handle_route(str_object_id, api, attr_count, attr_list);
            break;

        default:

            if (info->isnonobjectid)
            {
                SWSS_LOG_THROW("passing non object id %s as generic object", info->objecttypename);
            }

            status = handle_generic(object_type, str_object_id, api, attr_count, attr_list);
            break;
    }

    if (api == SAI_COMMON_API_GET)
    {
        if (status != SAI_STATUS_SUCCESS)
        {
            SWSS_LOG_WARN("get API for key: %s op: %s returned status: %s", key.c_str(), op.c_str(),
                    sai_serialize_status(status).c_str());
        }

        internal_syncd_get_send(object_type, str_object_id, status, attr_count, attr_list);
    }
    else if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_ERROR("failed to execute api: %s, key: %s, status: %s", op.c_str(), key.c_str(), sai_serialize_status(status).c_str());

        for (const auto&v: values)
        {
            SWSS_LOG_ERROR(" field: %s, value: %s", fvField(v).c_str(), fvValue(v).c_str());
        }

        exit_and_notify(EXIT_FAILURE);
    }

    return status;
}

void printUsage()
{
    std::cout << "Usage: syncd [-N] [-d] [-p profile] [-i interval] [-t [cold|warm|fast]] [-h] [-u]" << std::endl;
    std::cout << "    -N --nocounters:" << std::endl;
    std::cout << "        Disable counter thread" << std::endl;
    std::cout << "    -d --diag:" << std::endl;
    std::cout << "        Enable diagnostic shell" << std::endl;
    std::cout << "    -p --profile profile:" << std::endl;
    std::cout << "        Provide profile map file" << std::endl;
    std::cout << "    -i --countersInterval interval:" << std::endl;
    std::cout << "        Provide counter thread interval" << std::endl;
    std::cout << "    -t --startType type:" << std::endl;
    std::cout << "        Specify cold|warm|fast start type" << std::endl;
    std::cout << "    -u --useTempView type:" << std::endl;
    std::cout << "        Use temporary view between init and apply" << std::endl;
#ifdef SAITHRIFT
    std::cout << "    -r --rpcserver:"           << std::endl;
    std::cout << "        Enable rpcserver"      << std::endl;
    std::cout << "    -m --portmap:"             << std::endl;
    std::cout << "        Specify port map file" << std::endl;
#endif // SAITHRIFT
    std::cout << "    -h --help:" << std::endl;
    std::cout << "        Print out this message" << std::endl;
}

void handleCmdLine(int argc, char **argv)
{
    SWSS_LOG_ENTER();

    const int defaultCountersThreadIntervalInSeconds = 1;

    options.countersThreadIntervalInSeconds = defaultCountersThreadIntervalInSeconds;

#ifdef SAITHRIFT
    options.run_rpc_server = false;
    const char* const optstring = "dNt:p:i:rm:hu";
#else
    const char* const optstring = "dNt:p:i:hu";
#endif // SAITHRIFT

    while(true)
    {
        static struct option long_options[] =
        {
            { "useTempView",      no_argument,       0, 'u' },
            { "diag",             no_argument,       0, 'd' },
            { "nocounters",       no_argument,       0, 'N' },
            { "startType",        required_argument, 0, 't' },
            { "profile",          required_argument, 0, 'p' },
            { "countersInterval", required_argument, 0, 'i' },
            { "help",             no_argument,       0, 'h' },
#ifdef SAITHRIFT
            { "rpcserver",        no_argument,       0, 'r' },
            { "portmap",          required_argument, 0, 'm' },
#endif // SAITHRIFT
            { 0,                  0,                 0,  0  }
        };

        int option_index = 0;

        int c = getopt_long(argc, argv, optstring, long_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
            case 'u':
                SWSS_LOG_NOTICE("enable use temp view");
                options.useTempView = true;
                break;

            case 'N':
                SWSS_LOG_NOTICE("disable counters thread");
                options.disableCountersThread = true;
                break;

            case 'd':
                SWSS_LOG_NOTICE("enable diag shell");
                options.diagShell = true;
                break;

            case 'p':
                SWSS_LOG_NOTICE("profile map file: %s", optarg);
                options.profileMapFile = std::string(optarg);
                break;

            case 'i':
                {
                    SWSS_LOG_NOTICE("counters thread interval: %s", optarg);

                    int interval = std::stoi(std::string(optarg));

                    if (interval == 0)
                    {
                        // use zero interval to disable counters thread
                        options.disableCountersThread = true;
                    }
                    else
                    {
                        options.countersThreadIntervalInSeconds =
                            std::max(defaultCountersThreadIntervalInSeconds, interval);
                    }

                    break;
                }

            case 't':
                SWSS_LOG_NOTICE("start type: %s", optarg);
                if (std::string(optarg) == "cold")
                {
                    options.startType = SAI_COLD_BOOT;
                }
                else if (std::string(optarg) == "warm")
                {
                    options.startType = SAI_WARM_BOOT;
                }
                else if (std::string(optarg) == "fast")
                {
                    options.startType = SAI_FAST_BOOT;
                }
                else
                {
                    SWSS_LOG_ERROR("unknown start type %s", optarg);
                    exit(EXIT_FAILURE);
                }
                break;

#ifdef SAITHRIFT
            case 'r':
                SWSS_LOG_NOTICE("enable rpc server");
                options.run_rpc_server = true;
                break;
            case 'm':
                SWSS_LOG_NOTICE("port map file: %s", optarg);
                options.portMapFile = std::string(optarg);
                break;
#endif // SAITHRIFT

            case 'h':
                printUsage();
                exit(EXIT_SUCCESS);

            case '?':
                SWSS_LOG_WARN("unknown option %c", optopt);
                printUsage();
                exit(EXIT_FAILURE);

            default:
                SWSS_LOG_ERROR("getopt_long failure");
                exit(EXIT_FAILURE);
        }
    }
}

void handleProfileMap(const std::string& profileMapFile)
{
    SWSS_LOG_ENTER();

    if (profileMapFile.size() == 0)
        return;

    std::ifstream profile(profileMapFile);

    if (!profile.is_open())
    {
        SWSS_LOG_ERROR("failed to open profile map file: %s : %s", profileMapFile.c_str(), strerror(errno));
        exit(EXIT_FAILURE);
    }

    std::string line;

    while(getline(profile, line))
    {
        if (line.size() > 0 && (line[0] == '#' || line[0] == ';'))
        {
            continue;
        }

        size_t pos = line.find("=");

        if (pos == std::string::npos)
        {
            SWSS_LOG_WARN("not found '=' in line %s", line.c_str());
            continue;
        }

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        gProfileMap[key] = value;

        SWSS_LOG_INFO("insert: %s:%s", key.c_str(), value.c_str());
    }
}

#ifdef SAITHRIFT
std::map<std::set<int>, std::string> gPortMap;

// FIXME: introduce common config format for SONiC
void handlePortMap(const std::string& portMapFile)
{
    if (portMapFile.size() == 0)
        return;

    std::ifstream portmap(portMapFile);

    if (!portmap.is_open())
    {
        std::cerr << "failed to open port map file:" << portMapFile.c_str() << " : "<< strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string line;

    while(getline(portmap, line))
    {
        if (line.size() > 0 && (line[0] == '#' || line[0] == ';'))
            continue;

        std::istringstream iss(line);
        std::string name, lanes, alias;
        iss >> name >> lanes >> alias;

        iss.str(lanes);
        std::string lane_str;
        std::set<int> lane_set;

        while (getline(iss, lane_str, ','))
        {
            int lane = stoi(lane_str);
            lane_set.insert(lane);
        }

        gPortMap.insert(std::pair<std::set<int>,std::string>(lane_set, name));
    }
}
#endif // SAITHRIFT

bool handleRestartQuery(swss::NotificationConsumer &restartQuery)
{
    SWSS_LOG_ENTER();

    std::string op;
    std::string data;
    std::vector<swss::FieldValueTuple> values;

    restartQuery.pop(op, data, values);

    SWSS_LOG_DEBUG("op = %s", op.c_str());

    if (op == "COLD")
    {
        SWSS_LOG_NOTICE("received COLD switch shutdown event");
        return false;
    }

    if (op == "WARM")
    {
        SWSS_LOG_NOTICE("received WARM switch shutdown event");
        return true;
    }

    SWSS_LOG_WARN("received '%s' unknown switch shutdown event, assuming COLD", op.c_str());
    return false;
}

bool isVeryFirstRun()
{
    std::lock_guard<std::mutex> lock(g_mutex);

    SWSS_LOG_ENTER();

    // if lane map is not defined in redis db then
    // we assume this is very first start of syncd
    // later on we can add additional checks here

    auto redisLaneMap = redisGetLaneMap();

    bool firstRun = redisLaneMap.size() == 0;

    SWSS_LOG_NOTICE("First Run: %s", firstRun ? "True" : "False");

    return firstRun;
}

int get_enum_value_from_name(
        _In_ const char *name,
        _In_ const sai_enum_metadata_t* metadata)
{

    for (uint32_t idx = 0; idx < metadata->valuescount; idx++)
    {
        if (strcmp(name, metadata->valuesnames[idx]) == 0)
        {
            return metadata->values[idx];
        }
    }

    SWSS_LOG_ERROR("not found %s", name);
    return 0;
}

void saiLoglevelNotify(std::string apiStr, std::string prioStr)
{
    using namespace swss;

    static const std::map<std::string, sai_log_level_t> saiLoglevelMap = {
        { "SAI_LOG_LEVEL_CRITICAL", SAI_LOG_LEVEL_CRITICAL },
        { "SAI_LOG_LEVEL_ERROR", SAI_LOG_LEVEL_ERROR },
        { "SAI_LOG_LEVEL_WARN", SAI_LOG_LEVEL_WARN },
        { "SAI_LOG_LEVEL_NOTICE", SAI_LOG_LEVEL_NOTICE },
        { "SAI_LOG_LEVEL_INFO", SAI_LOG_LEVEL_INFO },
        { "SAI_LOG_LEVEL_DEBUG", SAI_LOG_LEVEL_DEBUG },
    };

    if (saiLoglevelMap.find(prioStr) == saiLoglevelMap.end())
    {
        SWSS_LOG_ERROR("Invalid SAI loglevel %s %s", apiStr.c_str(), prioStr.c_str());
        return;
    }

    sai_api_t api = (sai_api_t)get_enum_value_from_name(apiStr.c_str(), &metadata_enum_sai_api_t);

    sai_status_t status = sai_log_set(api, saiLoglevelMap.at(prioStr));

    if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_ERROR("Failed to set SAI loglevel %s %s", apiStr.c_str(), prioStr.c_str());
        return;
    }

    SWSS_LOG_NOTICE("Setting SAI loglevel %s to %s", apiStr.c_str(), prioStr.c_str());
}

void set_sai_api_loglevel()
{
    /*
     * We want main to be logged as debug to be shown in syslog when new
     * process starts, rest can be logged as notice.
     */

    swss::Logger::getInstance().setMinPrio(swss::Logger::SWSS_DEBUG);

    SWSS_LOG_ENTER();

    for (uint32_t idx = 0; idx < metadata_enum_sai_api_t.valuescount; ++idx)
    {
        swss::Logger::linkToDb(metadata_enum_sai_api_t.valuesnames[idx], saiLoglevelNotify, "SAI_LOG_LEVEL_NOTICE");
    }
}

int main(int argc, char **argv)
{
    SWSS_LOG_ENTER();

    set_sai_api_loglevel();

    swss::Logger::linkToDbNative("syncd");

    meta_init_db();

    handleCmdLine(argc, argv);

    handleProfileMap(options.profileMapFile);

#ifdef SAITHRIFT
    if (options.portMapFile.size() > 0)
    {
        handlePortMap(options.portMapFile);
    }
#endif // SAITHRIFT

    std::shared_ptr<swss::DBConnector> db = std::make_shared<swss::DBConnector>(ASIC_DB, swss::DBConnector::DEFAULT_UNIXSOCKET, 0);
    std::shared_ptr<swss::DBConnector> dbNtf = std::make_shared<swss::DBConnector>(ASIC_DB, swss::DBConnector::DEFAULT_UNIXSOCKET, 0);

    g_redisClient = std::make_shared<swss::RedisClient>(db.get());

    std::shared_ptr<swss::ConsumerTable> asicState = std::make_shared<swss::ConsumerTable>(db.get(), ASIC_STATE_TABLE);
    std::shared_ptr<swss::NotificationConsumer> restartQuery = std::make_shared<swss::NotificationConsumer>(db.get(), "RESTARTQUERY");

    // at the end we cant use producer consumer concept since
    // if one proces will restart there may be something in the queue
    // also "remove" from response queue will also trigger another "response"
    getResponse  = std::make_shared<swss::ProducerTable>(db.get(), "GETRESPONSE");
    notifications = std::make_shared<swss::NotificationProducer>(dbNtf.get(), "NOTIFICATIONS");

    g_veryFirstRun = isVeryFirstRun();

    if (options.startType == SAI_WARM_BOOT)
    {
        const char *warmBootReadFile = profile_get_value(0, SAI_KEY_WARM_BOOT_READ_FILE);

        SWSS_LOG_NOTICE("using warmBootReadFile: '%s'", warmBootReadFile);

        if (warmBootReadFile == NULL || access(warmBootReadFile, F_OK) == -1)
        {
            SWSS_LOG_WARN("user requested warmStart but warmBootReadFile is not specified or not accesible, forcing cold start");

            options.startType = SAI_COLD_BOOT;
        }
    }

    if (options.startType == SAI_WARM_BOOT && g_veryFirstRun)
    {
        SWSS_LOG_WARN("warm start requested, but this is very first syncd start, forcing cold start");

        // we force cold start since if it's first run then redis db is not complete
        // so redis asic view will not reflect warm boot asic state, if this happen
        // then orch agent needs to be restarted as well to repopulate asic view
        options.startType = SAI_COLD_BOOT;
    }

    gProfileMap[SAI_KEY_BOOT_TYPE] = std::to_string(options.startType);

    sai_status_t status = sai_api_initialize(0, (service_method_table_t*)&test_services);

    if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_ERROR("fail to sai_api_initialize: %d", status);
        exit_and_notify(EXIT_FAILURE);
    }

    int failed = sai_meta_apis_query(sai_api_query);

    if (failed > 0)
    {
        SWSS_LOG_WARN("sai_api_query failed for %d apis", failed);
    }

    // TODO user should create switch from OA, or should we create it here?
    // and which switch if there are multiple switches ?

    if (options.diagShell)
    {
        SWSS_LOG_NOTICE("starting diag shell thread");

        // diag shell must be created after swtich is created ?

        std::thread diag_shell_thread = std::thread(sai_diag_shell, SAI_NULL_OBJECT_ID); // TODO actual switch id

        diag_shell_thread.detach();
    }

#ifdef SAITHRIFT
    if (options.run_rpc_server)
    {
        start_sai_thrift_rpc_server(SWITCH_SAI_THRIFT_RPC_SERVER_PORT);
        SWSS_LOG_NOTICE("rpcserver started");
    }
#endif // SAITHRIFT

    SWSS_LOG_NOTICE("syncd started");

    bool warmRestartHint = false;

    start_cli();

    try
    {
        SWSS_LOG_NOTICE("before onSyncdStart");
        onSyncdStart(options.startType == SAI_WARM_BOOT);
        SWSS_LOG_NOTICE("after onSyncdStart");

        if (options.disableCountersThread == false)
        {
            SWSS_LOG_NOTICE("starting counters thread");

            startCountersThread(options.countersThreadIntervalInSeconds);
        }

        SWSS_LOG_NOTICE("syncd listening for events");

        swss::Select s;

        s.addSelectable(asicState.get());
        s.addSelectable(restartQuery.get());

        SWSS_LOG_NOTICE("starting main loop");

        while(true)
        {
            swss::Selectable *sel = NULL;

            int fd;

            int result = s.select(&sel, &fd);

            if (sel == restartQuery.get())
            {
                /*
                 * This is actual a bad design, since selectable may pick up
                 * multiple events from the queue, and after restart those
                 * events will be forgoten since they were consumed already and
                 * this may lead to forget populate object table which will
                 * lead to unable to find some objects.
                 */

                warmRestartHint = handleRestartQuery(*restartQuery);
                break;
            }

            if (result == swss::Select::OBJECT)
            {
                processEvent(*(swss::ConsumerTable*)sel);
            }
        }
    }
    catch(const std::exception &e)
    {
        SWSS_LOG_ERROR("Runtime error: %s", e.what());

        exit_and_notify(EXIT_FAILURE);
    }

    endCountersThread();

    if (warmRestartHint)
    {
        const char *warmBootWriteFile = profile_get_value(0, SAI_KEY_WARM_BOOT_WRITE_FILE);

        SWSS_LOG_NOTICE("using warmBootWriteFile: '%s'", warmBootWriteFile);

        if (warmBootWriteFile == NULL)
        {
            SWSS_LOG_WARN("user requested warm shutdown but warmBootWriteFile is not specified, forcing cold shutdown");

            warmRestartHint = false;
        }
    }

    SWSS_LOG_NOTICE("calling api uninitialize");

    sai_api_uninitialize();

    SWSS_LOG_NOTICE("uninitialize finished");

    stop_cli();

    return EXIT_SUCCESS;
}
