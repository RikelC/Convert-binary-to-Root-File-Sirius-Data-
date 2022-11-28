/*!
 * \file ReadDataFile.cc
 * \author Rikel Chakma
 * \date 18/11/2022
 * \brief To read the binary data file and convert to root file
 *
 */
#include "ReadDataFile.hh"
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------
/*!
 * To get the size of the file in bytes
 */
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
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------
/*!
 * Constructor
 */
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
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------
/*!
 * Destructor
 */
ReadDataFile::~ReadDataFile()
{
	if (fFrame)  delete (fFrame);
	if(fInsideframe) delete fInsideframe;
	if(fMergeframe) delete fMergeframe;
	if(fSiriusframe) delete fSiriusframe;
	if(fGenericframe) delete fGenericframe;
	if(fCoboframe) delete fCoboframe;
}
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------
/*!
 * Initialize the Ttree
 */
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
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------
/*!
 * Save the Ttree
 */
void ReadDataFile::save_ttree(){
	oTreeFile->cd();
	oTree->Write();
	oTreeFile->Close();
	cout<<"Ttree saved..."<<endl;	
}
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------
/*!
 * Read a file upto nEvents
 *
 */
void ReadDataFile::Read(const char * filename, const char * nEvent )
{

	long fileSize = get_file_size(filename);
	long fileSize_kbytes = fileSize/1024.;
	std::string filename2 = filename;
	size_t pos = filename2.find_last_of('/');
	if(pos!= std::string::npos)filename2.erase(0,pos+1);
	char * p_end_nEvent;
	max_nEvents_to_process = strtol(nEvent, &p_end_nEvent, 10);
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
	cout << "------------------------------------------------------------------" << endl;
	if(max_nEvents_to_process <=0){// Read all the events
		while (true) {
			cout<<"\r| File : "<<filename2<<" | Converting : "<<int(double(fPtInFile)/double(fileSize) * 100) << "% of "<<fileSize_kbytes<<" KB" <<std::flush;
			framesize = fFrame->ReadInFile(&fLun, pvector, &vectorsize);
			fPtInFile+=framesize;
			if (framesize <= 0) break;
			ReadUserFrame(fFrame);
			fFrame->SetAttributs();
			fNbOfEvent++;
		}

	}
	else{
		while (true) {// Read maximum number of events defined by the user
			cout<<"\r| File : "<<filename2<<" | Read: "<<int(double(fPtInFile)/double(fileSize) * 100) << "% of "<<fileSize_kbytes<<" KB" <<std::flush;
			framesize = fFrame->ReadInFile(&fLun, pvector, &vectorsize);
			fPtInFile+=framesize;
			if (framesize <= 0) break;
			ReadUserFrame(fFrame);
			fFrame->SetAttributs();
			if(fNbOfEvent == max_nEvents_to_process) break;
			fNbOfEvent++;
		}

	}
	cout << "\n------------------------------------------------------------------" << endl;
	fLun = close(fLun);
	cout << endl;
}
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------
/*!
 * Read each frames and fill the TTree
 */
void ReadDataFile::ReadUserFrame(MFMCommonFrame* commonframe) {
	type = commonframe->GetFrameType();
	commonframe->SetAttributs();

	switch (type) {
		case MFM_COBOF_FRAME_TYPE: 
		case MFM_COBO_FRAME_TYPE: 
			ReadCoboFrame(commonframe);
			if(oTree)oTree->Fill();
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
			if(oTree)oTree->Fill();
			break;
		case MFM_SIRIUS_FRAME_TYPE: 
			ReadSiriusFrame(commonframe);
			if(oTree)oTree->Fill();
			break;
		case MFM_MERGE_EN_FRAME_TYPE:
		case MFM_MERGE_TS_FRAME_TYPE:
			//ReadMergeFrame(commonframe);
			break;
		default: 
			break;
	}// end of switch
	
}
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------
/*!
 * Read the merge frames
 */
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
/*!
 * Read generic frames of the tunnel detector boards
 */
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
/*!
 * read the sirius frames of the DSSD boards
 */
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
/*!
 * Read the cobo frames for the tracker data
 */
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
