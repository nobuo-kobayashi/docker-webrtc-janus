
#include "gst-webrtc-timer.h"

WebRTCTimer::WebRTCTimer()
{
  mTimeoutId = 0;
  mListener = nullptr;
}

WebRTCTimer::~WebRTCTimer()
{
  stop();
}

bool WebRTCTimer::isRunning() {
  return mTimeoutId != 0;
}

void WebRTCTimer::start(guint interval)
{
  if (mTimeoutId) {
    return;
  }

  mTimeoutId = g_timeout_add(interval, onTimeoutCallback, this);
}

void WebRTCTimer::stop()
{
  if (mTimeoutId) {
    g_source_remove(mTimeoutId);
    mTimeoutId = 0;
  }
}

gboolean WebRTCTimer::onTimeoutCallback(gpointer userData)
{
  WebRTCTimer *timer = (WebRTCTimer *) userData;
  if (timer) {
    if (timer->mListener) {
      timer->mListener->onFire(timer);
    }
  }
  // FALSE を返却するとタイムアウトが停止する。
  return timer->mTimeoutId != 0;
}