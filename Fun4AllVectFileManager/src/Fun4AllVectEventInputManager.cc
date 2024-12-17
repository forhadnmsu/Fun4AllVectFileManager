 #include "Fun4AllVectEventInputManager.h"
 #include <interface_main/SQHit_v1.h>
 #include <interface_main/SQHitVector_v1.h>
 #include <interface_main/SQEvent_v1.h>
 #include <interface_main/SQRun_v1.h>
 #include <interface_main/SQSpill_v2.h>
 #include <interface_main/SQSpillMap_v1.h>
 #include <interface_main/SQStringMap.h>
 #include <interface_main/SQScaler_v1.h>
 #include <interface_main/SQSlowCont_v1.h>
 #include <fun4all/Fun4AllServer.h>
 #include <fun4all/Fun4AllSyncManager.h>
 #include <fun4all/Fun4AllReturnCodes.h>
 #include <fun4all/Fun4AllUtils.h>
 #include <fun4all/PHTFileServer.h>
 #include <ffaobjects/RunHeader.h>
 #include <ffaobjects/SyncObjectv2.h>
 #include <phool/getClass.h>
 #include <phool/PHCompositeNode.h>
 #include <phool/PHDataNode.h>
 #include <phool/recoConsts.h>
 #include <cstdlib>
 #include <memory>
 #include <TFile.h>
 #include <TTree.h>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

//Constructor
Fun4AllVectEventInputManager::Fun4AllVectEventInputManager(const std::string& name, const std::string& topnodename):
   Fun4AllInputManager(name, ""),
   segment(-999),
   isopen(0),
   events_total(0),
   events_thisfile(0),
   topNodeName(topnodename),
   _tree_name("save"),
   run_header(nullptr),
   spill_map(nullptr),
   event_header(nullptr),
   hit_vec(nullptr),
   trig_hit_vec(nullptr),
   _fin(nullptr),
   _tin(nullptr)
{
   Fun4AllServer* se = Fun4AllServer::instance();
   topNode = se->topNode(topNodeName.c_str());
   PHNodeIterator iter(topNode);
  
   PHCompositeNode* runNode = static_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "RUN"));
   if (!runNode) {
     runNode = new PHCompositeNode("RUN");
     topNode->addNode(runNode);
   }
  
   PHCompositeNode* eventNode = static_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
   if (!eventNode) {
     eventNode = new PHCompositeNode("DST");
     topNode->addNode(eventNode);
   }
  
   syncobject = new SyncObjectv2();
}

Fun4AllVectEventInputManager::~Fun4AllVectEventInputManager()
 {
   if(isopen)
   {
     fileclose();
   }
   delete syncobject;
 }

void Fun4AllVectEventInputManager::enable_E1039_translation()
 {
     Fun4AllServer* se = Fun4AllServer::instance();
     topNode = se->topNode(topNodeName.c_str());
  
     PHNodeIterator iter(topNode);
     PHCompositeNode* runNode = static_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "RUN"));
     PHCompositeNode* eventNode = static_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
  
     run_header = new SQRun_v1();
     PHIODataNode<PHObject>* runHeaderNode = new PHIODataNode<PHObject>(run_header, "SQRun", "PHObject");
     runNode->addNode(runHeaderNode);
   
     spill_map = new SQSpillMap_v1();
     PHIODataNode<PHObject>* spillNode = new PHIODataNode<PHObject>(spill_map, "SQSpillMap", "PHObject");
     runNode->addNode(spillNode);
  
     event_header = new SQEvent_v1();
     PHIODataNode<PHObject>* eventHeaderNode = new PHIODataNode<PHObject>(event_header, "SQEvent", "PHObject");
     eventNode->addNode(eventHeaderNode);
  
     hit_vec = new SQHitVector_v1();
     PHIODataNode<PHObject>* hitNode = new PHIODataNode<PHObject>(hit_vec, "SQHitVector", "PHObject");
     eventNode->addNode(hitNode);
  
     trig_hit_vec = new SQHitVector_v1();
     PHIODataNode<PHObject>* triggerhitNode = new PHIODataNode<PHObject>(trig_hit_vec, "SQTriggerHitVector", "PHObject");
     eventNode->addNode(triggerhitNode);
 }

