#ifndef _FUN4ALL_UNIVERSAL_OUTPUT_MANAGER__H_
#define _FUN4ALL_UNIVERSAL_OUTPUT_MANAGER__H_

#include <fun4all/Fun4AllOutputManager.h>
#include <string>
#include <vector>
#include <map>
#include <TStopwatch.h>

class TFile;
class TTree;
class PHCompositeNode;
class SQEvent;
class SQSpillMap;
class SQHitVector;


class Fun4AllVectEventOutputManager : public Fun4AllOutputManager {
public:
    Fun4AllVectEventOutputManager(const std::string &myname = "UNIVERSALOUT");
    virtual ~Fun4AllVectEventOutputManager();

    void SetTreeName(const std::string& name) { m_tree_name = name; }
    void SetFileName(const std::string& name) { m_file_name = name; }
    virtual int Write(PHCompositeNode* startNode);
    TStopwatch timer;
    void ResetBranches();
protected:
    int OpenFile(PHCompositeNode* startNode);
    void CloseFile();

private:
    std::string m_tree_name;
    std::string m_file_name;
    std::string m_dir_base;
    bool m_dimuon_mode;
   
    TFile* m_file;
    TTree* m_tree;

    SQEvent* m_evt;
    SQSpillMap* m_sp_map;
    SQHitVector* m_hit_vec;
    SQHitVector* m_trig_hit_vec;

  int trig_bits;
  int RunID;
  int SpillID;
  int EventID;
  int RFID;
  int TurnID;
  int Intensity[33];
  int fpga_triggers[5] = {0};
  int nim_triggers[5] = {0};


  std::vector<int>    DetectorID;
  std::vector<int>    ElementID;
  std::vector<double> TdcTime;
  std::vector<double> DriftDistance;
  std::vector<bool> hit_in_time;

  std::vector<int>    Trig_DetectorID;
  std::vector<int>    Trig_ElementID;
  std::vector<double> Trig_TdcTime;
  std::vector<double> Trig_DriftDistance;
  std::vector<bool> Trig_hit_in_time;
};

#endif /* _FUN4ALL_VECT_EVENT_OUTPUT_MANAGER__H_ */

