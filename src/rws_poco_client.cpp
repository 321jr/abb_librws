/***********************************************************************************************************************
 *
 * Copyright (c) 2015, ABB Schweiz AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that
 * the following conditions are met:
 *
 *    * Redistributions of source code must retain the
 *      above copyright notice, this list of conditions
 *      and the following disclaimer.
 *    * Redistributions in binary form must reproduce the
 *      above copyright notice, this list of conditions
 *      and the following disclaimer in the documentation
 *      and/or other materials provided with the
 *      distribution.
 *    * Neither the name of ABB nor the names of its
 *      contributors may be used to endorse or promote
 *      products derived from this software without
 *      specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***********************************************************************************************************************
 */

#include <sstream>

#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/NetException.h"
#include "Poco/StreamCopier.h"

#include "abb_librws/rws_poco_client.h"

using namespace Poco;
using namespace Poco::Net;

namespace abb
{
namespace rws
{
/***********************************************************************************************************************
 * Struct definitions: POCOClient::POCOResult
 */

/************************************************************
 * Primary methods
 */

void POCOClient::POCOResult::addHTTPRequestInfo(const Poco::Net::HTTPRequest& request,
                                                const std::string request_content)
{
  poco_info.http.request.method = request.getMethod();
  poco_info.http.request.uri = request.getURI();
  poco_info.http.request.content = request_content;
}

void POCOClient::POCOResult::addHTTPResponseInfo(const Poco::Net::HTTPResponse& response,
                                                 const std::string response_content)
{
  std::string header_info;

  for (HTTPResponse::ConstIterator i = response.begin(); i != response.end(); ++i)
  {
    header_info += i->first + "=" + i->second + "\n";
  }

  poco_info.http.response.status = response.getStatus();
  poco_info.http.response.header_info = header_info;
  poco_info.http.response.content = response_content;
}

void POCOClient::POCOResult::addWebSocketFrameInfo(const int flags,
                                                   const std::string frame_content)
{
  poco_info.websocket.flags = flags;
  poco_info.websocket.frame_content = frame_content;
}

/************************************************************
 * Auxiliary methods
 */

std::string POCOClient::POCOResult::mapGeneralStatus() const
{
  std::string result;

  switch (status)
  {
    case POCOResult::UNKNOWN:
      result = "UNKNOWN";
    break;

    case POCOResult::OK:
      result = "OK";
    break;

    case POCOResult::WEBSOCKET_NOT_ALLOCATED:
      result = "WEBSOCKET_NOT_ALLOCATED";
    break;

    case POCOResult::EXCEPTION_POCO_TIMEOUT:
      result = "EXCEPTION_POCO_TIMEOUT";
    break;

    case POCOResult::EXCEPTION_POCO_NET:
      result = "EXCEPTION_POCO_NET";
    break;

    case POCOResult::EXCEPTION_POCO_WEBSOCKET:
      result = "EXCEPTION_POCO_WEBSOCKET";
    break;

    default:
      result = "UNDEFINED";
    break;
  }

  return result;
}

std::string POCOClient::POCOResult::mapWebSocketOpcode() const
{
  std::string result;

  switch (poco_info.websocket.flags & WebSocket::FRAME_OP_BITMASK)
  {
    case WebSocket::FRAME_OP_CONT:
      result = "FRAME_OP_CONT";
    break;

    case WebSocket::FRAME_OP_TEXT:
      result = "FRAME_OP_TEXT";
    break;

    case WebSocket::FRAME_OP_BINARY:
      result = "FRAME_OP_BINARY";
    break;

    case WebSocket::FRAME_OP_CLOSE:
      result = "FRAME_OP_CLOSE";
    break;

    case WebSocket::FRAME_OP_PING:
      result = "FRAME_OP_PING";
    break;

    case WebSocket::FRAME_OP_PONG:
      result = "FRAME_OP_PONG";
    break;
    
    default:
      result = "FRAME_OP_UNDEFINED";
    break;
  }

  return result;
}

std::string POCOClient::POCOResult::toString(const bool verbose, const size_t indent) const
{
  std::stringstream ss;
  
  std::string seperator = (indent == 0 ? " | " : "\n" + std::string(indent, ' '));

  ss << "General status: " << mapGeneralStatus();

  if (!poco_info.http.request.method.empty())
  {
    ss << seperator << "HTTP Request: " << poco_info.http.request.method << " " << poco_info.http.request.uri;

    if (status == OK)
    {
      ss << seperator << "HTTP Response: " << poco_info.http.response.status << " - "
         << HTTPResponse::getReasonForStatus(poco_info.http.response.status);

      if (verbose)
      {
        ss << seperator << "HTTP Response Content: " << poco_info.http.response.content;
      }
    }
  }
  else if (status == OK)
  {
    ss << seperator << "WebSocket frame: " << mapWebSocketOpcode();
  }

  return ss.str();
}




/***********************************************************************************************************************
 * Class definitions: POCOClient
 */

/************************************************************
 * Primary methods
 */

POCOClient::POCOResult POCOClient::httpGet(const std::string uri)
{
  return makeHTTPRequest(HTTPRequest::HTTP_GET, uri);
}

POCOClient::POCOResult POCOClient::httpPost(const std::string uri, const std::string content)
{
  return makeHTTPRequest(HTTPRequest::HTTP_POST, uri, content);
}

POCOClient::POCOResult POCOClient::httpPut(const std::string uri, const std::string content)
{
  return makeHTTPRequest(HTTPRequest::HTTP_PUT, uri, content);
}

POCOClient::POCOResult POCOClient::httpDelete(const std::string uri)
{
  return makeHTTPRequest(HTTPRequest::HTTP_DELETE, uri);
}

POCOClient::POCOResult POCOClient::makeHTTPRequest(const std::string method,
                                                   const std::string uri,
                                                   const std::string content)
{
  // Lock the object's mutex. It is released when the method goes out of scope.
  ScopedLock<Mutex> lock(http_mutex_);

  // Result of the communication.
  POCOResult result;

  // The response and the request.
  HTTPResponse response;
  HTTPRequest request(method, uri, HTTPRequest::HTTP_1_1);
  request.setCookies(cookies_);
  request.setContentLength(content.length());
  if (method == HTTPRequest::HTTP_POST || !content.empty())
  {
    request.setContentType("application/x-www-form-urlencoded");
  }

  // Attempt the communication.
  try
  {
    sendAndReceive(result, request, response, content);

    // Check if the request was approved, else add credentials.
    if (response.getStatus() == HTTPResponse::HTTP_UNAUTHORIZED)
    {
      authenticate(result, request, response, content);
    }

    result.status = POCOResult::OK;
  }
  catch (TimeoutException& e)
  {
    result.status = POCOResult::EXCEPTION_POCO_TIMEOUT;
    result.exception_message = e.displayText();
  }
  catch (NetException& e)
  {
    result.status = POCOResult::EXCEPTION_POCO_NET;
    result.exception_message = e.displayText();
  }

  if (result.status != POCOResult::OK)
  {
    cookies_.clear();
    client_session_.reset();
  }

  return result;
}

POCOClient::POCOResult POCOClient::webSocketConnect(const std::string uri, const std::string protocol)
{
  // Lock the object's mutex. It is released when the method goes out of scope.
  ScopedLock<Mutex> lock(http_mutex_);

  // Result of the communication.
  POCOResult result;

  // The response and the request.
  HTTPResponse response;
  HTTPRequest request(HTTPRequest::HTTP_GET, uri, HTTPRequest::HTTP_1_1);
  request.set("Sec-WebSocket-Protocol", protocol);
  request.setCookies(cookies_);

  // Attempt the communication.
  try
  {
    result.addHTTPRequestInfo(request);
    p_websocket_ = new WebSocket(client_session_, request, response);
    p_websocket_->setReceiveTimeout(Poco::Timespan(60000000));
      
    result.addHTTPResponseInfo(response);
    result.status = POCOResult::OK;
  }
  catch (TimeoutException& e)
  {
    result.status = POCOResult::EXCEPTION_POCO_TIMEOUT;
    result.exception_message = e.displayText();
  }
  catch (WebSocketException& e)
  {
    result.status = POCOResult::EXCEPTION_POCO_WEBSOCKET;
    result.exception_message = e.displayText();
  }
  catch (NetException& e)
  {
    result.status = POCOResult::EXCEPTION_POCO_NET;
    result.exception_message = e.displayText();
  }

  if (result.status != POCOResult::OK)
  {
    client_session_.reset();
  }

  return result;
}

POCOClient::POCOResult POCOClient::webSocketRecieveFrame()
{
  // Lock the object's mutex. It is released when the method goes out of scope.
  ScopedLock<Mutex> lock(websocket_mutex_);

  // Result of the communication.
  POCOResult result;

  // Attempt the communication.
  try
  {
    if (!p_websocket_.isNull())
    {
      int flags = 0;
      std::string content;

      // Wait for (non-ping) WebSocket frames.
      do
      {
        flags = 0;
        int number_of_bytes_received = p_websocket_->receiveFrame(websocket_buffer_, sizeof(websocket_buffer_), flags);
        content = std::string(websocket_buffer_, number_of_bytes_received);

        // Check for ping frame.
        if ((flags & WebSocket::FRAME_OP_BITMASK) == WebSocket::FRAME_OP_PING)
        {
          // Reply with a pong frame.
          p_websocket_->sendFrame(websocket_buffer_,
                                  number_of_bytes_received,
                                  WebSocket::FRAME_FLAG_FIN | WebSocket::FRAME_OP_PONG);
        }
      } while ((flags & WebSocket::FRAME_OP_BITMASK) == WebSocket::FRAME_OP_PING);

      // Check for closing frame.
      if ((flags & WebSocket::FRAME_OP_BITMASK) == WebSocket::FRAME_OP_CLOSE)
      {
        // Do not pass content of a closing frame to end user,
        // according to "The WebSocket Protocol" RFC6455.
        content.clear();

        // Shutdown the WebSocket.
        p_websocket_->shutdown();
        p_websocket_ = 0;
      }

      result.addWebSocketFrameInfo(flags, content);
      result.status = POCOResult::OK;
    }
    else
    {
      result.status = POCOResult::WEBSOCKET_NOT_ALLOCATED;
    }
  }
  catch (TimeoutException& e)
  {
    result.status = POCOResult::EXCEPTION_POCO_TIMEOUT;
    result.exception_message = e.displayText();
  }
  catch (WebSocketException& e)
  {
    result.status = POCOResult::EXCEPTION_POCO_WEBSOCKET;
    result.exception_message = e.displayText();
  }
  catch (NetException& e)
  {
    result.status = POCOResult::EXCEPTION_POCO_NET;
    result.exception_message = e.displayText();
  }

  if (result.status != POCOResult::OK)
  {
    client_session_.reset();
  }

  return result;
}

/************************************************************
 * Auxiliary methods
 */

void POCOClient::sendAndReceive(POCOResult& result,
                                HTTPRequest& request,
                                HTTPResponse& response,
                                const std::string request_content)
{
  // Add request info to the result.
  result.addHTTPRequestInfo(request, request_content);

  // Contact the server.
  std::string response_content;
  client_session_.sendRequest(request) << request_content;
  StreamCopier::copyToString(client_session_.receiveResponse(response), response_content);

  // Add response info to the result.
  result.addHTTPResponseInfo(response, response_content);
}

void POCOClient::authenticate(POCOResult& result,
                              HTTPRequest& request,
                              HTTPResponse& response,
                              const std::string request_content)
{
  // Remove any old cookies and add credentials.
  cookies_.clear();
  digest_credentials_.authenticate(request, response);

  // Contact the server, and extract and store the received cookies.
  sendAndReceive(result, request, response, request_content);
  std::vector<HTTPCookie> temp_cookies;
  response.getCookies(temp_cookies);

  for (size_t i = 0; i < temp_cookies.size(); ++i)
  {
    extractAndStoreCookie(temp_cookies[i].toString());
  }
}

void POCOClient::extractAndStoreCookie(const std::string cookie_string)
{
  // Find the positions of the cookie delimiters.
  size_t position_1 = cookie_string.find_first_of("=");
  size_t position_2 = cookie_string.find_first_of(";");

  // Extract and store the cookie (if the delimiters was found).
  if (position_1 != std::string::npos && position_2 != std::string::npos)
  {
    std::string result = cookie_string.substr(0, position_1++);
    std::string result2 = cookie_string.substr(position_1, position_2 - position_1);

    cookies_.add(result, result2);
  }
}

std::string POCOClient::findSubstringContent(const std::string whole_string,
                                             const std::string substring_start,
                                             const std::string substring_end)
{
  std::string result;
  size_t start_postion = whole_string.find(substring_start);

  if (start_postion != std::string::npos)
  {
    start_postion += substring_start.size();
    size_t end_postion = whole_string.find_first_of(substring_end, start_postion);

    if (end_postion != std::string::npos)
    {
      result = whole_string.substr(start_postion, end_postion - start_postion);
    }
  }

  std::string quot = "&quot;";
  size_t quot_position = 0;
  do
  {
    quot_position = result.find(quot);
    if (quot_position != std::string::npos)
    {
      result.replace(quot_position, quot.size(), "");
    }
  } while (quot_position != std::string::npos);

  return result;
}

} // end namespace rws
} // end namespace abb