void Fun4AllVectEventInputManager::VectToE1039() {
	// Example: Initialize default triggers
	event_header->set_run_id(RunID);
	SQSpill* spill = spill_map->get(SpillID);
	if(!spill) {
		spill = new SQSpill_v2();
		spill->set_spill_id(SpillID);
		spill->set_run_id(RunID);

		spill_map->insert(spill);
		run_header->set_n_spill(spill_map->size());
	}
	event_header->set_spill_id(SpillID);
	event_header->set_event_id(EventID);
	event_header->set_data_quality(0);
	for(int i = -16; i < 16; ++i) {event_header->set_qie_rf_intensity(i, Intensity[i+16]);} // need to be added
	//for(int i = -16; i < 16; ++i) {cout << "intensity: "<< Intensity[i+16] <<endl;} // need to be added
	


	if (event_header) {
		// Apply the FPGA triggers to the event header
		event_header->set_trigger(SQEvent::MATRIX1, fpga_triggers[0]);
		event_header->set_trigger(SQEvent::MATRIX2, fpga_triggers[1]);
		event_header->set_trigger(SQEvent::MATRIX3, fpga_triggers[2]);
		event_header->set_trigger(SQEvent::MATRIX4, fpga_triggers[3]);
		event_header->set_trigger(SQEvent::MATRIX5, fpga_triggers[4]);
		// Apply the NIM triggers to the event header
		event_header->set_trigger(SQEvent::NIM1, nim_triggers[0]);
		event_header->set_trigger(SQEvent::NIM2, nim_triggers[1]);
		event_header->set_trigger(SQEvent::NIM3, nim_triggers[2]);
		event_header->set_trigger(SQEvent::NIM4, nim_triggers[3]);
		event_header->set_trigger(SQEvent::NIM5, nim_triggers[4]);
	}
	//we need to fill from all hits vector
	for (size_t i = 0; i < ElementID->size(); ++i) {
		SQHit* hit = new SQHit_v1();
		hit->set_hit_id(i);
		hit->set_detector_id(DetectorID->at(i));
		hit->set_element_id(ElementID->at(i));
		hit->set_tdc_time(TdcTime->at(i));
		hit->set_drift_distance(DriftDistance->at(i));
		hit->set_in_time(hit_in_time->at(i));
		//cout << " hit in time: "<< hit_in_time->at(i) <<endl;
		//cout << "drift dis: " << DriftDistance->at(i) <<endl;
		hit_vec->push_back(hit);
	}

	//we need to fill from the trig-hits vector (need to work on this later)
	for (size_t i = 0; i < Trig_ElementID->size(); ++i) {
		SQHit* hit = new SQHit_v1();
		hit->set_hit_id(i);
		hit->set_detector_id(Trig_DetectorID->at(i));
		hit->set_element_id(Trig_ElementID->at(i));
		hit->set_tdc_time( Trig_TdcTime->at(i));
		hit->set_drift_distance(Trig_DriftDistance->at(i));
		hit->set_in_time(Trig_hit_in_time->at(i));
		trig_hit_vec->push_back(hit);
	}
}

