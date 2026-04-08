/*
// Copyright (c) 2025 MiTAC Computing Technology Corp.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/
#include "mctp-spdm-generic-driver.hpp"
#include <iostream>
#include <stop_token>
#include <thread>
#include <fstream>

#define CERT_CAP                    0x00000002
#define CHAL_CAP                    0x00000004
#define ENCRYPT_CAP                 0x00000040
#define MAC_CAP                     0x00000080
#define MUT_AUTH_CAP                0x00000100
#define KEY_EX_CAP                  0x00000200
#define PSK_CAP                     0x00000C00
#define ENCAP_CAP                   0x00002000
#define HBEAT_CAP                   0x00002000
#define KEY_UPD_CAP                 0x00004000
#define HANDSHAKE_IN_THE_CLEAR_CAP  0x00008000
#define PUB_KEY_ID_CAP              0x00010000
#define CHUNK_CAP                   0x00020000

namespace com::mitac_computing::mctp
{
#pragma pack(push, 1)
struct SPDMGeneralReq {
    std::uint8_t type;
    std::uint8_t SPDMVersion;
    std::uint8_t RequestResponseCode;
    std::uint8_t Param1;
    std::uint8_t Param2;
};

struct GetVersionReq {
    std::uint8_t type;
    std::uint8_t SPDMVersion;
    std::uint8_t RequestResponseCode;
    std::uint8_t Param1;
    std::uint8_t Param2;
};

struct GetCapabilityReq_V10 {
    std::uint8_t type;
    std::uint8_t SPDMVersion;
    std::uint8_t RequestResponseCode;
    std::uint8_t Param1;
    std::uint8_t Param2;
};
struct GetCapabilityReq_V11 {
    std::uint8_t type;
    std::uint8_t SPDMVersion;
    std::uint8_t RequestResponseCode;
    std::uint8_t Param1;
    std::uint8_t Param2;
    std::uint8_t Reserved1;
    std::uint8_t CTExponent;
    std::uint16_t Reserved2;
    std::uint32_t Flags;
};
struct GetCapabilityReq_V12 {
    std::uint8_t type;
    std::uint8_t SPDMVersion;
    std::uint8_t RequestResponseCode;
    std::uint8_t Param1;
    std::uint8_t Param2;
    std::uint8_t Reserved1;
    std::uint8_t CTExponent;
    std::uint16_t Reserved2;
    std::uint32_t Flags;
    std::uint32_t DataTransferSize;
    std::uint32_t MaxSPDMmsgSize;
};

struct FixedReqAlgStruct {
    std::uint8_t AlgType;
    std::uint8_t AlgCount;
    std::uint8_t AlgSupported[2];
};

struct NegotiateAlgorithmsReq_V12 {
    std::uint8_t type;
    std::uint8_t SPDMVersion;
    std::uint8_t RequestResponseCode;
    std::uint8_t Param1;
    std::uint8_t Param2;
    std::uint16_t Length;
    std::uint8_t MeasurementSpecification;
    std::uint8_t OtherParamsSupport;
    std::uint8_t BaseAsymAlgo[4];
    std::uint8_t BaseHashAlgo[4];
    std::uint8_t Reserved[12];
    std::uint8_t ExtAsymCount;
    std::uint8_t ExtHashCount;
    std::uint8_t Reserved2[2];
    struct FixedReqAlgStruct fras[4];
};

struct GetCertificateReq {
    std::uint8_t type;
    std::uint8_t SPDMVersion;
    std::uint8_t RequestResponseCode;
    std::uint8_t Param1;
    std::uint8_t Param2;
    std::uint16_t Offset;
    std::uint16_t Size;
};

struct GetCertificateResp {
    std::uint8_t SPDMVersion;
    std::uint8_t RequestResponseCode;
    std::uint8_t Param1;
    std::uint8_t Param2;
    std::uint16_t PortionLength;
    std::uint16_t RemainderLength;
    std::uint8_t* CertChain;
};

struct GetChallengeReq {
    std::uint8_t type;
    std::uint8_t SPDMVersion;
    std::uint8_t RequestResponseCode;
    std::uint8_t Param1;
    std::uint8_t Param2;
    std::uint8_t Nonce[32];
};

#pragma pack(pop)

extern "C" MCTPDriverIntf* getDriver() {
    std::cout << "Hello from the shared library!" << std::endl;
    return new SpdmDriverIntf();
}

static bool isNormalResponse(std::uint8_t* respData, ssize_t respSize) {
    if (respData == nullptr)
        return false;

    if (respSize < 2) {
        std::cerr << "  Invalid response length." << std::endl;
        return false;
    }

    const std::uint8_t SPDMRespCode = respData[1];
    if (SPDMRespCode == 0x7F) {
        if (respSize < 3) {
            std::cerr << "  Invalid ERROR response message format." << std::endl;
            return false;
        }
        switch (respData[2]) {
            case 0x00:
            case 0x02:
                std::cerr << "  ERROR: Reserved." << std::endl;
                break;
            case 0x01:
                std::cerr << "  ERROR: InvalidRequest." << std::endl;
                break;
            case 0x03:
                std::cerr << "  ERROR: Busy." << std::endl;
                break;
            case 0x04:
                std::cerr << "  ERROR: UnexpectedRequest." << std::endl;
                break;
            case 0x05:
                std::cerr << "  ERROR: Unspecified." << std::endl;
                break;
            case 0x06:
                std::cerr << "  ERROR: DecryptError." << std::endl;
                break;
            case 0x07:
                std::cerr << "  ERROR: UnsupportedRequest." << std::endl;
                break;
            case 0x08:
                std::cerr << "  ERROR: RequestInFlight." << std::endl;
                break;
            case 0x09:
                std::cerr << "  ERROR: InvalidResponseCode." << std::endl;
                break;
            case 0x0A:
                std::cerr << "  ERROR: SessionLimitExceeded." << std::endl;
                break;
            case 0x0B:
                std::cerr << "  ERROR: SessionRequired." << std::endl;
                break;
            case 0x0C:
                std::cerr << "  ERROR: ResetRequired." << std::endl;
                break;
            case 0x0D:
                std::cerr << "  ERROR: ResponseTooLarge." << std::endl;
                break;
            case 0x0E:
                std::cerr << "  ERROR: RequestTooLarge." << std::endl;
                break;
            case 0x0F:
                std::cerr << "  ERROR: LargeResponse." << std::endl;
                break;
            case 0x10:
                std::cerr << "  ERROR: MessageLost." << std::endl;
                break;
            case 0x11:
                std::cerr << "  ERROR: InvalidPolicy." << std::endl;
                break;
            case 0x41:
                std::cerr << "  ERROR: VersionMismatch." << std::endl;
                break;
            case 0x42:
                std::cerr << "  ERROR: ResponseNotReady." << std::endl;
                break;
            case 0x43:
                std::cerr << "  ERROR: RequestResynch." << std::endl;
                break;
            case 0x44:
                std::cerr << "  ERROR: OperationFailed." << std::endl;
                break;
            case 0x45:
                std::cerr << "  ERROR: NoPendingRequests." << std::endl;
                break;
            default:
                std::cerr << "  ERROR: Unknown error code: " << (std::uint16_t) respData[2] << std::endl;
                break;
        }
        return false;
    }

    switch (SPDMRespCode) {
        case 0x01:
            std::cout << "  Return type: DIGESTS" << std::endl;
            break;
        case 0x02:
            std::cout << "  Return type: CERTIFICATE" << std::endl;
            break;
        case 0x03:
            std::cout << "  Return type: CHALLENGE_AUTH" << std::endl;
            break;
        case 0x04:
            std::cout << "  Return type: VERSION" << std::endl;
            break;
        case 0x05:
            std::cout << "  Return type: CHUNK_SEND_ACK" << std::endl;
            break;
        case 0x06:
            std::cout << "  Return type: CHUNK_RESPONSE" << std::endl;
            break;
        case 0x07:
            std::cout << "  Return type: ENDPOINT_INFO" << std::endl;
            break;
        case 0x60:
            std::cout << "  Return type: MEASUREMENTS" << std::endl;
            break;
        case 0x61:
            std::cout << "  Return type: CAPABILITIES" << std::endl;
            break;
        case 0x62:
            std::cout << "  Return type: SUPPORTED_EVENT_TYPES" << std::endl;
            break;
        case 0x63:
            std::cout << "  Return type: ALGORITHMS" << std::endl;
            break;
        case 0x64:
            std::cout << "  Return type: KEY_EXCHANGE_RSP" << std::endl;
            break;
        case 0x65:
            std::cout << "  Return type: FINISH_RSP" << std::endl;
            break;
        case 0x66:
            std::cout << "  Return type: PSK_EXCHANGE_RSP" << std::endl;
            break;
        case 0x67:
            std::cout << "  Return type: PSK_FINISH_RSP" << std::endl;
            break;
        case 0x68:
            std::cout << "  Return type: HEARTBEAT_ACK" << std::endl;
            break;
        case 0x69:
            std::cout << "  Return type: KEY_UPDATE_ACK" << std::endl;
            break;
        case 0x6A:
            std::cout << "  Return type: ENCAPSULATED_REQUEST" << std::endl;
            break;
        case 0x6B:
            std::cout << "  Return type: ENCAPSULATED_RESPONSE_ACK" << std::endl;
            break;
        case 0x6C:
            std::cout << "  Return type: END_SESSION_ACK" << std::endl;
            break;
        case 0x6D:
            std::cout << "  Return type: CSR" << std::endl;
            break;
        case 0x6E:
            std::cout << "  Return type: SET_CERTIFICATE_RSP" << std::endl;
            break;
        case 0x6F:
            std::cout << "  Return type: MEASUREMENT_EXTENSION_LOG" << std::endl;
            break;
        case 0x70:
            std::cout << "  Return type: SUBSCRIBE_EVENT_TYPES_ACK" << std::endl;
            break;
        case 0x71:
            std::cout << "  Return type: EVENT_ACK" << std::endl;
            break;
        case 0x7C:
            std::cout << "  Return type: KEY_PAIR_INFO" << std::endl;
            break;
        case 0x7D:
            std::cout << "  Return type: SET_KEY_PAIR_INFO_ACK" << std::endl;
            break;
        case 0x7E:
            std::cout << "  Return type: VENDOR_DEFINED_RESPONSE" << std::endl;
            break;
        case 0x7F:
            std::cout << "  Return type: ERROR" << std::endl;
            break;
        default:
            std::cout << "  Return type: Reserved" << std::endl;
            break;
    }

    return true;
}

SpdmDriverIntf::SpdmDriverIntf() {

}

bool SpdmDriverIntf::attachDevice(MCTPEndPointBase* mctpEp)
{
    // Use APIs provided by MCTPEndPointBase to construct runtime resource.
    //return new MCTPDriverHandle(this);
    if (isSupported(mctpEp)) {
        const MCTPDriverHandle* handle = new SpdmDriverHandle(this, mctpEp);
        mctpEp->installDriverHandle(handle);
        return true;
    }
    return false;
};

bool SpdmDriverIntf::isSupported(const MCTPEndPointBase* mctpEp) const {
    if (mctpEp == nullptr)
        return false;

    const std::uint8_t MESSAGE_TYPE_SPDM = 5;
    const std::uint8_t MESSAGE_TYPE_NVMEMI = 4;
    // FIXME: Found that SDPM command might be halt when trying to access BRCM Storelib card.
    //if (mctpEp->isMessageTypeSupported(MESSAGE_TYPE_SPDM) && mctpEp->isMessageTypeSupported(MESSAGE_TYPE_NVMEMI)) {
    if (mctpEp->isMessageTypeSupported(MESSAGE_TYPE_SPDM)) {
        return true;
    }

    return false;
}

std::string SpdmDriverIntf::getDriverUUID() const{
    // Return SHA256 of class name. SHA256(SpdmDriverIntf) = 25bcb8fc27e897a334db9900e79261ed43aa4fafc37413bfbcfebc2f95ac5dc9
    return typeid(*this).name();
}


SpdmDriverHandle::SpdmDriverHandle(const MCTPDriverIntf* drvIntf, const MCTPEndPointBase* mctpEp)
    :MCTPDriverHandle(drvIntf, mctpEp), _inited(false)
{
}

SpdmDriverHandle::~SpdmDriverHandle()
{
}

bool SpdmDriverHandle::runTask()
{
    std::cout << "Net " << (std::uint16_t) this->getNet() << " EID " << (std::uint16_t) this->getEId() << " runTask: " << getDriverUUID() << " inited: " << _inited << std::endl;
    std::uint8_t negotiatedVer = 0x12;
    if (!_inited) {
        if (this->getVersion(negotiatedVer)) {
            std::uint32_t cap = 0;
            this->getCapability(negotiatedVer, cap);
            std::stringstream stream;
            stream << "0x" << std::hex <<  (uint32_t) cap;
            std::cout << "  Capability: " << stream.str() << std::endl;
            this->negotiateAlgo(negotiatedVer);
            this->getDigest(negotiatedVer);
            this->getCertificate(negotiatedVer);
            _inited = true;
        }
    }
    if (this->getVersion(negotiatedVer)) {
        std::uint32_t cap = 0;
        this->getCapability(negotiatedVer, cap);
        std::stringstream stream;
        stream << "0x" << std::hex <<  (uint32_t) cap;
        std::cout << "  Capability: " << stream.str() << std::endl;
        this->negotiateAlgo(negotiatedVer);
        this->challenge(negotiatedVer);
    }
    return true;
}

bool SpdmDriverHandle::getVersion(std::uint8_t& maxVer)
{
    //std::cout << "SpdmDriverHandle::getVersion()" << std::endl;
    MCTPPacket<struct GetVersionReq> dataPack(false, MCTP_MSG_TYPE_SPDM);
    struct GetVersionReq* req = dataPack.getReqData();

    req->SPDMVersion = 0x10;
    req->RequestResponseCode = 0x84;
    req->Param1 = 0;
    req->Param2 = 0;
    if (!_mctpEp->doSendReceive(&dataPack))
        return false;
    if (!isNormalResponse(dataPack.respData, dataPack.respSize))
        return false;

    const std::uint8_t SupportedVerOffset = 5;
    if (SupportedVerOffset > dataPack.respSize)
        return false;

    const std::uint8_t SupportedVerCnt = dataPack.respData[SupportedVerOffset];
    if ((SupportedVerOffset + (SupportedVerCnt * 2)) > dataPack.respSize)
        return false;

    if (SupportedVerCnt == 0x00)
        return false;

    if (dataPack.respData[SupportedVerOffset + (SupportedVerCnt * 2)] < maxVer)
        maxVer = dataPack.respData[SupportedVerOffset + (SupportedVerCnt * 2)];

    /*
    std::stringstream stream;
    stream << "0x" << std::hex <<  (uint16_t) maxVer << " ";
    std::cout << "  SpdmDriverHandle::getVersion() done. Use version = " << stream.str() << std::endl;
    */
    return true;
}

