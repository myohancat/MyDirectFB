#include "rtsp_client.h"

#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

#include <algorithm>

#include "netutil.h"
#include "util.h"
#include "log.h"

#define USER_AGENT "myocat client"
#define DEFAULT_CONNECT_TIMEOUT (10*1000) /* 10 Sec */
#define DEFAULT_RECV_TIMEOUT    (-1)      /* INFINTE */
#define DEFAULT_SEND_TIMEOUT    (3*1000)  /* 3 Sec */

RtspClient::RtspClient()
           : mExitTask(false),
             mExitPipe { -1, -1 },
             mSock(-1),
             mIP(""),
             mPort(0),
             mNextCSeq(1),
             mPresentationUrl(""),
             mSession(""),
             mSessionTimeout(-1)
{
}

RtspClient::~RtspClient()
{
    stop();
}

bool RtspClient::start(const char* ip, int port)
{
    int ret = pipe(mExitPipe);
    if(ret < 0)
    {
        LOG_ERROR("pipe failed (%d). %s\n", errno, strerror(errno));
        return false;
    }

    mIP   = ip;
    mPort = port;
    mExitTask = false;

    mNextCSeq = 1;
    mPresentationUrl = "";
    mSession = "";

    return Task::start();
}

void RtspClient::stop()
{
    mExitTask = false;
    write(mExitPipe[1], "T", 1);

    Task::stop();

    if (mExitPipe[0] >= 0)
        close(mExitPipe[0]);
    if (mExitPipe[1] >= 0)
        close(mExitPipe[1]);

    mExitPipe[0] = -1;
    mExitPipe[1] = -1;
}

int RtspClient::appendCommonResponse(std::string& response, int cseq)
{
    char strDate[128];
    time_t now = time(NULL);
    struct tm *curtime = gmtime(&now);
    strftime(strDate, sizeof(strDate), "%a, %d %b %Y %H:%M:%S %z", curtime);

    response.append("Date: ");
    response.append(strDate);
    response.append("\r\n");
    response.append("User-Agent: " USER_AGENT "\r\n");
    response.append("CSeq: " + std::to_string(cseq) + "\r\n");

    return 0;
}

void RtspClient::onOptionsRequest(char** lines, int lineCnt)
{
    std::string response = "RTSP/1.0 200 OK\r\n";
    int cseq = -1;

    for (int ii =1; ii < lineCnt; ii++)
    {
        char* line = lines[ii];
        char* p = NULL;

        if ((p = strstr(line, "CSeq:")))
        {
            p += 5;
            cseq = strtol(ltrim(p), NULL, 10);
        }
    }
    
    appendCommonResponse(response, cseq);
    response.append("Public: org.wfa.wfd1.0, GET_PARAMETER, SET_PARAMETER\r\n");
    response.append("\r\n");

    LOG_DEBUG("[RESPONSE]\n%s", response.c_str());
    if (sendMessage(response) == 0)
    {
        sendM2();
    }
    else
    {
        LOG_ERROR("Failed to send message\n");
    }
}

