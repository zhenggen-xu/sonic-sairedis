#include "sai_meta.h"
#include "sai_extra.h"
#include "saiserialize.h"

#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <memory>
#include <map>
#include <iterator>

// TODO add check for all objects are belong to same switch
// also we need to add per switch references
//
// TODO fix adding switch id to apis

// TODO move to metadata utils
bool is_ipv6_mask_valid(
        _In_ const uint8_t* mask)
{
    if (mask == NULL)
    {
        SWSS_LOG_ERROR("mask is null");

        return false;
    }

    int ones = 0;
    bool zeros = false;

    for (uint8_t i = 0; i < 128; i++)
    {
        bool bit = mask[i/8] & (1 << (7 - (i%8)));

        if (zeros && bit)
        {
            return false;
        }

        zeros |= !bit;

        if (bit)
        {
            ones++;
        }
    }

    return true;
}
//
//int get_ipv6_mask(
//        _In_ const uint8_t* mask)
//{
//    if (mask == NULL)
//    {
//        SWSS_LOG_ERROR("FATAL: mask is null");
//        throw;
//    }
//
//    int ones = 0;
//    bool zeros = false;
//
//    for (uint8_t i = 0; i < 128; i++)
//    {
//        bool bit = mask[i/8] & (1 << (7 - (i%8)));
//
//        if (zeros && bit)
//        {
//            SWSS_LOG_ERROR("FATAL: mask is not correct");
//            throw;
//        }
//
//        zeros |= !bit;
//
//        if (bit)
//        {
//            ones++;
//        }
//    }
//
//    return ones;
//}
//
//std::string ip_addr_to_string(
//        _In_ sai_ip_addr_family_t family, const void* mem)
//{
//    char str[INET6_ADDRSTRLEN];
//
//    switch (family)
//    {
//        case SAI_IP_ADDR_FAMILY_IPV4:
//
//            {
//                struct sockaddr_in sa;
//
//                memcpy(&sa.sin_addr, mem, 4);
//
//                if (inet_ntop(AF_INET, &(sa.sin_addr), str, INET_ADDRSTRLEN) == NULL)
//                {
//                    SWSS_LOG_ERROR("FATAL: failed to convert IPv4 address, errno: %d", errno);
//                    throw;
//                }
//
//                break;
//            }
//
//        case SAI_IP_ADDR_FAMILY_IPV6:
//
//            {
//                struct sockaddr_in6 sa6;
//
//                memcpy(&sa6.sin6_addr, mem, 16);
//
//                if (inet_ntop(AF_INET6, &(sa6.sin6_addr), str, INET6_ADDRSTRLEN) == NULL)
//                {
//                    SWSS_LOG_ERROR("FATAL: failed to convert IPv6 address, errno: %d", errno);
//                    throw;
//                }
//
//                break;
//            }
//
//        default:
//
//            SWSS_LOG_ERROR("FATAL: invalid ip address family: %d", family);
//            throw;
//    }
//
//    return str;
//}

//std::string ip_address_to_string(
//        _In_ const sai_ip_address_t& ip)
//{
//    return "[" + ip_addr_to_string(ip.addr_family, &ip.addr) + "]";
//}
//
//std::string ip_prefix_to_string(
//        _In_ const sai_ip_prefix_t& prefix)
//{
//    if (prefix.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
//    {
//        return "[" + ip_addr_to_string(prefix.addr_family, &prefix.addr) + "/" +
//            ip_addr_to_string(prefix.addr_family, &prefix.mask) + "]";
//    }
//
//    return "[" + ip_addr_to_string(prefix.addr_family, &prefix.addr) + "/" +
//        std::to_string(get_ipv6_mask(prefix.mask.ip6)) + "]";
//}

// TODO move to metadata
const sai_attribute_t* get_attribute_by_id(
        _In_ sai_attr_id_t id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    SWSS_LOG_ENTER();

    if (attr_list == NULL)
    {
        SWSS_LOG_ERROR("attribute list is null");

        return NULL;
    }

    for (uint32_t i = 0; i < attr_count; ++i)
    {
        if (attr_list[i].id == id)
        {
            return &attr_list[i];
        }
    }

    return NULL;
}

std::vector<const sai_attr_metadata_t*> get_attributes_metadata(
        _In_ sai_object_type_t objecttype)
{
    SWSS_LOG_ENTER();

    SWSS_LOG_DEBUG("objecttype: %s", sai_serialize_object_type(objecttype).c_str());

    auto meta = sai_all_object_type_infos[objecttype]->attrmetadata;

    std::vector<const sai_attr_metadata_t*> attrs;

    for (size_t index = 0; meta[index] != NULL; ++index)
    {
        attrs.push_back(meta[index]);
    }

    return attrs;
}

class SaiAttrWrapper
{
    public:

        SaiAttrWrapper(
                _In_ const sai_attr_metadata_t* meta,
                _In_ const sai_attribute_t& attr):
            m_meta(meta),
            m_attr(attr)
    {
        SWSS_LOG_ENTER();

        m_attr.id = attr.id;

        std::string s = sai_serialize_attr_value(*meta, attr, false);

        sai_deserialize_attr_value(s, *meta, m_attr, false);
    }

        ~SaiAttrWrapper()
        {
            SWSS_LOG_ENTER();

            sai_deserialize_free_attribute_value(m_meta->attrvaluetype, m_attr);
        }

        const sai_attribute_t* getattr() const
        {
            return &m_attr;
        }

    private:

        SaiAttrWrapper(const SaiAttrWrapper&);
        SaiAttrWrapper& operator=(const SaiAttrWrapper&);

        const sai_attr_metadata_t* m_meta;
        sai_attribute_t m_attr;
};

std::string get_attr_info(const sai_attr_metadata_t& md)
{
    return std::string(md.attridname) + ":" + sai_serialize_attr_value_type(md.attrvaluetype);
}

#define META_LOG_ERROR(md, format, ...) SWSS_LOG_ERROR("%s " format, get_attr_info(md).c_str(), ##__VA_ARGS__)
#define META_LOG_DEBUG(md, format, ...) SWSS_LOG_DEBUG("%s " format, get_attr_info(md).c_str(), ##__VA_ARGS__)
#define META_LOG_NOTICE(md, format, ...) SWSS_LOG_NOTICE("%s " format, get_attr_info(md).c_str(), ##__VA_ARGS__)
#define META_LOG_INFO(md, format, ...) SWSS_LOG_INFO("%s " format, get_attr_info(md).c_str(), ##__VA_ARGS__)

// traps and vlan will be converted to oid
// fdb, route, neighbor don't need reference count,
// they are leafs and can be removed at any time
std::unordered_map<sai_object_id_t,int32_t> ObjectReferences;
std::unordered_map<std::string,std::string> AttributeKeys;
std::unordered_map<std::string,std::unordered_map<sai_attr_id_t,std::shared_ptr<SaiAttrWrapper>>> ObjectAttrHash;

// GENERIC REFERENCE FUNCTIONS

bool object_reference_exists(
        _In_ sai_object_id_t oid)
{
    SWSS_LOG_ENTER();

    bool exists = ObjectReferences.find(oid) != ObjectReferences.end();

    SWSS_LOG_DEBUG("object 0x%lx refrence: %s", oid, exists ? "exists" : "missing");

    return exists;
}

void object_reference_inc(
        _In_ sai_object_id_t oid)
{
    SWSS_LOG_ENTER();

    if (oid == SAI_NULL_OBJECT_ID)
    {
        return;
    }

    if (!object_reference_exists(oid))
    {
        SWSS_LOG_ERROR("FATAL: object oid 0x%lx not in reference map", oid);
        throw;
    }

    ObjectReferences[oid]++;

    SWSS_LOG_DEBUG("increased reference on oid 0x%lx to %d", oid, ObjectReferences[oid]);
}

void object_reference_dec(
        _In_ sai_object_id_t oid)
{
    SWSS_LOG_ENTER();

    if (oid == SAI_NULL_OBJECT_ID)
    {
        return;
    }

    if (!object_reference_exists(oid))
    {
        SWSS_LOG_ERROR("FATAL: object oid 0x%lx not in reference map", oid);
        throw;
    }

    ObjectReferences[oid]--;

    if (ObjectReferences[oid] < 0)
    {
        SWSS_LOG_ERROR("FATAL: object oid 0x%lx reference count is negative!", oid);
        throw;
    }

    SWSS_LOG_DEBUG("decreased reference on oid 0x%lx to %d", oid, ObjectReferences[oid]);
}

void object_reference_dec(
        _In_ const sai_object_list_t& list)
{
    SWSS_LOG_ENTER();

    for (uint32_t i = 0; i < list.count; ++i)
    {
        object_reference_dec(list.list[i]);
    }
}

void object_reference_inc(
        _In_ const sai_object_list_t& list)
{
    SWSS_LOG_ENTER();

    for (uint32_t i = 0; i < list.count; ++i)
    {
        object_reference_inc(list.list[i]);
    }
}

void object_reference_insert(
        _In_ sai_object_id_t oid)
{
    SWSS_LOG_ENTER();

    if (object_reference_exists(oid))
    {
        SWSS_LOG_ERROR("FATAL: object oid 0x%lx already in reference map", oid);
        throw;
    }

    ObjectReferences[oid] = 0;

    SWSS_LOG_DEBUG("inserted reference on 0x%lx", oid);
}

int32_t object_reference_count(
        _In_ sai_object_id_t oid)
{
    SWSS_LOG_ENTER();

    if (object_reference_exists(oid))
    {
        int32_t count = ObjectReferences[oid];

        SWSS_LOG_DEBUG("reference count on oid 0x%lx is %d", oid, count);

        return count;
    }

    SWSS_LOG_ERROR("FATAL: object oid 0x%lx reference not in map", oid);
    throw;
}

void object_reference_remove(
        _In_ sai_object_id_t oid)
{
    SWSS_LOG_ENTER();

    if (object_reference_exists(oid))
    {
        int32_t count = object_reference_count(oid);

        if (count > 0)
        {
            SWSS_LOG_ERROR("FATAL: removing object oid 0x%lx but reference count is: %d", oid, count);
            throw;
        }
    }

    SWSS_LOG_DEBUG("removing object oid 0x%lx reference", oid);

    ObjectReferences.erase(oid);
}


bool object_exists(
        _In_ const std::string& key)
{
    SWSS_LOG_ENTER();

    bool exists = ObjectAttrHash.find(key) != ObjectAttrHash.end();

    SWSS_LOG_DEBUG("%s %s", key.c_str(), exists ? "exists" : "missing");

    return exists;
}

bool object_exists(
        _In_ const sai_object_meta_key_t meta_key)
{
    SWSS_LOG_ENTER();

    std::string key = sai_serialize_object_meta_key(meta_key);

    return object_exists(key);
}

sai_status_t meta_init_db()
{
    SWSS_LOG_ENTER();

    // TODO this init should be done after creating first switch !

    // we need local db to keep track if
    // we are performing "get" or set on
    // conditional attribute and whether
    // condition is in force

    ObjectReferences.clear();
    ObjectAttrHash.clear();
    AttributeKeys.clear();

    // init switch TODO we need to create object id

    sai_object_meta_key_t meta_key = { .objecttype = SAI_OBJECT_TYPE_SWITCH, .objectkey = { .key = { } } };

    std::string switch_key = sai_serialize_object_meta_key(meta_key);

    ObjectAttrHash[switch_key] = { };

    // init default vlan
    //
    // TODO default vlan is switch object

    //sai_object_meta_key_t meta_key_vlan = { .object_type = SAI_OBJECT_TYPE_VLAN, .key = { .vlan_id = DEFAULT_VLAN_NUMBER } };

    //atd::string vlan_key = sai_serialize_object_meta_key(meta_key_vlan);

    //bjectAttrHash[vlan_key] = { };

    //vlan_reference_insert(DEFAULT_VLAN_NUMBER);
    //vlan_reference_inc(DEFAULT_VLAN_NUMBER);

    return SAI_STATUS_SUCCESS;
}

const sai_attribute_t* get_object_previous_attr(
        _In_ const sai_object_meta_key_t meta_key,
        _In_ const sai_attr_metadata_t& md)
{
    SWSS_LOG_ENTER();

    std::string key = sai_serialize_object_meta_key(meta_key);

    auto it = ObjectAttrHash.find(key);

    if (it == ObjectAttrHash.end())
    {
        SWSS_LOG_ERROR("object key %s not found", key.c_str());

        return NULL;
    }

    auto ita = it->second.find(md.attrid);

    if (ita == it->second.end())
    {
        // attribute id not found
        return NULL;
    }

    // NOTE: this is actually dangerous since
    // we possibly expose memory on attribute
    // list that could be already freed
    return ita->second->getattr();
}

void set_object(
        _In_ const sai_object_meta_key_t meta_key,
        _In_ const sai_attr_metadata_t& md,
        _In_ const sai_attribute_t *attr)
{
    SWSS_LOG_ENTER();

    std::string key = sai_serialize_object_meta_key(meta_key);

    if (!object_exists(key))
    {
        SWSS_LOG_ERROR("FATAL: object %s don't exists", key.c_str());
        throw std::runtime_error("FATAL: object don't exists" + key);
    }

    META_LOG_DEBUG(md, "set attribute %d on %s", attr->id, key.c_str());

    auto p = new SaiAttrWrapper(&md,*attr);

    ObjectAttrHash[key][attr->id] = std::shared_ptr<SaiAttrWrapper>(p);
}

