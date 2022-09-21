#include "gst-webrtc-janus.h"

#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

WebRTCJanus::WebRTCJanus(std::string& pipeline,std::string& transaction, std::string& display, gint videoRoomId) 
    : mPipelineStr(pipeline), mTransaction(transaction), mDisplay(display), mVideoRoomId(videoRoomId)
{
  mTimer = nullptr;
  mPipeline = nullptr;
  mSessionId = 0;
  mHandleId = 0;
  mVideoRoomId = 1234;
  mState = NONE;
}

WebRTCJanus::~WebRTCJanus()
{
  stopPipeline();
}

// WebRTCPipelineListener implements.

void WebRTCJanus::onSendSdp(WebRTCPipeline *pipeline, gint type, gchar *sdp_string)
{
  json j;
  j["janus"] = "message";
  j["session_id"] = mSessionId;
  j["handle_id"] = mHandleId;
  j["transaction"] = mTransaction.c_str();
  j["body"] = json{
    {"request", "publish"}
  };
  j["jsep"] = json{
    {"type", "offer"},
    {"sdp", sdp_string},
    {"trickle", true}
  };
  std::string msg = j.dump();
  sendMessage(msg);
}

void WebRTCJanus::onSendIceCandidate(WebRTCPipeline *pipeline, guint mlineindex, gchar *candidate)
{
  json j;
  j["janus"] = "trickle";
  j["session_id"] = mSessionId;
  j["handle_id"] = mHandleId;
  j["transaction"] = mTransaction.c_str();
  j["candidate"] = json{
    {"candidate", candidate},
    {"sdpMLineIndex", mlineindex},
  };
  std::string msg = j.dump();
  sendMessage(msg);
}

void WebRTCJanus::onAddStream(WebRTCPipeline *pipeline, GstPad *pad)
{
}

void WebRTCJanus::onDataChannelConnected(WebRTCPipeline *pipeline)
{
}

void WebRTCJanus::onDataChannelDisconnected(WebRTCPipeline *pipeline)
{
}

void WebRTCJanus::onDataChannel(WebRTCPipeline *pipeline, std::string& message)
{
}

// WebRTCTimerListener implements.

void WebRTCJanus::onFire(WebRTCTimer *timer)
{
  if (mPipeline) {
    json j;
    j["janus"] = "keepalive";
    j["session_id"] = mSessionId;
    j["handle_id"] = mHandleId;
    j["transaction"] = mTransaction.c_str();
    std::string msg = j.dump();
    sendMessage(msg);
  }
}

// public function implements.

void WebRTCJanus::onReceivedMessage(std::string& message)
{
  switch (mState) {
  case NONE:
    break;
  case CREATE:
    parseSession(message);
    break;
  case ATTACH:
    parseAttachVideoRoom(message);
    break;
  case JOIN_PUBLISHER:
    parseJoinPublisher(message);
    break;
  case CONNECTED:
    parseAnswer(message);
    break;
  }
}

// private functions.

void WebRTCJanus::sendMessage(std::string& message)
{
  if (mListener) {
    mListener->onSendMessage(this, message);
  }
}

void WebRTCJanus::startTimer()
{
  if (mTimer) {
    return;
  }

  mTimer = new WebRTCTimer();
  mTimer->setListener(this);
  mTimer->start(10000);
}

void WebRTCJanus::stopTimer()
{
  if (mTimer) {
    delete mTimer;
    mTimer = nullptr;
  }
}

void WebRTCJanus::startPipeline()
{
  stopPipeline();

  mPipeline = new WebRTCPipeline();
  mPipeline->setListener(this);
  mPipeline->startPipeline(mPipelineStr);

  startTimer();
}

void WebRTCJanus::stopPipeline()
{
  stopTimer();

  if (mPipeline) {
    delete mPipeline;
    mPipeline = nullptr;
  }
}

bool WebRTCJanus::checkJanusResult(std::string& message)
{
  json j = json::parse(message);
  if (j.find("janus") != j.end()) {
    if (j["janus"].is_string()) {
      auto result = j["janus"].get<std::string>();
      if (result.compare("success") == 0) {
        return true;
      }
    }
  }
  return false;
}

void WebRTCJanus::createSession()
{
  mState = CREATE;

  json j;
  j["janus"] = "create";
  j["transaction"] = mTransaction.c_str();
  std::string msg = j.dump();
  sendMessage(msg);
}

void WebRTCJanus::parseSession(std::string& message)
{
  if (!checkJanusResult(message)) {
    return;
  }

  json j = json::parse(message);
  if (j.find("data") != j.end()) {
    auto data = j["data"];
    if (data.is_object()) {
      if (data.find("id") != data.end()) {
        mSessionId = data["id"].get<glong>();
      }
    }
  }

  attachVideoRoom();
}

void WebRTCJanus::attachVideoRoom()
{
  mState = ATTACH;

  json j;
  j["janus"] = "attach";
  j["session_id"] = mSessionId;
  j["plugin"] = "janus.plugin.videoroom";
  j["transaction"] = mTransaction.c_str();
  std::string msg = j.dump();
  sendMessage(msg);
}


void WebRTCJanus::parseAttachVideoRoom(std::string& message)
{
  if (!checkJanusResult(message)) {
    return;
  }

  json j = json::parse(message);
  if (j.find("data") != j.end()) {
    auto data = j["data"];
    if (data.is_object()) {
      if (data.find("id") != data.end()) {
        mHandleId = data["id"].get<glong>();
      }
    }
  }

  joinPublisher();
}

void WebRTCJanus::joinPublisher()
{
  mState = JOIN_PUBLISHER;

  json j;
  j["janus"] = "message";
  j["session_id"] = mSessionId;
  j["handle_id"] = mHandleId;
  j["transaction"] = mTransaction.c_str();
  j["body"] = json{
    {"request", "join"}, 
    {"ptype", "publisher"}, 
    {"room", mVideoRoomId},
    {"display", mDisplay.c_str()},
  };
  std::string msg = j.dump();
  sendMessage(msg);
}

void WebRTCJanus::parseJoinPublisher(std::string& message)
{
  mState = CONNECTED;

  startPipeline();
}

void WebRTCJanus::parseAnswer(std::string& message)
{
  json j = json::parse(message);
  if (j.find("jsep") != j.end()) {
    auto data = j["jsep"];
    if (data.is_object()) {
      if (data.find("sdp") != data.end()) {
        std::string sdp = data["sdp"].get<std::string>();
        mPipeline->onAnswerReceived(sdp.c_str());
      }
    }
  }

  if (j.find("candidate") != j.end()) {
    auto candidate = j["candidate"];
    if (candidate.is_object()) {
      if (candidate.find("candidate") != candidate.end() &&
          candidate.find("sdpMLineIndex") != candidate.end()) {
        std::string candidateString = candidate["candidate"].get<std::string>();
        int sdpMLineIndex = candidate["sdpMLineIndex"].get<int>();
        mPipeline->onIceReceived(sdpMLineIndex, candidateString.c_str());
      }
    }
  }
}
