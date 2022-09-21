#pragma once

#include <glib.h>

class WebRTCTimer;

class WebRTCTimerListener {
public:
  virtual void onFire(WebRTCTimer *timer) {}
};

class WebRTCTimer {
private:
  guint mTimeoutId;
  WebRTCTimerListener *mListener;

  static gboolean onTimeoutCallback(gpointer userData);

public:
  WebRTCTimer();
  virtual ~WebRTCTimer();

  inline void setListener(WebRTCTimerListener *listener) {
    mListener = listener;
  }

  bool isRunning();
  void start(guint interval);
  void stop();
};