const std::vector<std::shared_ptr<SaiAttrWrapper>> get_object(
        _In_ const sai_object_meta_key_t meta_key)
{
    std::string key = sai_serialize_object_meta_key(meta_key);

    if (!object_exists(key))
    {
        SWSS_LOG_ERROR("FATAL: object %s don't exists", key.c_str());
        throw;
    }

    std::vector<std::shared_ptr<SaiAttrWrapper>> attrs;

    const auto& hash = ObjectAttrHash[key];

    for (auto it = hash.begin(); it != hash.end(); ++it)
    {
        attrs.push_back(it->second);
    }

    return attrs;
}

void remove_object(
        _In_ const sai_object_meta_key_t meta_key)
{
    SWSS_LOG_ENTER();

    std::string key = sai_serialize_object_meta_key(meta_key);

    if (!object_exists(key))
    {
        SWSS_LOG_ERROR("FATAL: object %s don't exists", key.c_str());
        throw;
    }

    SWSS_LOG_DEBUG("removing object %s", key.c_str());

    ObjectAttrHash.erase(key);
}

void create_object(
        _In_ const sai_object_meta_key_t& meta_key)
{
    SWSS_LOG_ENTER();

    std::string key = sai_serialize_object_meta_key(meta_key);

    if (object_exists(key))
    {
        SWSS_LOG_ERROR("FATAL: object %s already exists", key.c_str());
        throw;
    }

    SWSS_LOG_DEBUG("creating object %s", key.c_str());

    ObjectAttrHash[key] = {};
}

