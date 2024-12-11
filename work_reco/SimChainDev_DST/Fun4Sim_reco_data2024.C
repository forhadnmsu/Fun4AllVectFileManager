R__LOAD_LIBRARY(libcalibrator)
R__LOAD_LIBRARY(libktracker)
R__LOAD_LIBRARY(libfun4all_vect_file_manager)

int Fun4Sim_reco_data2024(const int n_evt = 10){
/*
  rc->set_IntFlag("RUNNUMBER", 5443);
  rc->set_DoubleFlag("FMAGSTR", -1.044);
  rc->set_DoubleFlag("KMAGSTR", -1.025);
  rc->set_BoolFlag("COARSE_MODE", false);
  rc->set_BoolFlag("REQUIRE_MUID", false);
  rc->set_CharFlag("HIT_MASK_MODE", "X");
    
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
    const double FMAGSTR = -1.044;
  const double KMAGSTR = -1.025;
  recoConsts *rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER", 5433); /// The geometry is selected based on run number.
  rc->set_DoubleFlag("FMAGSTR", FMAGSTR);
  rc->set_DoubleFlag("KMAGSTR", KMAGSTR);
  rc->set_DoubleFlag("SIGX_BEAM", 0.3);
  rc->set_DoubleFlag("SIGY_BEAM", 0.3);
  rc->set_DoubleFlag("Z_UPSTREAM", -700.);
  

  Fun4AllServer* se = Fun4AllServer::instance();
  //se->setRun(run_id);
  se->Verbosity(100);
/*
  CalibHitElementPos* cal_ele_pos = new CalibHitElementPos();
  cal_ele_pos->CalibTriggerHit(false);
  se->registerSubsystem(cal_ele_pos);
  
  CalibHodoInTime* cal_hodo = new CalibHodoInTime();
  cal_hodo->SkipCalibration();
  cal_hodo->DeleteOutTimeHit();
  se->registerSubsystem(cal_hodo);
  
  CalibDriftDist* cal_dd = new CalibDriftDist();
  cal_dd->Verbosity(999);
  cal_dd->DeleteOutTimeHit();
  se->registerSubsystem(cal_dd);
*/
  SQReco* reco = new SQReco();
  //reco->Verbosity(100);
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
  vtx->Verbosity(100);
  //vtx->set_legacy_rec_container(true); // default = false
  //vtx->set_single_retracking(true);
  se->registerSubsystem(vtx);

 Fun4AllInputManager *in = new Fun4AllDstInputManager("DST");
 se->registerInputManager(in);
 in->fileopen("/seaquest/users/mhossain/HitMatrix/e1039-analysis/SimChainDev/data/JPsi_vec_test/1/out/DST.root");

 ///////////////////////////////////////////
  // Output
  ///////////////////////////////////////////

  // DST output manager
  Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", "DST.root");
  se->registerOutputManager(out);



  se->run(n_evt);
  se->End();
  //rc->WriteToFile("recoConsts.tsv");
  delete se;
  exit(0); //return 0;
}
