#ifndef TCPNEWRENOPLUS_H
#define TCPNEWRENOPLUS_H

#include "tcp-congestion-ops.h"

namespace ns3 {

class TcpNewRenoPlus : public TcpNewReno
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  TcpNewRenoPlus ();

  /**
   * \brief Copy constructor.
   * \param sock object to copy.
   */
  TcpNewRenoPlus (const TcpNewRenoPlus& sock);

  ~TcpNewRenoPlus ();

  std::string GetName () const;

  virtual void IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);
  virtual uint32_t GetSsThresh (Ptr<const TcpSocketState> tcb,
                                uint32_t bytesInFlight);

  virtual Ptr<TcpCongestionOps> Fork ();

protected:
  virtual uint32_t SlowStart (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);
  virtual void CongestionAvoidance (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);
};
}
#endif