sai_status_t meta_generic_validation_objlist(
        _In_ const sai_attr_metadata_t& md,
        _In_ uint32_t count,
        _In_ const sai_object_id_t* list)
{
    SWSS_LOG_ENTER();

    if (count > MAX_LIST_COUNT)
    {
        META_LOG_ERROR(md, "object list count %u is > then max list count %u", count, MAX_LIST_COUNT);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (list == NULL)
    {
        if (count == 0)
        {
            return SAI_STATUS_SUCCESS;
        }

        META_LOG_ERROR(md, "object list is null, but count is %u", count);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    std::set<sai_object_id_t> oids;

    sai_object_type_t object_type = SAI_OBJECT_TYPE_NULL;

    for (uint32_t i = 0; i < count; ++i)
    {
        sai_object_id_t oid = list[i];

        if (oids.find(oid) != oids.end())
        {
            META_LOG_ERROR(md, "object on list [%u] oid 0x%lx is duplicated, but not allowed", i, oid);

            return SAI_STATUS_INVALID_PARAMETER;
        }

        oids.insert(oid);

        if (oid == SAI_NULL_OBJECT_ID)
        {
            if (md.allownullobjectid)
            {
                // ok, null object is allowed
                continue;
            }

            META_LOG_ERROR(md, "object on list [%u] is NULL, but not allowed", i);

            return SAI_STATUS_INVALID_PARAMETER;
        }

        sai_object_type_t ot = sai_object_type_query(oid);

        if (ot == SAI_NULL_OBJECT_ID)
        {
            META_LOG_ERROR(md, "object on list [%u] oid 0x%lx is not valid, returned null object id", i, oid);

            return SAI_STATUS_INVALID_PARAMETER;
        }

        if (!sai_metadata_is_allowed_object_type(&md, ot))
        {
            META_LOG_ERROR(md, "object on list [%u] oid 0x%lx object type %d is not allowed on this attribute", i, oid, ot);

            return SAI_STATUS_INVALID_PARAMETER;
        }

        if (!object_reference_exists(oid))
        {
            META_LOG_ERROR(md, "object on list [%u] oid 0x%lx object type %d does not exists in local DB", i, oid, ot);

            return SAI_STATUS_INVALID_PARAMETER;
        }

        if (i > 1)
        {
            // currently all objects on list must be the same type
            if (object_type != ot)
            {
                META_LOG_ERROR(md, "object list contain's mixed object types: %d vs %d, not allowed", object_type, ot);

                return SAI_STATUS_INVALID_PARAMETER;
            }
        }

        object_type = ot;
    }

    return SAI_STATUS_SUCCESS;
}

template<typename T> sai_status_t meta_genetic_validation_list(const sai_attr_metadata_t& md, const T&list)
{
    uint32_t count = list.count;

    if (count > MAX_LIST_COUNT)
    {
        META_LOG_ERROR(md, "list count %u is > then max list count %u", count, MAX_LIST_COUNT);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (count == 0 && list.list != NULL)
    {
        META_LOG_ERROR(md, "when count is zero, list must be NULL");

        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (list.list == NULL)
    {
        if (count == 0)
        {
            return SAI_STATUS_SUCCESS;
        }

        META_LOG_ERROR(md, "list is null, but count is %u", count);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}

#define VALIDATION_LIST(md,list) \
{\
    auto status = meta_genetic_validation_list(md,list);\
    if (status != SAI_STATUS_SUCCESS)\
    {\
        return status;\
    }\
}

std::string construct_key(
        _In_ const sai_object_meta_key_t& meta_key,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t* attr_list)
{
    SWSS_LOG_ENTER();

    // use map to make sure that kays will be always sorded by id

    std::map<int32_t, std::string> keys;

    for (uint32_t idx = 0; idx < attr_count; ++idx)
    {
        const sai_attribute_t* attr = &attr_list[idx];

        const auto& md = *sai_metadata_get_attr_metadata(meta_key.objecttype, attr->id);

        const sai_attribute_value_t& value = attr->value;

        if (!HAS_FLAG_KEY(md.flags))
        {
            continue;
        }

        std::string name = md.attridname + std::string(":");

        switch (md.attrvaluetype)
        {
            case SAI_ATTR_VALUE_TYPE_UINT32_LIST: // only for port

                // TODO this list should be sorted
                for (uint32_t i = 0; i < value.u32list.count; ++i)
                {
                    name += std::to_string(value.u32list.list[i]);

                    if (i != value.u32list.count - 1)
                    {
                        name += ",";
                    }
                }

                break;

            case SAI_ATTR_VALUE_TYPE_INT32:
                name += std::to_string(value.s32); // if enum then get enum name?
                break;

            case SAI_ATTR_VALUE_TYPE_UINT32:
                name += std::to_string(value.u32);
                break;

            case SAI_ATTR_VALUE_TYPE_UINT8:
                name += std::to_string(value.u8);
                break;

            default:
                META_LOG_ERROR(md, "FATAL: marked as key, but have invalid serialization type");
                throw;
        }

        keys[md.attrid] = name;
    }

    std::string key;

    for (auto& k: keys)
    {
        key += k.second + ";";
    }

    SWSS_LOG_DEBUG("constructed key: %s", key.c_str());

    return key;
}

sai_status_t meta_generic_validation_create(
        _In_ const sai_object_meta_key_t& meta_key,
        _In_ const uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    SWSS_LOG_ENTER();

    if (attr_count > MAX_LIST_COUNT)
    {
        SWSS_LOG_ERROR("create attribute count is too large %u > then max list count %u", attr_count, MAX_LIST_COUNT);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (attr_count > 0 && attr_list == NULL)
    {
        SWSS_LOG_ERROR("attr count is %u but attribute list pointer is NULL", attr_count);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    std::unordered_map<sai_attr_id_t, const sai_attribute_t*> attrs;

    SWSS_LOG_DEBUG("attr count = %u", attr_count);

    bool haskeys = false;

    // check each attribute separetly
    for (uint32_t idx = 0; idx < attr_count; ++idx)
    {
        const sai_attribute_t* attr = &attr_list[idx];

        auto mdp = sai_metadata_get_attr_metadata(meta_key.objecttype, attr->id);

        if (mdp == NULL)
        {
            SWSS_LOG_ERROR("unable to find attribute metadata %d:%d", meta_key.objecttype, attr->id);

            return SAI_STATUS_FAILURE;
        }

        const sai_attribute_value_t& value = attr->value;

        const sai_attr_metadata_t& md = *mdp;

        META_LOG_DEBUG(md, "(create)");

        if (attrs.find(attr->id) != attrs.end())
        {
            META_LOG_ERROR(md, "attribute id (%u) is defined on attr list multiple times", attr->id);

            return SAI_STATUS_INVALID_PARAMETER;
        }

        attrs[attr->id] = attr;

        if (HAS_FLAG_READ_ONLY(md.flags))
        {
            META_LOG_ERROR(md, "attr is read only and cannot be created");

            return SAI_STATUS_INVALID_PARAMETER;
        }

        if (HAS_FLAG_KEY(md.flags))
        {
            haskeys = true;
            META_LOG_DEBUG(md, "attr is key");
        }

        // if we set OID check if exists and if type is correct

        switch (md.attrvaluetype)
        {
            case SAI_ATTR_VALUE_TYPE_CHARDATA:

                {
                    const char* chardata = value.chardata;

                    size_t len = strnlen(chardata, HOSTIF_NAME_SIZE);

                    if (len == HOSTIF_NAME_SIZE)
                    {
                        META_LOG_ERROR(md, "host interface name is too long");

                        return SAI_STATUS_INVALID_PARAMETER;
                    }

                    if (len == 0)
                    {
                        META_LOG_ERROR(md, "host interface name is zero");

                        return SAI_STATUS_INVALID_PARAMETER;
                    }

                    for (size_t i = 0; i < len; ++i)
                    {
                        char c = chardata[i];

                        if (c < 0x20 || c > 0x7e)
                        {
                            META_LOG_ERROR(md, "interface name contains invalid character 0x%02x", c);

                            return SAI_STATUS_INVALID_PARAMETER;
                        }
                    }

                    // TODO check whether name is not used by other host interface
                    break;
                }

            case SAI_ATTR_VALUE_TYPE_BOOL:
            case SAI_ATTR_VALUE_TYPE_UINT8:
            case SAI_ATTR_VALUE_TYPE_INT8:
            case SAI_ATTR_VALUE_TYPE_UINT16:
            case SAI_ATTR_VALUE_TYPE_INT16:
            case SAI_ATTR_VALUE_TYPE_UINT32:
            case SAI_ATTR_VALUE_TYPE_INT32:
            case SAI_ATTR_VALUE_TYPE_UINT64:
            case SAI_ATTR_VALUE_TYPE_INT64:
            case SAI_ATTR_VALUE_TYPE_MAC:
            case SAI_ATTR_VALUE_TYPE_IPV4:
            case SAI_ATTR_VALUE_TYPE_IPV6:
                break;

            case SAI_ATTR_VALUE_TYPE_IP_ADDRESS:

                {
                    switch (value.ipaddr.addr_family)
                    {
                        case SAI_IP_ADDR_FAMILY_IPV4:
                        case SAI_IP_ADDR_FAMILY_IPV6:
                            break;

                        default:

                            SWSS_LOG_ERROR("invalid address family: %d", value.ipaddr.addr_family);

                            return SAI_STATUS_INVALID_PARAMETER;
                    }

                    break;
                }

            case SAI_ATTR_VALUE_TYPE_OBJECT_ID:

                {
                    sai_status_t status = meta_generic_validation_objlist(md, 1, &value.oid);

                    if (status != SAI_STATUS_SUCCESS)
                    {
                        return status;
                    }

                    break;
                }

            case SAI_ATTR_VALUE_TYPE_OBJECT_LIST:

                {
                    sai_status_t status = meta_generic_validation_objlist(md, value.objlist.count, value.objlist.list);

                    if (status != SAI_STATUS_SUCCESS)
                    {
                        return status;
                    }

                    break;
                }

                // case SAI_ATTR_VALUE_TYPE_VLAN_LIST:
                // require test for vlan existence

                // ACL FIELD

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_BOOL:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT8:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT16:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT16:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT32:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT32:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_MAC:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_IPV4:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_IPV6:
                // primitives
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_ID:

                {
                    sai_status_t status = meta_generic_validation_objlist(md, 1, &value.aclfield.data.oid);

                    if (status != SAI_STATUS_SUCCESS)
                    {
                        return status;
                    }

                    break;
                }

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_LIST:

                {
                    sai_status_t status = meta_generic_validation_objlist(md, value.aclfield.data.objlist.count, value.aclfield.data.objlist.list);

                    if (status != SAI_STATUS_SUCCESS)
                    {
                        return status;
                    }

                    break;
                }

                // case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8_LIST:

                // ACL ACTION

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT8:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT8:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT16:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT16:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT32:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT32:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_MAC:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_IPV4:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_IPV6:
                // primitives
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_ID:

                {
                    sai_status_t status = meta_generic_validation_objlist(md, 1, &value.aclaction.parameter.oid);

                    if (status != SAI_STATUS_SUCCESS)
                    {
                        return status;
                    }

                    break;
                }

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_LIST:

                {
                    sai_status_t status = meta_generic_validation_objlist(md, value.aclaction.parameter.objlist.count, value.aclaction.parameter.objlist.list);

                    if (status != SAI_STATUS_SUCCESS)
                    {
                        return status;
                    }

                    break;
                }

                // ACL END

            case SAI_ATTR_VALUE_TYPE_UINT8_LIST:
                VALIDATION_LIST(md, value.u8list);
                break;
            case SAI_ATTR_VALUE_TYPE_INT8_LIST:
                VALIDATION_LIST(md, value.s8list);
                break;
            case SAI_ATTR_VALUE_TYPE_UINT16_LIST:
                VALIDATION_LIST(md, value.u16list);
                break;
            case SAI_ATTR_VALUE_TYPE_INT16_LIST:
                VALIDATION_LIST(md, value.s16list);
                break;
            case SAI_ATTR_VALUE_TYPE_UINT32_LIST:
                VALIDATION_LIST(md, value.u32list);
                break;
            case SAI_ATTR_VALUE_TYPE_INT32_LIST:
                VALIDATION_LIST(md, value.s32list);
                break;
            case SAI_ATTR_VALUE_TYPE_QOS_MAP_LIST:
                VALIDATION_LIST(md, value.qosmap);
                break;
            case SAI_ATTR_VALUE_TYPE_TUNNEL_MAP_LIST:
                VALIDATION_LIST(md, value.tunnelmap);
                break;

            case SAI_ATTR_VALUE_TYPE_UINT32_RANGE:

                if (value.u32range.min > value.u32range.max)
                {
                    META_LOG_ERROR(md, "invalid range %u .. %u", value.u32range.min, value.u32range.max);

                    return SAI_STATUS_INVALID_PARAMETER;
                }

                break;

            case SAI_ATTR_VALUE_TYPE_INT32_RANGE:

                if (value.s32range.min > value.s32range.max)
                {
                    META_LOG_ERROR(md, "invalid range %u .. %u", value.s32range.min, value.s32range.max);

                    return SAI_STATUS_INVALID_PARAMETER;
                }

                break;

            default:

                META_LOG_ERROR(md, "serialization type is not supported yet FIXME");
                throw;
        }

        if (md.isenum)
        {
            int32_t val = value.s32;

            switch (md.attrvaluetype)
            {
                case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT32:
                    val = value.aclfield.data.s32;
                    break;

                case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT32:
                    val = value.aclaction.parameter.s32;
                    break;

                default:
                    val = value.s32;
                    break;
            }

            if (!sai_metadata_is_allowed_enum_value(&md, val))
            {
                META_LOG_ERROR(md, "is enum, but value %d not found on allowed values list", val);

                return SAI_STATUS_INVALID_PARAMETER;
            }
        }

        if (md.isenumlist)
        {
            // we allow repeats on enum list
            if (value.s32list.count != 0 && value.s32list.list == NULL)
            {
                META_LOG_ERROR(md, "enum list is NULL");

                return SAI_STATUS_INVALID_PARAMETER;
            }

            for (uint32_t i = value.s32list.count; i < value.s32list.count; ++i)
            {
                int32_t s32 = value.s32list.list[i];

                if (!sai_metadata_is_allowed_enum_value(&md, s32))
                {
                    META_LOG_ERROR(md, "is enum list, but value %d not found on allowed values list", s32);

                    return SAI_STATUS_INVALID_PARAMETER;
                }
            }
        }

        if (md.isvlan)
        {
        }

        // conditions are checked later on
    }

    // we are creating object, no need for check if exists (only key values needs to be checked)

    switch (meta_key.objecttype)
    {
        case SAI_OBJECT_TYPE_ROUTE_ENTRY:
        case SAI_OBJECT_TYPE_FDB_ENTRY:
        case SAI_OBJECT_TYPE_NEIGHBOR_ENTRY:

            {
                // just sanity check if object already exists
                std::string key = sai_serialize_object_meta_key(meta_key);

                if (object_exists(key))
                {
                    SWSS_LOG_ERROR("object key %s already exists", key.c_str());

                    return SAI_STATUS_ITEM_ALREADY_EXISTS;
                }

                break;
            }

        default:
            // TODO check if it's oid object
            // we are creating OID object, and we don't have it's value yet
            break;
    }

    const auto& metadata = get_attributes_metadata(meta_key.objecttype);

    if (metadata.empty())
    {
        SWSS_LOG_ERROR("get attributes metadata returned empty list for object type: %d", meta_key.objecttype);

        return SAI_STATUS_FAILURE;
    }

    // check if all mandatory attrributes were passed

    for (auto mdp: metadata)
    {
        const sai_attr_metadata_t& md = *mdp;

        if (!HAS_FLAG_MANDATORY_ON_CREATE(md.flags))
        {
            continue;
        }

        // is conditional
        if (md.conditions != NULL)
        {
            // skip conditional attributes for now
            continue;
        }

        const auto &it = attrs.find(md.attrid);

        if (it == attrs.end())
        {
            META_LOG_ERROR(md, "attribute is mandatory but not passed in attr list");

            return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
        }
    }

    // check if we need any conditional attributes
    for (auto mdp: metadata)
    {
        const sai_attr_metadata_t& md = *mdp;

        // is not conditional
        if (md.conditions == NULL)
        {
            continue;
        }

        // this is conditional attribute, check if it's required

        bool any = false;

        for (size_t index = 0; md.conditions[index] != NULL; index++)
        {
            const auto& c = *md.conditions[index];

            // condtions may only be on the same object type
            const auto& cmd = *sai_metadata_get_attr_metadata(meta_key.objecttype, c.attrid);

            const sai_attribute_value_t* cvalue = cmd.defaultvalue;

            const sai_attribute_t *cattr = get_attribute_by_id(c.attrid, attr_count, attr_list);

            if (cattr != NULL)
            {
                META_LOG_DEBUG(md, "condition attr %d was passed, using it's value", c.attrid);

                cvalue = &cattr->value;
            }

            if (cmd.attrvaluetype == SAI_ATTR_VALUE_TYPE_BOOL)
            {
                if (c.condition.booldata == cvalue->booldata)
                {
                    META_LOG_DEBUG(md, "bool condition was met on attr %d = %d", cmd.attrid, c.condition.booldata);

                    any = true;
                    break;
                }
            }
            else // enum condition
            {
                int32_t val = cvalue->s32;

                switch (cmd.attrvaluetype)
                {
                    case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT32:
                        val = cvalue->aclfield.data.s32;
                        break;

                    case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT32:
                        val = cvalue->aclaction.parameter.s32;
                        break;

                    default:
                        val = cvalue->s32;
                        break;
                }

                if (c.condition.s32 == val)
                {
                    META_LOG_DEBUG(md, "enum condition was met on attr id %d, val = %d", cmd.attrid, val);

                    any = true;
                    break;
                }
            }
        }

        if (!any)
        {
            // maybe we can let it go here?
            if (attrs.find(md.attrid) != attrs.end())
            {
                META_LOG_ERROR(md, "conditional, but condition was not met, this attribute is not required, but passed");

                return SAI_STATUS_INVALID_PARAMETER;
            }

            continue;
        }

        // is required, check if user passed it
        const auto &it = attrs.find(md.attrid);

        if (it == attrs.end())
        {
            META_LOG_ERROR(md, "attribute is conditional and is mandatory but not passed in attr list");

            return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
        }
    }

    if (haskeys)
    {
        std::string key = construct_key(meta_key, attr_count, attr_list);

        for (auto& it: AttributeKeys)
        {
            // since we didn't created oid yet, we don't know if attribute key exists, check all

            if (it.second == key)
            {
                SWSS_LOG_ERROR("attribute key %s already exists, can't create", key.c_str());

                return SAI_STATUS_INVALID_PARAMETER;
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t meta_generic_validation_remove(
        _In_ const sai_object_meta_key_t& meta_key)
{
    SWSS_LOG_ENTER();

    std::string key = sai_serialize_object_meta_key(meta_key);

    if (!object_exists(key))
    {
        SWSS_LOG_ERROR("object key %s don't exists", key.c_str());

        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch (meta_key.objecttype)
    {
        case SAI_OBJECT_TYPE_ROUTE_ENTRY:
        case SAI_OBJECT_TYPE_FDB_ENTRY:
        case SAI_OBJECT_TYPE_NEIGHBOR_ENTRY:
            // we don't keep reference of those since those are leafs
            break;


        default:

            // TODO check if it's object id

            {
                sai_object_id_t oid = meta_key.objectkey.key.object_id;

                if (oid == SAI_NULL_OBJECT_ID)
                {
                    SWSS_LOG_ERROR("can't remove null object id");

                    return SAI_STATUS_INVALID_PARAMETER;
                }

                sai_object_type_t object_type = sai_object_type_query(oid);

                if (object_type == SAI_NULL_OBJECT_ID)
                {
                    SWSS_LOG_ERROR("oid 0x%lx is not valid, returned null object id", oid);

                    return SAI_STATUS_INVALID_PARAMETER;
                }

                if (object_type != meta_key.objecttype)
                {
                    SWSS_LOG_ERROR("oid 0x%lx type %d is not accepted, expected object type %d", oid, object_type, meta_key.objecttype);

                    return SAI_STATUS_INVALID_PARAMETER;
                }

                if (!object_reference_exists(oid))
                {
                    SWSS_LOG_ERROR("object 0x%lx reference don't exists", oid);

                    return SAI_STATUS_INVALID_PARAMETER;
                }

                int count = object_reference_count(oid);

                if (count != 0)
                {
                    SWSS_LOG_ERROR("object 0x%lx reference count is %d, can't remove", oid, count);

                    return SAI_STATUS_INVALID_PARAMETER;
                }

                // should be safe to remove

                break;
            }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t meta_generic_validation_set(
        _In_ const sai_object_meta_key_t& meta_key,
        _In_ const sai_attribute_t *attr)
{
    SWSS_LOG_ENTER();

    if (attr == NULL)
    {
        SWSS_LOG_ERROR("attribute pointer is NULL");

        return SAI_STATUS_INVALID_PARAMETER;
    }

    auto mdp = sai_metadata_get_attr_metadata(meta_key.objecttype, attr->id);

    if (mdp == NULL)
    {
        SWSS_LOG_ERROR("unable to find attribute metadata %d:%d", meta_key.objecttype, attr->id);

        return SAI_STATUS_FAILURE;
    }

    const sai_attribute_value_t& value = attr->value;

    const sai_attr_metadata_t& md = *mdp;

    META_LOG_DEBUG(md, "(set)");

    if (HAS_FLAG_READ_ONLY(md.flags))
    {
        META_LOG_ERROR(md, "attr is read only and cannot be modified");

        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (HAS_FLAG_CREATE_ONLY(md.flags))
    {
        META_LOG_ERROR(md, "attr is create only and cannot be modified");

        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (HAS_FLAG_KEY(md.flags))
    {
        META_LOG_ERROR(md, "attr is key and cannot be modified");

        return SAI_STATUS_INVALID_PARAMETER;
    }

    // if we set OID check if exists and if type is correct

    switch (md.attrvaluetype)
    {
        case SAI_ATTR_VALUE_TYPE_BOOL:
            // case SAI_ATTR_VALUE_TYPE_CHARDATA:
        case SAI_ATTR_VALUE_TYPE_UINT8:
        case SAI_ATTR_VALUE_TYPE_INT8:
        case SAI_ATTR_VALUE_TYPE_UINT16:
        case SAI_ATTR_VALUE_TYPE_INT16:
        case SAI_ATTR_VALUE_TYPE_UINT32:
        case SAI_ATTR_VALUE_TYPE_INT32:
        case SAI_ATTR_VALUE_TYPE_UINT64:
        case SAI_ATTR_VALUE_TYPE_INT64:
        case SAI_ATTR_VALUE_TYPE_MAC:
        case SAI_ATTR_VALUE_TYPE_IPV4:
        case SAI_ATTR_VALUE_TYPE_IPV6:
            // primitives
            break;

        case SAI_ATTR_VALUE_TYPE_IP_ADDRESS:

            {
                switch (value.ipaddr.addr_family)
                {
                    case SAI_IP_ADDR_FAMILY_IPV4:
                    case SAI_IP_ADDR_FAMILY_IPV6:
                        break;

                    default:

                        SWSS_LOG_ERROR("invalid address family: %d", value.ipaddr.addr_family);

                        return SAI_STATUS_INVALID_PARAMETER;
                }

                break;
            }

        case SAI_ATTR_VALUE_TYPE_OBJECT_ID:

            {
                sai_status_t status = meta_generic_validation_objlist(md, 1, &value.oid);

                if (status != SAI_STATUS_SUCCESS)
                {
                    return status;
                }

                break;
            }

        case SAI_ATTR_VALUE_TYPE_OBJECT_LIST:

            {
                sai_status_t status = meta_generic_validation_objlist(md, value.objlist.count, value.objlist.list);

                if (status != SAI_STATUS_SUCCESS)
                {
                    return status;
                }

                break;
            }

            // case SAI_ATTR_VALUE_TYPE_VLAN_LIST:
            // will require test vlan existence

            // ACL FIELD

        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_BOOL:
        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8:
        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT8:
        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT16:
        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT16:
        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT32:
        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT32:
        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_MAC:
        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_IPV4:
        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_IPV6:
            // primitives
            break;

        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_ID:

            {
                sai_status_t status = meta_generic_validation_objlist(md, 1, &value.aclfield.data.oid);

                if (status != SAI_STATUS_SUCCESS)
                {
                    return status;
                }

                break;
            }

        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_LIST:

            {
                sai_status_t status = meta_generic_validation_objlist(md, value.aclfield.data.objlist.count, value.aclfield.data.objlist.list);

                if (status != SAI_STATUS_SUCCESS)
                {
                    return status;
                }

                break;
            }

            // case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8_LIST:

            // ACL ACTION
            //
            // NOTE: when we are going to disable action parameter is not
            // needed (we don't need to validate it) also same object can be
            // enabled/disabled without changing oid? this makes things
            // complicated

        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT8:
        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT8:
        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT16:
        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT16:
        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT32:
        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT32:
        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_MAC:
        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_IPV4:
        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_IPV6:
            break;

        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_ID:

            {
                sai_status_t status = meta_generic_validation_objlist(md, 1, &value.aclaction.parameter.oid);

                if (status != SAI_STATUS_SUCCESS)
                {
                    return status;
                }

                break;
            }

        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_LIST:

            {
                sai_status_t status = meta_generic_validation_objlist(md, value.aclaction.parameter.objlist.count, value.aclaction.parameter.objlist.list);

                if (status != SAI_STATUS_SUCCESS)
                {
                    return status;
                }

                break;
            }

            // ACL END

        case SAI_ATTR_VALUE_TYPE_UINT8_LIST:
            VALIDATION_LIST(md, value.u8list);
            break;
        case SAI_ATTR_VALUE_TYPE_INT8_LIST:
            VALIDATION_LIST(md, value.s8list);
            break;
        case SAI_ATTR_VALUE_TYPE_UINT16_LIST:
            VALIDATION_LIST(md, value.u16list);
            break;
        case SAI_ATTR_VALUE_TYPE_INT16_LIST:
            VALIDATION_LIST(md, value.s16list);
            break;
        case SAI_ATTR_VALUE_TYPE_UINT32_LIST:
            VALIDATION_LIST(md, value.u32list);
            break;
        case SAI_ATTR_VALUE_TYPE_INT32_LIST:
            VALIDATION_LIST(md, value.s32list);
            break;
        case SAI_ATTR_VALUE_TYPE_QOS_MAP_LIST:
            VALIDATION_LIST(md, value.qosmap);
            break;
        case SAI_ATTR_VALUE_TYPE_TUNNEL_MAP_LIST:
            VALIDATION_LIST(md, value.tunnelmap);
            break;

        case SAI_ATTR_VALUE_TYPE_UINT32_RANGE:

            if (value.u32range.min > value.u32range.max)
            {
                META_LOG_ERROR(md, "invalid range %u .. %u", value.u32range.min, value.u32range.max);

                return SAI_STATUS_INVALID_PARAMETER;
            }

            break;

        case SAI_ATTR_VALUE_TYPE_INT32_RANGE:

            if (value.s32range.min > value.s32range.max)
            {
                META_LOG_ERROR(md, "invalid range %u .. %u", value.s32range.min, value.s32range.max);

                return SAI_STATUS_INVALID_PARAMETER;
            }

            break;

        default:

            META_LOG_ERROR(md, "serialization type is not supported yet FIXME");
            throw;
    }

    if (md.isenum)
    {
        int32_t val = value.s32;

        switch (md.attrvaluetype)
        {
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT32:
                val = value.aclfield.data.s32;
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT32:
                val = value.aclaction.parameter.s32;
                break;

            default:
                val = value.s32;
                break;
        }

        if (!sai_metadata_is_allowed_enum_value(&md, val))
        {
            META_LOG_ERROR(md, "is enum, but value %d not found on allowed values list", val);

            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    if (md.isenumlist)
    {
        // we allow repeats on enum list
        if (value.s32list.count != 0 && value.s32list.list == NULL)
        {
            META_LOG_ERROR(md, "enum list is NULL");

            return SAI_STATUS_INVALID_PARAMETER;
        }

        for (uint32_t i = value.s32list.count; i < value.s32list.count; ++i)
        {
            int32_t s32 = value.s32list.list[i];

            if (!sai_metadata_is_allowed_enum_value(&md, s32))
            {
                SWSS_LOG_ERROR("is enum list, but value %d not found on allowed values list", s32);

                return SAI_STATUS_INVALID_PARAMETER;
            }
        }
    }

    if (md.isvlan)
    {
    }

    // is conditional
    if (md.conditions != NULL)
    {
        // check if it was set on local DB
        // (this will not respect create_only with default)
        if (get_object_previous_attr(meta_key, md) == NULL)
        {
            META_LOG_ERROR(md, "set for conditional, but not found in local db");

            return SAI_STATUS_INVALID_PARAMETER;
        }

        META_LOG_DEBUG(md, "conditional attr found in local db");
    }

    // check if object on which we perform operation exists

    std::string key = sai_serialize_object_meta_key(meta_key);

    if (!object_exists(key))
    {
        META_LOG_ERROR(md, "object key %s don't exists", key.c_str());

        return SAI_STATUS_INVALID_PARAMETER;
    }

    // object exists in DB so we can do "set" operation

    switch (meta_key.objecttype)
    {
        case SAI_OBJECT_TYPE_ROUTE_ENTRY:
        case SAI_OBJECT_TYPE_FDB_ENTRY:
        case SAI_OBJECT_TYPE_NEIGHBOR_ENTRY:

            SWSS_LOG_DEBUG("object key exists: %s", key.c_str());
            break;

        default:

            // TODO check if it's object id

            {
                // check if object we are calling SET is the same object type
                // as the type of SET function

                sai_object_id_t oid = meta_key.objectkey.key.object_id;

                sai_object_type_t object_type = sai_object_type_query(oid);

                if (object_type == SAI_NULL_OBJECT_ID)
                {
                    META_LOG_ERROR(md, "oid 0x%lx is not valid, returned null object id", oid);

                    return SAI_STATUS_INVALID_PARAMETER;
                }

                if (object_type != meta_key.objecttype)
                {
                    META_LOG_ERROR(md, "oid 0x%lx type %d is not accepted, expected object type %d", oid, object_type, meta_key.objecttype);

                    return SAI_STATUS_INVALID_PARAMETER;
                }
            }

            break;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t meta_generic_validation_get(
        _In_ const sai_object_meta_key_t& meta_key,
        _In_ const uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    SWSS_LOG_ENTER();

    if (attr_count < 1)
    {
        SWSS_LOG_ERROR("expected at least 1 attribute when calling get, zero given");

        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (attr_count > MAX_LIST_COUNT)
    {
        SWSS_LOG_ERROR("get attribute count is too large %u > then max list count %u", attr_count, MAX_LIST_COUNT);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (attr_list == NULL)
    {
        SWSS_LOG_ERROR("attribute list pointer is NULL");

        return SAI_STATUS_INVALID_PARAMETER;
    }

    SWSS_LOG_DEBUG("attr count = %u", attr_count);

    for (uint32_t i = 0; i < attr_count; ++i)
    {
        const sai_attribute_t* attr = &attr_list[i];

        auto mdp = sai_metadata_get_attr_metadata(meta_key.objecttype, attr->id);

        if (mdp == NULL)
        {
            SWSS_LOG_ERROR("unable to find attribute metadata %d:%d", meta_key.objecttype, attr->id);

            return SAI_STATUS_FAILURE;
        }

        const sai_attribute_value_t& value = attr->value;

        const sai_attr_metadata_t& md = *mdp;

        META_LOG_DEBUG(md, "(get)");

        // is conditional
        if (md.conditions != NULL)
        {
            // check if it was set on local DB
            // (this will not respect create_only with default)
            if (get_object_previous_attr(meta_key, md) == NULL)
            {
                META_LOG_ERROR(md, "request for conditional, but not found in local db");

                return SAI_STATUS_INVALID_PARAMETER;
            }

            META_LOG_DEBUG(md, "conditional attr found in local db");
        }

        switch (md.attrvaluetype)
        {
            case SAI_ATTR_VALUE_TYPE_BOOL:
            case SAI_ATTR_VALUE_TYPE_CHARDATA:
            case SAI_ATTR_VALUE_TYPE_UINT8:
            case SAI_ATTR_VALUE_TYPE_INT8:
            case SAI_ATTR_VALUE_TYPE_UINT16:
            case SAI_ATTR_VALUE_TYPE_INT16:
            case SAI_ATTR_VALUE_TYPE_UINT32:
            case SAI_ATTR_VALUE_TYPE_INT32:
            case SAI_ATTR_VALUE_TYPE_UINT64:
            case SAI_ATTR_VALUE_TYPE_INT64:
            case SAI_ATTR_VALUE_TYPE_MAC:
            case SAI_ATTR_VALUE_TYPE_IPV4:
            case SAI_ATTR_VALUE_TYPE_IPV6:
            case SAI_ATTR_VALUE_TYPE_IP_ADDRESS:
                // primitives
                break;

            case SAI_ATTR_VALUE_TYPE_OBJECT_ID:
                break;

            case SAI_ATTR_VALUE_TYPE_OBJECT_LIST:
                VALIDATION_LIST(md, value.objlist);
                break;

            case SAI_ATTR_VALUE_TYPE_VLAN_LIST:

                {
                    if (value.vlanlist.count == 0 && value.vlanlist.list != NULL)
                    {
                        META_LOG_ERROR(md, "vlan list count is zero, but list not NULL");

                        return SAI_STATUS_INVALID_PARAMETER;
                    }

                    if (value.vlanlist.count != 0 && value.vlanlist.list == NULL)
                    {
                        META_LOG_ERROR(md, "vlan list count is %u, but list is NULL", value.vlanlist.count);

                        return SAI_STATUS_INVALID_PARAMETER;
                    }

                    if (value.vlanlist.count > MAXIMUM_VLAN_NUMBER)
                    {
                        META_LOG_ERROR(md, "vlan count is too big %u > %u", value.vlanlist.count, MAXIMUM_VLAN_NUMBER);

                        return SAI_STATUS_INVALID_PARAMETER;
                    }

                    break;
                }

                // ACL FIELD

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_BOOL:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT8:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT16:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT16:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT32:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT32:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_MAC:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_IPV4:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_IPV6:
                // primitives
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_ID:
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_LIST:
                VALIDATION_LIST(md, value.aclfield.data.objlist);
                break;

                // case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8_LIST:

                // ACL ACTION

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT8:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT8:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT16:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT16:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT32:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT32:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_MAC:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_IPV4:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_IPV6:
                // primitives
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_ID:
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_LIST:
                VALIDATION_LIST(md, value.aclaction.parameter.objlist);
                break;

                // ACL END

            case SAI_ATTR_VALUE_TYPE_UINT8_LIST:
                VALIDATION_LIST(md, value.u8list);
                break;
            case SAI_ATTR_VALUE_TYPE_INT8_LIST:
                VALIDATION_LIST(md, value.s8list);
                break;
            case SAI_ATTR_VALUE_TYPE_UINT16_LIST:
                VALIDATION_LIST(md, value.u16list);
                break;
            case SAI_ATTR_VALUE_TYPE_INT16_LIST:
                VALIDATION_LIST(md, value.s16list);
                break;
            case SAI_ATTR_VALUE_TYPE_UINT32_LIST:
                VALIDATION_LIST(md, value.u32list);
                break;
            case SAI_ATTR_VALUE_TYPE_INT32_LIST:
                VALIDATION_LIST(md, value.s32list);
                break;
            case SAI_ATTR_VALUE_TYPE_QOS_MAP_LIST:
                VALIDATION_LIST(md, value.qosmap);
                break;
            case SAI_ATTR_VALUE_TYPE_TUNNEL_MAP_LIST:
                VALIDATION_LIST(md, value.tunnelmap);
                break;

            case SAI_ATTR_VALUE_TYPE_UINT32_RANGE:
            case SAI_ATTR_VALUE_TYPE_INT32_RANGE:
                // primitives
                break;

            default:

                // acl capability will is more complex since is in/out we need to check stage

                META_LOG_ERROR(md, "serialization type is not supported yet FIXME");
                throw;
        }
    }

    std::string key = sai_serialize_object_meta_key(meta_key);

    if (!object_exists(key))
    {
        SWSS_LOG_ERROR("object key %s don't exists", key.c_str());

        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch (meta_key.objecttype)
    {
        case SAI_OBJECT_TYPE_ROUTE_ENTRY:
        case SAI_OBJECT_TYPE_FDB_ENTRY:
        case SAI_OBJECT_TYPE_NEIGHBOR_ENTRY:

            SWSS_LOG_DEBUG("object key exists: %s", key.c_str());

            break;

        default:

            // TODO check if it's object id

            {
                // check if object we are calling GET is the same object type
                // as the type of GET function

                sai_object_id_t oid = meta_key.objectkey.key.object_id;

                sai_object_type_t object_type = sai_object_type_query(oid);

                if (object_type == SAI_NULL_OBJECT_ID)
                {
                    SWSS_LOG_ERROR("oid 0x%lx is not valid, returned null object id", oid);

                    return SAI_STATUS_INVALID_PARAMETER;
                }

                if (object_type != meta_key.objecttype)
                {
                    SWSS_LOG_ERROR("oid 0x%lx type %d is not accepted, expected object type %d", oid, object_type, meta_key.objecttype);

                    return SAI_STATUS_INVALID_PARAMETER;
                }
            }

            break;
    }

    // object exists in DB so we can do "get" operation

    return SAI_STATUS_SUCCESS;
}

void meta_generic_validation_post_create(
        _In_ const sai_object_meta_key_t& meta_key,
        _In_ const uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    SWSS_LOG_ENTER();

    std::string key = sai_serialize_object_meta_key(meta_key);

    if (object_exists(key))
    {
        SWSS_LOG_ERROR("object key %s already exists (vendor bug?)", key.c_str());

        // this may produce inconsistency
    }

    create_object(meta_key);

    // TODO convert to automatic oid check from metadata struct members
    switch (meta_key.objecttype)
    {
        case SAI_OBJECT_TYPE_ROUTE_ENTRY:
            object_reference_inc(meta_key.objectkey.key.route_entry.vr_id);
            break;

        case SAI_OBJECT_TYPE_NEIGHBOR_ENTRY:
            object_reference_inc(meta_key.objectkey.key.neighbor_entry.rif_id);
            break;

        case SAI_OBJECT_TYPE_FDB_ENTRY:
            // NOTE: we don't increase vlan reference on FDB entries, ignored
            // vlan_reference_inc(meta_key.key.fdb_entry.vlan_id);
            break;

        default:

            // TODO check if its object id
            {
                // check if object created was expected type
                // as the type of SET function

                sai_object_id_t oid = meta_key.objectkey.key.object_id;

                if (oid == SAI_NULL_OBJECT_ID)
                {
                    SWSS_LOG_ERROR("created oid is null object id (vendor bug?)");
                    break;
                }

                sai_object_type_t object_type = sai_object_type_query(oid);

                if (object_type == SAI_NULL_OBJECT_ID)
                {
                    SWSS_LOG_ERROR("created oid 0x%lx is not valid object type after create, returned null object id (vendor bug?)", oid);
                    break;
                }

                if (object_type != meta_key.objecttype)
                {
                    SWSS_LOG_ERROR("created oid 0x%lx type %d is wrong type, expected object type %d (vendor bug?)", oid, object_type, meta_key.objecttype);
                    break;
                }

                object_reference_insert(oid);

                break;
            }
    }

    bool haskeys;

    for (uint32_t idx = 0; idx < attr_count; ++idx)
    {
        const sai_attribute_t* attr = &attr_list[idx];

        auto mdp = sai_metadata_get_attr_metadata(meta_key.objecttype, attr->id);

        const sai_attribute_value_t& value = attr->value;

        const sai_attr_metadata_t& md = *mdp;

        if (HAS_FLAG_KEY(md.flags))
        {
            haskeys = true;
            META_LOG_DEBUG(md, "attr is key");
        }

        // increase reference on object id types

        switch (md.attrvaluetype)
        {
            case SAI_ATTR_VALUE_TYPE_BOOL:
            case SAI_ATTR_VALUE_TYPE_CHARDATA:
            case SAI_ATTR_VALUE_TYPE_UINT8:
            case SAI_ATTR_VALUE_TYPE_INT8:
            case SAI_ATTR_VALUE_TYPE_UINT16:
            case SAI_ATTR_VALUE_TYPE_INT16:
            case SAI_ATTR_VALUE_TYPE_UINT32:
            case SAI_ATTR_VALUE_TYPE_INT32:
            case SAI_ATTR_VALUE_TYPE_UINT64:
            case SAI_ATTR_VALUE_TYPE_INT64:
            case SAI_ATTR_VALUE_TYPE_MAC:
            case SAI_ATTR_VALUE_TYPE_IPV4:
            case SAI_ATTR_VALUE_TYPE_IPV6:
            case SAI_ATTR_VALUE_TYPE_IP_ADDRESS:
                // primitives
                break;

            case SAI_ATTR_VALUE_TYPE_OBJECT_ID:
                object_reference_inc(value.oid);
                break;

            case SAI_ATTR_VALUE_TYPE_OBJECT_LIST:
                object_reference_inc(value.objlist);
                break;

            case SAI_ATTR_VALUE_TYPE_VLAN_LIST:
                break;

                // ACL FIELD

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_BOOL:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT8:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT16:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT16:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT32:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT32:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_MAC:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_IPV4:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_IPV6:
                // primitives
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_ID:
                object_reference_inc(value.aclfield.data.oid);
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_LIST:
                object_reference_inc(value.aclfield.data.objlist);
                break;

                // case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8_LIST:

                // ACL ACTION

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT8:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT8:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT16:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT16:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT32:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT32:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_MAC:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_IPV4:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_IPV6:
                // primitives
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_ID:
                object_reference_inc(value.aclaction.parameter.oid);
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_LIST:
                object_reference_inc(value.aclaction.parameter.objlist);
                break;

                // ACL END

            case SAI_ATTR_VALUE_TYPE_UINT8_LIST:
            case SAI_ATTR_VALUE_TYPE_INT8_LIST:
            case SAI_ATTR_VALUE_TYPE_UINT16_LIST:
            case SAI_ATTR_VALUE_TYPE_INT16_LIST:
            case SAI_ATTR_VALUE_TYPE_UINT32_LIST:
            case SAI_ATTR_VALUE_TYPE_INT32_LIST:
            case SAI_ATTR_VALUE_TYPE_QOS_MAP_LIST:
            case SAI_ATTR_VALUE_TYPE_TUNNEL_MAP_LIST:
            case SAI_ATTR_VALUE_TYPE_UINT32_RANGE:
            case SAI_ATTR_VALUE_TYPE_INT32_RANGE:
                // no special action required
                break;

            default:

                META_LOG_ERROR(md, "serialization type is not supported yet FIXME");
                throw;
        }

        if (md.isvlan)
        {
        }

        set_object(meta_key, md, attr);
    }

    if (haskeys)
    {
        std::string ok = sai_serialize_object_meta_key(meta_key);

        AttributeKeys[ok] = construct_key(meta_key, attr_count, attr_list);
    }
}

void meta_generic_validation_post_remove(
        _In_ const sai_object_meta_key_t& meta_key)
{
    SWSS_LOG_ENTER();

    // get all attributes that was set

    for (auto&it: get_object(meta_key))
    {
        const sai_attribute_t* attr = it->getattr();

        auto mdp = sai_metadata_get_attr_metadata(meta_key.objecttype, attr->id);

        const sai_attribute_value_t& value = attr->value;

        const sai_attr_metadata_t& md = *mdp;

        // decrease reference on object id types

        switch (md.attrvaluetype)
        {
            case SAI_ATTR_VALUE_TYPE_BOOL:
            case SAI_ATTR_VALUE_TYPE_CHARDATA:
            case SAI_ATTR_VALUE_TYPE_UINT8:
            case SAI_ATTR_VALUE_TYPE_INT8:
            case SAI_ATTR_VALUE_TYPE_UINT16:
            case SAI_ATTR_VALUE_TYPE_INT16:
            case SAI_ATTR_VALUE_TYPE_UINT32:
            case SAI_ATTR_VALUE_TYPE_INT32:
            case SAI_ATTR_VALUE_TYPE_UINT64:
            case SAI_ATTR_VALUE_TYPE_INT64:
            case SAI_ATTR_VALUE_TYPE_MAC:
            case SAI_ATTR_VALUE_TYPE_IPV4:
            case SAI_ATTR_VALUE_TYPE_IPV6:
            case SAI_ATTR_VALUE_TYPE_IP_ADDRESS:
                // primitives, ok
                break;

            case SAI_ATTR_VALUE_TYPE_OBJECT_ID:
                object_reference_dec(value.oid);
                break;

            case SAI_ATTR_VALUE_TYPE_OBJECT_LIST:
                object_reference_dec(value.objlist);
                break;

                //case SAI_ATTR_VALUE_TYPE_VLAN_LIST:
                // will require dec vlan references

                // ACL FIELD

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_BOOL:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT8:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT16:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT16:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT32:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT32:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_MAC:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_IPV4:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_IPV6:
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_ID:
                object_reference_dec(value.aclfield.data.oid);
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_LIST:
                object_reference_dec(value.aclfield.data.objlist);
                break;

                // case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8_LIST:

                // ACL ACTION

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT8:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT8:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT16:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT16:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT32:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT32:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_MAC:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_IPV4:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_IPV6:
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_ID:
                object_reference_dec(value.aclaction.parameter.oid);
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_LIST:
                object_reference_dec(value.aclaction.parameter.objlist);
                break;

                // ACL END

            case SAI_ATTR_VALUE_TYPE_UINT8_LIST:
            case SAI_ATTR_VALUE_TYPE_INT8_LIST:
            case SAI_ATTR_VALUE_TYPE_UINT16_LIST:
            case SAI_ATTR_VALUE_TYPE_INT16_LIST:
            case SAI_ATTR_VALUE_TYPE_UINT32_LIST:
            case SAI_ATTR_VALUE_TYPE_INT32_LIST:
            case SAI_ATTR_VALUE_TYPE_QOS_MAP_LIST:
            case SAI_ATTR_VALUE_TYPE_TUNNEL_MAP_LIST:
            case SAI_ATTR_VALUE_TYPE_UINT32_RANGE:
            case SAI_ATTR_VALUE_TYPE_INT32_RANGE:
                // no special action required
                break;

            default:
                META_LOG_ERROR(md, "serialization type is not supported yet FIXME");
                throw;
        }

        if (md.isvlan)
        {
        }
    }

    // we don't keep track of fdb, neighbor, route since
    // those are safe to remove any time (leafs)

    switch (meta_key.objecttype)
    {
        case SAI_OBJECT_TYPE_ROUTE_ENTRY:
            object_reference_dec(meta_key.objectkey.key.route_entry.vr_id);
            break;

        case SAI_OBJECT_TYPE_NEIGHBOR_ENTRY:
            object_reference_dec(meta_key.objectkey.key.neighbor_entry.rif_id);
            break;

        case SAI_OBJECT_TYPE_FDB_ENTRY:
            // NOTE: we don't decrease vlan reference on FDB entries, ignored
            // vlan_reference_dec(meta_key.key.fdb_entry.vlan_id);
            break;

        default:

            // TODO check if it's oid
            object_reference_remove(meta_key.objectkey.key.object_id);
            break;
    }

    remove_object(meta_key);

    std::string ok = sai_serialize_object_meta_key(meta_key);

    if (AttributeKeys.find(ok) != AttributeKeys.end())
    {
        SWSS_LOG_DEBUG("erasing attributes key %s", AttributeKeys[ok].c_str());

        AttributeKeys.erase(ok);
    }
}

void meta_generic_validation_post_set(
        _In_ const sai_object_meta_key_t& meta_key,
        _In_ const sai_attribute_t *attr)
{
    SWSS_LOG_ENTER();

    auto mdp = sai_metadata_get_attr_metadata(meta_key.objecttype, attr->id);

    const sai_attribute_value_t& value = attr->value;

    const sai_attr_metadata_t& md = *mdp;

    switch (md.attrvaluetype)
    {
        case SAI_ATTR_VALUE_TYPE_BOOL:
        case SAI_ATTR_VALUE_TYPE_CHARDATA:
        case SAI_ATTR_VALUE_TYPE_UINT8:
        case SAI_ATTR_VALUE_TYPE_INT8:
        case SAI_ATTR_VALUE_TYPE_UINT16:
        case SAI_ATTR_VALUE_TYPE_INT16:
        case SAI_ATTR_VALUE_TYPE_UINT32:
        case SAI_ATTR_VALUE_TYPE_INT32:
        case SAI_ATTR_VALUE_TYPE_UINT64:
        case SAI_ATTR_VALUE_TYPE_INT64:
        case SAI_ATTR_VALUE_TYPE_MAC:
        case SAI_ATTR_VALUE_TYPE_IPV4:
        case SAI_ATTR_VALUE_TYPE_IPV6:
        case SAI_ATTR_VALUE_TYPE_IP_ADDRESS:
            // primitives, ok
            break;

        case SAI_ATTR_VALUE_TYPE_OBJECT_ID:

            {
                const sai_attribute_t *previous_attr = get_object_previous_attr(meta_key, md);

                if (previous_attr != NULL)
                {
                    // decrease previous if it was set
                    object_reference_dec(previous_attr->value.oid);
                }

                object_reference_inc(value.oid);

                break;
            }

        case SAI_ATTR_VALUE_TYPE_OBJECT_LIST:

            {
                const sai_attribute_t *previous_attr = get_object_previous_attr(meta_key, md);

                if (previous_attr != NULL)
                {
                    // decrease previous if it was set
                    object_reference_dec(previous_attr->value.objlist);
                }

                object_reference_inc(value.objlist);

                break;
            }

            // case SAI_ATTR_VALUE_TYPE_VLAN_LIST:
            // will require increase vlan references

            // ACL FIELD

        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_BOOL:
        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8:
        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT8:
        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT16:
        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT16:
        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT32:
        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT32:
        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_MAC:
        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_IPV4:
        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_IPV6:
            break;

        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_ID:

            {
                const sai_attribute_t *previous_attr = get_object_previous_attr(meta_key, md);

                if (previous_attr != NULL)
                {
                    // decrease previous if it was set
                    object_reference_dec(previous_attr->value.aclfield.data.oid);
                }

                object_reference_inc(value.aclfield.data.oid);

                break;
            }

        case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_LIST:

            {
                const sai_attribute_t *previous_attr = get_object_previous_attr(meta_key, md);

                if (previous_attr != NULL)
                {
                    // decrease previous if it was set
                    object_reference_dec(previous_attr->value.aclfield.data.objlist);
                }

                object_reference_inc(value.aclfield.data.objlist);

                break;
            }

            // case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8_LIST:

            // ACL ACTION

        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT8:
        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT8:
        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT16:
        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT16:
        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT32:
        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT32:
        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_MAC:
        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_IPV4:
        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_IPV6:
            // primitives
            break;

        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_ID:

            {
                const sai_attribute_t *previous_attr = get_object_previous_attr(meta_key, md);

                if (previous_attr != NULL)
                {
                    // decrease previous if it was set
                    object_reference_dec(previous_attr->value.aclaction.parameter.oid);
                }

                object_reference_inc(value.aclaction.parameter.oid);
                break;
            }

        case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_LIST:

            {
                const sai_attribute_t *previous_attr = get_object_previous_attr(meta_key, md);

                if (previous_attr != NULL)
                {
                    // decrease previous if it was set
                    object_reference_dec(previous_attr->value.aclaction.parameter.objlist);
                }

                object_reference_inc(value.aclaction.parameter.objlist);

                break;
            }

            // ACL END

        case SAI_ATTR_VALUE_TYPE_UINT8_LIST:
        case SAI_ATTR_VALUE_TYPE_INT8_LIST:
        case SAI_ATTR_VALUE_TYPE_UINT16_LIST:
        case SAI_ATTR_VALUE_TYPE_INT16_LIST:
        case SAI_ATTR_VALUE_TYPE_UINT32_LIST:
        case SAI_ATTR_VALUE_TYPE_INT32_LIST:
        case SAI_ATTR_VALUE_TYPE_QOS_MAP_LIST:
        case SAI_ATTR_VALUE_TYPE_TUNNEL_MAP_LIST:
        case SAI_ATTR_VALUE_TYPE_UINT32_RANGE:
        case SAI_ATTR_VALUE_TYPE_INT32_RANGE:
            // no special action required
            break;

        default:
            META_LOG_ERROR(md, "serialization type is not supported yet FIXME");
            throw;
    }

    if (md.isvlan)
    {
    }

    // only on create we need to increase entry object types members
    // save actual attributes and values to local db

    set_object(meta_key, md, attr);
}

void meta_generic_validation_post_get_objlist(
        _In_ const sai_object_meta_key_t& meta_key,
        _In_ const sai_attr_metadata_t& md,
        _In_ uint32_t count,
        _In_ const sai_object_id_t* list)
{
    SWSS_LOG_ENTER();

    if (count > MAX_LIST_COUNT)
    {
        META_LOG_ERROR(md, "returned get object list count %u is > then max list count %u", count, MAX_LIST_COUNT);
    }

    if (list == NULL)
    {
        // query was for length
        return;
    }

    std::set<sai_object_id_t> oids;

    for (uint32_t i = 0; i < count; ++i)
    {
        sai_object_id_t oid = list[i];

        if (oids.find(oid) != oids.end())
        {
            META_LOG_ERROR(md, "returned get object on list [%u] is duplicated, but not allowed", i);
            continue;
        }

        oids.insert(oid);

        if (oid == SAI_NULL_OBJECT_ID)
        {
            if (md.allownullobjectid)
            {
                // ok, null object is allowed
                continue;
            }

            META_LOG_ERROR(md, "returned get object on list [%u] is NULL, but not allowed", i);
            continue;
        }

        sai_object_type_t ot = sai_object_type_query(oid);

        if (ot == SAI_NULL_OBJECT_ID)
        {
            META_LOG_ERROR(md, "returned get object on list [%u] oid 0x%lx is not valid, returned null object id", i, oid);
            continue;
        }

        if (!sai_metadata_is_allowed_object_type(&md, ot))
        {
            META_LOG_ERROR(md, "returned get object on list [%u] oid 0x%lx object type %d is not allowed on this attribute", i, oid, ot);
        }

        if (!object_reference_exists(oid))
        {
            // NOTE: there may happen that user will request multiple object lists
            // and first list was retrived ok, but second failed with overflow
            // then we may forget to snoop

            META_LOG_INFO(md, "returned get object on list [%u] oid 0x%lx object type %d does not exists in local DB (snoop)", i, oid, ot);

            sai_object_meta_key_t key = { .objecttype = ot, .objectkey = { .key = { .object_id = oid } } };

            object_reference_insert(oid);

            if (!object_exists(key))
            {
                create_object(key);
            }
        }
    }
}

#define VALIDATION_LIST_GET(md, list) \
{\
    if (list.count > MAX_LIST_COUNT)\
    {\
        META_LOG_ERROR(md, "list count %u is > then max list count %u", list.count, MAX_LIST_COUNT);\
    }\
}

void meta_generic_validation_post_get(
        _In_ const sai_object_meta_key_t& meta_key,
        _In_ const uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    SWSS_LOG_ENTER();

    for (uint32_t idx = 0; idx < attr_count; ++idx)
    {
        const sai_attribute_t* attr = &attr_list[idx];

        auto mdp = sai_metadata_get_attr_metadata(meta_key.objecttype, attr->id);

        const sai_attribute_value_t& value = attr->value;

        const sai_attr_metadata_t& md = *mdp;

        switch (md.attrvaluetype)
        {
            case SAI_ATTR_VALUE_TYPE_BOOL:
            case SAI_ATTR_VALUE_TYPE_CHARDATA:
            case SAI_ATTR_VALUE_TYPE_UINT8:
            case SAI_ATTR_VALUE_TYPE_INT8:
            case SAI_ATTR_VALUE_TYPE_UINT16:
            case SAI_ATTR_VALUE_TYPE_INT16:
            case SAI_ATTR_VALUE_TYPE_UINT32:
            case SAI_ATTR_VALUE_TYPE_INT32:
            case SAI_ATTR_VALUE_TYPE_UINT64:
            case SAI_ATTR_VALUE_TYPE_INT64:
            case SAI_ATTR_VALUE_TYPE_MAC:
            case SAI_ATTR_VALUE_TYPE_IPV4:
            case SAI_ATTR_VALUE_TYPE_IPV6:
            case SAI_ATTR_VALUE_TYPE_IP_ADDRESS:
                // primitives, ok
                break;

            case SAI_ATTR_VALUE_TYPE_OBJECT_ID:
                meta_generic_validation_post_get_objlist(meta_key, md, 1, &value.oid);
                break;

            case SAI_ATTR_VALUE_TYPE_OBJECT_LIST:
                meta_generic_validation_post_get_objlist(meta_key, md, value.objlist.count, value.objlist.list);
                break;

            case SAI_ATTR_VALUE_TYPE_VLAN_LIST:

                {
                    uint32_t count = value.vlanlist.count;

                    if (count > MAXIMUM_VLAN_NUMBER)
                    {
                        META_LOG_ERROR(md, "too many vlans returned on vlan list (vendor bug?)");
                    }

                    if (value.vlanlist.list == NULL)
                    {
                        break;
                    }

                    for (uint32_t i = 0; i < count; ++i)
                    {
                        // TODO check if vlans are in range

                        // META_LOG_ERROR(md, "vlan id %d not exists, but returned on list [%u] (snoop?)", vlan_id, i);
                    }

                    break;
                }

                // ACL FIELD

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_BOOL:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT8:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT16:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT16:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT32:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT32:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_MAC:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_IPV4:
            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_IPV6:
                // primitives
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_ID:
                meta_generic_validation_post_get_objlist(meta_key, md, 1, &value.aclfield.data.oid);
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_OBJECT_LIST:
                meta_generic_validation_post_get_objlist(meta_key, md, value.aclfield.data.objlist.count, value.aclfield.data.objlist.list);
                break;

                // case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8_LIST: (2 lists)

                // ACL ACTION

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT8:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT8:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT16:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT16:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT32:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT32:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_MAC:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_IPV4:
            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_IPV6:
                // primitives
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_ID:
                meta_generic_validation_post_get_objlist(meta_key, md, 1, &value.aclaction.parameter.oid);
                break;

            case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_OBJECT_LIST:
                meta_generic_validation_post_get_objlist(meta_key, md, value.aclaction.parameter.objlist.count, value.aclaction.parameter.objlist.list);
                break;

                // ACL END

            case SAI_ATTR_VALUE_TYPE_UINT8_LIST:
                VALIDATION_LIST_GET(md, value.u8list);
                break;
            case SAI_ATTR_VALUE_TYPE_INT8_LIST:
                VALIDATION_LIST_GET(md, value.s8list);
                break;
            case SAI_ATTR_VALUE_TYPE_UINT16_LIST:
                VALIDATION_LIST_GET(md, value.u16list);
                break;
            case SAI_ATTR_VALUE_TYPE_INT16_LIST:
                VALIDATION_LIST_GET(md, value.s16list);
                break;
            case SAI_ATTR_VALUE_TYPE_UINT32_LIST:
                VALIDATION_LIST_GET(md, value.u32list);
                break;
            case SAI_ATTR_VALUE_TYPE_INT32_LIST:
                VALIDATION_LIST_GET(md, value.s32list);
                break;
            case SAI_ATTR_VALUE_TYPE_QOS_MAP_LIST:
                VALIDATION_LIST_GET(md, value.qosmap);
                break;
            case SAI_ATTR_VALUE_TYPE_TUNNEL_MAP_LIST:
                VALIDATION_LIST_GET(md, value.tunnelmap);
                break;

            case SAI_ATTR_VALUE_TYPE_UINT32_RANGE:

                if (value.u32range.min > value.u32range.max)
                {
                    META_LOG_ERROR(md, "invalid range %u .. %u", value.u32range.min, value.u32range.max);
                }

                break;

            case SAI_ATTR_VALUE_TYPE_INT32_RANGE:

                if (value.s32range.min > value.s32range.max)
                {
                    META_LOG_ERROR(md, "invalid range %u .. %u", value.s32range.min, value.s32range.max);
                }

                break;

            default:

                META_LOG_ERROR(md, "serialization type is not supported yet FIXME");
                throw;
        }

        if (md.isenum)
        {
            int32_t val = value.s32;

            switch (md.attrvaluetype)
            {
                case SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_INT32:
                    val = value.aclfield.data.s32;
                    break;

                case SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_INT32:
                    val = value.aclaction.parameter.s32;
                    break;

                default:
                    val = value.s32;
                    break;
            }

            if (!sai_metadata_is_allowed_enum_value(&md, val))
            {
                META_LOG_ERROR(md, "is enum, but value %d not found on allowed values list", val);
                continue;
            }
        }

        if (md.isenumlist)
        {
            if (value.s32list.list == NULL)
            {
                continue;
            }

            for (uint32_t i = value.s32list.count; i < value.s32list.count; ++i)
            {
                int32_t s32 = value.s32list.list[i];

                if (!sai_metadata_is_allowed_enum_value(&md, s32))
                {
                    META_LOG_ERROR(md, "is enum list, but value %d not found on allowed values list", s32);
                }
            }
        }

        if (md.isvlan)
        {
        }
    }
}

// FDB ENTRY

sai_status_t meta_sai_validate_fdb_entry(
        _In_ const sai_fdb_entry_t* fdb_entry,
        _In_ bool create,
        _In_ bool get = false)
{
    SWSS_LOG_ENTER();

    if (fdb_entry == NULL)
    {
        SWSS_LOG_ERROR("fdb_entry pointer is NULL");

        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_vlan_id_t vlan_id = fdb_entry->vlan_id;

    if (vlan_id < MINIMUM_VLAN_NUMBER || vlan_id > MAXIMUM_VLAN_NUMBER)
    {
        SWSS_LOG_ERROR("invalid vlan number %d expected <%d..%d>", vlan_id, MINIMUM_VLAN_NUMBER, MAXIMUM_VLAN_NUMBER);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    // check if vlan exists
    // NOTE: this is disabled on purpose, we can create/set/get/remove fdb entris with non existing vlans

    /*
    sai_object_meta_key_t meta_key_vlan = { .object_type = SAI_OBJECT_TYPE_VLAN, .key = { .vlan_id = vlan_id } };

    std::string key_vlan = sai_serialize_object_meta_key(meta_key_vlan);

    if (!object_exists(key_vlan))
    {
        SWSS_LOG_ERROR("object key %s don't exists", key_vlan.c_str());

        return SAI_STATUS_INVALID_PARAMETER;
    }
    */

    // check if fdb entry exists

    sai_object_meta_key_t meta_key_fdb = { .objecttype = SAI_OBJECT_TYPE_FDB_ENTRY, .objectkey = { .key = { .fdb_entry = *fdb_entry } } };

    std::string key_fdb = sai_serialize_object_meta_key(meta_key_fdb);

    if (create)
    {
        if (object_exists(key_fdb))
        {
            SWSS_LOG_ERROR("object key %s already exists", key_fdb.c_str());

            return SAI_STATUS_ITEM_ALREADY_EXISTS;
        }

        return SAI_STATUS_SUCCESS;
    }

    // set, get, remove

    if (!object_exists(key_fdb) && !get)
    {
        SWSS_LOG_ERROR("object key %s don't exists", key_fdb.c_str());

        return SAI_STATUS_INVALID_PARAMETER;
    }

    // fdb entry is valid

    return SAI_STATUS_SUCCESS;
}

sai_status_t meta_sai_create_fdb_entry(
        _In_ const sai_fdb_entry_t* fdb_entry,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list,
        _In_ sai_create_fdb_entry_fn create)
{
    SWSS_LOG_ENTER();

    sai_status_t status = meta_sai_validate_fdb_entry(fdb_entry, true);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    sai_object_meta_key_t meta_key = { .objecttype = SAI_OBJECT_TYPE_FDB_ENTRY, .objectkey = { .key = { .fdb_entry = *fdb_entry  } } };

    status = meta_generic_validation_create(meta_key, attr_count, attr_list);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    if (create == NULL)
    {
        SWSS_LOG_ERROR("create function pointer is NULL");

        return SAI_STATUS_FAILURE;
    }

    status = create(fdb_entry, attr_count, attr_list);

    if (status == SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_DEBUG("create status: %d", status);
    }
    else
    {
        SWSS_LOG_ERROR("create status: %d", status);
    }

    if (status == SAI_STATUS_SUCCESS)
    {
        meta_generic_validation_post_create(meta_key, attr_count, attr_list);
    }

    return status;
}

sai_status_t meta_sai_remove_fdb_entry(
        _In_ const sai_fdb_entry_t* fdb_entry,
        _In_ sai_remove_fdb_entry_fn remove)
{
    SWSS_LOG_ENTER();

    sai_status_t status = meta_sai_validate_fdb_entry(fdb_entry, false);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    sai_object_meta_key_t meta_key = { .objecttype = SAI_OBJECT_TYPE_FDB_ENTRY, .objectkey = { .key = { .fdb_entry = *fdb_entry  } } };

    status = meta_generic_validation_remove(meta_key);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    if (remove == NULL)
    {
        SWSS_LOG_ERROR("remove function pointer is NULL");

        return SAI_STATUS_FAILURE;
    }

    status = remove(fdb_entry);

    if (status == SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_DEBUG("remove status: %d", status);
    }
    else
    {
        SWSS_LOG_ERROR("remove status: %d", status);
    }

    if (status == SAI_STATUS_SUCCESS)
    {
        meta_generic_validation_post_remove(meta_key);
    }

    return status;
}

sai_status_t meta_sai_set_fdb_entry(
        _In_ const sai_fdb_entry_t* fdb_entry,
        _In_ const sai_attribute_t *attr,
        _In_ sai_set_fdb_entry_attribute_fn set)
{
    SWSS_LOG_ENTER();

    sai_status_t status = meta_sai_validate_fdb_entry(fdb_entry, false);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    sai_object_meta_key_t meta_key = { .objecttype = SAI_OBJECT_TYPE_FDB_ENTRY, .objectkey = { .key = { .fdb_entry = *fdb_entry  } } };

    status = meta_generic_validation_set(meta_key, attr);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    if (set == NULL)
    {
        SWSS_LOG_ERROR("set function pointer is NULL");

        return SAI_STATUS_FAILURE;
    }

    status = set(fdb_entry, attr);

    if (status == SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_DEBUG("set status: %d", status);
    }
    else
    {
        SWSS_LOG_ERROR("set status: %d", status);
    }

    if (status == SAI_STATUS_SUCCESS)
    {
        meta_generic_validation_post_set(meta_key, attr);
    }

    return status;
}

sai_status_t meta_sai_get_fdb_entry(
        _In_ const sai_fdb_entry_t* fdb_entry,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list,
        _In_ sai_get_fdb_entry_attribute_fn get)
{
    SWSS_LOG_ENTER();

    // NOTE: when doing get, entry may not exist on metadata db

    sai_status_t status = meta_sai_validate_fdb_entry(fdb_entry, false, true);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    sai_object_meta_key_t meta_key = { .objecttype = SAI_OBJECT_TYPE_FDB_ENTRY, .objectkey = { .key = { .fdb_entry = *fdb_entry } } };

    status = meta_generic_validation_get(meta_key, attr_count, attr_list);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    if (get == NULL)
    {
        SWSS_LOG_ERROR("get function pointer is NULL");

        return SAI_STATUS_FAILURE;
    }

    status = get(fdb_entry, attr_count, attr_list);

    if (status == SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_DEBUG("get status: %d", status);
    }
    else
    {
        SWSS_LOG_ERROR("get status: %d", status);
    }

    if (status == SAI_STATUS_SUCCESS)
    {
        meta_generic_validation_post_get(meta_key, attr_count, attr_list);
    }

    return status;
}

// NEIGHBOR ENTRY

sai_status_t meta_sai_validate_neighbor_entry(
        _In_ const sai_neighbor_entry_t* neighbor_entry,
        _In_ bool create)
{
    SWSS_LOG_ENTER();

    if (neighbor_entry == NULL)
    {
        SWSS_LOG_ERROR("neighbor_entry pointer is NULL");

        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch (neighbor_entry->ip_address.addr_family)
    {
        case SAI_IP_ADDR_FAMILY_IPV4:
        case SAI_IP_ADDR_FAMILY_IPV6:
            break;

        default:

            SWSS_LOG_ERROR("invalid address family: %d", neighbor_entry->ip_address.addr_family);

            return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_object_id_t rif = neighbor_entry->rif_id;

    if (rif == SAI_NULL_OBJECT_ID)
    {
        SWSS_LOG_ERROR("router interface is set to null object id");

        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_object_type_t object_type = sai_object_type_query(rif);

    if (object_type == SAI_NULL_OBJECT_ID)
    {
        SWSS_LOG_ERROR("router interface oid 0x%lx is not valid object type, returned null object id", rif);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_object_type_t expected = SAI_OBJECT_TYPE_ROUTER_INTERFACE;

    if (object_type != expected)
    {
        SWSS_LOG_ERROR("router interface oid 0x%lx type %d is wrong type, expected object type %d", rif, object_type, expected);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    // check if router interface exists

    sai_object_meta_key_t meta_key_rif = { .objecttype = expected, .objectkey = { .key = { .object_id = rif } } };

    std::string key_rif = sai_serialize_object_meta_key(meta_key_rif);

    if (!object_exists(key_rif))
    {
        SWSS_LOG_ERROR("object key %s don't exists", key_rif.c_str());

        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_object_meta_key_t meta_key_neighbor = { .objecttype = SAI_OBJECT_TYPE_NEIGHBOR_ENTRY, .objectkey = { .key = { .neighbor_entry = *neighbor_entry } } };

    std::string key_neighbor = sai_serialize_object_meta_key(meta_key_neighbor);

    if (create)
    {
        if (object_exists(key_neighbor))
        {
            SWSS_LOG_ERROR("object key %s already exists", key_neighbor.c_str());

            return SAI_STATUS_ITEM_ALREADY_EXISTS;
        }

        return SAI_STATUS_SUCCESS;
    }

    // set, get, remove

    if (!object_exists(key_neighbor))
    {
        SWSS_LOG_ERROR("object key %s don't exists", key_neighbor.c_str());

        return SAI_STATUS_INVALID_PARAMETER;
    }

    // neighbor entry is valid

    return SAI_STATUS_SUCCESS;
}

sai_status_t meta_sai_create_neighbor_entry(
        _In_ const sai_neighbor_entry_t* neighbor_entry,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list,
        _In_ sai_create_neighbor_entry_fn create)
{
    SWSS_LOG_ENTER();

    sai_status_t status = meta_sai_validate_neighbor_entry(neighbor_entry, true);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    sai_object_meta_key_t meta_key = { .objecttype = SAI_OBJECT_TYPE_NEIGHBOR_ENTRY, .objectkey = { .key = { .neighbor_entry = *neighbor_entry  } } };

    status = meta_generic_validation_create(meta_key, attr_count, attr_list);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    if (create == NULL)
    {
        SWSS_LOG_ERROR("create function pointer is NULL");

        return SAI_STATUS_FAILURE;
    }

    status = create(neighbor_entry, attr_count, attr_list);

    if (status == SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_DEBUG("create status: %d", status);
    }
    else
    {
        SWSS_LOG_ERROR("create status: %d", status);
    }

    if (status == SAI_STATUS_SUCCESS)
    {
        meta_generic_validation_post_create(meta_key, attr_count, attr_list);
    }

    return status;
}

sai_status_t meta_sai_remove_neighbor_entry(
        _In_ const sai_neighbor_entry_t* neighbor_entry,
        _In_ sai_remove_neighbor_entry_fn remove)
{
    SWSS_LOG_ENTER();

    sai_status_t status = meta_sai_validate_neighbor_entry(neighbor_entry, false);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    sai_object_meta_key_t meta_key = { .objecttype = SAI_OBJECT_TYPE_NEIGHBOR_ENTRY, .objectkey = { .key = { .neighbor_entry = *neighbor_entry  } } };

    status = meta_generic_validation_remove(meta_key);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    if (remove == NULL)
    {
        SWSS_LOG_ERROR("remove function pointer is NULL");

        return SAI_STATUS_FAILURE;
    }

    status = remove(neighbor_entry);

    if (status == SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_DEBUG("remove status: %d", status);
    }
    else
    {
        SWSS_LOG_ERROR("remove status: %d", status);
    }

    if (status == SAI_STATUS_SUCCESS)
    {
        meta_generic_validation_post_remove(meta_key);
    }

    return status;
}

sai_status_t meta_sai_set_neighbor_entry(
        _In_ const sai_neighbor_entry_t* neighbor_entry,
        _In_ const sai_attribute_t *attr,
        _In_ sai_set_neighbor_entry_attribute_fn set)
{
    SWSS_LOG_ENTER();

    sai_status_t status = meta_sai_validate_neighbor_entry(neighbor_entry, false);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    sai_object_meta_key_t meta_key = { .objecttype = SAI_OBJECT_TYPE_NEIGHBOR_ENTRY, .objectkey = { .key = { .neighbor_entry = *neighbor_entry  } } };

    status = meta_generic_validation_set(meta_key, attr);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    if (set == NULL)
    {
        SWSS_LOG_ERROR("set function pointer is NULL");

        return SAI_STATUS_FAILURE;
    }

    status = set(neighbor_entry, attr);

    if (status == SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_DEBUG("set status: %d", status);
    }
    else
    {
        SWSS_LOG_ERROR("set status: %d", status);
    }

    if (status == SAI_STATUS_SUCCESS)
    {
        meta_generic_validation_post_set(meta_key, attr);
    }

    return status;
}

sai_status_t meta_sai_get_neighbor_entry(
        _In_ const sai_neighbor_entry_t* neighbor_entry,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list,
        _In_ sai_get_neighbor_entry_attribute_fn get)
{
    SWSS_LOG_ENTER();

    sai_status_t status = meta_sai_validate_neighbor_entry(neighbor_entry, false);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    sai_object_meta_key_t meta_key = { .objecttype = SAI_OBJECT_TYPE_NEIGHBOR_ENTRY, .objectkey = { .key = { .neighbor_entry = *neighbor_entry } } };

    status = meta_generic_validation_get(meta_key, attr_count, attr_list);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    if (get == NULL)
    {
        SWSS_LOG_ERROR("get function pointer is NULL");

        return SAI_STATUS_FAILURE;
    }

    status = get(neighbor_entry, attr_count, attr_list);

    if (status == SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_DEBUG("get status: %d", status);
    }
    else
    {
        SWSS_LOG_ERROR("get status: %d", status);
    }

    if (status == SAI_STATUS_SUCCESS)
    {
        meta_generic_validation_post_get(meta_key, attr_count, attr_list);
    }

    return status;
}

// ROUTE ENTRY

sai_status_t meta_sai_validate_route_entry(
        _In_ const sai_route_entry_t* route_entry,
        _In_ bool create)
{
    SWSS_LOG_ENTER();

    if (route_entry == NULL)
    {
        SWSS_LOG_ERROR("route_entry pointer is NULL");

        return SAI_STATUS_INVALID_PARAMETER;
    }

    auto family = route_entry->destination.addr_family;

    switch (family)
    {
        case SAI_IP_ADDR_FAMILY_IPV4:
            break;

        case SAI_IP_ADDR_FAMILY_IPV6:

            if (!is_ipv6_mask_valid(route_entry->destination.mask.ip6))
            {
                SWSS_LOG_ERROR("invalid ipv6 mask: %s", sai_serialize_ipv6(route_entry->destination.mask.ip6).c_str());

                return SAI_STATUS_INVALID_PARAMETER;
            }

            break;

        default:

            SWSS_LOG_ERROR("invalid prefix family: %d", family);

            return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_object_id_t vr = route_entry->vr_id;

    if (vr == SAI_NULL_OBJECT_ID)
    {
        SWSS_LOG_ERROR("virtual router is set to null object id");

        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_object_type_t object_type = sai_object_type_query(vr);

    if (object_type == SAI_NULL_OBJECT_ID)
    {
        SWSS_LOG_ERROR("virtual router oid 0x%lx is not valid object type, returned null object id", vr);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_object_type_t expected = SAI_OBJECT_TYPE_VIRTUAL_ROUTER;

    if (object_type != expected)
    {
        SWSS_LOG_ERROR("virtual router oid 0x%lx type %d is wrong type, expected object type %d", vr, object_type, expected);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    // check if virtual router exists

    sai_object_meta_key_t meta_key_vr = { .objecttype = expected, .objectkey = { .key = { .object_id = vr } } };

    std::string key_vr = sai_serialize_object_meta_key(meta_key_vr);

    if (!object_exists(key_vr))
    {
        SWSS_LOG_ERROR("object key %s don't exists", key_vr.c_str());

        return SAI_STATUS_INVALID_PARAMETER;
    }

    // check if route entry exists

    sai_object_meta_key_t meta_key_route = { .objecttype = SAI_OBJECT_TYPE_ROUTE_ENTRY, .objectkey = { .key = { .route_entry = *route_entry } } };

    std::string key_route = sai_serialize_object_meta_key(meta_key_route);

    if (create)
    {
        if (object_exists(key_route))
        {
            SWSS_LOG_ERROR("object key %s already exists", key_route.c_str());

            return SAI_STATUS_ITEM_ALREADY_EXISTS;
        }

        return SAI_STATUS_SUCCESS;
    }

    // set, get, remove

    if (!object_exists(key_route))
    {
        SWSS_LOG_ERROR("object key %s don't exists", key_route.c_str());

        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t meta_sai_create_route_entry(
        _In_ const sai_route_entry_t* route_entry,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list,
        _In_ sai_create_route_entry_fn create)
{
    SWSS_LOG_ENTER();

    sai_status_t status = meta_sai_validate_route_entry(route_entry, true);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    sai_object_meta_key_t meta_key = { .objecttype = SAI_OBJECT_TYPE_ROUTE_ENTRY, .objectkey = { .key = { .route_entry = *route_entry  } } };

    status = meta_generic_validation_create(meta_key, attr_count, attr_list);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    if (create == NULL)
    {
        SWSS_LOG_ERROR("create function pointer is NULL");

        return SAI_STATUS_FAILURE;
    }

    status = create(route_entry, attr_count, attr_list);

    if (status == SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_DEBUG("create status: %d", status);
    }
    else
    {
        SWSS_LOG_ERROR("create status: %d", status);
    }

    if (status == SAI_STATUS_SUCCESS)
    {
        meta_generic_validation_post_create(meta_key, attr_count, attr_list);
    }

    return status;
}

sai_status_t meta_sai_remove_route_entry(
        _In_ const sai_route_entry_t* route_entry,
        _In_ sai_remove_route_entry_fn remove)
{
    SWSS_LOG_ENTER();

    sai_status_t status = meta_sai_validate_route_entry(route_entry, false);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    sai_object_meta_key_t meta_key = { .objecttype = SAI_OBJECT_TYPE_ROUTE_ENTRY, .objectkey = { .key = { .route_entry = *route_entry  } } };

    status = meta_generic_validation_remove(meta_key);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    if (remove == NULL)
    {
        SWSS_LOG_ERROR("remove function pointer is NULL");

        return SAI_STATUS_FAILURE;
    }

    status = remove(route_entry);

    if (status == SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_DEBUG("remove status: %d", status);
    }
    else
    {
        SWSS_LOG_ERROR("remove status: %d", status);
    }

    if (status == SAI_STATUS_SUCCESS)
    {
        meta_generic_validation_post_remove(meta_key);
    }

    return status;
}

sai_status_t meta_sai_set_route_entry(
        _In_ const sai_route_entry_t* route_entry,
        _In_ const sai_attribute_t *attr,
        _In_ sai_set_route_entry_attribute_fn set)
{
    SWSS_LOG_ENTER();

    sai_status_t status = meta_sai_validate_route_entry(route_entry, false);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    sai_object_meta_key_t meta_key = { .objecttype = SAI_OBJECT_TYPE_ROUTE_ENTRY, .objectkey = { .key = { .route_entry = *route_entry  } } };

    status = meta_generic_validation_set(meta_key, attr);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    if (set == NULL)
    {
        SWSS_LOG_ERROR("set function pointer is NULL");

        return SAI_STATUS_FAILURE;
    }

    status = set(route_entry, attr);

    if (status == SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_DEBUG("set status: %d", status);
    }
    else
    {
        SWSS_LOG_ERROR("set status: %d", status);
    }

    if (status == SAI_STATUS_SUCCESS)
    {
        meta_generic_validation_post_set(meta_key, attr);
    }

    return status;
}

sai_status_t meta_sai_get_route_entry(
        _In_ const sai_route_entry_t* route_entry,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list,
        _In_ sai_get_route_entry_attribute_fn get)
{
    SWSS_LOG_ENTER();

    sai_status_t status = meta_sai_validate_route_entry(route_entry, false);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    sai_object_meta_key_t meta_key = { .objecttype = SAI_OBJECT_TYPE_ROUTE_ENTRY, .objectkey = { .key = { .route_entry = *route_entry } } };

    status = meta_generic_validation_get(meta_key, attr_count, attr_list);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    if (get == NULL)
    {
        SWSS_LOG_ERROR("get function pointer is NULL");

        return SAI_STATUS_FAILURE;
    }

    status = get(route_entry, attr_count, attr_list);

    if (status == SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_DEBUG("get status: %d", status);
    }
    else
    {
        SWSS_LOG_ERROR("get status: %d", status);
    }

    if (status == SAI_STATUS_SUCCESS)
    {
        meta_generic_validation_post_get(meta_key, attr_count, attr_list);
    }

    return status;
}

// GENERIC

sai_status_t meta_sai_validate_oid(
        _In_ sai_object_type_t object_type,
        _In_ sai_object_id_t* object_id,
        _In_ bool create)
{
    SWSS_LOG_ENTER();

    if (object_type <= SAI_OBJECT_TYPE_NULL ||
            object_type >= SAI_OBJECT_TYPE_MAX)
    {
        SWSS_LOG_ERROR("invalid object type specified: %d, FIXME", object_type);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    const char* otname =  sai_metadata_get_enum_value_name(&metadata_enum_sai_object_type_t, object_type);

    switch (object_type)
    {
        case SAI_OBJECT_TYPE_FDB_ENTRY:
        case SAI_OBJECT_TYPE_ROUTE_ENTRY:
        case SAI_OBJECT_TYPE_NEIGHBOR_ENTRY:

            SWSS_LOG_ERROR("invalid object type (%s) specified as generic, FIXME", otname);
            throw;

        default:

            // TODO check if is oid object type
            break;
    }

    SWSS_LOG_DEBUG("generic object type: %s", otname);

    if (object_id == NULL)
    {
        SWSS_LOG_ERROR("oid pointer is NULL");

        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (create)
    {
        return SAI_STATUS_SUCCESS;
    }

    sai_object_id_t oid = *object_id;

    if (oid == SAI_NULL_OBJECT_ID)
    {
        SWSS_LOG_ERROR("oid is set to null object id");

        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_object_type_t ot = sai_object_type_query(oid);

    if (ot == SAI_NULL_OBJECT_ID)
    {
        SWSS_LOG_ERROR("%s oid 0x%lx is not valid object type, returned null object id", otname, oid);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_object_type_t expected = object_type;

    if (ot != expected)
    {
        SWSS_LOG_ERROR("%s oid 0x%lx type %d is wrong type, expected object type %d", otname, oid, ot, expected);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    // check if object exists

    sai_object_meta_key_t meta_key_oid = { .objecttype = expected, .objectkey = { .key = { .object_id = oid } } };

    std::string key_oid = sai_serialize_object_meta_key(meta_key_oid);

    if (!object_exists(key_oid))
    {
        SWSS_LOG_ERROR("object key %s don't exists", key_oid.c_str());

        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t meta_sai_create_oid(
        _In_ sai_object_type_t object_type,
        _Out_ sai_object_id_t* object_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list,
        _In_ sai_create_generic_fn create)
{
    SWSS_LOG_ENTER();

    // TODO handle switch_id

    sai_status_t status = meta_sai_validate_oid(object_type, object_id, true);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    sai_object_meta_key_t meta_key = { .objecttype = object_type, .objectkey = { .key = { .object_id  = SAI_NULL_OBJECT_ID } } };

    status = meta_generic_validation_create(meta_key, attr_count, attr_list);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    if (create == NULL)
    {
        SWSS_LOG_ERROR("create function pointer is NULL");

        return SAI_STATUS_FAILURE;
    }

    status = create(object_type, object_id, switch_id, attr_count, attr_list);

    if (status == SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_DEBUG("create status: %d", status);
    }
    else
    {
        SWSS_LOG_ERROR("create status: %d", status);
    }

    if (status == SAI_STATUS_SUCCESS)
    {
        // new oid was created, save id and validate it

        meta_key.objectkey.key.object_id = *object_id;
        meta_generic_validation_post_create(meta_key, attr_count, attr_list);
    }

    return status;
}

sai_status_t meta_sai_remove_oid(
        _In_ sai_object_type_t object_type,
        _In_ sai_object_id_t object_id,
        _In_ sai_remove_generic_fn remove)
{
    SWSS_LOG_ENTER();

    sai_status_t status = meta_sai_validate_oid(object_type, &object_id, false);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    sai_object_meta_key_t meta_key = { .objecttype = object_type, .objectkey = { .key = { .object_id  = object_id } } };

    status = meta_generic_validation_remove(meta_key);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    if (remove == NULL)
    {
        SWSS_LOG_ERROR("remove function pointer is NULL");

        return SAI_STATUS_FAILURE;
    }

    status = remove(object_type, object_id);

    if (status == SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_DEBUG("remove status: %d", status);
    }
    else
    {
        SWSS_LOG_ERROR("remove status: %d", status);
    }

    if (status == SAI_STATUS_SUCCESS)
    {
        meta_generic_validation_post_remove(meta_key);
    }

    return status;
}

sai_status_t meta_sai_set_oid(
        _In_ sai_object_type_t object_type,
        _In_ sai_object_id_t object_id,
        _In_ const sai_attribute_t *attr,
        _In_ sai_set_generic_attribute_fn set)
{
    SWSS_LOG_ENTER();

    sai_status_t status = meta_sai_validate_oid(object_type, &object_id, false);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    sai_object_meta_key_t meta_key = { .objecttype = object_type, .objectkey = { .key = { .object_id  = object_id } } };

    status = meta_generic_validation_set(meta_key, attr);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    if (set == NULL)
    {
        SWSS_LOG_ERROR("set function pointer is NULL");

        return SAI_STATUS_FAILURE;
    }

    status = set(object_type, object_id, attr);

    if (status == SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_DEBUG("set status: %d", status);
    }
    else
    {
        SWSS_LOG_ERROR("set status: %d", status);
    }

    if (status == SAI_STATUS_SUCCESS)
    {
        meta_generic_validation_post_set(meta_key, attr);
    }

    return status;
}

sai_status_t meta_sai_get_oid(
        _In_ sai_object_type_t object_type,
        _In_ sai_object_id_t object_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list,
        _In_ sai_get_generic_attribute_fn get)
{
    SWSS_LOG_ENTER();

    sai_status_t status = meta_sai_validate_oid(object_type, &object_id, false);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    sai_object_meta_key_t meta_key = { .objecttype = object_type, .objectkey = { .key = { .object_id  = object_id } } };

    status = meta_generic_validation_get(meta_key, attr_count, attr_list);

    if (status != SAI_STATUS_SUCCESS)
    {
        return status;
    }

    if (get == NULL)
    {
        SWSS_LOG_ERROR("get function pointer is NULL");

        return SAI_STATUS_FAILURE;
    }

    status = get(object_type, object_id, attr_count, attr_list);

    if (status == SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_DEBUG("get status: %d", status);
    }
    else
    {
        SWSS_LOG_ERROR("get status: %d", status);
    }

    if (status == SAI_STATUS_SUCCESS)
    {
        meta_generic_validation_post_get(meta_key, attr_count, attr_list);
    }

    return status;
}

// NOTIFICATIONS

void meta_sai_on_fdb_event_single(
        _In_ const sai_fdb_event_notification_data_t& data)
{
    SWSS_LOG_ENTER();

    const sai_object_meta_key_t meta_key_fdb = { .objecttype = SAI_OBJECT_TYPE_FDB_ENTRY, .objectkey = { .key = { .fdb_entry = data.fdb_entry } } };

    std::string key_fdb = sai_serialize_object_meta_key(meta_key_fdb);

    switch (data.event_type)
    {
        case SAI_FDB_EVENT_LEARNED:

            if (object_exists(key_fdb))
            {
                SWSS_LOG_WARN("object key %s alearedy exists, but received LEARNED event", key_fdb.c_str());
                break;
            }

            {
                sai_attribute_t *list = data.attr;
                uint32_t count = data.attr_count;

                sai_attribute_t local[2]; // 2 for port id and type

                if (count == 1)
                {
                    // workaround for missing "TYPE" attribute on notification

                    local[0] = data.attr[0]; // copy 1st attr
                    local[1].id = SAI_FDB_ENTRY_ATTR_TYPE;
                    local[1].value.s32 = SAI_FDB_ENTRY_TYPE_DYNAMIC; // assume learned entries are always dynamic

                    list = local;
                    count = 2; // now we added type
                }

                sai_status_t status = meta_generic_validation_create(meta_key_fdb, count, list);

                if (status == SAI_STATUS_SUCCESS)
                {
                    meta_generic_validation_post_create(meta_key_fdb, count, list);
                }
                else
                {
                    SWSS_LOG_ERROR("failed to insert %s received in notification: %s", key_fdb.c_str(), sai_serialize_status(status).c_str());
                }
            }

            break;

        case SAI_FDB_EVENT_AGED:
        case SAI_FDB_EVENT_FLUSHED:

            if (!object_exists(key_fdb))
            {
                SWSS_LOG_WARN("object key %s don't exist but received AGED/FLUSHED event", key_fdb.c_str());
                break;
            }

            meta_generic_validation_post_remove(meta_key_fdb);

            break;

        default:

            SWSS_LOG_ERROR("got FDB_ENTRY notification with unknown event_type %d, bug?", data.event_type);
            break;
    }
}

void meta_sai_on_fdb_event(
        _In_ uint32_t count,
        _In_ sai_fdb_event_notification_data_t *data)
{
    SWSS_LOG_ENTER();

    for (uint32_t i = 0; i < count; ++i)
    {
        meta_sai_on_fdb_event_single(data[i]);
    }
}
