#ifndef EUDAQ_INCLUDED_RunControl
#define EUDAQ_INCLUDED_RunControl

#include "eudaq/TransportServer.hh"
#include "eudaq/Logger.hh"
#include "eudaq/Status.hh"
#include <pthread.h>
#include <string>

namespace eudaq {

  /** Implements the functionality of the Run Control application.
   *
   */
  class RunControl {
  public:
    explicit RunControl(const std::string & listenaddress = "");

    void StartServer(const std::string & listenaddress);
    void StopServer();

    void Configure(const std::string & settings); ///< Send 'Configure' command with settings
    void Reset();     ///< Send 'Reset' command
    void GetStatus();    ///< Send 'Status' command to get status
    void StartRun(const std::string & msg = "");  ///< Send 'StartRun' command with run number
    void StopRun();   ///< Send 'StopRun' command
    void Terminate(); ///< Send 'Terminate' command

    virtual void OnConnect(const ConnectionInfo & /*id*/) {}
    virtual void OnDisconnect(const ConnectionInfo & /*id*/) {}
    virtual void OnReceive(const ConnectionInfo & /*id*/, counted_ptr<Status>) {}
    virtual ~RunControl();

    void CommandThread();
    size_t NumConnections() const { return m_transport->NumConnections(); }
    const ConnectionInfo & GetConnection(size_t i) const { return m_transport->GetConnection(i); }
  private:
    void InitLog(const ConnectionInfo & id);
    void InitData(const ConnectionInfo & id);
    void InitOther(const ConnectionInfo & id);
    void SendCommand(const std::string & cmd, const std::string & param = "",
                     const ConnectionInfo & id = ConnectionInfo::ALL);
    std::string SendReceiveCommand(const std::string & cmd, const std::string & param = "",
                                   const ConnectionInfo & id = ConnectionInfo::ALL);
    void CommandHandler(TransportEvent & ev);
    bool m_done;
    bool m_listening;
  protected:
    unsigned m_runnumber;   ///< The current run number
  private:
    TransportServer * m_transport; ///< Transport for sending commands
    pthread_t m_thread;
    pthread_attr_t m_threadattr;
    size_t m_idata, m_ilog;
    std::string m_dataaddr, m_logaddr;
  };

}

#endif // EUDAQ_INCLUDED_RunControl