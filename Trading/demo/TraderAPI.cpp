// A simple example that describes the use of CApexFtdcTraderApi and CApexFtdcTraderSpi interfaces.
// This example shows the process of order entry operation.

#include <stdio.h>
#include <string>
#include "ApexFtdcTraderApi.h"

class CSimpleHandler : public CApexFtdcTraderSpi {
public:
  CSimpleHandler(CApexFtdcTraderApi *api) : m_pTraderApi(api) {}

  ~CSimpleHandler() {}

  virtual void OnFrontConnected() {

    CApexFtdcReqUserLoginField reqUserLogin{};

    // Get ParticipantID
    printf("participantid:");
    scanf("%s", &m_participantId);
    strcpy(reqUserLogin.ParticipantID, m_participantId);

    // Get UserID
    printf("userid:");
    scanf("%s", &m_userId);
    strcpy(reqUserLogin.UserID, m_userId);

    // Get password
    printf("password:");
    scanf("%s", &reqUserLogin.Password);

    // Send the login request
    m_pTraderApi->ReqUserLogin(&reqUserLogin, 0);
  }

  // This method is called when Member System disconnects. Since the API will try to reconnect automatically, Member System is not required to do anything.
  virtual void OnFrontDisconnected(int nReason) {
    printf("OnFrontDisconnected.\n");
  }

  // After Member System sent the login request, this method is called to notify Member System whether the login is successful or not. 
  virtual void OnRspUserLogin(CApexFtdcRspUserLoginField *pRspUserLogin, CApexFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {

    printf("OnRspUserLogin:\n");
    printf("ErrorCode=[%d], ErrorMsg=[%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);

    if (pRspInfo->ErrorID != 0) {
      // In case of login failure, Member System is required to perform error-processing. 
      printf("Failed to login, errorcode=%d errormsg=%s requestid=%d chain=%d", pRspInfo->ErrorID, pRspInfo->ErrorMsg, nRequestID, bIsLast);
      exit(-1);
    }

    // In case of successful login, send an order entry request. 
    CApexFtdcInputOrderField ord = CreateOrder();

    m_pTraderApi->ReqOrderInsert(&ord, 1);
  }

  // Response to order entry
  virtual void OnRspOrderInsert(CApexFtdcInputOrderField *pInputOrder, CApexFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    printf("OnRspOrderInsert:\n");
    printf("ErrorCode=[%d], ErrorMsg=[%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
  };

  // Return on order
  virtual void OnRtnOrder(CApexFtdcOrderField *pOrder) {
    printf("OnRtnOrder:\n");
    printf("OrderSysID=[%s]\n", pOrder->OrderSysID);
  }

  // Response to erroneous user request
  virtual void OnRspError(CApexFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {

    printf("OnRspError:\n");
    printf("ErrorCode=[%d], ErrorMsg=[%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);

    // Member System is required to perform error-processing
    exit(-1);
  }

  CApexFtdcInputOrderField CreateOrder() {

    CApexFtdcInputOrderField ord{};

    // Member code
    strcpy(ord.ParticipantID, m_participantId);
    // Client code
    strcpy(ord.ClientID, "CLIENT");
    // Transaction user's code
    strcpy(ord.UserID, m_userId);
    // Contract code
    strcpy(ord.InstrumentID, "PF2001");
    // Conditions of order price 
    ord.OrderPriceType = APEX_FTDC_OPT_LimitPrice;
    // Buy-sell direction 
    ord.Direction = APEX_FTDC_D_Buy;
    // Flag of position opening and closing-out in a portfolio
    strcpy(ord.CombOffsetFlag, "0");
    // Flag of speculation and hedge in a portfolio
    strcpy(ord.CombHedgeFlag, "1");
    // Price
    ord.LimitPrice = 540.0;
    // Quantity
    ord.VolumeTotalOriginal = 10;
    // Type of valid period 
    ord.TimeCondition = APEX_FTDC_TC_GFD;
    // GTD DATE
    strcpy(ord.GTDDate, "");
    // Volume type
    ord.VolumeCondition = APEX_FTDC_VC_AV;
    // The Min.volume
    ord.MinVolume = 0;
    // Trigger conditions
    ord.ContingentCondition = APEX_FTDC_CC_Immediately;
    // Stop-loss price
    ord.StopPrice = 0;
    // Reasons for forced closing-out
    ord.ForceCloseReason = APEX_FTDC_FCC_NotForceClose;
    // Local order No.
    strcpy(ord.OrderLocalID, "0000000001");
    // Flag of auto-suspension
    ord.IsAutoSuspend = 0;

    return ord;
  }

private:
  CApexFtdcTraderApi *m_pTraderApi;
  TApexFtdcParticipantIDType m_participantId;
  TApexFtdcUserIDType m_userId;
};

int main() {

  // Create a CApexFtdcTraderApi instance
  CApexFtdcTraderApi *pTraderApi = CApexFtdcTraderApi::CreateFtdcTraderApi("./flow/");

  // Create an event-handling instance
  CSimpleHandler handler(pTraderApi);

  // Register the event-handling instance
  pTraderApi->RegisterSpi(&handler);

  // Subscription of topics
  //   TERT_RESTART: retransmit all messages of the current trading day
  //   TERT_RESUME: retransmit messages by resuming the last transmission 
  //   TERT_QUICK: only transmit messages after login
  pTraderApi->SubscribePublicTopic(APEX_TERT_RESUME);
  pTraderApi->SubscribeUserTopic(APEX_TERT_RESUME);

  // Set heartbeat timeout
  pTraderApi->SetHeartbeatTimeout(10);

  // Registers the NameServers of Trading System
  char *addresses[] = {
    "tcp://123.123.123.123:4901",
    "tcp://123.123.123.123:4902",
  };

  for (int i = 0; i < 2; i++) {
    pTraderApi->RegisterNameServer(addresses[i]);
  }

  // Member System starts to connect to Trading System
  pTraderApi->Init();

  // Release the CApexFtdcTraderApi instance
  pTraderApi->Release();

  return 0;
}