void RtspClient::onGetParameterRequest(char** lines, int lineCnt)
{
    std::string response = "RTSP/1.0 200 OK\r\n";
    std::string msg = "";
    char option[1024];
    int cseq = -1;

    for (int ii =1; ii < lineCnt; ii++)
    {
        char* line = lines[ii];
        char* p = NULL;

        if ((p = strstr(line, "CSeq:")))
        {
            p += 5;
            cseq = strtol(ltrim(p), NULL, 10);
        }
        else if((p = strstr(line, "wfd_video_formats")))
        {
            sprintf(option, "%s: %s\r\n", p, makeWfdVideoFormat().c_str());
            msg.append(option);
        }
        else if((p = strstr(line, "wfd_audio_codecs")))
        {
            sprintf(option, "%s: %s\r\n", p, makeWfdAudioFormat().c_str());
            msg.append(option);
        }
        else if((p = strstr(line, "wfd_client_rtp_ports")))
        {
            sprintf(option, "%s: %s\r\n", p, "RTP/AVP/UDP;unicast 19000 0 mode=play");
            msg.append(option);
        }
        else if((p = strstr(line, "wfd_3d_video_formats")))
        {
            sprintf(option, "%s: %s\r\n", p, "none");
            msg.append(option);
        }
        else if((p = strstr(line, "wfd_connector_type")))
        {
            /*
                0 : Video Graphics Array (VGA) Connector
                1 : S-Video connector
                2 : Composite video connector
                3 : Component video connector
                4 : Digital Video Interface (DVI) connector
                5 : High-Definition Multimedia Interface (HDMI) connector
                6 : Reserved
                7 : Wi-Fi Display
                8 : Japanese D connector. (A connector conforming to the EIAJ RC-5237 standard)
                9 : Serial Digital Image (SDI) connector
                10 : A Display Port connector (DP)
                11 : Reserved
                12 : A Unified Display Interface (UDI)
                13-254 : Reserved
                255 : Indicates a type of physical connector that is not listed from value 0 to 254
             */
            sprintf(option, "%s: %s\r\n", p, "07");
            msg.append(option);
        }
        else if((p = strstr(line, "wfd_display_edid")))
        {
            sprintf(option, "%s: %s\r\n", p, makeWfdDisplayEdid().c_str());
            msg.append(option);
        }
        else if((p = strstr(line, "wfd_standby_resume_capability")))
        {
            sprintf(option, "%s: %s\r\n", p, "supported");
            msg.append(option);
        }
        else if((p = strstr(line, "wfd_coupled_sink")))
        {
            sprintf(option, "%s: %s\r\n", p, "none");
            msg.append(option);
        }
        else if((p = strstr(line, "wfd_idr_request_capability")))
        {
            sprintf(option, "%s: %s\r\n", p, "1");
            msg.append(option);
        }
        else if((p = strstr(line, "wfd_content_protection")))
        {
            sprintf(option, "%s: %s\r\n", p, makeWfdContentProtection().c_str());
            msg.append(option);
        }
    }
    
    appendCommonResponse(response, cseq);
    if (msg.length() > 0)
    {
        response.append("Content-Type: text/parameters\r\n");
        response.append("Content-Length: " + std::to_string(msg.length()) + "\r\n\r\n");
        response.append(msg);
    }
    else
        response.append("\r\n");

    LOG_DEBUG("[RESPONSE]\n%s", response.c_str());
    sendMessage(response);
        
}

