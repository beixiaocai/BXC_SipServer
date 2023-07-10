//
// Created bxc on 2022/11/25.
//

#include "SipServer.h"

#ifndef WIN32
// Linux系统
#include <arpa/inet.h>
#else
#include <WinSock2.h>
#endif // !WIN32

#pragma comment(lib, "ws2_32.lib")

#include <cstring>
#include "Utils/Log.h"

extern "C"{
#include "Utils/HTTPDigest.h"
}
SipServer::SipServer(ServerInfo *info):
        mQuit(false),
        mSipCtx(nullptr),
        mInfo(info){
    LOGI("%s:%d",mInfo->getIp(),mInfo->getPort());
#ifdef WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        LOGE("WSAStartup Error");
        return;
    }
#endif // WIN32

}
SipServer::~SipServer() {
    LOGI("");
    this->clearClientMap();
#ifdef WIN32
    WSACleanup();
#endif // WIN32
}

int SipServer::sip_event_handle(eXosip_event_t *evtp) {

    switch(evtp->type) {
        case EXOSIP_CALL_MESSAGE_NEW://14
            LOGI("EXOSIP_CALL_MESSAGE_NEW type=%d", evtp->type);
            this->dump_request(evtp);
            this->dump_response(evtp);
            break;

        case EXOSIP_CALL_CLOSED://21
            LOGI("EXOSIP_CALL_CLOSED type=%d",evtp->type);
            this->dump_request(evtp);
            this->dump_response(evtp);
            break;

        case EXOSIP_CALL_RELEASED://22
            LOGI("EXOSIP_CALL_RELEASED type=%d", evtp->type);
            this->dump_request(evtp);
            this->dump_response(evtp);

            this->clearClientMap();
            break;
        case EXOSIP_MESSAGE_NEW://23
            LOGI("EXOSIP_MESSAGE_NEW type=%d",evtp->type);

            if (MSG_IS_REGISTER(evtp->request)) {
                this->response_register(evtp);
            }
            else if (MSG_IS_MESSAGE(evtp->request)) {
                this->response_message(evtp);
            }
            else if(strncmp(evtp->request->sip_method, "BYE", 3) != 0){
                LOGE("unknown1");
            }
            else{
                LOGE("unknown2");
            }
            break;
        case EXOSIP_MESSAGE_ANSWERED:
            this->dump_request(evtp);
            break;
        case EXOSIP_MESSAGE_REQUESTFAILURE:
            LOGI("EXOSIP_MESSAGE_REQUESTFAILURE type=%d: Receive feedback on sending failure after actively sending a message", evtp->type);
            this->dump_request(evtp);
            this->dump_response(evtp);
            break;
        case EXOSIP_CALL_INVITE:
            LOGI("EXOSIP_CALL_INVITE type=%d: The server receives the Invite request actively sent by the client", evtp->type);
            break;
        case EXOSIP_CALL_PROCEEDING://5
            LOGI("EXOSIP_CALL_PROCEEDING type=%d: When the server receives the Invite (SDP) confirmation reply from the client", evtp->type);
            this->dump_request(evtp);
            this->dump_response(evtp);
            break;
        case EXOSIP_CALL_ANSWERED:// 7
            LOGI("EXOSIP_CALL_ANSWERED type=%d: The server receives an invite (SDP) confirmation reply from the client", evtp->type);
            this->dump_request(evtp);
            this->dump_response(evtp);

            this->response_invite_ack(evtp);
            break;
        case EXOSIP_CALL_SERVERFAILURE:
            LOGI("EXOSIP_CALL_SERVERFAILURE type=%d", evtp->type);
            break;
        case EXOSIP_IN_SUBSCRIPTION_NEW:
            LOGI("EXOSIP_IN_SUBSCRIPTION_NEW type=%d", evtp->type);
            break;
        default:
            LOGI("type=%d unknown", evtp->type);
            break;
    }

    return 0;
}

