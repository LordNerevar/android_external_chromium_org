// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_SPDY_SPDY_STREAM_H_
#define NET_SPDY_SPDY_STREAM_H_
#pragma once

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/linked_ptr.h"
#include "base/ref_counted.h"
#include "base/scoped_ptr.h"
#include "net/base/bandwidth_metrics.h"
#include "net/base/io_buffer.h"
#include "net/base/net_log.h"
#include "net/spdy/spdy_framer.h"
#include "net/spdy/spdy_protocol.h"

namespace net {

class SpdySession;
class SSLCertRequestInfo;
class SSLInfo;

// The SpdyStream is used by the SpdySession to represent each stream known
// on the SpdySession.  This class provides interfaces for SpdySession to use.
// Streams can be created either by the client or by the server.  When they
// are initiated by the client, both the SpdySession and client object (such as
// a SpdyNetworkTransaction) will maintain a reference to the stream.  When
// initiated by the server, only the SpdySession will maintain any reference,
// until such a time as a client object requests a stream for the path.
class SpdyStream : public base::RefCounted<SpdyStream> {
 public:
  // Delegate handles protocol specific behavior of spdy stream.
  class Delegate {
   public:
    Delegate() {}

    // Called when SYN frame has been sent.
    // Returns true if no more data to be sent after SYN frame.
    virtual bool OnSendHeadersComplete(int status) = 0;

    // Called when stream is ready to send data.
    // Returns network error code. OK when it successfully sent data.
    virtual int OnSendBody() = 0;

    // Called when data has been sent. |status| indicates network error
    // or number of bytes has been sent.
    // Returns true if no more data to be sent.
    virtual bool OnSendBodyComplete(int status) = 0;

    // Called when SYN_STREAM or SYN_REPLY received. |status| indicates network
    // error. Returns network error code.
    virtual int OnResponseReceived(const spdy::SpdyHeaderBlock& response,
                                   base::Time response_time,
                                   int status) = 0;

    // Called when data is received.
    virtual void OnDataReceived(const char* data, int length) = 0;

    // Called when data is sent.
    virtual void OnDataSent(int length) = 0;

    // Called when SpdyStream is closed.
    virtual void OnClose(int status) = 0;

   protected:
    friend class base::RefCounted<Delegate>;
    virtual ~Delegate() {}

   private:
    DISALLOW_COPY_AND_ASSIGN(Delegate);
  };

  // SpdyStream constructor
  SpdyStream(SpdySession* session, spdy::SpdyStreamId stream_id, bool pushed);

  // Set new |delegate|. |delegate| must not be NULL.
  // If it already received SYN_REPLY or data, OnResponseReceived() or
  // OnDataReceived() will be called.
  void SetDelegate(Delegate* delegate);
  Delegate* GetDelegate() { return delegate_; }

  // Detach delegate from the stream. It will cancel the stream if it was not
  // cancelled yet.  It is safe to call multiple times.
  void DetachDelegate();

  // Is this stream a pushed stream from the server.
  bool pushed() const { return pushed_; }

  spdy::SpdyStreamId stream_id() const { return stream_id_; }
  void set_stream_id(spdy::SpdyStreamId stream_id) { stream_id_ = stream_id; }

  bool response_received() const { return response_received_; }
  void set_response_received() { response_received_ = true; }

  // For pushed streams, we track a path to identify them.
  const std::string& path() const { return path_; }
  void set_path(const std::string& path) { path_ = path; }

  int priority() const { return priority_; }
  void set_priority(int priority) { priority_ = priority; }

  int send_window_size() const { return send_window_size_; }
  void set_send_window_size(int window_size) {
    send_window_size_ = window_size;
  }

  void set_stalled_by_flow_control(bool stalled) {
    stalled_by_flow_control_ = stalled;
  }

  // Increases |send_window_size_| with delta extracted from a WINDOW_UPDATE
  // frame; sends a RST_STREAM if delta overflows |send_window_size_| and
  // removes the stream from the session.
  void IncreaseSendWindowSize(int delta_window_size);

  // Decreases |send_window_size_| by the given number of bytes.
  void DecreaseSendWindowSize(int delta_window_size);

  const BoundNetLog& net_log() const { return net_log_; }
  void set_net_log(const BoundNetLog& log) { net_log_ = log; }

  const linked_ptr<spdy::SpdyHeaderBlock>& spdy_headers() const;
  void set_spdy_headers(const linked_ptr<spdy::SpdyHeaderBlock>& headers);
  base::Time GetRequestTime() const;
  void SetRequestTime(base::Time t);

  // Called by the SpdySession when a response (e.g. a SYN_STREAM or SYN_REPLY)
  // has been received for this stream. Returns a status code.
  int OnResponseReceived(const spdy::SpdyHeaderBlock& response);

