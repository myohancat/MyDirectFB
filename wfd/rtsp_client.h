#ifndef __RTSP_CLIENT_H_
#define __RTSP_CLIENT_H_

#include "task.h"

#include <list>
#include <string>

class IRtspClientListener
{
public:
    virtual ~IRtspClientListener() { }
    
    virtual void onRtspConnected()       = 0;
    virtual void onRtspFailed(int error) = 0;
    virtual void onRtspClosed()          = 0;

    virtual void onRtspSessionStarted()  = 0;
    virtual void onRtspSessionEnded()    = 0;

    virtual void onPlay() = 0;
};

class RtspClient : public Task
{
public:
    RtspClient();
    ~RtspClient();

    bool start(const char* ip, int port);
    void stop();

    void addListener(IRtspClientListener* listener);
    void removeListener(IRtspClientListener* listener);

private:
    bool mExitTask;
    int  mExitPipe[2];

    int  mSock;

    std::string mIP;
    int         mPort;

    int         mNextCSeq;

    std::string mPresentationUrl;

    std::string mSession;
    int         mSessionTimeout;
    
    typedef std::list<IRtspClientListener*> RtspClientListenerList;
    RtspClientListenerList mRtspClientListeners;
    
private:
    void run();

    int  appendCommonResponse(std::string& response, int cseq);

    int  sendM2();
    int  sendSetup();
    int  sendPlay();

    void onRtspResponse(char** lines, int lineCnt);
    void onOptionsRequest(char** lines, int lineCnt);
    void onGetParameterRequest(char** lines, int lineCnt);
    void onSetParameterRequest(char** lines, int lineCnt);

    void onRecvMessage(char* data, int len);
    int  sendMessage(const std::string& msg);
    
    const std::string& getPresentationUrl();

    void notifyConnected();
    void notifyFailed(int error);
    void notifyClosed();

    void notifySessionStarted();
    void notifySessionEnded();

    std::string makeWfdVideoFormat();
    std::string makeWfdAudioFormat();
    std::string makeWfdDisplayEdid();
    std::string makeWfdContentProtection();
};

#endif /* __RTSP_CLIENT_H_ */