void RtspClient::onSetParameterRequest(char** lines, int lineCnt)
{
    std::string msg = "";
    int cseq = -1;
    char* trigger_method = NULL;

    for (int ii =1; ii < lineCnt; ii++)
    {
        char* line = lines[ii];
        char* p = NULL;

        if ((p = strstr(line, "CSeq:")))
        {
            p += 5;
            cseq = strtol(ltrim(p), NULL, 10);
        }
        /* wfd_video_formats: 00 00 02 04 00000080 00000000 00000000 00 0000 0000 00 none none */
        else if ((p = strstr(line, "wfd_video_formats:")))
        {
            char resolution[16];
            strncpy(resolution, &p[31], 8);
            resolution[8] = 0;

            LOG_INFO("video resolution : %s\n", resolution);
            /* TODO IMPLEMENTS HERE */
        }
        /* wfd_audio_codecs: AAC 00000001 00 */
        else if ((p = strstr(line, "wfd_audio_codecs:")))
        {
            p += 17;
            LOG_INFO("wfd_audio_codecs : %s\n", p);
            
            /* TODO IMPLEMENTS HERE */
        }
        else if ((p = strstr(line, "wfd_presentation_URL:")))
        {
            char* end;
            p += 21;
            p = ltrim(p);

            end = strchr(p, ' ');
            if (end) *end = 0;

            LOG_INFO("wfd_presentation_URL: %s\n", p);
            mPresentationUrl = p;
        }
        else if ((p = strstr(line, "wfd_client_rtp_ports:")))
        {
            p += 21;
            LOG_DEBUG("[[[[[ recv wfd_client_rtp_ports(M4) request ]]]]]\n");
        
            LOG_INFO("wfd_client_rtp_ports: %s\n", p);
            /* TODO IMPLEMENTS HERE */
        }
        else if ((p = strstr(line, "wfd_trigger_method:")))
        {
            p += 19;
            trigger_method = ltrim(p);
        }
    }

    std::string response = "RTSP/1.0 200 OK\r\n";
    appendCommonResponse(response, cseq);
    response.append("\r\n");
    LOG_DEBUG("[RESPONSE]\n%s", response.c_str());
    sendMessage(response);

    if (trigger_method)
    {
        if (strstr(trigger_method, "SETUP"))
        {
            LOG_DEBUG("[[[[[ recv SETUP(M5) request ]]]]]\n");
            sendSetup();
        }
        else if (strstr(trigger_method, "PLAY"))
        {
            LOG_DEBUG("[[[[[ recv PLAY(M7) request ]]]]]\n");
            // TODO
        }
        else if (strstr(trigger_method, "TEARDOWN"))
        {
            LOG_DEBUG("[[[[[ recv TEARDOWN(M8) request ]]]]]\n");
            // TODO
        }
        else if (strstr(trigger_method, "PAUSE"))
        {
            LOG_DEBUG("[[[[[ recv PAUSE(M9) request ]]]]]\n");
            // TODO
        }
        else
        {
            LOG_WARN("Unkown trigger method : %s\n", trigger_method);
        }
    }
}

const std::string& RtspClient::getPresentationUrl()
{
    if (mPresentationUrl == "")
    {
        mPresentationUrl = "rtsp://" + mIP + "/wfd1.0/streamid=0";
    }
    
    return mPresentationUrl;
}

int RtspClient::sendM2()
{
    std::string msg = "OPTIONS * RTSP/1.0\r\n";
    
    appendCommonResponse(msg, mNextCSeq++);
    msg.append("Require: org.wfa.wfd1.0\r\n");
    msg.append("\r\n");
    
    LOG_DEBUG("[[[[[ send M2 request ]]]]]\n");
    LOG_DEBUG("[SEND REQUEST]\n%s", msg.c_str());
    return sendMessage(msg);
}

int RtspClient::sendSetup()
{
    std::string msg = "SETUP " + getPresentationUrl() + " RTSP/1.0\r\n";
    appendCommonResponse(msg, mNextCSeq++);
    msg.append("Transport: RTP/AVP/UDP;unicast;client_port=19000-19001\r\n");
    msg.append("\r\n");
    
    LOG_DEBUG("[SEND REQUEST]\n%s", msg.c_str());
    return sendMessage(msg);
}

int RtspClient::sendPlay()
{
    std::string msg = "PLAY " + getPresentationUrl() + " RTSP/1.0\r\n";
    appendCommonResponse(msg, mNextCSeq++);
    msg.append("Session: " + mSession + "\r\n");
    msg.append("\r\n");
    
    LOG_DEBUG("[SEND REQUEST]\n%s", msg.c_str());
    return sendMessage(msg);
}

int RtspClient::sendMessage(const std::string& msg)
{
    int ret;

    if (mSock < -1)
    {
        LOG_ERROR("Invalid socket descriptor.\n");
        return -1;
    }

    ret = NetUtil::fd_poll(mSock, POLL_REQ_OUT, DEFAULT_SEND_TIMEOUT, mExitPipe[0]);
    if (ret == POLL_SUCCESS)
    {
        ret = write(mSock, msg.data(), msg.length());
        if (ret < 0)
        {
            LOG_ERROR("send message failed : ret : %d, errno : %d,%s\n", ret, errno, strerror(errno));
        }
        else if (ret != msg.length())
        {
            LOG_ERROR("Message length is invalid !\n");
        }
        else
            ret = 0;
    }

    return ret;
}

