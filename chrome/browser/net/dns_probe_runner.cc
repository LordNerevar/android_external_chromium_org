// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/net/dns_probe_runner.h"

#include "base/bind.h"
#include "content/public/browser/browser_thread.h"
#include "net/base/address_list.h"
#include "net/base/ip_endpoint.h"
#include "net/base/net_errors.h"
#include "net/base/net_log.h"
#include "net/base/net_util.h"
#include "net/base/network_change_notifier.h"
#include "net/dns/dns_client.h"
#include "net/dns/dns_protocol.h"
#include "net/dns/dns_response.h"
#include "net/dns/dns_transaction.h"

using base::TimeDelta;
using content::BrowserThread;
using net::AddressList;
using net::BoundNetLog;
using net::DnsClient;
using net::DnsResponse;
using net::DnsTransaction;
using net::IPAddressNumber;
using net::IPEndPoint;
using net::NetLog;
using net::NetworkChangeNotifier;
using net::ParseIPLiteralToNumber;

namespace chrome_browser_net {

const char* DnsProbeRunner::kKnownGoodHostname = "google.com";

namespace {

DnsProbeRunner::Result EvaluateResponse(
    int net_error,
    const DnsResponse* response) {
  switch (net_error) {
    case net::OK:
      break;

    // ERR_NAME_NOT_RESOLVED maps to NXDOMAIN, which means the server is working
    // but gave us a wrong answer.
    case net::ERR_NAME_NOT_RESOLVED:
      return DnsProbeRunner::INCORRECT;

    // These results mean we heard *something* from the DNS server, but it was
    // unsuccessful (SERVFAIL) or malformed.
    case net::ERR_DNS_MALFORMED_RESPONSE:
    case net::ERR_DNS_SERVER_REQUIRES_TCP:  // Shouldn't happen; DnsTransaction
                                            // will retry with TCP.
    case net::ERR_DNS_SERVER_FAILED:
    case net::ERR_DNS_SORT_ERROR:  // Can only happen if the server responds.
      return DnsProbeRunner::FAILING;

    // Any other error means we never reached the DNS server in the first place.
    case net::ERR_DNS_TIMED_OUT:
    default:
      // Something else happened, probably at a network level.
      return DnsProbeRunner::UNREACHABLE;
  }

  AddressList addr_list;
  TimeDelta ttl;
  DnsResponse::Result result = response->ParseToAddressList(&addr_list, &ttl);

  if (result != DnsResponse::DNS_PARSE_OK) {
    return DnsProbeRunner::FAILING;
  }

  if (addr_list.empty()) {
    return DnsProbeRunner::INCORRECT;
  }

  return DnsProbeRunner::CORRECT;
}

}  // namespace

DnsProbeRunner::DnsProbeRunner() : weak_factory_(this), result_(UNKNOWN) {}

DnsProbeRunner::~DnsProbeRunner() {}

void DnsProbeRunner::SetClient(scoped_ptr<net::DnsClient> client) {
  client_ = client.Pass();
}

void DnsProbeRunner::RunProbe(const base::Closure& callback) {
  DCHECK(!callback.is_null());
  DCHECK(client_.get());
  DCHECK(callback_.is_null());
  DCHECK(!transaction_.get());

  callback_ = callback;
  transaction_ = client_->GetTransactionFactory()->CreateTransaction(
      kKnownGoodHostname,
      net::dns_protocol::kTypeA,
      base::Bind(&DnsProbeRunner::OnTransactionComplete,
                 weak_factory_.GetWeakPtr()),
      BoundNetLog());

  int rv = transaction_->Start();
  if (rv != net::ERR_IO_PENDING)
    OnTransactionComplete(transaction_.get(), rv, NULL);
}

bool DnsProbeRunner::IsRunning() const {
  return !callback_.is_null();
}

void DnsProbeRunner::OnTransactionComplete(
    DnsTransaction* transaction,
    int net_error,
    const DnsResponse* response) {
  DCHECK(!callback_.is_null());
  DCHECK(transaction_.get());
  DCHECK_EQ(transaction_.get(), transaction);

  result_ = EvaluateResponse(net_error, response);
  transaction_.reset();

  BrowserThread::PostTask(
      BrowserThread::IO,
      FROM_HERE,
      base::Bind(&DnsProbeRunner::CallCallback,
                 weak_factory_.GetWeakPtr()));
}

void DnsProbeRunner::CallCallback() {
  DCHECK(!callback_.is_null());
  DCHECK(!transaction_.get());

  // Clear callback in case it starts a new probe immediately.
  const base::Closure callback = callback_;
  callback_.Reset();
  callback.Run();
}

}  // namespace chrome_browser_net
