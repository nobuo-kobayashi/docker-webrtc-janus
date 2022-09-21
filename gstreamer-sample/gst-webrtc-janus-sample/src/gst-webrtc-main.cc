#include "gst-webrtc-main.h"

#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

WebRTCMain::WebRTCMain(std::string& pipeline) : mPipelineStr(pipeline)
{
  mClient = nullptr;
}

WebRTCMain::~WebRTCMain()
{
  deleteAllSessions();
  disconnectSignallingServer();
}

void WebRTCMain::connectSignallingServer(std::string& url, std::string& origin)
{
  std::vector<std::string> protocols = {
    "janus-protocol"
  };

  disconnectSignallingServer();

  mClient = new WebsocketClient();
  mClient->setListener(this);
  mClient->connectAsync(url, origin, protocols);
}

void WebRTCMain::disconnectSignallingServer()
{
  if (mClient) {
    delete mClient;
    mClient = nullptr;
  }
}

// WebsocketClientListener implements.

void WebRTCMain::onConnected(WebsocketClient *client)
{
  createSession();
}

void WebRTCMain::onDisconnected(WebsocketClient *client)
{
  
}

void WebRTCMain::onMessage(WebsocketClient *client, std::string& message)
{
  json j = json::parse(message);
  if (j.find("transaction") != j.end()) {
    if (j["transaction"].is_string()) {
      std::string transaction = j["transaction"].get<std::string>();
      auto janus = mSessions.find(transaction.c_str());
      if (janus != end(mSessions)) {
        janus->second->onReceivedMessage(message);
      } else {
        g_print("Not found transaction. transaction=%s\n", transaction.c_str());
      }
    }
  }
}

// WebRTCJanusListener implements.

void WebRTCMain::onSendMessage(WebRTCJanus *janus, std::string& message)
{
  if (mClient) {
    mClient->sendMessage(message);
  }
}

// private function implements.

void WebRTCMain::createSession()
{
  std::string transaction = "0123456789";
  std::string display = "gclue";
  gint videoRoomId = 1234;

  WebRTCJanus *janus = new WebRTCJanus(mPipelineStr, transaction, display, videoRoomId);

  mSessions.insert(std::make_pair(transaction, janus));

  janus->setListener(this);
  janus->createSession();
}

void WebRTCMain::deleteAllSessions()
{
  for (auto itr = mSessions.begin(); itr != mSessions.end(); ++itr) {
    delete itr->second;
  }
  mSessions.clear();
}