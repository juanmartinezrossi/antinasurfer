// BoardIo.cpp
//
// Board-specific data flow and hardware I/O
// Copyright 2006 Innovative Integration
//---------------------------------------------------------------------------

#include <Malibu_Mb.h>
#include <Analysis_Mb.h>
#include <IppMemoryUtils_Mb.h>

#include "phal_hw_api.h"

#include "BoardIo.h"
#include <SystemSupport_Mb.h>
#include <StringSupport_Mb.h>
#include <limits>
#include <cmath>
#include <numeric>
#include <Magic_Mb.h>
#include <Exception_Mb.h>
#include <FileSupport_Mb.h>
#include <DeviceEnum_Mb.h>
#include <Buffer_Mb.h>
#include <BufferDatagrams_Mb.h>
#include <BufferHeader_Mb.h>
#include <sstream>
#include <cstdio>
#include <iostream>

using namespace Innovative;
using namespace std;

Buffer Packet_Sin;


//===========================================================================
//  CLASS BoardIo  -- Hardware Access and Board I/O Class
//===========================================================================
//---------------------------------------------------------------------------
//  constructor for class BoardIo
//---------------------------------------------------------------------------

BoardIo::BoardIo() {

}

//---------------------------------------------------------------------------
//  destructor for class BoardIo
//---------------------------------------------------------------------------

BoardIo::~BoardIo() {
	Close();
}

//---------------------------------------------------------------------------
//  BoardIo::BoardCount() -- Query number of installed boards
//---------------------------------------------------------------------------

unsigned int BoardIo::BoardCount() {
	return static_cast<unsigned int> (Module.BoardCount());
}

//---------------------------------------------------------------------------
//  BoardIo::Open() -- Open Hardware & set up callbacks
//---------------------------------------------------------------------------

void BoardIo::Open() {

	ad_enabled=false;
	da_enabled=false;
	do_sin=false;
	da_print=false;
	ad_print=false;
	da_sendad=false;


	DisplayLogicVersion();
}

//---------------------------------------------------------------------------
//  BoardIo::Close() -- Close Hardware & set up callbacks
//---------------------------------------------------------------------------

void BoardIo::Close() {
	if (!Opened)
		return;

	Module.Led(false);
	Stream.Disconnect();
	StreamConnected = false;

	Module.Close();
	Opened = false;

	cout << "Stream Disconnected..." << endl;
}

//---------------------------------------------------------------------------
//  BoardIo::StartStreaming() --  Initiate data flow
//---------------------------------------------------------------------------



void BoardIo::CfgMain(int SampFreq, int ClockInternal, int SyncAD, float *rx, float *tx,
		void(*sync)(void), int direct) {

	rxBuffer = rx;
	txBuffer = tx;
	SampleRate = SampFreq;
	syncTs = sync;
	bool dma = (direct==0)?false:true;
	//
	//  Configure Stream Event Handlers
	
	
	if (!dma) {
		Stream.OnDataAvailable.SetEvent(this, &BoardIo::HandleDataAvailable);
		Stream.OnDataRequired.SetEvent(this, &BoardIo::HandleDataRequired);
		Stream.DirectDataMode(false);
	} else {
		Stream.DirectDataMode(true);
		Stream.OnDirectDataAvailable.SetEvent(this, &BoardIo::HandleDirectDataAvailable);
		Stream.OnDirectDataRequired.SetEvent(this, &BoardIo::HandleDirectDataRequired);
	}

	Module.Alerts().OnInputFifoOverrun.SetEvent(this,
			&BoardIo::HandleInputFifoOverrunAlert);
	Module.Alerts().OnOutputFifoUnderrun.SetEvent(this,
			&BoardIo::HandleOutputFifoUnderrunAlert);

	FChannels = Module.Input().Info().Channels().Channels();

	// Insure BM size is a multiple of four MB

	Module.IncomingBusMasterSize(0x400000);
	Module.OutgoingBusMasterSize(0x400000);
	Module.Target(0);
	//
	//  Open Device
	try {
		Module.Open();

		std::stringstream msg;
		cout << "Bus master size: 4 MB" << endl;
	}

	catch (Innovative::MalibuException & e) {
		cout << "Module Device Open Failure!" << endl;
		cout << e.what();
		return;
	}

	catch (...) {
		cout << "Module open failure..." << endl;
		Opened = false;
		return;
	}

	Module.Led(true);
	Module.Reset();
	cout << "Module device opened successfully..." << endl;
	Opened = true;

	//
	//  Connect Stream
	Stream.ConnectTo(&Module);
	StreamConnected = true;
	cout << "Stream Connected..." << endl;

	if (SyncAD>0) {
		syncAd=true;
	} else {
		syncAd=false;
	}

	if (!StreamConnected) {
		cout << "Stream not connected! -- Open the boards" << endl;
		return;
	}
	if (SampleRate > Module.Input().Info().MaxRate()) {
		cout << "Sample rate too high." << endl;
		StopStreaming();
		return;
	}
	// Disable triggers initially
	SetInputSoftwareTrigger(false);
	SetOutputSoftwareTrigger(false);
	Module.Input().ExternalTrigger(false);
	Module.Output().ExternalTrigger(false);

	// Route clock to active analog devices
	Module.Clock().Source((ClockInternal > 0) ? X5ClockIntf::csInternal
			: X5ClockIntf::csExternal);
	Module.Clock().Frequency(SampleRate);

	Module.Alerts().AlertEnable(IUsesX5Alerts::alertInputFifoOverrun, true);
	Module.Alerts().AlertEnable(IUsesX5Alerts::alertOutputFifoUnderrun, true);


}

