// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/jingle_glue/log_to_server.h"

#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "remoting/jingle_glue/mock_objects.h"
#include "remoting/jingle_glue/server_log_entry_unittest.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using buzz::XmlElement;
using buzz::QName;
using testing::_;
using testing::DeleteArg;
using testing::InSequence;
using testing::Return;

namespace remoting {

namespace {

const char kTestBotJid[] = "remotingunittest@bot.talk.google.com";
const char kClientJid[] = "host@domain.com/1234";

MATCHER_P2(IsLogEntry, key, value, "") {
  XmlElement* entry = GetSingleLogEntryFromStanza(arg);
  if (!entry) {
    return false;
  }

  return entry->Attr(QName(std::string(), key)) == value;
}

}  // namespace

class LogToServerTest : public testing::Test {
 public:
  LogToServerTest() {}
  virtual void SetUp() OVERRIDE {
    EXPECT_CALL(signal_strategy_, AddListener(_));
    EXPECT_CALL(signal_strategy_, RemoveListener(_));
    log_to_server_.reset(
        new LogToServer(ServerLogEntry::ME2ME, &signal_strategy_, kTestBotJid));
  }

 protected:
  base::MessageLoop message_loop_;
  base::RunLoop run_loop_;
  MockSignalStrategy signal_strategy_;
  scoped_ptr<LogToServer> log_to_server_;
};

TEST_F(LogToServerTest, LogWhenConnected) {
  {
    InSequence s;
    EXPECT_CALL(signal_strategy_, GetLocalJid())
        .WillRepeatedly(Return(kClientJid));
    EXPECT_CALL(signal_strategy_, AddListener(_));
    EXPECT_CALL(signal_strategy_, GetNextId());
    EXPECT_CALL(signal_strategy_, SendStanzaPtr(IsLogEntry("a", "1")))
        .WillOnce(DoAll(DeleteArg<0>(), Return(true)));
    EXPECT_CALL(signal_strategy_, GetNextId());
    EXPECT_CALL(signal_strategy_, SendStanzaPtr(IsLogEntry("b", "2")))
        .WillOnce(DoAll(DeleteArg<0>(), Return(true)));
    EXPECT_CALL(signal_strategy_, RemoveListener(_))
        .RetiresOnSaturation();
  }

  ServerLogEntry entry1;
  ServerLogEntry entry2;
  entry1.Set("a", "1");
  entry2.Set("b", "2");
  log_to_server_->Log(entry1);
  log_to_server_->OnSignalStrategyStateChange(SignalStrategy::CONNECTED);
  log_to_server_->Log(entry2);
  run_loop_.RunUntilIdle();
}

TEST_F(LogToServerTest, DontLogWhenDisconnected) {
  EXPECT_CALL(signal_strategy_, GetLocalJid())
      .WillRepeatedly(Return(kClientJid));
  EXPECT_CALL(signal_strategy_, SendStanzaPtr(_)).Times(0);

  ServerLogEntry entry;
  entry.Set("foo", "bar");
  log_to_server_->Log(entry);
  run_loop_.RunUntilIdle();
}

}  // namespace remoting
