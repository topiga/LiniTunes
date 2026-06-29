#pragma once
#include <plist/plist.h>
#include <QString>
#include <QDateTime>
#include <cstdlib>

namespace plist_helpers {

inline QString stringVal(plist_t dict, const char *key)
{
    if (!dict || plist_get_node_type(dict) != PLIST_DICT)
        return {};
    plist_t node = plist_dict_get_item(dict, key);
    if (!node || plist_get_node_type(node) != PLIST_STRING)
        return {};
    char *val = nullptr;
    plist_get_string_val(node, &val);
    const QString result = val ? QString::fromUtf8(val) : QString();
    free(val);
    return result;
}

inline QString uintStr(plist_t dict, const char *key)
{
    if (!dict)
        return {};
    plist_t node = plist_dict_get_item(dict, key);
    if (!node)
        return {};
    uint64_t val = 0;
    plist_get_uint_val(node, &val);
    return QString::number(val);
}

inline bool boolVal(plist_t node)
{
    if (!node || plist_get_node_type(node) != PLIST_BOOLEAN)
        return false;
    uint8_t val = 0;
    plist_get_bool_val(node, &val);
    return val != 0;
}

inline void setString(plist_t dict, const char *key, const QString &value)
{
    if (!value.isEmpty())
        plist_dict_set_item(dict, key, plist_new_string(value.toUtf8().constData()));
}

inline void setCurrentDate(plist_t dict, const char *key)
{
    plist_dict_set_item(dict, key, plist_new_unix_date(QDateTime::currentSecsSinceEpoch()));
}

} // namespace plist_helpers
