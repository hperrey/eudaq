%module RunControl
 %{
 /* Includes the header in the wrapper code */
 #include "../include/eudaq/RunControl.hh"
 %}

/* makes SWIG handle Window-isms like __declspec(dllexport) */ 
%include <windows.i>
namespace eudaq {
class RunControl {
    public:
      RunControl();

      void StartServer(const std::string & listenaddress);
      void StopServer();
      void Configure(const std::string & settings, int geoid = 0); ///< Send 'Configure' command with settings
      void Reset();     ///< Send 'Reset' command
      void GetStatus();    ///< Send 'Status' command to get status
      virtual void StartRun(const std::string & msg = "");  ///< Send 'StartRun' command with run number
      virtual void StopRun(bool listen = true);   ///< Send 'StopRun' command
      void Terminate(); ///< Send 'Terminate' command
      size_t NumConnections() const { return m_cmdserver->NumConnections(); }
  };
 }