int Fun4AllVectEventInputManager::fileopen(const std::string &filenam) {
    if (isopen) {
        std::cout << "Closing currently open file "
                  << filename
                  << " and opening " << filenam << std::endl;
        fileclose();
    }   
    filename = filenam;

    // Print the absolute path for debugging
    std::cout << "Attempting to open file: " << std::filesystem::absolute(filenam) << std::endl;

    if (!std::filesystem::exists(filenam)) {
        std::cerr << "!!ERROR!! File does not exist: " << filenam << std::endl;
        return -1; 
    }   

    if (verbosity > 0) {
        std::cout << ThisName << ": opening file " << filename.c_str() << std::endl;
    }   

    events_thisfile = 0;

    _fin = TFile::Open(filenam.c_str(), "READ"); // Open the file dynamically
    if (!_fin || _fin->IsZombie()) {
        std::cerr << "!!ERROR!! Failed to open file " << filenam << std::endl;
        return -1;
    }

    _tin = (TTree*) _fin->Get(_tree_name.c_str());
    if (!_tin) {
        std::cerr << "!!ERROR!! Tree " << _tree_name << " not found in file " << filenam << std::endl;
        return -1; 
    }

    // Set branch addresses for scalar and vector branches
    _tin->SetBranchAddress("EventID", &EventID);           
    _tin->SetBranchAddress("RunID", &RunID);               
    _tin->SetBranchAddress("SpillID", &SpillID);           
    _tin->SetBranchAddress("fpga_triggers", fpga_triggers);
    _tin->SetBranchAddress("nim_triggers", nim_triggers);
    _tin->SetBranchAddress("Intensity", Intensity);

    _tin->SetBranchAddress("DetectorID", &DetectorID);     
    _tin->SetBranchAddress("ElementID", &ElementID);       
    _tin->SetBranchAddress("DriftDistance", &DriftDistance);       
    _tin->SetBranchAddress("TdcTime", &TdcTime);           
    _tin->SetBranchAddress("hit_in_time", &hit_in_time);           

    _tin->SetBranchAddress("Trig_DriftDistance", &Trig_DriftDistance);    
    _tin->SetBranchAddress("Trig_ElementID", &Trig_ElementID);    
    _tin->SetBranchAddress("Trig_TdcTime", &Trig_TdcTime);    
    _tin->SetBranchAddress("Trig_hit_in_time", &Trig_hit_in_time); 

    segment = 0;
    isopen = 1;
    AddToFileOpened(filenam); // Add file to the list of opened files
    return 0;
}

int Fun4AllVectEventInputManager::run(const int nevents) {
    readagain:
    if (!isopen) {
        if (filelist.empty()) {
            if (verbosity > 0) {
                std::cout << Name() << ": No Input file open" << std::endl;
            }
            return -1;
        } else {
            if (OpenNextFile()) {
                std::cout << Name() << ": No Input file from filelist opened" << std::endl;
                return -1;
            }
        }
    }

    if (verbosity > 3) {
        std::cout << "Getting Event from " << Name() << std::endl;
    }

    if (events_thisfile >= _tin->GetEntries()) {
        fileclose();
        goto readagain;
    }

    _tin->GetEntry(events_thisfile);
    events_thisfile++;
    events_total++;
    //std::cout << "EventID: "<< EventID<<std::endl;
    //std::cout << "Fpga1 trigger: "<<fpga_triggers[0] <<std::endl;

   SetRunNumber                (RunID);
   mySyncManager->PrdfEvents   (events_thisfile);
   mySyncManager->SegmentNumber(SpillID);
   mySyncManager->CurrentEvent (EventID);
  
   syncobject->RunNumber       (RunID);
   syncobject->EventCounter    (events_thisfile);
   syncobject->SegmentNumber   (SpillID);
   syncobject->EventNumber     (EventID);
      VectToE1039();
    if (RejectEvent() != Fun4AllReturnCodes::EVENT_OK) {
        ResetEvent();
        goto readagain;
    }
    return 0;
}

int Fun4AllVectEventInputManager::ResetEvent()
{
    syncobject->Reset();
    return 0;
}

