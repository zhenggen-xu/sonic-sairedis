#ifndef __SYNCD_H__
#define __SYNCD_H__

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <thread>
#include <set>

#include <unistd.h>
#include <execinfo.h>
#include <signal.h>
#include <getopt.h>

#ifdef SAITHRIFT
#include <utility>
#include <algorithm>
#include <switch_sai_rpc_server.h>
#endif // SAITHRIFT

#include "string.h"
extern "C" {
#include "sai.h"
}

#include "meta/saiserialize.h"
#include "meta/saiattributelist.h"
#include "swss/redisclient.h"
#include "swss/dbconnector.h"
#include "swss/producertable.h"
#include "swss/consumertable.h"
#include "swss/notificationconsumer.h"
#include "swss/notificationproducer.h"
#include "swss/selectableevent.h"
#include "swss/select.h"
#include "swss/logger.h"
#include "swss/table.h"

#define UNREFERENCED_PARAMETER(X)

#define DEFAULT_VLAN_NUMBER         1

#define VIDTORID                    "VIDTORID"
#define RIDTOVID                    "RIDTOVID"
#define VIDCOUNTER                  "VIDCOUNTER"
#define LANES                       "LANES"
#define HIDDEN                      "HIDDEN"

// TODO should we use sai_switch_attr_t enums ?
#define DEFAULT_VIRTUAL_ROUTER_ID   "DEFAULT_VIRTUAL_ROUTER_ID"
#define DEFAULT_TRAP_GROUP_ID       "DEFAULT_TRAP_GROUP_ID"
#define DEFAULT_STP_INSTANCE_ID     "DEFAULT_STP_INSTANCE_ID"
#define CPU_PORT_ID                 "CPU_PORT_ID"

#define SAI_COLD_BOOT               0
#define SAI_WARM_BOOT               1
#define SAI_FAST_BOOT               2

#ifdef SAITHRIFT
#define SWITCH_SAI_THRIFT_RPC_SERVER_PORT 9092
#endif // SAITHRIFT

extern std::mutex g_mutex;
extern std::set<sai_object_id_t> g_defaultPriorityGroupsRids;
extern std::set<sai_object_id_t> g_defaultSchedulerGroupsRids;
extern std::set<sai_object_id_t> g_defaultQueuesRids;
extern std::set<sai_object_id_t> g_defaultPortsRids;

void onSyncdStart(bool warmStart);
void hardReinit();

sai_object_id_t replaceVidToRid(const sai_object_id_t &virtual_object_id);
std::unordered_map<sai_object_id_t, sai_object_id_t> redisGetVidToRidMap();
std::unordered_map<sai_object_id_t, sai_object_id_t> redisGetRidToVidMap();
std::vector<std::string> redisGetAsicStateKeys();
sai_object_id_t redisGetDefaultVirtualRouterId();
sai_object_id_t redisGetDefaultTrapGroupId();
sai_object_id_t redisGetDefaultStpInstanceId();
sai_object_id_t redisGetCpuId();
void redisClearVidToRidMap();
void redisClearRidToVidMap();

extern std::shared_ptr<swss::ProducerTable>         getResponse;
extern std::shared_ptr<swss::NotificationProducer>  notifications;

extern std::shared_ptr<swss::RedisClient>   g_redisClient;

sai_object_id_t redis_create_virtual_object_id(
        _In_ sai_object_id_t switch_id,
        _In_ sai_object_type_t object_type);

sai_object_id_t translate_rid_to_vid(
        _In_ sai_object_id_t rid);

void translate_rid_to_vid_list(
        _In_ sai_object_type_t object_type,
        _In_ uint32_t attr_count,
        _In_ sai_attribute_t *attr_list);

typedef sai_status_t (*create_fn)(
        _Out_ sai_object_id_t *stp_id,
        _In_  uint32_t attr_count,
        _In_  const sai_attribute_t *attr_list);

typedef sai_status_t (*remove_fn)(
        _In_ sai_object_id_t stp_id);

typedef sai_status_t (*set_attribute_fn)(
        _In_ sai_object_id_t object_id,
        _In_ const sai_attribute_t *attr);

typedef sai_status_t (*get_attribute_fn)(
        _In_ sai_object_id_t stp_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list);

void initialize_common_api_pointers();
void populate_sai_apis();

void startCountersThread(int intervalInSeconds);
void endCountersThread();

std::unordered_map<sai_uint32_t, sai_object_id_t> redisGetLaneMap();
//
//std::vector<sai_object_id_t> saiGetPortList();
//
//sai_object_type_t getObjectTypeFromVid(sai_object_id_t sai_object_id);

void start_cli();
void stop_cli();

sai_status_t applyViewTransition();
sai_status_t syncdApplyView();

void check_notifications_pointers(
        _In_ uint32_t attr_count,
        _In_ sai_attribute_t *attr_list);

class SaiSwitch
{
    public:

