#include "ReadDataFile.hh"

long get_file_size(const char * filename){
	FILE *fd = fopen(filename, "r");
	if(fd == NULL) {
		cerr << "Error opening file\n";
		return 0;
	}
	fseek(fd, 0, SEEK_END);
	long fileSize = ftell(fd);
	fclose(fd);
	return fileSize;
}

ReadDataFile::ReadDataFile()
{
	fFrame                         = new MFMCommonFrame();
	fCoboframe                     = new MFMCoboFrame();
	fInsideframe                   = new MFMCommonFrame();
	fMergeframe                    = new MFMMergeFrame();
	fSiriusframe                   = new MFMSiriusFrame();
	fGenericframe                  = new MFMReaGenericFrame();
	eventnumber                    = 0;
	timestamp                      = 0;
	gain                           = 0;
	trace_size                     = 0;
	framesize                      = 0;
	channel                        = 0;
	board                          = 0;
	value                          = 0;
	Energy                         = 0;
	type                           = 0;
	NbItems                        = 0;


}

ReadDataFile::~ReadDataFile()
{
	if (fFrame)  delete (fFrame);
	if(fInsideframe) delete fInsideframe;
	if(fMergeframe) delete fMergeframe;
	if(fSiriusframe) delete fSiriusframe;
	if(fGenericframe) delete fGenericframe;
	if(fCoboframe) delete fCoboframe;
}


void ReadDataFile::initialize_ttree(const char* treeFileName, const char* treeName){
	oTreeFile = new TFile(treeFileName, "RECREATE");
	oTree = new TTree(treeName, treeName);
	oTree->Branch("EventNo",  &eventnumber, "EventNo/i");
	oTree->Branch("Gain",  &gain, "Gain/s");
	oTree->Branch("BoardID",  &board, "BoardID/s");
	oTree->Branch("ChannelID",  &channel, "ChannelID/s");
	oTree->Branch("Energy",  &Energy, "Energy/s");
	oTree->Branch("Time", &timestamp, "Time/l");
	oTree->Branch("TraceSize",  &trace_size, "trace_size/s");
	oTree->Branch("Trace",  Trace, "Trace[trace_size]/s");

}
void ReadDataFile::save_ttree(){
	oTreeFile->cd();
	oTree->Write();
	oTreeFile->Close();
	cout<<"Ttree saved..."<<endl;	
}



void ReadDataFile::Read(const char * filename, const char * nEvent )
{

	long fileSize = get_file_size(filename);
	long fileSize_kbytes = fileSize/1024.;

	max_nEvents_to_process = atoi(nEvent);
	int fLun = 0; // Logical Unit Number
	fLun = open(filename, (O_RDONLY));
	long long fPtInFile =0;
	long long fNbOfEvent=0;
	char* vector;
	char ** pvector;
	int vectorsize = 8; // minsizeheader		
	vector = (char*) (malloc(vectorsize));
	pvector = &vector;
	int framesize = 0;

	if (fLun <= 0) {
		printf("Error open file : %s", filename);
	}
	cout << endl;
	cout << "-----------------------------------------------------------------------------" << endl;
	cout << "| File : " << filename << endl;
	cout << "-----------------------------------------------------------------------------" << endl;
	cout << endl;

	while (true) {
		framesize = fFrame->ReadInFile(&fLun, pvector, &vectorsize);
		fPtInFile+=framesize;
		cout<<"\rRead: "<<int(double(fPtInFile)/double(fileSize) * 100) << "% of "<<fileSize_kbytes<<" KB" <<std::flush;
		if (framesize <= 0) break;
		ReadUserFrame(fFrame);
		fFrame->SetAttributs();
		if(fNbOfEvent == max_nEvents_to_process) break;
		fNbOfEvent++;
	}

	fLun = close(fLun);
	cout << endl;
	cout << "-------------------End--------with "<<fNbOfEvent<<" frames----------------" << endl;

}

