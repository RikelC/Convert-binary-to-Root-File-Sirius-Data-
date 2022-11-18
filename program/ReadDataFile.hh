/*!
 * \file ReadDataFile.hh
 * \author Rikel Chakme
 * \date 18/11/2022
 * \brief header file of the ReadDataFile class
 *
 */

#ifndef ReadDataFile_h
#define ReadDataFile_h 1
#include "TFile.h"
#include "TTree.h"
#include "MFMExogamFrame.h"
#include "MFMMergeFrame.h"
#include "MFMCoboFrame.h"
#include "MFMExogamFrame.h"
#include "MFMEbyedatFrame.h"
#include "MFMNumExoFrame.h"
#include "MFMSiriusFrame.h"
#include "MFMAllFrames.h"
#include "MFMReaGenericFrame.h"
#include "Rtypes.h"
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------
using namespace std;
class ReadDataFile
{
	private:
		MFMCommonFrame            * fFrame ; 
		MFMMergeFrame             * fMergeframe ;/**< MergerFrame type */
		MFMCoboFrame              * fCoboframe ;/**< CoboFrame for the tracker data */
		MFMCommonFrame            * fInsideframe ; /**< MFMCommonFrame type. Used for pointing to frames inside a MFMMergeFrame */
		MFMSiriusFrame            * fSiriusframe ; /**< MFMSiriusFrame type for treating DSSD data.*/
		MFMReaGenericFrame        * fGenericframe;/**< MFMReaGenericFrame type for treating TUnnel data. */

		UInt_t       eventnumber;
		ULong64_t    timestamp;
		UShort_t     gain;
		UShort_t     trace_size;
		UShort_t     Trace[992];
		UShort_t     framesize;
		UShort_t     channel;
		UShort_t     board;
		uint16_t     value;
		uint16_t     Energy;
		int          type;
		int          NbItems;

		long int max_nEvents_to_process;
		TFile* oTreeFile;
		TTree* oTree = NULL;
		void ReadUserFrame(MFMCommonFrame* commonframe);
		void ReadDefaultFrame(MFMCommonFrame* commonframe) ;
		void ReadSiriusFrame(MFMCommonFrame* commonframe) ;
		void ReadMergeFrame(MFMCommonFrame * commonframe);
		void ReadGenericFrame(MFMCommonFrame * commonframe);
		void ReadCoboFrame(MFMCommonFrame * commonframe);
	public:
		ReadDataFile();
		~ReadDataFile();

		void Read(const char *, const char *);
		void save_ttree();
		void initialize_ttree(const char*, const char*);
};
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------
#endif
