//////////////////////////////////////////////////////////////////////////////////////////
// A cross platform socket APIs, support ios & android & wp8 & window store
// universal app version: 3.2
//////////////////////////////////////////////////////////////////////////////////////////
/*
The MIT License (MIT)

Copyright (c) 2012-2018 halx99

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. 
*/

#ifndef _ASYNC_SOCKET_IO_H_
#define _ASYNC_SOCKET_IO_H_
#include "deadline_timer.h"
#include "endian_portable.h"
#include "object_pool.h"
#include "select_interrupter.hpp"
#include "singleton.h"
#include "xxsocket.h"
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <ctime>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#define _USE_ARES_LIB 0
#define _USE_SHARED_PTR 1
#define _USE_OBJECT_POOL 1
#define _ENABLE_SEND_CB 0

#if !defined(_ARRAYSIZE)
#define _ARRAYSIZE(A) (sizeof(A) / sizeof((A)[0]))
#endif

namespace purelib {

namespace inet {
enum channel_type {
  CHANNEL_CLIENT = 1,
  CHANNEL_SERVER = 2,
  CHANNEL_TCP = 1 << 4,
  CHANNEL_UDP = 1 << 5,
  CHANNEL_TCP_CLIENT = CHANNEL_TCP | CHANNEL_CLIENT,
  CHANNEL_TCP_SERVER = CHANNEL_TCP | CHANNEL_SERVER,
  CHANNEL_UDP_CLIENT = CHANNEL_UDP | CHANNEL_CLIENT,
  CHANNEL_UDP_SERVER = CHANNEL_UDP | CHANNEL_SERVER,
};

enum class channel_state {
  INACTIVE,
  REQUEST_CONNECT,
  CONNECTING,
  CONNECTED,
};

enum class resolve_state {
  READY,
  DIRTY,
  INPRROGRESS,
  FAILED = -1,
};

enum error_number {
  ERR_OK,                         // NO ERROR
  ERR_CONNECT_FAILED = -201,      // connect failed
  ERR_CONNECT_TIMEOUT,            // connect timeout
  ERR_SEND_FAILED,                // send error, failed
  ERR_SEND_TIMEOUT,               // send timeout
  ERR_RECV_FAILED,                // recv failed
  ERR_NETWORK_UNREACHABLE,        // wifi or 2,3,4G not open
  ERR_CONNECTION_LOST,            // connection lost
  ERR_DPL_ILLEGAL_PDU,            // decode pdu error.
  ERR_RESOLVE_HOST_FAILED,        // resolve host failed.
  ERR_RESOLVE_HOST_TIMEOUT,       // resolve host ip timeout.
  ERR_RESOLVE_HOST_IPV6_REQUIRED, // resolve host ip failed, a valid ipv6 host
                                  // required.
  ERR_INVALID_PORT,               // invalid port.
};

enum {
  socket_event_read = 1,
  socket_event_write = 2,
  socket_event_except = 4,
};

typedef std::function<void()> vdcallback_t;

static const int socket_recv_buffer_size = 65536; // 64K

class a_pdu; // application layer protocol data unit.

#if _USE_SHARED_PTR
typedef std::shared_ptr<a_pdu> a_pdu_ptr;
#else
typedef a_pdu *a_pdu_ptr;
#endif

typedef std::function<void(error_number)> send_pdu_callback_t;
typedef std::function<void(std::vector<char>)> recv_pdu_callback_t;

class async_socket_io;
struct channel_endpoint {
  std::string address_;
  u_short port_;
};

struct channel_transport;

struct channel_context {
  channel_context(async_socket_io &service);
  std::shared_ptr<xxsocket> socket_;
  channel_state
      state_; // 0: INACTIVE, 1: REQUEST_CONNECT, 2: CONNECTING, 3: CONNECTED

  int type_ = 0;

  std::string address_;
  u_short port_;

  std::vector<ip::endpoint> endpoints_;
  resolve_state resolve_state_;

  int index_ = -1;

  // The deadline timer for resolve & connect
  deadline_timer deadline_timer_;

  int ready_events_ = 0;
  std::shared_ptr<channel_transport> transport_; // ONLY for client
  void reset();
};

struct channel_transport {
  channel_transport(channel_context *ctx) : ctx_(ctx) {}
  channel_context *ctx_;
  std::shared_ptr<xxsocket> socket_;
  char buffer_[socket_recv_buffer_size + 1]; // recv buffer
  int offset_ = 0;                           // recv buffer offset

  std::vector<char> receiving_pdu_;
  int receiving_pdu_elen_ = -1;
  int error_ = 0; // socket error(>= -1), application error(< -1)
  int ready_events_ = 0;
  std::recursive_mutex send_queue_mtx_;
  std::deque<a_pdu_ptr> send_queue_;

  int update_socket_error() {
    error_ = xxsocket::get_last_errno();
    return error_;
  }
};

class deadline_timer;
/*
Usage:
purelib::inet::channel_endpoint endpoints[] = {
    { "172.31.238.193", 8888 },
    { "www.baidu.com", 443 },
    //{ "www.tencent.com", 443 },
};
tcpcli->start_service(endpoints, _ARRAYSIZE(endpoints));

tcpcli->set_callbacks(decode_pdu_length,
    [](size_t, bool, int) {},
    [](size_t, int error, const char* errormsg) {printf("The connection is lost
%d, %s", error, errormsg); },
    [](std::vector<char>&&) {},
    [](const vdcallback_t& callback) {callback(); });
*/
class async_socket_io {
public:
  // End user pdu decode length func
  typedef bool (*decode_pdu_length_func)(char *data, size_t datalen, int &len);