void BoardIo::CfgAD(int ADsamples, int ADdecimate, int ADprint, int _ADscale) {
	ADPacketSize = ADsamples;
	ADscale = _ADscale;

	Module.Input().Info().Channels().DisableAll();
	Module.Input().Info().Channels().Enabled(0, true);
	Module.Input().PacketSize(ADPacketSize / 2 + 2);
	Module.Input().Decimation(ADdecimate);
	Module.Input().Unframed();

	if (ADprint>0) {
		ad_print=true;
	} else {
		ad_print=false;
	}
	ad_enabled = true;
}

void BoardIo::CfgDA(int DAsamples, int DAdecimate, int DAinterpolate, int DAdivider,
		int DAdosin, int DAsendad, int DAprint, int DAsinfreq, int _DAscale) {
	DAPacketSize = DAsamples;
	DAscale = _DAscale;
	Module.Output().Info().Channels().DisableAll();
	Module.Output().Info().Channels().Enabled(0, true);

	Module.Clock().DacDivider(DAdivider);
	Module.Output().Decimation(DAdecimate);
	Module.Output().Interpolate(DAinterpolate);
	Module.Output().Unframed();

	if (DAdosin > 0) {
		do_sin = true;
		ShortDG Packet_DG(Packet_Sin);
		Packet_DG.Resize(DAsamples);
		PacketBufferHeader PktBufferHdr(Packet_Sin);
		PktBufferHdr.PacketSize(DAsamples);
		PktBufferHdr.PeripheralId(Module.Output().PacketId());
		PktBufferHdr[1] = 0x0;
		int n=0;
		for (int i = 0; i < Packet_DG.size(); i++) {
			if (DAsinfreq==1)  {
				Packet_DG[i]=100*(i%100-50);
			} else if (DAsinfreq<1 && DAsinfreq>0) {
				Packet_DG[i]=5000+(short) (5000*cos(2*3.1415*DAsinfreq*i));
			} else {
				Packet_DG[i] = (i % 2) ? 10000 : -10000;
			}
		}
	}
	if (DAsendad>0) {
		da_sendad=true;
	} else {
		da_sendad=false;
	}
	printf("DA=%d\n",DAprint);
	if (DAprint>0) {
		da_print=true;
	} else {
		da_print=false;
	}

	da_enabled = true;
}

void BoardIo::StartStreaming() {
	int ActualSampleRate;
	Stopped = false;
	Stream.Start();
	cout << "Stream Mode started" << endl;

	ActualSampleRate = static_cast<float> (Module.Clock().FrequencyActual());
	cout << "Actual sampling rate: " << ActualSampleRate / 1.e6 << " MHz"
			<< endl;

	FTicks = 0;

	if (ad_enabled)
		SetInputSoftwareTrigger(true);
	if (da_enabled)
		SetOutputSoftwareTrigger(true);

}

