#include "sai_redis.h"
#include "sairedis.h"
#include <thread>

#include "swss/selectableevent.h"
#include "meta/saiserialize.h"

bool g_switchInitialized = false;
volatile bool g_asicInitViewMode = false; // default mode is apply mode
volatile bool g_run = false;
volatile bool g_useTempView = false;

// TODO support pointers with notifications !
// move part of this to interface query ? or create new file sai_redis.cpp

std::shared_ptr<std::thread> notification_thread;

// this event is used to nice end notifications thread
swss::SelectableEvent g_redisNotificationTrheadEvent;

void ntf_thread()
{
    SWSS_LOG_ENTER();

    swss::Select s;

    s.addSelectable(g_redisNotifications);
    s.addSelectable(&g_redisNotificationTrheadEvent);

    while (g_run)
    {
        swss::Selectable *sel;

        int fd;

        int result = s.select(&sel, &fd);

        if (sel == &g_redisNotificationTrheadEvent)
        {
            // user requested shutdown_switch
            break;
        }

        if (result == swss::Select::OBJECT)
        {
            swss::KeyOpFieldsValuesTuple kco;

            std::string op;
            std::string data;
            std::vector<swss::FieldValueTuple> values;

            g_redisNotifications->pop(op, data, values);

            SWSS_LOG_DEBUG("notification: op = %s, data = %s", op.c_str(), data.c_str());

            handle_notification(op, data, values);
        }
    }
}

void clear_local_state()
{
    SWSS_LOG_ENTER();

    sai_status_t status = meta_init_db();

    if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_ERROR("failed to init meta db FIXME");
        throw std::runtime_error("failed to init meta db");
    }
}

sai_status_t redis_initialize_switch()
{
    // TODO move after switch create or api init
    std::lock_guard<std::mutex> apilock(g_apimutex);

    SWSS_LOG_ENTER();

    if (g_switchInitialized)
    {
        SWSS_LOG_ERROR("switch is already initialized");

        return SAI_STATUS_FAILURE;
    }

    g_switchInitialized = true;

    clear_local_state();

    g_run = true;

    g_asicInitViewMode = false;

    g_useTempView = false;

    setRecording(g_record);

    SWSS_LOG_DEBUG("creating notification thread");

    notification_thread = std::make_shared<std::thread>(std::thread(ntf_thread));

    return SAI_STATUS_SUCCESS;
}

void redis_shutdown_switch(
        _In_ bool warm_restart_hint)
{
    std::lock_guard<std::mutex> apilock(g_apimutex);

    SWSS_LOG_ENTER();

    if (!g_switchInitialized)
    {
        SWSS_LOG_ERROR("not initialized");

        return;
    }

    g_run = false;

    // notify thread that it should end
    g_redisNotificationTrheadEvent.notify();

    notification_thread->join();

    g_switchInitialized = false;
}

sai_status_t sai_redis_internal_notify_syncd(
        _In_ const std::string& key)
{
    SWSS_LOG_ENTER();

    std::vector<swss::FieldValueTuple> entry;

    // ASIC INIT/APPLY view with small letter 'a'
    // and response is recorded as capital letter 'A'

    if (g_record)
    {
        recordLine("a|" + key);
    }

    g_asicState->set(key, entry, "notify");

    swss::Select s;

    s.addSelectable(g_redisGetConsumer);

    while (true)
    {
        SWSS_LOG_NOTICE("wait for notify response");

        swss::Selectable *sel;

        int fd;

        int result = s.select(&sel, &fd, GET_RESPONSE_TIMEOUT);

        if (result == swss::Select::OBJECT)
        {
            swss::KeyOpFieldsValuesTuple kco;

            g_redisGetConsumer->pop(kco);

            const std::string &op = kfvOp(kco);
            const std::string &opkey = kfvKey(kco);

            SWSS_LOG_NOTICE("notify response: %s", opkey.c_str());

            if (op != "notify")
            {
                continue;
            }

            if (g_record)
            {
                recordLine("A|" + opkey);
            }

            sai_status_t status;
            sai_deserialize_status(opkey, status);

            return status;
        }

        SWSS_LOG_ERROR("notify syncd failed to get response result from select: %d", result);
        break;
    }

    SWSS_LOG_ERROR("notify syncd failed to get response");

    if (g_record)
    {
        recordLine("A|SAI_STATUS_FAILURE");
    }

    return SAI_STATUS_FAILURE;
}

