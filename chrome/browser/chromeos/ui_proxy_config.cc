// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/ui_proxy_config.h"

#include "base/logging.h"
#include "base/values.h"
#include "chrome/browser/chromeos/policy/proto/chrome_device_policy.pb.h"
#include "chrome/browser/prefs/proxy_config_dictionary.h"
#include "net/proxy/proxy_config.h"

namespace chromeos {

UIProxyConfig::UIProxyConfig()
    : mode(MODE_DIRECT),
      state(ProxyPrefs::CONFIG_UNSET),
      user_modifiable(true) {
}

UIProxyConfig::~UIProxyConfig() {
}

void UIProxyConfig::SetPacUrl(const GURL& pac_url) {
  mode = UIProxyConfig::MODE_PAC_SCRIPT;
  automatic_proxy.pac_url = pac_url;
}

void UIProxyConfig::SetSingleProxy(const net::ProxyServer& server) {
  mode = UIProxyConfig::MODE_SINGLE_PROXY;
  single_proxy.server = server;
}

void UIProxyConfig::SetProxyForScheme(const std::string& scheme,
                                      const net::ProxyServer& server) {
  ManualProxy* proxy = MapSchemeToProxy(scheme);
  if (!proxy) {
    NOTREACHED() << "Cannot set proxy: invalid scheme [" << scheme << "]";
    return;
  }
  mode = UIProxyConfig::MODE_PROXY_PER_SCHEME;
  proxy->server = server;
}

void UIProxyConfig::SetBypassRules(const net::ProxyBypassRules& rules) {
  if (mode != UIProxyConfig::MODE_SINGLE_PROXY &&
      mode != UIProxyConfig::MODE_PROXY_PER_SCHEME) {
    NOTREACHED() << "Cannot set bypass rules for proxy mode [" << mode << "]";
    return;
  }
  bypass_rules = rules;
}

bool UIProxyConfig::FromNetProxyConfig(const net::ProxyConfig& net_config) {
  *this = UIProxyConfig();  // Reset to default.
  const net::ProxyConfig::ProxyRules& rules = net_config.proxy_rules();
  switch (rules.type) {
    case net::ProxyConfig::ProxyRules::TYPE_NO_RULES:
      if (!net_config.HasAutomaticSettings()) {
        mode = UIProxyConfig::MODE_DIRECT;
      } else if (net_config.auto_detect()) {
        mode = UIProxyConfig::MODE_AUTO_DETECT;
      } else if (net_config.has_pac_url()) {
        mode = UIProxyConfig::MODE_PAC_SCRIPT;
        automatic_proxy.pac_url = net_config.pac_url();
      } else {
        return false;
      }
      return true;
    case net::ProxyConfig::ProxyRules::TYPE_SINGLE_PROXY:
      if (rules.single_proxies.IsEmpty())
        return false;
      mode = MODE_SINGLE_PROXY;
      single_proxy.server = rules.single_proxies.Get();
      bypass_rules = rules.bypass_rules;
      return true;
    case net::ProxyConfig::ProxyRules::TYPE_PROXY_PER_SCHEME:
      // Make sure we have valid server for at least one of the protocols.
      if (rules.proxies_for_http.IsEmpty() &&
          rules.proxies_for_https.IsEmpty() &&
          rules.proxies_for_ftp.IsEmpty() &&
          rules.fallback_proxies.IsEmpty()) {
        return false;
      }
      mode = MODE_PROXY_PER_SCHEME;
      if (!rules.proxies_for_http.IsEmpty())
        http_proxy.server = rules.proxies_for_http.Get();
      if (!rules.proxies_for_https.IsEmpty())
        https_proxy.server = rules.proxies_for_https.Get();
      if (!rules.proxies_for_ftp.IsEmpty())
        ftp_proxy.server = rules.proxies_for_ftp.Get();
      if (!rules.fallback_proxies.IsEmpty())
        socks_proxy.server = rules.fallback_proxies.Get();
      bypass_rules = rules.bypass_rules;
      return true;
    default:
      NOTREACHED() << "Unrecognized proxy config mode";
      break;
  }
  return false;
}

base::DictionaryValue* UIProxyConfig::ToPrefProxyConfig() const {
  switch (mode) {
    case MODE_DIRECT: {
      return ProxyConfigDictionary::CreateDirect();
    }
    case MODE_AUTO_DETECT: {
      return ProxyConfigDictionary::CreateAutoDetect();
    }
    case MODE_PAC_SCRIPT: {
      return ProxyConfigDictionary::CreatePacScript(
          automatic_proxy.pac_url.spec(), false);
    }
    case MODE_SINGLE_PROXY: {
      std::string spec;
      if (single_proxy.server.is_valid())
        spec = single_proxy.server.ToURI();
      return ProxyConfigDictionary::CreateFixedServers(
          spec, bypass_rules.ToString());
    }
    case MODE_PROXY_PER_SCHEME: {
      std::string spec;
      EncodeAndAppendProxyServer("http", http_proxy.server, &spec);
      EncodeAndAppendProxyServer("https", https_proxy.server, &spec);
      EncodeAndAppendProxyServer("ftp", ftp_proxy.server, &spec);
      EncodeAndAppendProxyServer("socks", socks_proxy.server, &spec);
      return ProxyConfigDictionary::CreateFixedServers(
          spec, bypass_rules.ToString());
    }
    default:
      break;
  }
  NOTREACHED() << "Unrecognized proxy config mode for preference";
  return NULL;
}

UIProxyConfig::ManualProxy* UIProxyConfig::MapSchemeToProxy(
    const std::string& scheme) {
  if (scheme == "http")
    return &http_proxy;
  if (scheme == "https")
    return &https_proxy;
  if (scheme == "ftp")
    return &ftp_proxy;
  if (scheme == "socks")
    return &socks_proxy;
  NOTREACHED() << "Invalid scheme: " << scheme;
  return NULL;
}

// static
void UIProxyConfig::EncodeAndAppendProxyServer(const std::string& url_scheme,
                                               const net::ProxyServer& server,
                                               std::string* spec) {
  if (!server.is_valid())
    return;

  if (!spec->empty())
    *spec += ';';

  if (!url_scheme.empty()) {
    *spec += url_scheme;
    *spec += "=";
  }
  *spec += server.ToURI();
}

}  // namespace chromeos