bool SpdmDriverHandle::getCapability(const std::uint8_t ver, std::uint32_t& cap)
{
    const std::uint8_t GET_CAPABILITY_REQ_CODE = 0xE1;
    const std::uint8_t CT_EXPONENT = 0x16;
    std::uint32_t CAPABILITY_FLAG = 0x00;

    std::cout << "SpdmDriverHandle::getCapability()" << std::endl;
    MCTPPacketWrap* dataPack = nullptr;

    switch(ver) {
        case 0x10:
            {
                if (dataPack == nullptr) {
                     MCTPPacket<struct GetCapabilityReq_V10>* getCapReqPacket = new MCTPPacket<struct GetCapabilityReq_V10>(false, MCTP_MSG_TYPE_SPDM);
                    struct GetCapabilityReq_V10* req = getCapReqPacket->getReqData();
                    if (req == nullptr)
                        return false;
                    req->SPDMVersion = ver;
                    req->RequestResponseCode = GET_CAPABILITY_REQ_CODE;
                    dataPack = getCapReqPacket;
                    break;
                }
                return false;

            }
        case 0x11:
            {
                CAPABILITY_FLAG = CERT_CAP|CHAL_CAP|ENCRYPT_CAP|MAC_CAP|MUT_AUTH_CAP|KEY_EX_CAP|ENCAP_CAP|HBEAT_CAP|KEY_UPD_CAP;
                CAPABILITY_FLAG |= 0x00000400; // PSK_CAP = 1
                if (dataPack == nullptr) {
                     MCTPPacket<struct GetCapabilityReq_V11>* getCapReqPacket = new MCTPPacket<struct GetCapabilityReq_V11>(false, MCTP_MSG_TYPE_SPDM);
                    struct GetCapabilityReq_V11* req = getCapReqPacket->getReqData();
                    if (req == nullptr)
                        return false;
                    req->SPDMVersion = ver;
                    req->RequestResponseCode = GET_CAPABILITY_REQ_CODE;
                    req->CTExponent = CT_EXPONENT;
                    // req->Flags = CAPABILITY_FLAG;
                    req->Flags = CAPABILITY_FLAG;   //FIXME:  Set to zero as Samsung return InvalidRequest when flas is non zero.
                    dataPack = getCapReqPacket;
                    break;
                }
                return false;
            }
        case 0x12:
        case 0x13:
        case 0x14:
            {
                // Case 2.4 in https://github.com/DMTF/SPDM-Responder-Validator/blob/main/doc/2.Capabilities.md
                CAPABILITY_FLAG = CERT_CAP|CHAL_CAP|MUT_AUTH_CAP|KEY_EX_CAP|ENCAP_CAP|HBEAT_CAP|KEY_UPD_CAP;
                CAPABILITY_FLAG |= 0x00000400; // PSK_CAP = 1
                CAPABILITY_FLAG = 0x0;
                if (dataPack == nullptr) {
                    MCTPPacket<struct GetCapabilityReq_V12>* getCapReqPacket = new MCTPPacket<struct GetCapabilityReq_V12>(false, MCTP_MSG_TYPE_SPDM);

                    struct GetCapabilityReq_V12* req = getCapReqPacket->getReqData();
                    if (req == nullptr)
                        return false;
                    req->SPDMVersion = ver;
                    req->RequestResponseCode = GET_CAPABILITY_REQ_CODE;
                    req->CTExponent = CT_EXPONENT;
                    // req->Flags = CAPABILITY_FLAG;
                    req->Flags = CAPABILITY_FLAG; //FIXME:  Set to zero as Samsung return InvalidRequest when flas is non zero.
                    req->DataTransferSize = 64;
                    req->MaxSPDMmsgSize = 2000;
                    dataPack = getCapReqPacket;
                    break;
                }
                return false;
            }
        default:
            {
                return false;
            }
    }

    if (!_mctpEp->doSendReceive(dataPack) || !isNormalResponse(dataPack->respData, dataPack->respSize))
    {
        delete dataPack;
        return false;
    }

    if (dataPack->respSize < 12)
        return false;

    if (dataPack->respData[8] & 0x02)
        std::cout << "    B0.1 CERT_CAP" << std::endl;
    if (dataPack->respData[8] & 0x04)
        std::cout << "    B0.2 CHAL_CAP (DEPRECATED since v1.2)" << std::endl;
    if (dataPack->respData[8] & 0x40)
        std::cout << "    B0.6 ENCRYPT_CAP" << std::endl;
    if (dataPack->respData[8] & 0x80)
        std::cout << "    B0.7 MAC_CAP" << std::endl;


    if (dataPack->respData[9] & 0x01)
        std::cout << "    B1.0 MUT_AUTH_CAP" << std::endl;
    if (dataPack->respData[9] & 0x02)
        std::cout << "    B1.1 KEY_EX_CAP" << std::endl;
    const std::uint16_t psk_cap = (dataPack->respData[9] & 0x0C) >> 3;
    std::cout << "    B1[2..3] PSK_CAP: " << psk_cap << std::endl;
    if (dataPack->respData[9] & 0x10)
        std::cout << "    B1.4 ENCAP_CAP" << std::endl;
    if (dataPack->respData[9] & 0x20)
        std::cout << "    B1.5 HBEAT_CAP" << std::endl;
    if (dataPack->respData[9] & 0x40)
        std::cout << "    B1.6 KEY_UPD_CAP" << std::endl;
    if (dataPack->respData[9] & 0x80)
        std::cout << "    B1.7 HANDSHAKE_IN_THE_CLEAR_CAP" << std::endl;

    if (dataPack->respData[10] & 0x01)
        std::cout << "    B2.0 PUB_KEY_ID_CAP" << std::endl;
    if (dataPack->respData[10] & 0x02)
        std::cout << "    B2.1 CHUNK_CAP" << std::endl;
    const std::uint16_t ep_info_cap = (dataPack->respData[10] & 0xC0) >> 6;
    std::cout << "    B2[6..7] EP_INFO_CAP: " << ep_info_cap << std::endl;

    if (dataPack->respData[11] & 0x02)
        std::cout << "    B3.1 EVENT_CAP" << std::endl;
    const std::uint16_t multi_key_cap = (dataPack->respData[11] & 0x0C) >> 3;
    std::cout << "    B3[2..3] MULTI_KEY_CAP: " << multi_key_cap << std::endl;
    if (dataPack->respData[11] & 0x80)
        std::cout << "    B3.7 LARGE_RESP_CAP" << std::endl;

    //cap = (std::uint32_t) dataPack->respData[11] + ((std::uint32_t) dataPack->respData[10] << 8) + ((std::uint32_t) dataPack->respData[9] << 16) + ((std::uint32_t) dataPack->respData[8] << 24);
    cap = (std::uint32_t) dataPack->respData[8] + (dataPack->respData[9] << 8) + (dataPack->respData[10] << 16) + (dataPack->respData[11] << 24);
    std::cout << "  SpdmDriverHandle::getCapability() done" << std::endl;
    delete dataPack;
    return true;
}

