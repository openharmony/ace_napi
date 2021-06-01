/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "net_server.h"

#include "utils/log.h"

#include <cstdlib>

using namespace std;

NetServer::NetServer(napi_env env, napi_value thisVar) : EventTarget(env, thisVar)
{
    loop_ = nullptr;
    napi_get_uv_event_loop(env, &loop_);
    tcpServer_ = { 0 };
    tcpServer_.data = this;
    serverClosed_ = 0;
    clients_ = nullptr;
}

NetServer::~NetServer() {}

int NetServer::Start(int port)
{
    struct sockaddr_in addr;
    int result = 0;

    uv_ip4_addr("0.0.0.0", port, &addr);

    result = uv_tcp_init(loop_, &tcpServer_);
    if (result) {
        this->Emit("error", nullptr);
        return -1;
    }

    result = uv_tcp_bind(&tcpServer_, (const struct sockaddr*)&addr, 0);
    if (result) {
        this->Emit("error", nullptr);
        return -1;
    }

    result = uv_listen((uv_stream_t*)&tcpServer_, SOMAXCONN, OnConnection);
    if (result) {
        this->Emit("error", nullptr);
        return -1;
    }

    Emit("started", nullptr);

    return 0;
}

void NetServer::Stop()
{
    Emit("closed", nullptr);
    uint32_t thisRefCount = 0;
    napi_reference_unref(env_, thisVarRef_, &thisRefCount);
}

void NetServer::OnClose(uv_handle_t* peer)
{
    if (peer == nullptr) {
        HILOG_ERROR("peer is null");
        return;
    }

    NetServer* that = (NetServer*)peer->data;
    that->Emit("disconnect", nullptr);
    free(peer);
}

void NetServer::OnConnection(uv_stream_t* server, int status)
{
    if (server == nullptr) {
        HILOG_ERROR("server is null");
        return;
    }

    NetServer* that = (NetServer*)server->data;

    if (status != 0) {
        that->Emit("error", nullptr);
    }

    if (that->clients_ == nullptr) {
        that->clients_ = new NetClient();
    } else {
        auto tmp = new NetClient();
        tmp->next = that->clients_;
        that->clients_ = tmp;
    }

    uv_tcp_init(that->loop_, (uv_tcp_t*)&that->clients_->tcp);
    that->clients_->tcp.data = server->data;
    uv_accept(server, (uv_stream_t*)&that->clients_->tcp);
    uv_read_start((uv_stream_t*)&that->clients_->tcp, EchoAlloc, AfterRead);

    that->Emit("connect", nullptr);
}

void NetServer::OnServerClose(uv_handle_t* handle)
{
    if (handle == nullptr) {
        HILOG_ERROR("handle is null");
        return;
    }

    NetServer* that = (NetServer*)handle->data;

    for (NetClient* i = that->clients_; i != nullptr; i = i->next) {
        uv_close((uv_handle_t*)&i->tcp, nullptr);
    }

    uint32_t thisRefCount = 0;
    napi_reference_unref(that->env_, that->thisVarRef_, &thisRefCount);
}

void NetServer::AfterWrite(uv_write_t* req, int status)
{
    if (req == nullptr) {
        HILOG_ERROR("req is null");
        return;
    }

    NetServer* that = (NetServer*)req->data;

    WriteReq* wr = (WriteReq*)req;

    free(wr->buf.base);
    free(wr);

    if (status == 0) {
        that->Emit("write", nullptr);
        return;
    }

    that->Emit("error", nullptr);
}

void NetServer::AfterRead(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf)
{
    if (handle == nullptr) {
        HILOG_ERROR("handle is null");
        return;
    }

    if (buf == nullptr) {
        HILOG_ERROR("buf is null");
        return;
    }

    NetServer* that = (NetServer*)handle->data;
    WriteReq* wr = nullptr;
    uv_shutdown_t* sreq = nullptr;

    if (nread < 0) {
        free(buf->base);
        sreq = (uv_shutdown_t*)malloc(sizeof(*sreq));
        if (sreq == nullptr) {
            HILOG_ERROR("sreq is null");
            return;
        }
        sreq->data = that;
        uv_shutdown(sreq, handle, AfterShutdown);
        return;
    }

    if (nread == 0) {
        free(buf->base);
        return;
    }

    if (!that->serverClosed_) {
        for (int i = 0; i < nread; i++) {
            if (buf->base[i] == 'Q') {
                if (i + 1 < nread && buf->base[i + 1] == 'S') {
                    free(buf->base);
                    uv_close((uv_handle_t*)handle, OnClose);
                    return;
                } else {
                    uv_close((uv_handle_t*)&that->tcpServer_, OnServerClose);
                    that->serverClosed_ = 1;
                    return;
                }
            }
        }
    }

    that->Emit("read", nullptr);

    wr = (WriteReq*)malloc(sizeof(WriteReq));
    if (wr == nullptr) {
        HILOG_ERROR("wr is null");
        free(buf->base);
        return;
    }

    wr->buf = uv_buf_init(buf->base, nread);

    wr->req.data = that;

    if (uv_write(&wr->req, handle, &wr->buf, 1, AfterWrite) != 0) {
        that->Emit("error", nullptr);
    }
}

void NetServer::AfterShutdown(uv_shutdown_t* req, int status)
{
    if (req == nullptr) {
        HILOG_ERROR("req is null");
        return;
    }

    uv_close((uv_handle_t*)req->handle, OnClose);
    free(req);
}

void NetServer::EchoAlloc(uv_handle_t* handle, size_t suggestedSize, uv_buf_t* buf)
{
    if (handle == nullptr) {
        HILOG_ERROR("handle is null");
        return;
    }

    if (buf == nullptr) {
        HILOG_ERROR("buf is null");
        return;
    }

    if (suggestedSize == 0) {
        HILOG_ERROR("suggestedSize = 0");
        return;
    }

    buf->base = (char*)malloc(suggestedSize);
    if (buf->base != nullptr) {
        HILOG_ERROR("buf->base is null");
        return;
    }

    buf->len = suggestedSize;
}