int SipServer::init_sip_server() {
    mSipCtx = eXosip_malloc();
    if (!mSipCtx) {
        LOGE("eXosip_malloc error");
        return -1;
    }
    if (eXosip_init(mSipCtx)) {
        LOGE("eXosip_init error");
        return -1;
    }
    if (eXosip_listen_addr(mSipCtx, IPPROTO_UDP, nullptr, mInfo->getPort(), AF_INET, 0)) {
        LOGE("eXosip_listen_addr error");
        return -1;
    }
    eXosip_set_user_agent(mSipCtx, mInfo->getUa());
    if (eXosip_add_authentication_info(mSipCtx, mInfo->getSipId(),mInfo->getSipId(), mInfo->getSipPass(), NULL, mInfo->getSipRealm())){
        LOGE("eXosip_add_authentication_info error");
        return -1;
    }

    return 0;
}

void SipServer::loop() {

    if(this->init_sip_server() !=0 ){
        return;
    }
    while(!mQuit) {
        eXosip_event_t *evtp = eXosip_event_wait(mSipCtx, 0, 20);
        if (!evtp){
            eXosip_automatic_action(mSipCtx);
            osip_usleep(100000);
            continue;
        }
        eXosip_automatic_action(mSipCtx);
        this->sip_event_handle(evtp);
        eXosip_event_free(evtp);
    }
}

void SipServer::response_message_answer(eXosip_event_t *evtp,int code){

    int returnCode = 0 ;
    osip_message_t * pRegister = nullptr;
    returnCode = eXosip_message_build_answer (mSipCtx,evtp->tid,code,&pRegister);
    bool bRegister = false;
    if(pRegister){
        bRegister = true;
    }
    if (returnCode == 0 && bRegister)
    {
        eXosip_lock(mSipCtx);
        eXosip_message_send_answer (mSipCtx,evtp->tid,code,pRegister);
        eXosip_unlock(mSipCtx);
    }
    else{
        LOGE("code=%d,returnCode=%d,bRegister=%d",code,returnCode,bRegister);
    }

}
void SipServer::response_register(eXosip_event_t *evtp) {


    osip_authorization_t * auth = nullptr;
    osip_message_get_authorization(evtp->request, 0, &auth);

    if(auth && auth->username){

        char *method = NULL, // REGISTER
        *algorithm = NULL, // MD5
        *username = NULL,// 340200000013200000024
        *realm = NULL, // sip服务器传给客户端，客户端携带并提交上来的sip服务域
        *nonce = NULL, //sip服务器传给客户端，客户端携带并提交上来的nonce
        *nonce_count = NULL,
        *uri = NULL; // sip:34020000002000000001@3402000000

        osip_contact_t *contact = nullptr;
        osip_message_get_contact (evtp->request, 0, &contact);

        method = evtp->request->sip_method;
        char calc_response[HASHHEXLEN];
        HASHHEX HA1, HA2 = "", Response;

#define SIP_STRDUP(field) if (auth->field) (field) = osip_strdup_without_quote(auth->field)

        SIP_STRDUP(algorithm);
        SIP_STRDUP(username);
        SIP_STRDUP(realm);
        SIP_STRDUP(nonce);
        SIP_STRDUP(nonce_count);
        SIP_STRDUP(uri);


        DigestCalcHA1(algorithm, username, realm, mInfo->getSipPass(), nonce, nonce_count, HA1);
        DigestCalcResponse(HA1, nonce, nonce_count, auth->cnonce, auth->message_qop, 0, method, uri, HA2, Response);

        HASHHEX temp_HA1;
        HASHHEX temp_response;
        DigestCalcHA1("REGISTER", username, mInfo->getSipRealm(), mInfo->getSipPass(), mInfo->getNonce(), NULL, temp_HA1);
        DigestCalcResponse(temp_HA1, mInfo->getNonce(), NULL, NULL, NULL, 0, method, uri, NULL, temp_response);
        memcpy(calc_response, temp_response, HASHHEXLEN);

        Client *client = new Client(strdup(contact->url->host),
                                    atoi(contact->url->port),
                                    strdup(username));

        if (!memcmp(calc_response, Response, HASHHEXLEN)) {
            this->response_message_answer(evtp,200);
            LOGI("Camera registration succee,ip=%s,port=%d,device=%s",client->getIp(),client->getPort(),client->getDevice());

            mClientMap.insert(std::make_pair(client->getDevice(),client));

            this->request_invite(client->getDevice(),client->getIp(),client->getPort());

        } else {
            this->response_message_answer(evtp,401);
            LOGI("Camera registration error, p=%s,port=%d,device=%s",client->getIp(),client->getPort(),client->getDevice());

            delete client;
        }

        osip_free(algorithm);
        osip_free(username);
        osip_free(realm);
        osip_free(nonce);
        osip_free(nonce_count);
        osip_free(uri);
    } else {
        response_register_401unauthorized(evtp);
    }

}