bool SpdmDriverHandle::negotiateAlgo(const std::uint8_t ver) {
    const std::uint8_t NEGOTIATE_ALGORITHMS_REQ_CODE = 0xE3;

    std::cout << "SpdmDriverHandle::negotiateAlgo() Ver = " << (std::uint16_t) ver << std::endl;
    MCTPPacketWrap* dataPack = nullptr;
    switch(ver) {
        case 0x12:
            {
                if (dataPack == nullptr) {
                    MCTPPacket<struct NegotiateAlgorithmsReq_V12>* negotiateAlgoReqPacket = new MCTPPacket<struct NegotiateAlgorithmsReq_V12>(false, MCTP_MSG_TYPE_SPDM);
                    struct NegotiateAlgorithmsReq_V12* req = negotiateAlgoReqPacket->getReqData();
                    if (req == nullptr)
                        return false;
                    dataPack = negotiateAlgoReqPacket;

                    req->SPDMVersion = ver;
                    req->RequestResponseCode = NEGOTIATE_ALGORITHMS_REQ_CODE;
                    req->Param1 = sizeof(req->fras)/sizeof(struct FixedReqAlgStruct);
                    req->Param2 = 0;
                    req->Length = sizeof(struct NegotiateAlgorithmsReq_V12) - 1; //req->type is not included in Length;
                    req->MeasurementSpecification = 0x01;
                    req->OtherParamsSupport = 0x02;
                    req->BaseAsymAlgo[0] = 0xff;
                    req->BaseAsymAlgo[1] = 0x0f;
                    req->BaseAsymAlgo[2] = 0x00;
                    req->BaseAsymAlgo[3] = 0x00;
                    req->BaseHashAlgo[0] = 0x7f;
                    req->BaseHashAlgo[1] = 0x00;
                    req->BaseHashAlgo[2] = 0x00;
                    req->BaseHashAlgo[3] = 0x00;
                    // req->Reserved
                    req->ExtAsymCount = 0;
                    req->ExtHashCount = 0;
                    req->fras[0].AlgType = 2;
                    req->fras[0].AlgCount = 0x20;
                    req->fras[0].AlgSupported[0] = 0x7f;
                    req->fras[0].AlgSupported[1] = 0x00;
                    req->fras[1].AlgType = 3;
                    req->fras[1].AlgCount = 0x20;
                    req->fras[1].AlgSupported[0] = 0x0f;
                    req->fras[1].AlgSupported[1] = 0x00;
                    req->fras[2].AlgType = 4;
                    req->fras[2].AlgCount = 0x20;
                    req->fras[2].AlgSupported[0] = 0xff;
                    req->fras[2].AlgSupported[1] = 0x0f;
                    req->fras[3].AlgType = 5;
                    req->fras[3].AlgCount = 0x20;
                    req->fras[3].AlgSupported[0] = 0x01;
                    req->fras[3].AlgSupported[1] = 0x00;
                    break;
                }
                return false;
            }
        default:
            return false;
    };
    if (!_mctpEp->doSendReceive(dataPack) || !isNormalResponse(dataPack->respData, dataPack->respSize))
    {
        delete dataPack;
        return false;
    }
    delete dataPack;
    return true;
}