  // connection callbacks
  typedef std::function<void(size_t channel_index, int error,
                             const char *errormsg)>
      connection_lost_callback_t;
  typedef std::function<void(size_t channel_index, bool succeed, int ec)>
      connect_response_callback_t;

public:
  async_socket_io();
  ~async_socket_io();

  // start async socket service
  void start_service(const channel_endpoint *channel_eps,
                     int channel_count = 1);

  void stop_service();

  void set_endpoint(size_t channel_index, const char *address, u_short port);

  void set_endpoint(size_t channel_index, const ip::endpoint &ep);

  size_t get_received_pdu_count(void) const;

  // must be call on main thread(such cocos2d-x opengl thread)
  void dispatch_received_pdu(int count = 512);

  // set callbacks, required API, must call by user
  /*
    threadsafe_call: for cocos2d-x should be:
    [](const vdcallback_t& callback) {
        cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread(callback);
    }
  */
  void set_callbacks(decode_pdu_length_func decode_length_func,
                     connect_response_callback_t on_connect_result,
                     connection_lost_callback_t on_connection_lost,
                     recv_pdu_callback_t on_pdu_recv,
                     std::function<void(const vdcallback_t &)> threadsafe_call);

  // set connect and send timeouts.
  void set_timeouts(long connect_timeout_secs, long send_timeout_secs);

  void set_auto_reconnect_timeout(
      long timeout_secs = -1 /*-1: disable auto connect */);

  // open a channel, default: TCP_CLIENT
  void open(size_t channel_index, int channel_type = CHANNEL_TCP_CLIENT);

  // close tcp_client
  void close(size_t channel_index = 0);

  // Whether the client-->server connection established.
  bool is_connected(size_t cahnnel_index = 0) const;

  void write(std::vector<char> &&data, size_t channel_index = 0
#if _ENABLE_SEND_CB
             ,
             send_pdu_callback_t callback = nullptr
#endif
  );

  void write(std::shared_ptr<channel_transport> transport,
             std::vector<char> &&data
#if _ENABLE_SEND_CB
             , send_pdu_callback_t callback = nullptr
#endif
  );

  // timer support
  void schedule_timer(deadline_timer *);
  void cancel_timer(deadline_timer *);

  void interrupt();

  // Async resolve handlers, It's only for internal use
  void start_async_resolve(channel_context *);
  void finish_async_resolve(channel_context *);

private:
  void open_internal(channel_context *);

  void perform_timeout_timers(); // ALL timer expired

  long long get_wait_duration(long long usec);

  int do_select(fd_set *fds_array, timeval &timeout);

  void do_nonblocking_connect(channel_context *);
  void do_nonblocking_connect_completion(fd_set *fds_array, channel_context *);

  void handle_connect_succeed(std::shared_ptr<channel_transport>);
  void handle_connect_failed(channel_context *, int error);

  void register_descriptor(const socket_native_type fd, int flags);
  void unregister_descriptor(const socket_native_type fd, int flags);

  // The major async event-loop
  void service(void);

  bool do_write(std::shared_ptr<channel_transport>);
  bool do_read(std::shared_ptr<channel_transport>);
  void do_unpack(std::shared_ptr<channel_transport>, int bytes_expected,
                 int bytes_transferred);

  void handle_packet(std::vector<char> packet);

  void handle_close(
      std::shared_ptr<channel_transport>); // TODO: add error_number parameter

  // new/delete client socket connection channel
  // please call this at initialization, don't new channel at runtime
  // dynmaically: because this API is not thread safe.
  channel_context *new_channel(const channel_endpoint &ep);

  // Clear all channels after service exit.
  void clear_channels(); // destroy all channels

  // int        set_errorno(channel_context* ctx, int error);

  // ensure event fd unregistered & closed.
  bool cleanup(std::shared_ptr<xxsocket> ctx);

  // Update resolve state for new endpoint set
  void update_resolve_state(channel_context *ctx);

  // @Optimize remember Application layer events to avoid call kernel API -->
  // ::select
  int flush_ready_events();
  void swap_ready_events(std::shared_ptr<channel_transport>);

  void handle_send_finished(a_pdu_ptr, error_number);

  // supporting P2P
  void do_nonblocking_accept(channel_context *);
  void do_nonblocking_accept_completion(fd_set *fds_array, channel_context *);

private:
  bool stopping_;
  bool thread_started_;
  std::thread worker_thread_;

  long long connect_timeout_;
  long long send_timeout_;
  long long auto_reconnect_timeout_;

  std::mutex recv_queue_mtx_;
  std::deque<std::vector<char>>
      recv_queue_; // the recev_queue_ for connections: local-->server,
                   // local-->peer, peer-->local

  std::vector<channel_context *> channels_;
  std::vector<std::shared_ptr<channel_transport>> transports_;
  // select interrupter
  select_interrupter interrupter_;

  // timer support
  std::vector<deadline_timer *> timer_queue_;
  std::recursive_mutex timer_queue_mtx_;

  // socket event set
  int maxfdp_;
  enum {
    read_op,  // for async read and write(trigger by interrupt)
    write_op, // for async connect
    except_op,
  };
  fd_set fds_array_[3];

  // Optimize record incomplete works
  int nfds_;

  // callbacks
  decode_pdu_length_func decode_pdu_length_;
  connect_response_callback_t on_connect_resposne_;
  connection_lost_callback_t on_connection_lost_;
  recv_pdu_callback_t on_recv_pdu_;
  std::function<void(const vdcallback_t &)> tsf_call_;

#if _USE_ARES_LIB
  // non blocking io dns resolve support
  void *ares_; //
  int ares_count_;
#endif
  int ipsv_state_; // local network state
};                 // async_socket_io
};                 // namespace inet
};                 /* namespace purelib */
#endif

#define myasio                                                                 \
  purelib::gc::singleton<purelib::inet::async_socket_io>::instance()