void SipServer::response_register_401unauthorized(eXosip_event_t *evtp) {

    char *dest = nullptr;
    osip_message_t * reg = nullptr;
    osip_www_authenticate_t * header = nullptr;

    osip_www_authenticate_init(&header);
    osip_www_authenticate_set_auth_type (header, osip_strdup("Digest"));
    osip_www_authenticate_set_realm(header,osip_enquote(mInfo->getSipRealm()));
    osip_www_authenticate_set_nonce(header,osip_enquote(mInfo->getNonce()));
    osip_www_authenticate_to_str(header, &dest);
    int ret = eXosip_message_build_answer (mSipCtx, evtp->tid, 401, &reg);
    if ( ret == 0 && reg != nullptr ) {
        osip_message_set_www_authenticate(reg, dest);
        osip_message_set_content_type(reg, "Application/MANSCDP+xml");
        eXosip_lock(mSipCtx);
        eXosip_message_send_answer (mSipCtx, evtp->tid,401, reg);
        eXosip_unlock(mSipCtx);
        LOGI("response_register_401unauthorized success");
    }else {
        LOGI("response_register_401unauthorized error");
    }

    osip_www_authenticate_free(header);
    osip_free(dest);

}

void SipServer::response_message(eXosip_event_t *evtp) {

    osip_body_t* body = nullptr;
    char CmdType[64] = {0};
    char DeviceID[64] = {0};
    osip_message_get_body(evtp->request, 0, &body);
    if(body){
        parse_xml(body->body, "<CmdType>", false, "</CmdType>", false, CmdType);
        parse_xml(body->body, "<DeviceID>", false, "</DeviceID>", false, DeviceID);
    }

//    Client *client = getClientByDevice(DeviceID);
//    if(client){
//        LOGI("response_message：%s 已注册",DeviceID);
//    }else{
//        LOGE("response_message：%s 未注册",DeviceID);
//    }
    LOGI("CmdType=%s,DeviceID=%s", CmdType,DeviceID);

    if(!strcmp(CmdType, "Catalog")) {
        this->response_message_answer(evtp,200);
        // 需要根据对方的Catelog请求，做一些相应的应答请求
    }
    else if(!strcmp(CmdType, "Keepalive")){
        this->response_message_answer(evtp,200);
    }else{
        this->response_message_answer(evtp,200);
    }

}