        SaiSwitch(
                _In_ sai_object_id_t switch_vid,
                _In_ sai_object_id_t switch_rid);
    private:

        /*
         * Fields.
         */

        sai_object_id_t m_switch_vid;
        sai_object_id_t m_switch_rid;

        /*
         * NOTE: Those default value will make sense only when we will do hard
         * reinit, since when doing warm restart syncd will restart but if for
         * example we removed some scheduler groups or added/removed ports,
         * those numbers won't match and we will throw.
         *
         * For that case we need special handling. We will implement that
         * later, when this scenario will happen.
         */

        sai_object_id_t m_cpu_rid;
        sai_object_id_t m_default_vlan_rid;
        sai_object_id_t m_default_virtual_router_rid;
        sai_object_id_t m_default_trap_group_rid;
        sai_object_id_t m_default_stp_instance_rid;

        /*
         * NOTE: Later we need to have this in redis with port mapping.
         */

        std::set<sai_object_id_t> m_default_priority_groups_rids;
        std::set<sai_object_id_t> m_default_queues_rids;
        std::set<sai_object_id_t> m_default_scheduler_groups_rids;
        std::set<sai_object_id_t> m_default_ports_rids;

        /*
         * Sai Methods.
         */

        sai_uint32_t saiGetPortCount();

        sai_object_id_t saiGetCpuId();

        std::vector<sai_object_id_t> saiGetPortList();

        std::unordered_map<sai_uint32_t, sai_object_id_t> saiGetHardwareLaneMap();

        sai_object_id_t saiGetDefaultTrapGroup();

        sai_object_id_t saiGetDefaultStpInstance();

        sai_object_id_t saiGetDefaultVirtualRouter();

        sai_uint32_t saiGetPortNumberOfQueues(
                _In_ sai_object_id_t port_vid);

        std::vector<sai_object_id_t> saiGetPortQueues(
                _In_ sai_object_id_t port_rid);

        sai_uint32_t saiGetPortNumberOfPriorityGroups(
                _In_ sai_object_id_t port_rid);

        std::vector<sai_object_id_t> saiGetPortPriorityGroups(
                _In_ sai_object_id_t port_rid);

        std::vector<sai_object_id_t> saiGetSchedulerGroupList(
                _In_ sai_object_id_t port_rid);

        std::vector<sai_object_id_t> saiGetSchedulerGroupChildList(
                _In_ sai_object_id_t sg_rid);

        uint32_t saiGetSchedulerGroupChildCount(
                _In_ sai_object_id_t sg_rid);

        uint32_t saiGetQosNumberOfSchedulerGroups(
                _In_ sai_object_id_t port_rid);

        uint32_t saiGetMaxNumberOfChildsPerSchedulerGroup();

        /*
         * Redis Methods.
         */

        std::unordered_map<sai_uint32_t, sai_object_id_t> redisGetLaneMap();

        std::unordered_map<sai_object_id_t, sai_object_id_t> redisGetObjectMap(
                _In_ const std::string &key);

        std::unordered_map<sai_object_id_t, sai_object_id_t> redisGetVidToRidMap();

        std::unordered_map<sai_object_id_t, sai_object_id_t> redisGetRidToVidMap();

        std::vector<std::string> redisGetAsicStateKeys();

        sai_object_id_t redisGetDefaultVirtualRouterId();

        sai_object_id_t redisGetDefaultTrapGroupId();

        sai_object_id_t redisGetDefaultStpInstanceId();

        sai_object_id_t redisGetCpuId();

        void redisSaveLaneMap(
                _In_ const std::unordered_map<sai_uint32_t, sai_object_id_t> &map);

        void redisClearLaneMap();

        void redisSetDefaultVirtualRouterId(
                _In_ sai_object_id_t vr_id);

        void redisSetDefaultTrapGroup(
                _In_ sai_object_id_t vr_id);

        void redisSetDefaultStpInstance(
                _In_ sai_object_id_t stp_id);

        void redisCreateRidAndVidMapping(
                _In_ sai_object_id_t rid,
                _In_ sai_object_id_t vid);

        void redisSetDummyAsicStateForRealObjectId(
                _In_ sai_object_id_t rid);

        void redisSetCpuId(
                _In_ sai_object_id_t cpuId);

        void redisCreateDummyEntryInAsicView(
                _In_ sai_object_id_t objectId);

        /*
         * Helper Methods.
         */

        void helperCheckLaneMap();
        void helperCheckDefaultVirtualRouterId();
        void helperCheckDefaultTrapGroup();
        void helperCheckDefaultStpInstance();
        void helperCheckCpuId();
        void helperCheckPortIds();
        void helperCheckVlanId();
        void helperCheckQueuesIds();
        void helperCheckPriorityGroupsIds();
        void helperCheckSchedulerGroupsIds();

        /*
         * Other Methods.
         */

        std::string getRedisLanesKey();
        std::string getRedisHiddenKey();
};


#endif // __SYNCD_H__