int Fun4AllVectEventInputManager::fileclose()
 {
         if (!isopen) {
                 cout << Name() << ": fileclose: No Input file open" << endl;
                 return -1;
         }
  
         _fin->Close();
         isopen = 0;
  
         // if we have a file list, move next entry to top of the list
         // or repeat the same entry again
         if (!filelist.empty()) {
                 if (repeat) {
                         filelist.push_back(*(filelist.begin()));
                         if (repeat > 0) {
                                 repeat--;
                         }
                 }
                 filelist.pop_front();
         }
  
         return 0;
 }
  
 void
 Fun4AllVectEventInputManager::Print(const string &what) const
 {
   Fun4AllInputManager::Print(what);
   return ;
 }
  
 int
 Fun4AllVectEventInputManager::OpenNextFile()
 {
   while (!filelist.empty())
     {
       list<string>::const_iterator iter = filelist.begin();
       if (verbosity)
         {
           cout << PHWHERE << " opening next file: " << *iter << endl;
         }
       if (fileopen(*iter))
         {
           cout << PHWHERE << " could not open file: " << *iter << endl;
           filelist.pop_front();
         }
       else
         {
           return 0;
         }
  
     }
   return -1;
 }
 
 int Fun4AllVectEventInputManager::PushBackEvents(const int i)
 {
   cerr << "!!ERROR!!  PushBackEvents():  Not implemented yet." << endl;
   // PushBackEvents is supposedly pushing events back on the stack which works
   // easily with root trees (just grab a different entry) but hard in these HepMC ASCII files.
   // A special case is when the synchronization fails and we need to only push back a single
   // event. In this case we save the evt pointer as save_evt which is used in the run method
   // instead of getting the next event.
 //  if (i > 0)
 //    {
 //      if (i == 1 && evt) // check on evt pointer makes sure it is not done from the cmd line
 //      {
 //        save_evt = evt;
 //        return 0;
 //      }
 //      cout << PHWHERE << ThisName
 //           << " Fun4AllVectEventInputManager cannot push back " << i << " events into file"
 //           << endl;
 //      return -1;
 //    }
 //  if (!parser->coda)
 //    {
 //      cout << PHWHERE << ThisName
 //         << " no file open" << endl;
 //      return -1;
 //    }
   // Skipping events is implemented as
   // pushing a negative number of events on the stack, so in order to implement
   // the skipping of events we read -i events.
 //  int nevents = -i; // negative number of events to push back -> skip num events
 //  int errorflag = 0;
 //  while (nevents > 0 && ! errorflag)
 //    {
 //                      int * data_ptr = nullptr;
 //                      unsigned int coda_id = 0;
 //                      if(parser->coda->NextCodaEvent(coda_id, data_ptr))
 //                              evt = new EVIO_Event(data_ptr);
 //      if (! evt)
 //      {
 //        cout << "Error after skipping " << i - nevents 
 //             << " file exhausted?" << endl;
 //        errorflag = -1;
 //          fileclose();
 //      }
 //      else
 //      {
 //        if (verbosity > 3)
 //          {
 //                      //TODO implement this
 //            //cout << "Skipping evt no: " << evt->getEvtSequence() << endl;
 //          }
 //      }
 //      delete evt;
 //      nevents--;
 //    }
 //  return errorflag;
   return -1;
 }
  
 int
 Fun4AllVectEventInputManager::GetSyncObject(SyncObject **mastersync)
 {
   // here we copy the sync object from the current file to the
   // location pointed to by mastersync. If mastersync is a 0 pointer
   // the syncobject is cloned. If mastersync allready exists the content
   // of syncobject is copied
   if (!(*mastersync))
     {
       if (syncobject) *mastersync = syncobject->clone();
     }
   else
     {
       *(*mastersync) = *syncobject; // copy syncobject content
     }
   return Fun4AllReturnCodes::SYNC_OK;
 }
  
 int
 Fun4AllVectEventInputManager::SyncIt(const SyncObject *mastersync)
 {
   if (!mastersync)
     {
       cout << PHWHERE << Name() << " No MasterSync object, cannot perform synchronization" << endl;
       cout << "Most likely your first file does not contain a SyncObject and the file" << endl;
       cout << "opened by the Fun4AllDstInputManager with Name " << Name() << " has one" << endl;
       cout << "Change your macro and use the file opened by this input manager as first input" << endl;
       cout << "and you will be okay. Fun4All will not process the current configuration" << endl << endl;
       return Fun4AllReturnCodes::SYNC_FAIL;
     }
   int iret = syncobject->Different(mastersync);
   if (iret)
     {
       cout << "big problem" << endl;
       exit(1);
     }
   return Fun4AllReturnCodes::SYNC_OK;
 }
