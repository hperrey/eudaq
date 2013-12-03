#include "eudaq/DataConverterPlugin.hh"
#include "eudaq/StandardEvent.hh"
#include "eudaq/Utils.hh"

// All LCIO-specific parts are put in conditional compilation blocks
// so that the other parts may still be used if LCIO is not available.
#if USE_LCIO
#  include "IMPL/LCEventImpl.h"
#  include "IMPL/TrackerRawDataImpl.h"
#  include "IMPL/LCCollectionVec.h"
#  include "lcio.h"
#endif
#include <iostream>
#define EVENTHEADERSIZE 14
#define MODULEHEADERSIZE 3
#define STREAMHEADERSIZE 3
#define STREAMESIZE 160
#define TOTALHEADERSIZE (EVENTHEADERSIZE+MODULEHEADERSIZE+STREAMHEADERSIZE)
#define STARTSTREAM0 TOTALHEADERSIZE
#define ENDSTREAM0 (STARTSTREAM0+STREAMESIZE)
#define STARTSTREAM1 (TOTALHEADERSIZE+STREAMHEADERSIZE+STREAMESIZE)
#define ENDSTREAM1 (STARTSTREAM1+STREAMESIZE)
#define TOTALMODULSIZE (MODULEHEADERSIZE+2*STREAMHEADERSIZE+2*STREAMESIZE)

void uchar2bool(std::vector<unsigned char>& in,int lOffset,int hOffset, std::vector<bool>& out){
	for (auto i=in.begin()+lOffset;i!=in.begin()+hOffset;++i)
	{
		for(int j=0;j<8;++j){
			out.push_back((*i)&(1<<j));
		}
	}


}

namespace eudaq {

  // The event type for which this converter plugin will be registered
  // Modify this to match your actual event type (from the Producer)
  static const char* EVENT_TYPE = "SCTupgrade";

  // Declare a new class that inherits from DataConverterPlugin
  class SCTupgradeConverterPlugin : public DataConverterPlugin {

    public:

      // This is called once at the beginning of each run.
      // You may extract information from the BORE and/or configuration
      // and store it in member variables to use during the decoding later.
      virtual void Initialize(const Event & bore,
          const Configuration & cnf) {
        m_exampleparam = bore.GetTag("SCTupgrade", 0);
#ifndef WIN32  //some linux Stuff //$$change
		(void)cnf; // just to suppress a warning about unused parameter cnf
#endif
        
      }

      // This should return the trigger ID (as provided by the TLU)
      // if it was read out, otherwise it can either return (unsigned)-1,
      // or be left undefined as there is already a default version.
      virtual unsigned GetTriggerID(const Event & ev) const {
        static const unsigned TRIGGER_OFFSET = 8;
        // Make sure the event is of class RawDataEvent
        if (const RawDataEvent * rev = dynamic_cast<const RawDataEvent *> (&ev)) {
          // This is just an example, modified it to suit your raw data format
          // Make sure we have at least one block of data, and it is large enough

            return rev->GetEventNumber();
          
        }
        // If we are unable to extract the Trigger ID, signal with (unsigned)-1
        return (unsigned)-1;
      }

