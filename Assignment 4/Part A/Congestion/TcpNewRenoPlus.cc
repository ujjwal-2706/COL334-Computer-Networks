#include "TcpNewRenoPlus.h"
// #include "tcp-socket-base.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpNewRenoPlus");

NS_OBJECT_ENSURE_REGISTERED (TcpNewRenoPlus);

TypeId
TcpNewRenoPlus::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpNewRenoPlus")
    .SetParent<TcpNewReno> ()
    .AddConstructor<TcpNewRenoPlus> ()
    .SetGroupName ("Internet")
  ;
  return tid;
}

TcpNewRenoPlus::TcpNewRenoPlus (void) : TcpNewReno ()
{
  NS_LOG_FUNCTION (this);
}

TcpNewRenoPlus::TcpNewRenoPlus (const TcpNewRenoPlus& sock) : TcpNewReno (sock)
{
  NS_LOG_FUNCTION (this);
}

TcpNewRenoPlus::~TcpNewRenoPlus (void)
{
}

uint32_t
TcpNewRenoPlus::SlowStart (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  if (segmentsAcked >= 1)
    {
      tcb->m_cWnd += pow(tcb->m_segmentSize,1.91)/(tcb->m_cWnd);
      NS_LOG_INFO ("In SlowStart, updated to cwnd " << tcb->m_cWnd << " ssthresh " << tcb->m_ssThresh);
      return segmentsAcked - 1;
    }

  return 0;
}

void
TcpNewRenoPlus::CongestionAvoidance (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  if (segmentsAcked > 0)
    {
      tcb->m_cWnd += 0.51*(tcb->m_segmentSize);
      NS_LOG_INFO ("In CongAvoid, updated to cwnd " << tcb->m_cWnd <<
                   " ssthresh " << tcb->m_ssThresh);
    }
}

void
TcpNewRenoPlus::IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  if (tcb->m_cWnd < tcb->m_ssThresh)
    {
      segmentsAcked = SlowStart (tcb, segmentsAcked);
    }

  if (tcb->m_cWnd >= tcb->m_ssThresh)
    {
      CongestionAvoidance (tcb, segmentsAcked);
    }

  /* At this point, we could have segmentsAcked != 0. This because RFC says
   * that in slow start, we should increase cWnd by min (N, SMSS); if in
   * slow start we receive a cumulative ACK, it counts only for 1 SMSS of
   * increase, wasting the others.
   *
   * // Incorrect assert, I am sorry
   * NS_ASSERT (segmentsAcked == 0);
   */
}

std::string
TcpNewRenoPlus::GetName () const
{
  return "TcpNewRenoPlus";
}

uint32_t
TcpNewRenoPlus::GetSsThresh (Ptr<const TcpSocketState> state,
                         uint32_t bytesInFlight)
{
  NS_LOG_FUNCTION (this << state << bytesInFlight);

  return std::max (2 * state->m_segmentSize, bytesInFlight / 2);
}

Ptr<TcpCongestionOps>
TcpNewRenoPlus::Fork ()
{
  return CopyObject<TcpNewRenoPlus> (this);
}

}
