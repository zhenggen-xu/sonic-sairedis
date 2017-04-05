#ifndef __SYNCD_SAISWITCH_H__
#define __SYNCD_SAISWITCH_H__

//#include "string.h"
extern "C" {
#include "sai.h"
}

class SaiSwitch
{
    public:

        SaiSwitch(
                _In_ sai_object_id_t switch_vid,
                _In_ sai_object_id_t switch_rid);

        /*
         * Methods.
         */

        sai_object_id_t getVid() const;
        sai_object_id_t getRid() const;

        std::string getHardwareInfo() const;

    private:

        /*
         * Fields.
         */

        sai_object_id_t m_switch_vid;
        sai_object_id_t m_switch_rid;

        std::string m_hardware_info;

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

        std::string saiGetHardwareInfo();

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

extern std::map<sai_object_id_t, std::shared_ptr<SaiSwitch>> switches;

#endif // __SYNCD_SAISWITCH_H__