sai_status_t sai_redis_notify_syncd(
        _In_ const sai_attribute_t *attr)
{
    SWSS_LOG_ENTER();

    // we need to use "GET" channel to be sure that
    // all previous operations were applied, if we don't
    // use GET channel then we may hit race condition
    // on syncd side where syncd will start compare view
    // when there are still objects in op queue
    //
    // other solution can be to use notify event
    // and then on syncd side read all the asic state queue
    // and apply changes before switching to init/apply mode

    std::string op;

    switch (attr->value.s32)
    {
        case SAI_REDIS_NOTIFY_SYNCD_INIT_VIEW:
            SWSS_LOG_NOTICE("sending syncd INIT view");
            op = SYNCD_INIT_VIEW;
            g_asicInitViewMode = true;
            break;

        case SAI_REDIS_NOTIFY_SYNCD_APPLY_VIEW:
            SWSS_LOG_NOTICE("sending syncd APPLY view");
            op = SYNCD_APPLY_VIEW;
            g_asicInitViewMode = false;
            break;

        default:
            SWSS_LOG_ERROR("invalid notify syncd attr value %d", attr->value.s32);
            return SAI_STATUS_FAILURE;
    }

    sai_status_t status = sai_redis_internal_notify_syncd(op);

    if (status != SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_ERROR("notify syncd failed: %s", sai_serialize_status(status).c_str());
        return status;
    }

    SWSS_LOG_NOTICE("notify syncd sycceeded");

    if (attr->value.s32 == SAI_REDIS_NOTIFY_SYNCD_INIT_VIEW)
    {
        SWSS_LOG_NOTICE("clearing current local state sinice init view is called on initialised switch");

        clear_local_state();
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * @brief Create switch
 *
 * SDK initialization/connect to SDK. After the call the capability attributes should be
 * ready for retrieval via sai_get_switch_attribute(). Same Switch Object id should be
 * given for create/connect for each NPU.
 *
 * @param[out] switch_id The Switch Object ID
 * @param[in] attr_count number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_create_switch(
        _Out_ sai_object_id_t* switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_create_oid(
            SAI_OBJECT_TYPE_SWITCH,
            switch_id,
            SAI_NULL_OBJECT_ID, // no switch id since we create switch
            attr_count,
            attr_list,
            &redis_generic_create);
}

/**
 * @brief Remove/disconnect Switch
 *
 * Release all resources associated with currently opened switch
 *
 * @param[in] switch_id The Switch id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t redis_remove_switch(
        _In_ sai_object_id_t switch_id)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_remove_oid(
            SAI_OBJECT_TYPE_SWITCH,
            switch_id,
            &redis_generic_remove);
}

/**
 * Routine Description:
 *    @brief Set switch attribute value
 *
 * Arguments:
 *    @param[in] attr - switch attribute
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t redis_set_switch_attribute(
        _In_ sai_object_id_t switch_id,
        _In_ const sai_attribute_t *attr)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    if (attr != NULL)
    {
        /*
         * NOTE: that this will work without
         * switch being created.
         */

        switch (attr->id)
        {
            case SAI_REDIS_SWITCH_ATTR_RECORD:
                setRecording(attr->value.booldata);
                return SAI_STATUS_SUCCESS;

            case SAI_REDIS_SWITCH_ATTR_NOTIFY_SYNCD:
                return sai_redis_notify_syncd(attr);

            case SAI_REDIS_SWITCH_ATTR_USE_TEMP_VIEW:
                g_useTempView = attr->value.booldata;
                return SAI_STATUS_SUCCESS;

            case SAI_REDIS_SWITCH_ATTR_USE_PIPELINE:
                g_asicState->setBuffered(attr->value.booldata);
                return SAI_STATUS_SUCCESS;

            case SAI_REDIS_SWITCH_ATTR_FLUSH:
                g_asicState->flush();
                return SAI_STATUS_SUCCESS;

            default:
                break;
        }
    }

    return meta_sai_set_oid(
            SAI_OBJECT_TYPE_SWITCH,
            switch_id,
            attr,
            &redis_generic_set);
}

/**
 * Routine Description:
 *    @brief Get switch attribute value
 *
 * Arguments:
 *    @param[in] attr_count - number of switch attributes
 *    @param[inout] attr_list - array of switch attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t redis_get_switch_attribute(
        _In_ sai_object_id_t switch_id,
        _In_ sai_uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    std::lock_guard<std::mutex> lock(g_apimutex);

    SWSS_LOG_ENTER();

    return meta_sai_get_oid(
            SAI_OBJECT_TYPE_SWITCH,
            switch_id,
            attr_count,
            attr_list,
            &redis_generic_get);
}

/**
 * @brief Switch method table retrieved with sai_api_query()
 */
const sai_switch_api_t redis_switch_api = {
    redis_create_switch,
    redis_remove_switch,
    redis_set_switch_attribute,
    redis_get_switch_attribute,
};