  // Called by the SpdySession when response data has been received for this
  // stream.  This callback may be called multiple times as data arrives
  // from the network, and will never be called prior to OnResponseReceived.
  // |buffer| contains the data received.  The stream must copy any data
  //          from this buffer before returning from this callback.
  // |length| is the number of bytes received or an error.
  //         A zero-length count does not indicate end-of-stream.
  void OnDataReceived(const char* buffer, int bytes);

  // Called by the SpdySession when a write has completed.  This callback
  // will be called multiple times for each write which completes.  Writes
  // include the SYN_STREAM write and also DATA frame writes.
  // |result| is the number of bytes written or a net error code.
  void OnWriteComplete(int bytes);

  // Called by the SpdySession when the request is finished.  This callback
  // will always be called at the end of the request and signals to the
  // stream that the stream has no more network events.  No further callbacks
  // to the stream will be made after this call.
  // |status| is an error code or OK.
  void OnClose(int status);

  void Cancel();
  bool cancelled() const { return cancelled_; }
  bool closed() const { return io_state_ == STATE_DONE; }

  // Interface for Spdy[Http|WebSocket]Stream to use.

  // Sends the request.
  // For non push stream, it will send SYN_STREAM frame.
  int SendRequest(bool has_upload_data);

  // Sends DATA frame.
  int WriteStreamData(IOBuffer* data, int length,
                      spdy::SpdyDataFlags flags);

  // Fills SSL info in |ssl_info| and returns true when SSL is in use.
  bool GetSSLInfo(SSLInfo* ssl_info, bool* was_npn_negotiated);

  // Fills SSL Certificate Request info |cert_request_info| and returns
  // true when SSL is in use.
  bool GetSSLCertRequestInfo(SSLCertRequestInfo* cert_request_info);

  bool is_idle() const {
    return io_state_ == STATE_OPEN || io_state_ == STATE_DONE;
  }

  int response_status() const { return response_status_; }

 private:
  enum State {
    STATE_NONE,
    STATE_SEND_HEADERS,
    STATE_SEND_HEADERS_COMPLETE,
    STATE_SEND_BODY,
    STATE_SEND_BODY_COMPLETE,
    STATE_WAITING_FOR_RESPONSE,
    STATE_OPEN,
    STATE_DONE
  };

  friend class base::RefCounted<SpdyStream>;
  virtual ~SpdyStream();

  // Try to make progress sending/receiving the request/response.
  int DoLoop(int result);

  // The implementations of each state of the state machine.
  int DoSendHeaders();
  int DoSendHeadersComplete(int result);
  int DoSendBody();
  int DoSendBodyComplete(int result);
  int DoReadHeaders();
  int DoReadHeadersComplete(int result);
  int DoOpen(int result);

  // Update the histograms.  Can safely be called repeatedly, but should only
  // be called after the stream has completed.
  void UpdateHistograms();

  // When a server pushed stream is first created, this function is posted on
  // the MessageLoop to replay all the data that the server has already sent.
  void PushedStreamReplayData();

  // There is a small period of time between when a server pushed stream is
  // first created, and the pushed data is replayed. Any data received during
  // this time should continue to be buffered.
  bool continue_buffering_data_;

  spdy::SpdyStreamId stream_id_;
  std::string path_;
  int priority_;

  // Flow control variables.
  bool stalled_by_flow_control_;
  int send_window_size_;

  const bool pushed_;
  ScopedBandwidthMetrics metrics_;
  bool response_received_;

  scoped_refptr<SpdySession> session_;

  // The transaction should own the delegate.
  SpdyStream::Delegate* delegate_;

  // The request to send.
  linked_ptr<spdy::SpdyHeaderBlock> request_;

  // The time at which the request was made that resulted in this response.
  // For cached responses, this time could be "far" in the past.
  base::Time request_time_;

  linked_ptr<spdy::SpdyHeaderBlock> response_;
  base::Time response_time_;

  State io_state_;

  // Since we buffer the response, we also buffer the response status.
  // Not valid until the stream is closed.
  int response_status_;

  bool cancelled_;
  bool has_upload_data_;

  BoundNetLog net_log_;

  base::TimeTicks send_time_;
  base::TimeTicks recv_first_byte_time_;
  base::TimeTicks recv_last_byte_time_;
  int send_bytes_;
  int recv_bytes_;
  bool histograms_recorded_;
  // Data received before delegate is attached.
  std::vector<scoped_refptr<IOBufferWithSize> > pending_buffers_;

  DISALLOW_COPY_AND_ASSIGN(SpdyStream);
};

}  // namespace net

#endif  // NET_SPDY_SPDY_STREAM_H_
