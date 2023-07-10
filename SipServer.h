//
// Created bxc on 2022/11/25.
//

#ifndef BXC_SIPSERVER_SIPSERVER_H
#define BXC_SIPSERVER_SIPSERVER_H
extern "C" {
#include <osip2/osip_mt.h>
#include <eXosip2/eXosip.h>
}
#include <map>
#include <string>

class ServerInfo {
public:
    ServerInfo(const char *ua,const char *nonce, const char *ip, int port, int rtpPort,
                        const char *sipId, const char *sipRealm, const char *sipPass, int sipTimeout, int sipExpiry):
                        mUa(ua),
                        mNonce(nonce),mIp(ip),mPort(port),mRtpPort(rtpPort),mSipId(sipId),
                        mSipRealm(sipRealm),mSipPass(sipPass),mSipTimeout(sipTimeout),
                        mSipExpiry(sipExpiry){}
    ~ServerInfo() = default;
public:
    const char *getUa() const{
        return mUa;
    }
    const char * getNonce() const{
        return mNonce;
    }
    const char * getIp() const{
        return mIp;
    }
    int getPort() const {
        return mPort;
    }
    int getRtpPort() const {
        return mRtpPort;
    }
    const char * getSipId() const{
        return mSipId;
    }
    const char * getSipRealm() const{
        return mSipRealm;
    }
    const char * getSipPass() const{
        return mSipPass;
    }
    int getTimeout() const {
        return mSipTimeout;
    }
    int getExpiry() const {
        return mSipExpiry;
    }

private:
    const char *mUa;
    const char *mNonce;//SIP服务随机数值
    const char *mIp;//SIP服务IP
    int         mPort;//SIP服务端口
    const char *mSipId; //SIP服务器ID
    const char *mSipRealm;//SIP服务器域
    const char *mSipPass;//SIP password
    int mSipTimeout; //SIP timeout
    int mSipExpiry;// SIP到期
    int mRtpPort; //SIP-RTP服务端口

};

class Client {
public:
    Client(const char *ip, int port, const char *device) :
            mIp(ip),
            mPort(port),
            mRtpPort(0),
            mDevice(device),
            mIsReg(false){

    }
    ~Client() = default;
public:

    void setRtpPort(int rtpPort) {
        mRtpPort = rtpPort;
    }

    void setReg(bool isReg) {
        mIsReg = isReg;
    }
    const char * getDevice() const{
        return mDevice;
    }
    const char * getIp() const{
        return mIp;
    }
    int getPort() const{
        return mPort;
    }

private:
    // step1
    const char *mIp; // client ip
    int mPort; // client port
    const char *mDevice;// 340200000013200000024
    // step2
    bool mIsReg;
    // step3
    int mRtpPort{};

};


class SipServer {
public:
    explicit SipServer(ServerInfo *info);
    ~SipServer();
public:
    void loop();
private:
    int init_sip_server();
    int sip_event_handle(eXosip_event_t *evtp);

    void response_message_answer(eXosip_event_t *evtp,int code);
    void response_register(eXosip_event_t *evtp);
    void response_register_401unauthorized(eXosip_event_t *evt);
    void response_message(eXosip_event_t *evtp);
    void response_invite_ack(eXosip_event_t *evtp);
    int request_bye(eXosip_event_t* evtp);// 通知相机停止推流
    int request_invite(const char *device,const char *userIp,int userPort);
    int parse_xml(const char* data, const char* s_mark, bool with_s_make, const char* e_mark, bool with_e_make, char* dest);
    void dump_request(eXosip_event_t *evtp);
    void dump_response(eXosip_event_t *evtp);

private:
    bool mQuit;
    struct eXosip_t *mSipCtx;
    ServerInfo *mInfo;

    std::map<std::string, Client *> mClientMap;// <DeviceID,SipClient>
    int clearClientMap();
    Client * getClientByDevice(const char * device);
};


#endif //BXC_SIPSERVER_SIPSERVER_H