void ReadDataFile::ReadUserFrame(MFMCommonFrame* commonframe) {
	type = commonframe->GetFrameType();
	commonframe->SetAttributs();

	switch (type) {
		case MFM_COBOF_FRAME_TYPE: 
		case MFM_COBO_FRAME_TYPE: 
			ReadCoboFrame(commonframe);
			break;
		case MFM_EBY_EN_FRAME_TYPE:
		case MFM_EBY_TS_FRAME_TYPE:
		case MFM_EBY_EN_TS_FRAME_TYPE: 
			break;
		case MFM_HELLO_FRAME_TYPE: 
			break;
		case MFM_XML_FILE_HEADER_FRAME_TYPE: 
			break;
		case MFM_REA_TRACE_FRAME_TYPE: 
			break;
		case MFM_REA_GENE_FRAME_TYPE: 
			ReadGenericFrame(commonframe);
			break;
		case MFM_SIRIUS_FRAME_TYPE: 
			ReadSiriusFrame(commonframe);
			break;
		case MFM_MERGE_EN_FRAME_TYPE:
		case MFM_MERGE_TS_FRAME_TYPE:
			//ReadMergeFrame(commonframe);
			break;
		default: 
			break;
	}// end of switch
	if(oTree)oTree->Fill();
}

//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------
void ReadDataFile::ReadMergeFrame(MFMCommonFrame* commonframe){
	/*	int i_insframe = 0;
		int nbinsideframe = 0;
		int dumpsize = 16;

		fMergeframe->SetAttributs(commonframe->GetPointHeader());

		nbinsideframe = fMergeframe->GetNbItems();
	//cout<<"------------------- ninside frame = "<<nbinsideframe<<endl;
	framesize= fMergeframe->GetFrameSize();
	fMergeframe->ResetReadInMem();
	for(i_insframe = 0; i_insframe < nbinsideframe; i_insframe++) {
	//fMergeframe->ReadInFrame(fInsideframe);
	fMergeframe->ReadInFrame(commonframe);
	ReadUserFrame(commonframe);
	//ReadUserFrame(fInsideframe);

	}
	*/


}
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------
void ReadDataFile::ReadGenericFrame(MFMCommonFrame* commonframe)
{
	fGenericframe->SetAttributs(commonframe->GetPointHeader());
	//	framesize = commonframe->GetFrameSize();
	eventnumber = fGenericframe->GetEventNumber();
	timestamp = fGenericframe->GetTimeStamp();
	channel =(UShort_t) fGenericframe->GetChannelId();
	board = (UShort_t) fGenericframe->GetBoardId();
	value =  fGenericframe->GetEnergy();
	Energy = value;
	trace_size = 0;
	gain = 0;
}
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------
void ReadDataFile::ReadSiriusFrame( MFMCommonFrame* commonframe){
	fSiriusframe->SetAttributs(commonframe->GetPointHeader());
	//framesize =fSiriusframe->GetFrameSize();
	eventnumber = fSiriusframe->GetEventNumber();
	timestamp = fSiriusframe->GetTimeStamp();
	channel = (int)fSiriusframe->GetChannelId();
	board = (int)fSiriusframe->GetBoardId();
	gain = fSiriusframe->GetGain();
	NbItems= fSiriusframe->GetNbItems();
	trace_size =NbItems;
	for (int i = 0; i < NbItems; i++) {
		fSiriusframe->GetParameters(i+1, &value);
		Trace[i] = value;
	}
	Energy = 0;
}
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------
void ReadDataFile::ReadCoboFrame(MFMCommonFrame* commonframe){
	fCoboframe->SetAttributs(commonframe->GetPointHeader());
	//	framesize=fCoboframe->GetFrameSize();
	eventnumber =fCoboframe->GetEventNumber();
	timestamp = (uint64_t)(fCoboframe->GetTimeStamp());
	channel = 0;
	board = 0;
	gain = 0;
	trace_size =0;
}
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------
