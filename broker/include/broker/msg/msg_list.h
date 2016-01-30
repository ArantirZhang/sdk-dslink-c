#ifndef BROKER_MSG_LIST_H
#define BROKER_MSG_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <jansson.h>
#include "broker/remote_dslink.h"
#include "broker/stream.h"
#include "broker/node.h"

int broker_msg_handle_list(RemoteDSLink *link, json_t *req);
void update_list_child(BrokerNode *node, BrokerListStream *stream, const char* name);

void send_list_updates(RemoteDSLink *reqLink,
                       BrokerListStream *stream,
                       uint32_t reqRid);


void broker_list_dslink(RemoteDSLink *reqLink,
                        DownstreamNode *node,
                        const char *path,
                        uint32_t reqRid);

void broker_list_dslink_response(RemoteDSLink *link,
                                 json_t *resp,
                                 BrokerListStream *stream);

#ifdef __cplusplus
}
#endif

#endif // BROKER_MSG_LIST_H