void SipServer::response_invite_ack(eXosip_event_t *evtp){

    osip_message_t* msg = nullptr;
    int ret = eXosip_call_build_ack(mSipCtx, evtp->did, &msg);
    if (!ret && msg) {
        eXosip_call_send_ack(mSipCtx, evtp->did, msg);
    } else {
        LOGE("eXosip_call_send_ack error=%d", ret);
    }

}
int SipServer::request_bye(eXosip_event_t* evtp) {

    eXosip_lock(mSipCtx);
    int ret = eXosip_call_terminate(mSipCtx, evtp->cid, evtp->did);
    eXosip_unlock(mSipCtx);

    return ret;
}
int SipServer::request_invite(const char *device, const char *userIp, int userPort) {
    LOGI("INVITE");

    char session_exp[1024] = { 0 };
    osip_message_t *msg = nullptr;
    char from[1024] = {0};
    char to[1024] = {0};
    char contact[1024] = {0};
    char sdp[2048] = {0};
    char head[1024] = {0};


    sprintf(from, "sip:%s@%s:%d", mInfo->getSipId(),mInfo->getIp(), mInfo->getPort());
    sprintf(contact, "sip:%s@%s:%d", mInfo->getSipId(),mInfo->getIp(), mInfo->getPort());
    sprintf(to, "sip:%s@%s:%d", device, userIp, userPort);
    snprintf (sdp, 2048,
              "v=0\r\n"
              "o=%s 0 0 IN IP4 %s\r\n"
              "s=Play\r\n"
              "c=IN IP4 %s\r\n"
              "t=0 0\r\n"
              "m=video %d TCP/RTP/AVP 96 98 97\r\n"
              "a=recvonly\r\n"
              "a=rtpmap:96 PS/90000\r\n"
              "a=rtpmap:98 H264/90000\r\n"
              "a=rtpmap:97 MPEG4/90000\r\n"
              "a=setup:passive\r\n"
              "a=connection:new\r\n"
              "y=0100000001\r\n"
              "f=\r\n", mInfo->getSipId(),mInfo->getIp(), mInfo->getIp(), mInfo->getRtpPort());

    int ret = eXosip_call_build_initial_invite(mSipCtx, &msg, to, from,  nullptr, nullptr);
    if (ret) {
        LOGE( "eXosip_call_build_initial_invite error: %s %s ret:%d", from, to, ret);
        return -1;
    }

    osip_message_set_body(msg, sdp, strlen(sdp));
    osip_message_set_content_type(msg, "application/sdp");
    snprintf(session_exp, sizeof(session_exp)-1, "%i;refresher=uac", mInfo->getTimeout());
    osip_message_set_header(msg, "Session-Expires", session_exp);
    osip_message_set_supported(msg, "timer");

    int call_id = eXosip_call_send_initial_invite(mSipCtx, msg);

    if (call_id > 0) {
        LOGI("eXosip_call_send_initial_invite success: call_id=%d",call_id);
    }else{
        LOGE("eXosip_call_send_initial_invite error: call_id=%d",call_id);
    }
    return ret;
}

int SipServer::clearClientMap(){
    std::map<std::string ,Client *>::iterator iter;
    for (iter=mClientMap.begin(); iter!=mClientMap.end(); iter++) {
        delete iter->second;

        iter->second = nullptr;
    }
    mClientMap.clear();

    return 0;
}
Client * SipServer::getClientByDevice(const char *device) {

    auto it = mClientMap.find(device);
    if(it == mClientMap.end()){
        return nullptr;
    }
    return it->second;

}

int SipServer::parse_xml(const char *data, const char *s_mark, bool with_s_make, const char *e_mark, bool with_e_make, char *dest) {
    const char* satrt = strstr( data, s_mark );

    if(satrt != NULL) {
        const char* end = strstr(satrt, e_mark);

        if(end != NULL){
            int s_pos = with_s_make ? 0 : strlen(s_mark);
            int e_pos = with_e_make ? strlen(e_mark) : 0;

            strncpy( dest, satrt+s_pos, (end+e_pos) - (satrt+s_pos) );
        }
        return 0;
    }
    return -1;

}
void SipServer::dump_request(eXosip_event_t *evtp) {
    char *s;
    size_t len;
    osip_message_to_str(evtp->request, &s, &len);
    LOGI("\nprint request start\ntype=%d\n%s\nprint request end\n",evtp->type,s);
}
void SipServer::dump_response(eXosip_event_t *evtp) {
    char *s;
    size_t len;
    osip_message_to_str(evtp->response, &s, &len);
    LOGI("\nprint response start\ntype=%d\n%s\nprint response end\n",evtp->type,s);
}