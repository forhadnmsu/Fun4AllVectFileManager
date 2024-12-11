//R__LOAD_LIBRARY(libcalibrator)
//R__LOAD_LIBRARY(libktracker)
//R__LOAD_LIBRARY(libg4detectors)
R__LOAD_LIBRARY(libfun4all_vect_file_manager)
R__LOAD_LIBRARY(libfun4all)
R__LOAD_LIBRARY(libPHPythia8)
R__LOAD_LIBRARY(libg4detectors)
R__LOAD_LIBRARY(libg4testbench)
R__LOAD_LIBRARY(libg4eval)
R__LOAD_LIBRARY(libg4dst)
R__LOAD_LIBRARY(libdptrigger)
R__LOAD_LIBRARY(libembedding)
R__LOAD_LIBRARY(libevt_filter)
R__LOAD_LIBRARY(libktracker)
R__LOAD_LIBRARY(libSQPrimaryGen)
R__LOAD_LIBRARY(libcalibrator)


int Fun4Sim_reco_data2024(const int n_evt = 100){
  const double FMAGSTR = -1.044;
  const double KMAGSTR = -1.025;
  const double target_coil_pos_z = -300;
  const int nmu = 1;
  int embedding_opt = 0;
  const bool legacy_rec_container = true;

  const bool do_collimator = true;
  const bool do_target = true;
  const bool do_e1039_shielding = true;
  const bool do_fmag = true;
  const bool do_kmag = true;
  const bool do_absorber = true;
  const bool do_dphodo = true;
  const bool do_station1DC = false;       //station-1 drift chamber should be turned off by default

  recoConsts *rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER", 5433); /// The geometry is selected based on run number.
  rc->set_DoubleFlag("FMAGSTR", FMAGSTR);
  rc->set_DoubleFlag("KMAGSTR", KMAGSTR);
  rc->set_DoubleFlag("SIGX_BEAM", 0.3);
  rc->set_DoubleFlag("SIGY_BEAM", 0.3);
  rc->set_DoubleFlag("Z_UPSTREAM", -700.);



  /*  
  rc->set_CharFlag("AlignmentMille", "config/align_mille_v10_a.txt");
  rc->set_CharFlag("AlignmentHodo", "");
  rc->set_CharFlag("AlignmentProp", "");
  rc->set_CharFlag("Calibration", "");
  rc->set_IntFlag ("MaxHitsDC0" , int(350)); // /3.0
  rc->set_IntFlag ("MaxHitsDC1" , int(350)); // /3.0
  rc->set_IntFlag ("MaxHitsDC2" , int(170)); // /3.0
  rc->set_IntFlag ("MaxHitsDC3p", int(140)); // /3.0
  rc->set_IntFlag ("MaxHitsDC3m", int(140)); // /3.0
  rc->set_DoubleFlag("RejectWinDC0" , 0.3);
  rc->set_DoubleFlag("RejectWinDC1" , 0.5);
  rc->set_DoubleFlag("RejectWinDC2" , 0.35);
  rc->set_DoubleFlag("RejectWinDC3p", 0.24);
  rc->set_DoubleFlag("RejectWinDC3m", 0.24);
  */
  Fun4AllServer* se = Fun4AllServer::instance();
  se->Verbosity(100);


  CalibHitElementPos* cal_ele_pos = new CalibHitElementPos();
  cal_ele_pos->CalibTriggerHit(false);
  se->registerSubsystem(cal_ele_pos);

 
  SQReco* reco = new SQReco();
 // reco->Verbosity(100);
  reco->set_legacy_rec_container(false); // default = true
  reco->set_geom_file_name((string)gSystem->Getenv("E1039_RESOURCE") + "/geometry/geom_run005433.root");
  reco->set_enable_KF(true);
  reco->setInputTy(SQReco::E1039);
  reco->setFitterTy(SQReco::KFREF);
  reco->set_evt_reducer_opt("none");
  reco->set_enable_eval_dst(true);
  for (int ii = 0; ii <= 3; ii++) reco->add_eval_list(ii);
  reco->set_enable_eval(true);
  reco->set_eval_file_name("eval.root");
  se->registerSubsystem(reco);

  //VertexFit* vtx_fit = new VertexFit();
  ////vtx_fit->set_eval_file_name(vtxevalloc);
  //se->registerSubsystem(vtx_fit);

  SQVertexing* vtx = new SQVertexing();
  vtx->Verbosity(99);
  //vtx->set_legacy_rec_container(true); // default = false
  //vtx->set_single_retracking(true);
  se->registerSubsystem(vtx);



 Fun4AllVectEventInputManager* in = new Fun4AllVectEventInputManager("VectIn");
 in->Verbosity(99);
 in->enable_E1039_translation();
 in->set_tree_name("tree");
 in->fileopen("/seaquest/users/mhossain/Fun4AllVectFileManager/convert/test.root");
 se->registerInputManager(in);
 ///////////////////////////////////////////
  // Output
  ///////////////////////////////////////////

  // DST output manager
  Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", "DST.root");
  se->registerOutputManager(out);



  se->run(n_evt);
  se->End();
  se->PrintTimer();
  rc->WriteToFile("recoConsts.tsv");
  std::cout << "All done" << std::endl;

  // cleanup - delete the server and exit
  delete se;
  gSystem->Exit(0);
  return 0;
}