      // Here, the data from the RawDataEvent is extracted into a StandardEvent.
      // The return value indicates whether the conversion was successful.
      // Again, this is just an example, adapted it for the actual data layout.
      virtual bool GetStandardSubEvent(StandardEvent & sev,
          const Event & ev) const {
        // If the event type is used for different sensors
        // they can be differentiated here
			
        std::string sensortype = "example";
        // Create a StandardPlane representing one sensor plane
        int id = 0;
       
        // Set the number of pixels
		 const RawDataEvent & rawev = dynamic_cast<const RawDataEvent &>(ev);
		 
		 sev.SetTag("DUT_time",rawev.GetTimestamp());
		 int noModules=(rawev.GetBlock(0).size()-EVENTHEADERSIZE)/TOTALMODULSIZE;
	//	 std::cout<<"rawev.GetBlock(0).size(): "<<rawev.GetBlock(0).size()<<" EVENTHEADERSIZE: "<< EVENTHEADERSIZE<<" TOTALMODULSIZE: "<<TOTALMODULSIZE<<std::endl;
	//	 std::cout<<"noModules: "<<noModules<<std::endl;
		// std::cout<<"rawev.GetBlock(0).size(): "<<rawev.GetBlock(0).size()<<"   noModules: "<<noModules<<std::endl;
		 std::vector<unsigned char> inputVector=rawev.GetBlock(0);
		 int y_pos=0;
		 StandardPlane plane(id, EVENT_TYPE, sensortype);
		 for (size_t k=1;k<=noModules;++k)
		 {
		 //Stream 0
			 y_pos=(k-1)*2+1;
		
		 std::vector<bool> outputStream0;
		//std::vector<unsigned char> dumm=rawev.GetBlock(0);
			
			int width = 1280, height = noModules*2;
			plane.SetSizeRaw(width, height);
			//std::cout<<"STARTSTREAM0+(k-1)*TOTALMODULSIZE: "<<STARTSTREAM0+(k-1)*TOTALMODULSIZE<<std::endl;
			uchar2bool(inputVector,STARTSTREAM0
							+(k-1)*TOTALMODULSIZE,
							ENDSTREAM0
							+(k-1)*TOTALMODULSIZE,outputStream0);
			for (size_t i=0; i<outputStream0.size();++i)
			{
			//plane.SetPixel(i,i,1,rawev.GetBlock(0).at(i));
				if (outputStream0.at(i))
				{
					plane.PushPixel(i,y_pos,1);
				}
			
			
			}
		
		
			// Set the trigger ID
			
		 

			//stream 1
		 
		++y_pos;
		 std::vector<bool> outputStream1;
		// std::cout<<"STARTSTREAM1+(k-1)*TOTALMODULSIZE: "<<STARTSTREAM1+(k-1)*TOTALMODULSIZE<< " ENDSTREAM1+(k-1)*TOTALMODULSIZE: "<<ENDSTREAM1+(k-1)*TOTALMODULSIZE<<std::endl;
			uchar2bool(inputVector,STARTSTREAM1+(k-1)*TOTALMODULSIZE,ENDSTREAM1+(k-1)*TOTALMODULSIZE,outputStream1);
		
			
			for (size_t i=0; i<outputStream1.size();++i)
			{
				//plane.SetPixel(i,i,1,rawev.GetBlock(0).at(i));
				if (outputStream1.at(i))
				{
					plane.PushPixel(i,y_pos,1);
				}


			}


	


		 
		}
	//	 auto tlu=plane.TLUEvent();
	//	 std::cout<<"sev.get_id(): "<<sev.get_id()<<"  GetTriggerID(ev): "<<GetTriggerID(ev) <<std::endl;
		 
		 // Set the trigger ID
		 plane.SetTLUEvent(GetTriggerID(ev));
	//	  std::cout<<"sev.get_id(): "<<sev.get_id()<<"  GetTriggerID(ev): "<<GetTriggerID(ev) <<std::endl;
		 // Add the plane to the StandardEvent
		 sev.AddPlane(plane);
		
        // Indicate that data was successfully converted
        return true;
      }

#if USE_LCIO
      // This is where the conversion to LCIO is done
    virtual lcio::LCEvent * GetLCIOEvent(const Event * /*ev*/) const {
        return 0;
      }
#endif

    private:

      // The constructor can be private, only one static instance is created
      // The DataConverterPlugin constructor must be passed the event type
      // in order to register this converter for the corresponding conversions
      // Member variables should also be initialized to default values here.
      SCTupgradeConverterPlugin()
        : DataConverterPlugin(EVENT_TYPE), m_exampleparam(0)
      {}

      // Information extracted in Initialize() can be stored here:
      unsigned m_exampleparam;

      // The single instance of this converter plugin
      static SCTupgradeConverterPlugin m_instance;
  }; // class ExampleConverterPlugin

  // Instantiate the converter plugin instance
  SCTupgradeConverterPlugin SCTupgradeConverterPlugin::m_instance;

} // namespace eudaq
