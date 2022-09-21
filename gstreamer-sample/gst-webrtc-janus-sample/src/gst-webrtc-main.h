#pragma once

#include "gst-websocket-client.h"
#include "gst-webrtc-janus.h"

#include <string>
#include <map>

class WebRTCMain : public WebsocketClientListener, WebRTCJanusListener {
private:
  WebsocketClient *mClient;
  std::string mPipelineStr;
  std::map<std::string, WebRTCJanus*> mSessions;

  void createSession();
  void deleteAllSessions();

public:
  WebRTCMain(std::string& pipeline);
  virtual ~WebRTCMain();

  void connectSignallingServer(std::string& url, std::string& origin);
  void disconnectSignallingServer();

  // WebsocketClientListener implements.
  virtual void onConnected(WebsocketClient *client);
  virtual void onDisconnected(WebsocketClient *client);
  virtual void onMessage(WebsocketClient *client, std::string& message);

  // WebRTCJanusListener implements.
  virtual void onSendMessage(WebRTCJanus *janus, std::string& message);
};
