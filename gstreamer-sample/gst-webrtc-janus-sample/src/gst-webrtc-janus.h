#pragma once

#include "gst-webrtc-pipeline.h"
#include "gst-webrtc-timer.h"
#include "gst-websocket-client.h"

#include <string>
#include <map>

class WebRTCJanus;

class WebRTCJanusListener {
public:
  virtual void onSendMessage(WebRTCJanus *janus, std::string& message) {}
};

class WebRTCJanus : public WebRTCPipelineListener, WebRTCTimerListener {
private:
  typedef enum _State {
    NONE,
    CREATE,
    ATTACH,
    JOIN_PUBLISHER,
    CONNECTED
  } State;

  WebRTCJanusListener *mListener;
  WebRTCPipeline *mPipeline;
  WebRTCTimer *mTimer;
  std::string mPipelineStr;
  std::string mRoomName;
  std::string mTransaction;
  std::string mDisplay;
  State mState;
  glong mSessionId;
  glong mHandleId;
  gint mVideoRoomId;

  void startTimer();
  void stopTimer();

  void startPipeline();
  void stopPipeline();

  bool checkJanusResult(std::string& message);

  void attachVideoRoom();
  void joinPublisher();

  void parseSession(std::string& message);
  void parseAttachVideoRoom(std::string& message);
  void parseJoinPublisher(std::string& message);
  void parseAnswer(std::string& message);

  void sendMessage(std::string& message);

public:
  WebRTCJanus(std::string& pipeline, std::string& transaction, std::string& display, gint videoRoomId);
  virtual ~WebRTCJanus();

  inline void setListener(WebRTCJanusListener *listener) {
    mListener = listener;
  }

  void createSession();
  void onReceivedMessage(std::string& message);

  // WebRTCPipelineListener implements.
  virtual void onSendSdp(WebRTCPipeline *pipeline, gint type, gchar *sdp_string);
  virtual void onSendIceCandidate(WebRTCPipeline *pipeline, guint mlineindex, gchar *candidate);
  virtual void onAddStream(WebRTCPipeline *pipeline, GstPad *pad);
  virtual void onDataChannelConnected(WebRTCPipeline *pipeline);
  virtual void onDataChannelDisconnected(WebRTCPipeline *pipeline);
  virtual void onDataChannel(WebRTCPipeline *pipeline, std::string& message);

  // WebRTCTimerListener implements.
  virtual void onFire(WebRTCTimer *timer);
};