//---------------------------------------------------------------------------
//  BoardIo::StopStreaming() --  Terminate data flow
//---------------------------------------------------------------------------

void BoardIo::StopStreaming() {
	if (!StreamConnected) {
		cout << "Stream not connected! -- Open the boards" << endl;
		return;
	}

	//
	//  Stop Streaming
	Stream.Stop();
	Stopped = true;
}
void BoardIo::HandleInputFifoOverrunAlert(Innovative::AlertSignalEvent & event) {
	time_t t;

	get_time(&t);

	printf("Input FIFO overrun at %d:%d\n", t.tv_sec, t.tv_usec);

	Module.Input().AcknowledgeAlert();
}

void BoardIo::HandleOutputFifoUnderrunAlert(
		Innovative::AlertSignalEvent & event) {
	time_t t;

	get_time(&t);

	printf("Output FIFO underrun at %d:%d\n", t.tv_sec, t.tv_usec);

	Module.Output().AcknowledgeAlert();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  Data Flow Event Handlers
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int cnt = 0;
time_t t2[3];
int n = 0;
void BoardIo::HandleDataRequired(PacketStreamDataEvent & Event) {


	if (da_print) {
		gettimeofday(&t2[2], NULL);
		get_time_interval(t2);
		t2[1].tv_sec = t2[2].tv_sec;
		t2[1].tv_usec = t2[2].tv_usec;
		printf("Data required at time %d:%d interval %d Freq %.2f\n", t2[2].tv_sec,
			t2[2].tv_usec, t2[0].tv_usec, (float) DAPacketSize / t2[0].tv_usec);
	}
	if (da_sendad) 
		return;
		
	if (do_sin) {
		Stream.Send(Packet_Sin);
	} else {
		Buffer Packet;
		ShortDG Packet_DG(Packet);
		Packet_DG.Resize(DAPacketSize);
		PacketBufferHeader PktBufferHdr(Packet);
		PktBufferHdr.PacketSize(DAPacketSize);
		PktBufferHdr.PeripheralId(Module.Output().PacketId());
		PktBufferHdr[1] = 0x0;
		for (int i = 0; i < Packet_DG.size(); i++) {
			Packet_DG[i] = (short) (DAscale * txBuffer[i]);
		}
		Stream.Send(Packet);
	}

	if (!syncAd) {
		syncTs();
	}

}
void BoardIo::HandleDirectDataRequired(PacketStreamDirectDataEvent & Event) {

	if (da_print) {
		gettimeofday(&t2[2], NULL);
		get_time_interval(t2);
		t2[1].tv_sec = t2[2].tv_sec;
		t2[1].tv_usec = t2[2].tv_usec;
		printf("Data required at time %d:%d interval %d Freq %.2f\n", t2[2].tv_sec,
			t2[2].tv_usec, t2[0].tv_usec, (float) DAPacketSize / t2[0].tv_usec);
	}

	if (da_sendad)
		return;

	if (do_sin) {
		Event.Sender->Send(Packet_Sin);
	} else {
		Buffer Packet;
		ShortDG Packet_DG(Packet);
		Packet_DG.Resize(DAPacketSize);
		PacketBufferHeader PktBufferHdr(Packet);
		PktBufferHdr.PacketSize(DAPacketSize);
		PktBufferHdr.PeripheralId(Module.Output().PacketId());
		PktBufferHdr[1] = 0x0;
		for (int i = 0; i < Packet_DG.size(); i++) {
			Packet_DG[i] = (short) (DAscale * txBuffer[i]);
		}
		Event.Sender->Send(Packet);
	}

	Event.Sender->DirectDataTransmitAck(DAPacketSize*2);

	if (!syncAd) {
		syncTs();
	}

}
//---------------------------------------------------------------------------
//  BoardIo::HandleDataAvailable() --  Handle received packet
//---------------------------------------------------------------------------
time_t t[3];
int c = 0;
void BoardIo::HandleDirectDataAvailable(PacketStreamDirectDataEvent & Event) {
	short * Address = reinterpret_cast<short*>(Event.Slab.Address()); 

	int NumSamples = Event.Slab.SizeInInts()*2;
	if (ad_print) {
		gettimeofday(&t[2], NULL);
		get_time_interval(t);
		t[1].tv_sec = t[2].tv_sec;
		t[1].tv_usec = t[2].tv_usec;
		printf("Data size %d arrived at time %d:%d interval %d Freq %.2f\n",
				NumSamples, t[2].tv_sec, t[2].tv_usec, t[0].tv_usec,
				(float) NumSamples / t[0].tv_usec);
	}
	for (int i = 0; i < NumSamples; i++) {
		rxBuffer[i]=(float) Address[i]/ADscale;
	}

	if (da_sendad) {
		if (do_sin) {
			Stream.Send(Packet_Sin);
		} else {
			Buffer Packet;
			ShortDG Packet_DG(Packet);
			Packet_DG.Resize(DAPacketSize);
			PacketBufferHeader PktBufferHdr(Packet);
			PktBufferHdr.PacketSize(DAPacketSize);
			PktBufferHdr.PeripheralId(Module.Output().PacketId());
			PktBufferHdr[1] = 0x0;
			for (int i = 0; i < Packet_DG.size(); i++) {
				Packet_DG[i] = (short) (DAscale * txBuffer[i]);
			}
			Stream.Send(Packet);
		}
	}
	if (syncAd) {
		syncTs();
	}

	Event.Sender->DirectDataReceiveAck(Event.Slab.SizeInInts());
}
int l=0;
void BoardIo::HandleDataAvailable(PacketStreamDataEvent & Event) {
	int i;

	if (Stopped)
		return;

	Buffer Packet;
	Stream.Recv(Packet);

	ShortDG Packet_DG(Packet);

	if (ad_print) {
		gettimeofday(&t[2], NULL);
		get_time_interval(t);
		t[1].tv_sec = t[2].tv_sec;
		t[1].tv_usec = t[2].tv_usec;
		l++;
		if (l==100) {
			l=0;
			printf("Data size %d arrived at time %d:%d interval %d Freq %.2f\n",
				Packet_DG.size(), t[2].tv_sec, t[2].tv_usec, t[0].tv_usec,
				(float) Packet_DG.size() / t[0].tv_usec);
		}
	}
	for (int i = 0; i < Packet_DG.size(); i++) {
		rxBuffer[i]=(float) Packet_DG[i]/ADscale;
//		if (i<20)
//			printf("%d, ",Packet_DG[i]);
	}
//	printf("\n");

	if (da_sendad) {
		if (do_sin) {
			Stream.Send(Packet_Sin);
		} else {
			Buffer Packet;
			ShortDG Packet_DG(Packet);
			Packet_DG.Resize(DAPacketSize);
			PacketBufferHeader PktBufferHdr(Packet);
			PktBufferHdr.PacketSize(DAPacketSize);
			PktBufferHdr.PeripheralId(Module.Output().PacketId());
			PktBufferHdr[1] = 0x0;
			for (int i = 0; i < Packet_DG.size(); i++) {
				Packet_DG[i] = (short) (DAscale * txBuffer[i]);
			}

			Stream.Send(Packet);
		}
	}
	if (syncAd) {
		syncTs();
	}

}

//------------------------------------------------------------------------------
//  BoardIo::DisplayLogicVersion() --  Log version info
//------------------------------------------------------------------------------

void BoardIo::DisplayLogicVersion() {
	cout << "Logic Version: " << Module.UserLogicVersion().FpgaLogicVersion()
			<< ", Variant: " << Module.UserLogicVersion().FpgaLogicVariant()
			<< ", Revision: " << Module.UserLogicVersion().FpgaLogicRevision()
			<< ", Type: " << Module.UserLogicVersion().FpgaLogicType();

	cout << std::hex << "PCI Version: "
			<< Module.PciLogicVersion().PciLogicRevision() << ", Family: "
			<< Module.PciLogicVersion().PciLogicFamily() << ", PCB: "
			<< Module.PciLogicVersion().PciLogicPcb() << ", Type: "
			<< Module.PciLogicVersion().PciLogicType();

	cout << "PCI Express Lanes: " << Module.Debug()->LaneCount() << endl;
}

void BoardIo::SetInputSoftwareTrigger(bool state) {
	Module.Trigger(X5_400M::tInput, state); //  "standard" method
}

void BoardIo::SetOutputSoftwareTrigger(bool state) {
	Module.Trigger(X5_400M::tOutput, state); //  "standard" method
}

