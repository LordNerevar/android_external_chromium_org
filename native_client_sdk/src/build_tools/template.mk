# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# GNU Makefile based on shared rules provided by the Native Client SDK.
# See README.Makefiles for more details.
[[]]
[[def ExpandDict(key, value_in, pre_list=[], post_list=[]):]]
[[  value = value_in or [] ]]
[[  pre = pre_list or [] ]]
[[  post = post_list or [] ]]
[[  if type(value) is not dict:]]
[[    out = pre]]
[[    out.extend(value)]]
[[    out.extend(post)]]
[[    if out:]]
{{key}} = {{' '.join(out)}}
[[    ]]
[[    return]]
[[  ]]
[[  for subkey in value:]]
[[    out = pre]]
[[    out.extend(value[subkey])]]
[[    out.extend(post)]]
{{key}}_{{subkey}} = {{' '.join(out)}}
[[  ]]
{{key}} = $({{key}}_$(TOOLCHAIN))
[[]]

VALID_TOOLCHAINS := {{' '.join(tools)}}
{{pre}}
NACL_SDK_ROOT ?= $(abspath $(CURDIR)/{{rel_sdk}})
include $(NACL_SDK_ROOT)/tools/common.mk

TARGET = {{targets[0]['NAME']}}
[[ExpandDict('DEPS', targets[0].get('DEPS', []))]]
[[ExpandDict('LIBS', targets[0].get('LIBS', []), pre_list=['$(DEPS)'])]]

[[for target in targets:]]
[[  source_list = (s for s in sorted(target['SOURCES']) if not s.endswith('.h'))]]
[[  source_list = ' \\\n  '.join(source_list)]]
[[  sources = target['NAME'] + '_SOURCES']]
[[  cflags = target['NAME'] + '_CFLAGS']]
[[  flags = target.get('CCFLAGS', [])]]
[[  flags.extend(target.get('CXXFLAGS', []))]]
[[  if len(targets) == 1:]]
[[    sources = 'SOURCES']]
[[    cflags = 'CFLAGS']]
[[  ]]
[[  ExpandDict(cflags, flags)]]
{{sources}} = {{source_list}}
[[]]

# Build rules generated by macros from common.mk:

[[if targets[0].get('DEPS'):]]
$(foreach dep,$(DEPS),$(eval $(call DEPEND_RULE,$(dep))))
[[if len(targets) > 1:]]
[[  for target in targets:]]
[[    name = target['NAME'] ]]
$(foreach src,$({{name}}_SOURCES),$(eval $(call COMPILE_RULE,$(src),$({{name}}_CFLAGS))))
[[else:]]
$(foreach src,$(SOURCES),$(eval $(call COMPILE_RULE,$(src),$(CFLAGS))))
[[]]

[[for target in targets:]]
[[  sources = target['NAME'] + '_SOURCES']]
[[  name = target['NAME'] ]]
[[  if len(targets) == 1:]]
[[    sources = 'SOURCES']]
[[    name = '$(TARGET)']]
[[  if target['TYPE'] == 'so':]]
$(eval $(call SO_RULE,{{name}},$({{sources}})))
[[  elif target['TYPE'] == 'so-standalone':]]
$(eval $(call SO_RULE,{{name}},$({{sources}}),,,1))
[[  else:]]
ifeq ($(CONFIG),Release)
$(eval $(call LINK_RULE,{{name}}_unstripped,$({{sources}}),$(LIBS),$(DEPS)))
$(eval $(call STRIP_RULE,{{name}},{{name}}_unstripped))
else
$(eval $(call LINK_RULE,{{name}},$({{sources}}),$(LIBS),$(DEPS)))
endif
[[]]

$(eval $(call NMF_RULE,$(TARGET),)){{post}}