void RtspClient::onRtspResponse(char** lines, int lineCnt)
{
    int resCode;
    char* p = lines[0] + 8;
    p = ltrim(p);
    resCode = strtol(p, NULL, 0);

    if (resCode == 200)
    {
        for (int ii = 1; ii < lineCnt; ii++)
        {
            char* line = lines[ii];
            if (strncmp(line, "Session:", 8) == 0)
            {
                char* p   = ltrim(line + 8);
                char* end = strchr(p, ';');
                
                mSessionTimeout = -1;
                if (end)
                {
                    *end++ = 0;

                    if(strncmp(end, "timeout=", 8) == 0)
                    {
                        mSessionTimeout = strtol(end + 8, NULL, 10);
                    }
                }
                mSession = p;
                
                LOG_INFO("SESSION: %s, SESSION_TIMEOUT: %d\n", mSession.c_str(), mSessionTimeout);
            }
            else if (strncmp(line, "Transport:", 10) == 0)
            {
                char* p;
                line += 10;

                sendPlay();
                notifySessionStarted();
            }
            else if (strncmp(line, "Range:", 6) == 0)
            {
                // TODO IMPELMENTS HERE
            }
        }
    }
    else
    {
        LOG_ERROR("[[[[ ERROR : %d ]]]]]]\n", resCode);
        // StopStream();

        // TODO IMPELEMENTS HERE
    }
}

#define MAX_LINE_CNT (32)
void RtspClient::onRecvMessage(char* data, int len)
{
    char* tok, *saveptr;
    char* lines[MAX_LINE_CNT];
    int   lineCnt = 0;

    data[len] = 0;

    LOG_TRACE("[RECV]\n%s", data); 
    /* Tokenize string */
    for (lineCnt = 0, tok = strtok_r(data, "\r\n", &saveptr); tok; tok = strtok_r(NULL, "\r\n", &saveptr), lineCnt++)
    {
        if (lineCnt >= MAX_LINE_CNT)
            break;
        lines[lineCnt] = tok;
    }

    if (strncmp(lines[0], "RTSP/1.0", 8) == 0)
    {
        LOG_DEBUG("[[[[[ recv RTSP resopnse ]]]]]\n");
        onRtspResponse(lines, lineCnt);
    }
    if (strncmp(lines[0], "OPTIONS", 7) == 0)
    {
        LOG_DEBUG("[[[[[ recv OPTIONS(M1) request ]]]]]\n");
        onOptionsRequest(lines, lineCnt);
    }
    else if (strncmp(lines[0], "GET_PARAMETER", 13) == 0)
    {
        LOG_DEBUG("[[[[[ recv GET_PARAMETER(M3/M16) request ]]]]]\n");
        onGetParameterRequest(lines, lineCnt);
    }
    else if (strncmp(lines[0], "SET_PARAMETER", 13) == 0)
    {
        LOG_DEBUG("[[[[[ recv SET_PARAMETER request ]]]]]\n");
        onSetParameterRequest(lines, lineCnt);
    }

}

