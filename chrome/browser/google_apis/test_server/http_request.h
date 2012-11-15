// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_GOOGLE_APIS_TEST_SERVER_HTTP_REQUEST_H_
#define CHROME_BROWSER_GOOGLE_APIS_TEST_SERVER_HTTP_REQUEST_H_

#include <map>
#include <string>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/string_piece.h"
#include "googleurl/src/gurl.h"

namespace drive {
namespace test_server {

// Methods of HTTP requests supported by the test HTTP server.
enum HttpMethod {
  METHOD_UNKNOWN,
  METHOD_GET,
  METHOD_HEAD,
  METHOD_POST,
  METHOD_PUT,
  METHOD_DELETE,
};

// Represents a HTTP request. Since it can be big, use scoped_ptr to pass it
// instead of copying.
struct HttpRequest {
  HttpRequest();
  ~HttpRequest();

  GURL url;
  HttpMethod method;
  std::map<std::string, std::string> headers;
  std::string content;
  bool has_content;

 private:
  DISALLOW_COPY_AND_ASSIGN(HttpRequest);
};

// Parses the input data and produces a valid HttpRequest object. If there is
// more than one request in one chunk, then only the first one will be parsed.
// The common use is as below:
// HttpRequestParser parser;
// (...)
// void OnDataChunkReceived(Socket* socket, const char* data, int size) {
//   parser.ProcessChunk(std::string(data, size));
//   if (parser.ParseRequest() == HttpRequestParser::ACCEPTED) {
//     scoped_ptr<HttpRequest> request = parser.GetRequest();
//     (... process the request ...)
//   }
class HttpRequestParser {
 public:
  // Parsing result.
  enum ParseResult {
    WAITING,  // A request is not completed yet, waiting for more data.
    ACCEPTED,  // A request has been parsed and it is ready to be processed.
  };

  // Parser state.
  enum State {
    STATE_HEADERS,  // Waiting for a request headers.
    STATE_CONTENT,  // Waiting for content data.
    STATE_ACCEPTED,  // Request has been parsed.
  };

  HttpRequestParser();
  ~HttpRequestParser();

  // Adds chunk of data into the internal buffer.
  void ProcessChunk(const base::StringPiece& data);

  // Parses the http request (including data - if provided).
  // If returns ACCEPTED, then it means that the whole request has been found
  // in the internal buffer (and parsed). After calling GetRequest(), it will be
  // ready to parse another request.
  ParseResult ParseRequest();

  // Retrieves parsed request. Can be only called, when the parser is in
  // STATE_ACCEPTED state. After calling it, the parser is ready to parse
  // another request.
  scoped_ptr<HttpRequest> GetRequest();

 private:
  scoped_ptr<HttpRequest> http_request_;
  std::string buffer_;
  size_t buffer_position_;  // Current position in the internal buffer.
  State state_;
  // Content length of the request currently being parsed.
  size_t declared_content_length_;

  HttpMethod GetMethodType(const std::string& token) const;

  // Parses headers and returns ACCEPTED if whole request was parsed. Otherwise
  // returns WAITING.
  ParseResult ParseHeaders();

  // Parses request's content data and returns ACCEPTED if all of it have been
  // processed. Chunked Transfer Encoding *is not* supported.
  ParseResult ParseContent();

  // Fetches the next line from the buffer. Result does not contain \r\n.
  // Returns an empty string for an empty line. It will assert if there is
  // no line available.
  std::string ShiftLine();
};

}  // namespace test_server
}  // namespace drive

#endif  // CHROME_BROWSER_GOOGLE_APIS_TEST_SERVER_HTTP_REQUEST_H_
