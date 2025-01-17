#include <string>
#include <iostream>
R__LOAD_LIBRARY(libfun4all_vect_file_manager)
R__LOAD_LIBRARY(libcalibrator)
R__LOAD_LIBRARY(libktracker)

int ConvertToVect (){
	const char* fn_udst= "uDST_mum.root";
	Fun4AllServer* se = Fun4AllServer::instance();
	Fun4AllInputManager *in = new Fun4AllDstInputManager("DUMMY");
	se->registerInputManager(in);
	in->fileopen(fn_udst);
	Fun4AllVectEventOutputManager* tree = new Fun4AllVectEventOutputManager();
	tree->Verbosity(100);
	tree->SetTreeName("tree");
	tree->SetFileName("Vector-In-Mum.root");
	se->registerOutputManager(tree);
	se->run(0);
	se->End();
	return 0;
}