void RtspClient::run()
{
    int ret, ii;
    char buf[4*1024];

    LOG_DEBUG("---- start rtsp client proc ----\n");

    if ((mSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        LOG_ERROR("Socket creation error \n");
        mSock = -1;
        goto EXIT;
    }
    
    for(ii = 0; ii < 10; ii++)
    {
        ret = NetUtil::connect2(mSock, mIP.c_str(), mPort, DEFAULT_CONNECT_TIMEOUT, mExitPipe[0]);
        if (ret == 0)
            break;

        sleep(1*1000);
    }

    if (ii >= 10)
    {
        notifyFailed(1);
        goto EXIT;
    }

    notifyConnected();

    while(!mExitTask)
    {
        int ret = NetUtil::fd_poll(mSock, POLL_REQ_IN, DEFAULT_RECV_TIMEOUT, mExitPipe[0]);
        if (ret == POLL_SUCCESS)
        {
            ret = read(mSock, buf, sizeof(buf));
            if (ret > 0)
            {
                onRecvMessage(buf, ret);
            }
            else
            {
                if (ret < 0)
                {
                    notifyFailed(2);
                    LOG_ERROR("read failed. errno : %d, %s\n", ret, errno, strerror(errno));
                }
            }
        }
        else if (ret == POLL_TIMEOUT) 
        { 
            continue; 
        }
        else if (ret == POLL_INTERRUPTED)
        {
            LOG_INFO("!!!! INTERRUPTED !!!!\n");
            mExitTask = true;
            continue;
        }
        else
        {
            LOG_ERROR("Polling failed ! Exit task\n");
            notifyFailed(3);
            mExitTask = true;
            continue;
        }
    }

EXIT:
    if (mSock >= 0)
    {
        close(mSock);
        mSock = -1;
    }

    LOG_DEBUG("---- end rtsp client proc ----\n");

    notifyClosed();
}

std::string RtspClient::makeWfdVideoFormat()
{
    // TODO IMPLMENTES HERE
    return "02 00 01 04 00019ceb 00000003 00000ff3 00 0000 0000 00 none none, 02 04 00019ceb 00000003 00000ff3 00 0000 0000 00 none none";
}

std::string RtspClient::makeWfdAudioFormat()
{
    // TODO IMPLMENTES HERE
    return "LPCM 00000002 00, AAC 00000001 00";
}

std::string RtspClient::makeWfdDisplayEdid()
{
    // TODO IMPLMENTES HERE
    return "none";
}

std::string RtspClient::makeWfdContentProtection()
{
    // TODO IMPLMENTES HERE
    //if(mSupportHdcp2x == true && mHdcp2xAvailable == true){
    //    msg.append("HDCP2.1 port=");
    //    msg.append(std::to_string(HDCP2x::kDefaultHDCP2xPort) + "\r\n");
    //    foundHDCP = true;
    //}
    return "none";
}

void RtspClient::addListener(IRtspClientListener* listener)
{
    if(!listener)
        return;

    RtspClientListenerList::iterator it = std::find(mRtspClientListeners.begin(), mRtspClientListeners.end(), listener);
    if(listener == *it)
    {
        LOG_ERROR("RtspClientListener is alreay exsit !!\n");
        return;
    }
    mRtspClientListeners.push_front(listener);
}

void RtspClient::removeListener(IRtspClientListener* listener)
{
    if(!listener)
        return;

    for(RtspClientListenerList::iterator it = mRtspClientListeners.begin(); it != mRtspClientListeners.end(); it++)
    {
        if(listener == *it)
        {
            mRtspClientListeners.erase(it);
            return;
        }
    }
}


void RtspClient::notifyConnected()
{
    for(RtspClientListenerList::iterator it = mRtspClientListeners.begin(); it != mRtspClientListeners.end(); it++)
    {
        (*it)->onRtspConnected();
    }
}

void RtspClient::notifyFailed(int error)
{
    for(RtspClientListenerList::iterator it = mRtspClientListeners.begin(); it != mRtspClientListeners.end(); it++)
    {
        (*it)->onRtspFailed(error);
    }
}

void RtspClient::notifyClosed()
{
    for(RtspClientListenerList::iterator it = mRtspClientListeners.begin(); it != mRtspClientListeners.end(); it++)
    {
        (*it)->onRtspClosed();
    }
}

void RtspClient::notifySessionStarted()
{
    for(RtspClientListenerList::iterator it = mRtspClientListeners.begin(); it != mRtspClientListeners.end(); it++)
    {
        (*it)->onRtspSessionStarted();
    }
}

void RtspClient::notifySessionEnded()
{
    for(RtspClientListenerList::iterator it = mRtspClientListeners.begin(); it != mRtspClientListeners.end(); it++)
    {
        (*it)->onRtspSessionEnded();
    }
}
