#include <stdlib.h>
#include <string.h>
#include <dslink/utils.h>
#include "broker/stream.h"

BrokerListStream *broker_stream_list_init() {
    BrokerListStream *stream = calloc(1, sizeof(BrokerListStream));
    if (!stream) {
        return NULL;
    }

    stream->type = LIST_STREAM;
    if (dslink_map_init(&stream->clients, dslink_map_uint32_cmp,
                        dslink_map_uint32_key_len_cal) != 0) {
        free(stream);
        return NULL;
    }

    stream->updates_cache = json_object();
    if (!stream->updates_cache) {
        free(stream);
        return NULL;
    }

    return stream;
}

BrokerInvokeStream *broker_stream_invoke_init() {
    BrokerInvokeStream *stream = calloc(1, sizeof(BrokerInvokeStream));
    if (!stream) {
        return NULL;
    }

    stream->type = INVOCATION_STREAM;
    return stream;
}

void broker_stream_free(BrokerStream *stream) {
    if (!stream) {
        return;
    }

    if (stream->type == LIST_STREAM) {

        BrokerListStream *s = (BrokerListStream *) stream;
        free(s->remotePath);
        json_decref(s->updates_cache);
    }
    free(stream);
}

static inline
void add_to_update(json_t *updates, const char *key, json_t *value) {
    json_t *update = json_array();

    // name
    json_array_append_new(update, json_string(key));
    //value
    json_array_append(update, value);

    json_array_append_new(updates, update);
}

json_t *broker_stream_list_get_cache(BrokerListStream *stream) {
    // TODO: check allocation

    size_t cacheSize = json_object_size(stream->updates_cache);
    if (cacheSize == 0) {
        return NULL;
    }

    json_t *updates = json_array();
    const char *key;
    json_t *value;


    json_t *valueBase = json_object_get(stream->updates_cache, "$base");
    if (valueBase != NULL) {
        add_to_update(updates, "$base", valueBase);
    }

    json_t *valueIs = json_object_get(stream->updates_cache, "$is");
    if (valueIs != NULL) {
        add_to_update(updates, "$is", valueIs);
    }

    json_object_foreach(stream->updates_cache, key, value) {
        if (value != valueIs && value != valueBase) {
            // $base and $is should be added before everything
            add_to_update(updates, key, value);
        }
    }
    return updates;
}

void broker_stream_list_reset_remote_cache(BrokerListStream *stream, RemoteDSLink *link) {
    json_object_clear(stream->updates_cache);
    if (link) {
        json_object_set_new_nocheck(stream->updates_cache,
                                    "$base", json_string_nocheck(link->path));
        if (strcmp(stream->remotePath, "/") == 0
            && link->linkData) {
            // add linkData into the updates_cache
            json_object_set_nocheck(stream->updates_cache,
                                    "$linkData", link->linkData);
        }
    } else {
        char ts[32];
        dslink_create_ts(ts, 32);
        json_object_set_new_nocheck(stream->updates_cache,
                                    "$disconnectedTs", json_string_nocheck(ts));
    }
}