bool SpdmDriverHandle::getDigest(const std::uint8_t ver) {
    std::cout << "SpdmDriverHandle::getDigest()" << std::endl;
    MCTPPacket<struct SPDMGeneralReq> dataPack(false, MCTP_MSG_TYPE_SPDM);
    struct SPDMGeneralReq* req = dataPack.getReqData();
    req->SPDMVersion = ver;
    req->RequestResponseCode = 0x81;
    req->Param1 = 0;
    req->Param2 = 0;
    if (!_mctpEp->doSendReceive(&dataPack))
        return false;
    if (!isNormalResponse(dataPack.respData, dataPack.respSize))
        return false;
    return true;
}

bool SpdmDriverHandle::getCertificate(const std::uint8_t ver) {
    std::cout << "SpdmDriverHandle::getCertificate()" << std::endl;
    std::uint16_t offset = 0x0;
    std::uint16_t remainSize = 0xFFFF;
    const uint16_t DEFAULT_PORTION_LENGTH = 50;
    std::uint16_t portionLength = DEFAULT_PORTION_LENGTH;
    const std::string filename = "/tmp/CertSPDM_Net" + std::to_string(_mctpEp->getNet()) + "_EID" + std::to_string(_mctpEp->getEId());

    try {
        std::ofstream outputFile(filename, std::ios::binary);
        if (outputFile.is_open()) {
            try {
                while (remainSize != 0) {
                    MCTPPacket<struct GetCertificateReq> dataPack(false, MCTP_MSG_TYPE_SPDM);
                    struct GetCertificateReq* req = dataPack.getReqData();
                    // dataPack.verboseLog = true; // Example about how to enable the dump of raw data for send and receive message of this packet.

                    req->SPDMVersion = ver;
                    req->RequestResponseCode = 0x82;
                    req->Param1 = 0;
                    req->Param2 = 0;
                    req->Offset = offset;
                    req->Size = portionLength;

                    if (!_mctpEp->doSendReceive(&dataPack))
                        throw std::runtime_error("doSendReceive retrun false");
                    if (!isNormalResponse(dataPack.respData, dataPack.respSize))
                        throw std::runtime_error("Abnormal SPDM response.");
                    if (dataPack.respData[0x01] != 0x02)
                        throw std::runtime_error("Return invalid format.");

                    struct GetCertificateResp* resp = (struct GetCertificateResp*) dataPack.respData;
                    outputFile.write(reinterpret_cast<char*>(&resp->CertChain), resp->PortionLength);
                    remainSize = resp->RemainderLength;
                    offset += resp->PortionLength;
                    portionLength = (portionLength < DEFAULT_PORTION_LENGTH) ? portionLength : DEFAULT_PORTION_LENGTH;
                }
                std::cout << "  Completed the download for certificate." << std::endl;
                outputFile.close();
                return true;
            } catch (const std::runtime_error& e) {
                std::cerr << "  Caught exception: " << e.what() << std::endl;
                outputFile.close();
                return false;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "  Caught exception: " << e.what() << std::endl;
        return false;
    }
    return false;
}

bool SpdmDriverHandle::challenge(const std::uint8_t ver) {
    std::cout << "SpdmDriverHandle::getChallenge()" << std::endl;
    MCTPPacket<struct GetChallengeReq> dataPack(false, MCTP_MSG_TYPE_SPDM);
    struct GetChallengeReq* req = dataPack.getReqData();
    const std::uint8_t MEASUREMENT_ALL = 0xff;
    const std::uint8_t MEASUREMENT_TCB = 0x01;
    const std::uint8_t MEASUREMENT_NONE = 0x00;

    req->SPDMVersion = ver;
    req->RequestResponseCode = 0x83;
    req->Param1 = 0;
    req->Param2 = MEASUREMENT_ALL;
    if (!_mctpEp->doSendReceive(&dataPack))
        return false;
    if (!isNormalResponse(dataPack.respData, dataPack.respSize)) {
        if ((dataPack.respData[1] == 0x7F) && (dataPack.respData[2] == 0x0F)){
            // Receive Large Message Response
            MCTPPacket<struct SPDMGeneralReq> dataPackLMR(false, MCTP_MSG_TYPE_SPDM);
            struct SPDMGeneralReq* reqLMR = dataPackLMR.getReqData();

            reqLMR->SPDMVersion = ver;
            reqLMR->RequestResponseCode = 0x86;
            reqLMR->Param1 = 0;
            reqLMR->Param2 = dataPack.respData[4];
            if (!_mctpEp->doSendReceive(&dataPackLMR))
                return false;

            return true;
        } else {
            return false;
        }
    }
    return true;
}

};
