// A simple example to illustrate usage of CApexFtdcMduserApi & CApexFtdcMduserSpi interfaces 
#include <stdio.h>
#include <string.h>
#include "ApexFtdcMduserApi.h"

class CSimpleHandler : public CApexFtdcMduserSpi
{
public:

  //constructed function, requires a valid pointer to point to an instance of CApexFtdcMduserApi
  CSimpleHandler(CApexFtdcMduserApi *pUserApi) : m_pUserApi(pUserApi) {}

  ~CSimpleHandler() {}

  // After the communication connection between Quotation Receiving System and the Trading System is established, Quotation Receiving System would require log in. 
  void OnFrontConnected() {
    CApexFtdcReqUserLoginField reqUserLogin;
    strcpy(reqUserLogin.ParticipantID, "member01");
    strcpy(reqUserLogin.UserID, "user01");
    strcpy(reqUserLogin.Password, "password01");

    m_pUserApi->ReqUserLogin(&reqUserLogin, 0);
  }

  // when the communication connection between Quotation Receiving System and the Trading System is interrupted, this method is called.  
  void OnFrontDisconnected() {
    // when disconnection happens, API would re-connect automatically, Quotation Receiving System does not need to handle.
    printf("OnFrontDisconnected.\n");
  }

  // after the Trading System sends out reply for login request, this method is called to inform Quotation Receiving System whether the login is successful 
  void OnRspUserLogin(CApexFtdcRspUserLoginField *pRspUserLogin, CApexFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    printf("OnRspUserLogin: ErrorCode=[%d], ErrorMsg=[%s]\n",
      pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);
    if (pRspInfo->ErrorID != 0) {
      //login failure, Quotation Receiving System would need to do error handling
      printf("Failed to login, errorcode=%d errormsg=%s requestid=%d chain=%d", pRspInfo->ErrorID, pRspInfo->ErrorMsg, nRequestID, bIsLast);
    }
  }

  //Depth quotation notification, the Trading System would inform automatically. 
  void OnRtnDepthMarketData(CApexFtdcDepthMarketDataField *pMarketData) {
    //Quotion Receiving System would deal with the returned data based on its own need
  }

  //error notification with respect to user request 
  void OnRspError(CApexFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    printf("OnRspError:\n");
    printf("ErrorCode=[%d], ErrorMsg=[%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);
    // Quotation Receiving System would need to do error handling
  }

private:
  // a pointer that points to an instance of CApexFtdcMduserApi
  CApexFtdcMduserApi *m_pUserApi;
};

int main() {
  printf("%s\n", CApexFtdcMduserApi::GetVersion());

  // creats an instance of CApexFtdcTraderApi
  CApexFtdcMduserApi *pUserApi = CApexFtdcMduserApi::CreateFtdcMduserApi();
  // creates an instance of event handling
  CSimpleHandler sh(pUserApi);
  // register to an instance of event handling
  pUserApi->RegisterSpi(&sh);
  // register to required depth quotation topic
  /// TERT_RESTART: to re-transmit from current trading day 
  /// TERT_RESUME: to re-transmit by resuming and continuing from last transmission
  /// TERT_QUICK: first transmit the quotation snapshot, and then transmit all quotation after that	
  pUserApi->SubscribeMarketDataTopic(111, APEX_TERT_RESUME);
  //set the timeout for heartbeat 
  pUserApi->SetHeartbeatTimeout(19);
  // set the Exchange FEP NameServer address
  char *addresses[] = {
    "tcp://123.123.123.123:4903"
  };

  for (int i = 0; i < 1; i++) {
    pUserApi->RegisterNameServer(addresses[i]);
  }

  // starts connection with quotation FEP of the Trading System
  pUserApi->Init();
  // release MduserAPI instance
  pUserApi->Release();
  return 0;
} 