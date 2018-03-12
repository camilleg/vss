// windows MMIO platform stuff (huge, so it gets its own file)

#ifdef VSS_WINDOWS

#include <windows.h>

const int vfdOutput = -1; //;; stub

const int csampBufMax = 132300; // 10x chickenfactor
static short rgsFrame[csampBufMax];
static short rgsFrameRd[csampBufMax];

#include <cstdlib>
#include <unistd.h>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <io.h>
#include <iostream>

#include "platform.h"
#include "winplatform.h"
#include "vssglobals.h"

#undef DISABLE_INPUT
//#define DISABLE_INPUT 1

int areal_internal_FSendNow(void);
void areal_internal_Send(short* rgsamp, int csamp);
#ifndef DISABLE_INPUT
int areal_internal_FRecvNow(void);
void areal_internal_Recv(short* rgsamp, int csamp);
static int vfSoundIn = 0;
#endif


class Mutex
	{
	private:
		CRITICAL_SECTION lock;
	public:
		Mutex(void) { InitializeCriticalSection(&lock); }
		~Mutex(void) { DeleteCriticalSection(&lock); }
		void Lock(void) { EnterCriticalSection(&lock); }
		void Unlock(void) { LeaveCriticalSection(&lock); }
	};

typedef HGLOBAL HWAVEHDR;

#ifndef DISABLE_INPUT
	static Mutex* CSAreal_SYS_Rd = NULL;

	static HWAVEHDR  hWaveHdrRd;
	static LPWAVEHDR lpWaveHdrRd;
	static void* lpDataRd;
	static HWAVEIN hWaveRd;

	static HWAVEHDR  hWaveHdr2Rd;
	static LPWAVEHDR lpWaveHdr2Rd;
	static void* lpData2Rd;

	static int fWhichRd = 0;
	static int cDoneRd = 0;
#endif

	static const int cBondMax = 4; // # of "bonded channels" of stereo output
	static int cBond = 1;

	static Mutex* CSAreal_SYS_Wr = NULL;

	static HWAVEHDR  hWaveHdrWr[cBondMax];
	static LPWAVEHDR lpWaveHdrWr[cBondMax];
	static void* lpDataWr[cBondMax];
	static HWAVEOUT hWaveWr[cBondMax];

	static HWAVEHDR  hWaveHdr2Wr[cBondMax];
	static LPWAVEHDR lpWaveHdr2Wr[cBondMax];
	static void* lpData2Wr[cBondMax];

	static int fWhichWr = 0;
	static int cDoneWr = 0;

static int cwFrame = 0;


static void CALLBACK waveOutCallback(HWND /*hWaveWr*/, int wMsg, DWORD /*dwInstance*/, DWORD /*dwParam1*/, DWORD /*dwParam2*/)
{
	if (wMsg == WOM_DONE)
		{
		CSAreal_SYS_Wr->Lock();
		// increment, don't just set: in case we fall behind, then we'll catch up again.
		cDoneWr++;
		CSAreal_SYS_Wr->Unlock();
		}
}

#ifndef DISABLE_INPUT
static void CALLBACK waveInCallback(HWND /*hWaveRd*/, int wMsg, DWORD /*dwInstance*/, DWORD /*dwParam1*/, DWORD /*dwParam2*/)
{
	if (wMsg == WIM_DATA)
		{
		CSAreal_SYS_Rd->Lock();
		// increment, don't just set: in case we fall behind, then we'll catch up again.
		cDoneRd++;
		CSAreal_SYS_Rd->Unlock();
		}
}
#endif

using std::cerr;
using std::endl;

static int areal_internal_FInitAudioWrite(int nchans)
{
	cDoneWr = 0;
	fWhichWr = 0;
	int cdev = waveOutGetNumDevs();
	if (cdev < 1)
		{
		cerr << "vss error: no MMIO audio output devices.\n";
		return FALSE;
		}
	if (cdev >= 4 && nchans > 2)
		{
		cerr << "vss warning: multichannel audio requested and >=4 MMIO devices present.\n\tAssuming you want to bond multiple stereo channels on a STUDI/O card\n\tstarting with the second device.\n";
		if (nchans % 2 != 0)
			cerr << "vss warning: using an even number of channels.\n";
		cBond = nchans/2;
		}
	else if (cdev > 1)
		{
		cerr << "vss remark: using default audio device (of " <<cdev <<" devices).\n";
		}
	if (cBond > cBondMax)
		{
		cerr << "vss error: can't bond " << cBond << " channels.\n"
		  << cBondMax << " is the maximum.\n";
		return FALSE;
		}

	UINT x;
    for (int iBond=0; iBond<cBond; iBond++)
		{
		int deviceID = cBond==1 ? WAVE_MAPPER : iBond+1;
		int nchansDev = cBond==1 ? nchans : nchans/cBond;
			{
			WAVEFORMATEX fmt;
			fmt.wFormatTag = WAVE_FORMAT_PCM;
			fmt.nChannels = nchansDev;
			fmt.nSamplesPerSec = (DWORD)globs.SampleRate;
			fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * 2 * nchansDev;
			fmt.nBlockAlign = 2 * nchansDev;
			fmt.wBitsPerSample = 16;
			fmt.cbSize = 0;
			int fCallback = cBond==1 || iBond==0; // Only 1 guy does callback!
			x = waveOutOpen(&hWaveWr[iBond], deviceID, (LPWAVEFORMATEX)&fmt,
				fCallback ? (DWORD)&waveOutCallback : 0,
				(DWORD)NULL,
				fCallback ? CALLBACK_FUNCTION : 0/*CALLBACK_NULL*/);
			}
		if (x != 0)
			{
			// handle error
			char sz[200];
			waveOutGetErrorText(x, sz, sizeof(sz)-1);
			// Filter out useless windows explanation.
			if (x == 32)
				cerr << "vss error: failed to open MMIO audio output for requested format.\n";
			else
				cerr << "vss error: failed to open MMIO audio output for requested format:\n\t" << sz << endl;

			WAVEOUTCAPS caps = {0};
			x = waveOutGetDevCaps(deviceID, &caps, sizeof(caps));
			if (!x)
				{
				// Typical values for caps.* are:
				// "SB Live! Wave Device": 65k, 11k,22k,44k.
				// "ESS Maestro":  65535-channel output at 11k,22k,44k.
				// "A device ID has been used that is out of range for your system."

				cerr <<"\nvss remark: soundcard is " << caps.szPname
					<<"\n\tand supports up to "
					<<caps.wChannels <<"-channel output\n";
				const int w = caps.dwFormats;
				if ((w & (WAVE_FORMAT_1M16|WAVE_FORMAT_1S16|WAVE_FORMAT_2M16|WAVE_FORMAT_2S16|WAVE_FORMAT_4M16|WAVE_FORMAT_4S16)) == 0)
					{
					cerr <<"vss error: soundcard is only 8-bit.  VSS needs a 16-bit soundcard.\n";
					return FALSE;
					}
#ifdef THIS_IS_NEVER_INFORMATIVE
				cerr <<"\tat sample rates of:\n";
				if ((w&WAVE_FORMAT_1M16) || (w&WAVE_FORMAT_1S16))
					cerr << "\t  11.025 kHz\n";
				if ((w&WAVE_FORMAT_2M16) || (w&WAVE_FORMAT_2S16))
					cerr << "\t  22.05  kHz\n";
				if ((w&WAVE_FORMAT_4M16) || (w&WAVE_FORMAT_4S16))
					cerr << "\t  44.1   kHz\n";
#endif
				}
			return FALSE;
			}

		const DWORD dwDataSize = cwFrame * 2L / cBond;
		lpDataWr[iBond] = new char[dwDataSize];
		if (!lpDataWr[iBond])
			{
			cerr <<"vss error initializing MMIO audio output: out of memory.\n";
			goto LAbort3;
			}
		hWaveHdrWr[iBond] = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (DWORD)sizeof(WAVEHDR));
		if (!hWaveHdrWr[iBond])
			{
			cerr <<"vss error initializing MMIO audio output: out of memory.\n";
			goto LAbort2;
			}
		lpWaveHdrWr[iBond] = (LPWAVEHDR) GlobalLock(hWaveHdrWr[iBond]);
		if (!lpWaveHdrWr[iBond])
			{
			cerr <<"vss error initializing MMIO audio output: GlobalLock failed.\n";
			goto LAbort1;
			}
		// weird, GlobalLock works here but not for lpDataWr[iBond].
		memset(lpDataWr[iBond], 0, dwDataSize);
		

		lpWaveHdrWr[iBond]->lpData = (char FAR *)lpDataWr[iBond];
		lpWaveHdrWr[iBond]->dwBufferLength = dwDataSize;
		lpWaveHdrWr[iBond]->dwFlags = 0;
		lpWaveHdrWr[iBond]->dwLoops = 0L;
		waveOutPrepareHeader(hWaveWr[iBond], lpWaveHdrWr[iBond], sizeof(WAVEHDR));
		
		if (cBond == 1)
			{
			x = waveOutWrite(hWaveWr[iBond], lpWaveHdrWr[iBond], sizeof(WAVEHDR));
			if (x != 0)
				{
				// handle error
				//char sz[200];
				//waveOutGetErrorText(x, sz, sizeof(sz)-1);
				cerr <<"vss error initializing MMIO audio output: waveOutWrite failed.\n";
				goto LAbort0;
				}
			}

		lpData2Wr[iBond] = new char[dwDataSize];
		if (!lpData2Wr[iBond])
			{
			cerr <<"vss error initializing MMIO audio output: out of memory.\n";
			goto LAbort_3;
			}
		hWaveHdr2Wr[iBond] = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (DWORD)sizeof(WAVEHDR));
		if (!hWaveHdr2Wr[iBond])
			{
			cerr <<"vss error initializing MMIO audio output: out of memory\n";
			goto LAbort_2;
			}
		lpWaveHdr2Wr[iBond] = (LPWAVEHDR) GlobalLock(hWaveHdr2Wr[iBond]);
		if (!lpWaveHdr2Wr[iBond])
			{
			cerr <<"vss error initializing MMIO audio output: GlobalLock failed.\n";
			goto LAbort_1;
			}
		memset(lpData2Wr[iBond], 0, dwDataSize);
		lpWaveHdr2Wr[iBond]->lpData = (char FAR *)lpData2Wr[iBond];
		lpWaveHdr2Wr[iBond]->dwBufferLength = dwDataSize;
		lpWaveHdr2Wr[iBond]->dwFlags = 0;
		lpWaveHdr2Wr[iBond]->dwLoops = 0L;
		
		waveOutPrepareHeader(hWaveWr[iBond], lpWaveHdr2Wr[iBond], sizeof(WAVEHDR));
		if (cBond == 1)
			{
			x = waveOutWrite(hWaveWr[iBond], lpWaveHdr2Wr[iBond], sizeof(WAVEHDR));
			if (x != 0)
				{
				// handle error
				//char sz[200];
				//waveOutGetErrorText(x, sz, sizeof(sz)-1);
				cerr << "vss error while initializing MMIO audio output: waveOutWrite failed.\n";
				goto LAbort_0;
				}
			}
		// write 2 buffers now, so when we get a "buf done" msg and queue up the next one, we're always 1 ahead.
		}

	if (cBond > 1)
		{
		// Do all the waveOutWrite's at once, not staggered.
		// Don't even attempt to handle errors properly.
		// In particular this means that the [Reset] button fails to free
		// all the audio devices and VSS will fail to restart.
		int iBond;
		for (iBond=0; iBond<cBond; iBond++)
			{
			x = waveOutWrite(hWaveWr[iBond], lpWaveHdrWr[iBond], sizeof(WAVEHDR));
			if (x != 0)
				{
				cerr << "vss error while initializing MMIO audio output: waveOutWrite failed.\n";
				goto LAbort_0;
				}
			}
		for (iBond=0; iBond<cBond; iBond++)
			{
			x = waveOutWrite(hWaveWr[iBond], lpWaveHdr2Wr[iBond], sizeof(WAVEHDR));
			if (x != 0)
				{
				cerr << "vss error while initializing MMIO audio output: waveOutWrite failed.\n";
				goto LAbort_0;
				}
			}
		}

	CSAreal_SYS_Wr = new Mutex;
	return TRUE;

LAbort_0:
	GlobalUnlock(hWaveHdr2Wr[0]);
LAbort_1:
	GlobalFree(hWaveHdr2Wr[0]);
LAbort_2:
LAbort_3:
LAbort0:		
	GlobalUnlock(hWaveHdrWr[0]);
LAbort1:		
	GlobalFree(hWaveHdrWr[0]);
LAbort2:
LAbort3:		
	waveOutReset(hWaveWr[0]);
	x = waveOutClose(hWaveWr[0]);
	// This LAbort code leaks resources if iBond>0 when it fails.  A rare case.
	cerr << "vss error 42: failed to open MMIO audio output.\n";
	return FALSE;
}

static int areal_internal_FInitAudioRead(int nchansIn)
{
#ifdef DISABLE_INPUT
	return TRUE;
#else
	fWhichRd = 0;
	if (waveInGetNumDevs() < 1)
		{
		cerr << "vss error: no MMIO audio input devices.\n";
		// no sound-recording devices
		return FALSE;
		}
	cerr << "vss remark: attempting MMIO audio input.\n";;;;
	UINT x;
    
		{
		WAVEFORMATEX fmt;
		fmt.wFormatTag = WAVE_FORMAT_PCM;
		fmt.nChannels = nchansIn;
		fmt.nSamplesPerSec = (DWORD)globs.SampleRate;
		fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * 2 * nchansIn;
		fmt.nBlockAlign = 2 * nchansIn;
		fmt.wBitsPerSample = 16;
		fmt.cbSize = 0;
		x = waveInOpen(&hWaveRd, WAVE_MAPPER, (LPWAVEFORMATEX)&fmt, (DWORD)&waveInCallback, (DWORD)NULL, CALLBACK_FUNCTION);
		}
	cerr << "x is " << x << endl;
	if (x != 0)
		{
		// handle error
		char sz[200];
		waveInGetErrorText(x, sz, sizeof(sz)-1);
		// Filter out useless windows explanation.
		if (x == 32)
			cerr << "vss error: failed to open MMIO audio input for requested format.\n";
		else
			cerr << "vss error: failed to open MMIO audio input for requested format:\n\t" << sz << endl;
		return FALSE;
		}

	const DWORD dwDataSize = cwFrame/*8192*/ * 2L;
	lpDataRd = new char[dwDataSize];
	if (!lpDataRd)
		{
		cerr <<"vss error initializing MMIO audio input: out of memory\n";
		goto LAbort3;
		}
	hWaveHdrRd = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (DWORD)sizeof(WAVEHDR));
	if (!hWaveHdrRd) goto LAbort2;
	lpWaveHdrRd = (LPWAVEHDR) GlobalLock(hWaveHdrRd);
	if (!lpWaveHdrRd) goto LAbort1;
	memset(lpDataRd, 0, dwDataSize);
	lpWaveHdrRd->lpData = (char FAR *)lpDataRd;
	lpWaveHdrRd->dwBufferLength = dwDataSize;
	lpWaveHdrRd->dwFlags = 0;
	waveInPrepareHeader(hWaveRd, lpWaveHdrRd, sizeof(WAVEHDR));
	x = waveInAddBuffer(hWaveRd, lpWaveHdrRd,  sizeof(WAVEHDR));
	if (x != 0)
		{
		// handle error
		//char sz[200];
		//waveInGetErrorText(x, sz, sizeof(sz)-1);
		goto LAbort0;
		}
	cerr <<"zxcv1\n";
	waveInStart(hWaveRd);
	cerr <<"zxcv2\n";

	lpData2Rd = new char[dwDataSize];
	cerr <<"zxcv2.1\n";
	if (!lpData2Rd)
		{
		cerr <<"vss error initializing MMIO audio input: out of memory\n";
		goto LAbort_3;
		}
	hWaveHdr2Rd = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (DWORD)sizeof(WAVEHDR));
	cerr <<"zxcv2.2\n";
	if (!hWaveHdr2Rd)
		goto LAbort_2;
	lpWaveHdr2Rd = (LPWAVEHDR) GlobalLock(hWaveHdr2Rd);
//	cerr <<"clobbered old value of lpWaveHdr2Rd!\n";
//	lpWaveHdr2Rd = new WAVEHDR;
	cerr <<"zxcv2.3\n";
	if (!lpWaveHdr2Rd)
		goto LAbort_1;
	memset(lpData2Rd, 0, dwDataSize);
	lpWaveHdr2Rd->lpData = (char FAR *)lpData2Rd;
	lpWaveHdr2Rd->dwBufferLength = dwDataSize;
	lpWaveHdr2Rd->dwFlags = 0;
	
	cerr <<"zxcv2.4\n";
	waveInPrepareHeader(hWaveRd, lpWaveHdr2Rd, sizeof(WAVEHDR));
	cerr <<"zxcv2.5\n";
	// This next line causes vss to crash.  Nuke ALL the GlobalLock's?
 	x = waveInAddBuffer(hWaveRd, lpWaveHdr2Rd, sizeof(WAVEHDR));
	cerr <<"zxcv3\n";
	if (x != 0)
		{
		// handle error
		//char sz[200];
		//waveInGetErrorText(x, sz, sizeof(sz)-1);
		cerr << "vss error while initializing MMIO audio input: waveInAddBuffer failed.\n";
		goto LAbort_0;
		}
 	// read 2 buffers now, so we're always 1 ahead.

	CSAreal_SYS_Rd = new Mutex;
	cerr <<"vss remark:  MMIO audio input initialized.\n\n";
	return TRUE;

LAbort_0:
	GlobalUnlock(hWaveHdr2Rd);
LAbort_1:
	GlobalFree(hWaveHdr2Rd);
LAbort_2:
LAbort_3:
LAbort0:		
	GlobalUnlock(hWaveHdrRd);
LAbort1:		
	GlobalFree(hWaveHdrRd);
LAbort2:
LAbort3:		
	waveInReset(hWaveRd);
	x = waveInClose(hWaveRd);
	cerr << "vss error: failed to open MMIO audio input.\n";
	return FALSE;
#endif
}


#include "fmod.h" // Just for FSOUND_STREAM, we don't really use anything.
extern void streamcallback(FSOUND_STREAM*, void *buff, int len, int param);
extern short* vrgsCallback;
/*extern int vcbCallback;*/

int areal_internal_FInitAudio(int nchansOut, int nchansIn, int fSoundIn, int cwFrameArg)
{
	cwFrame = cwFrameArg;
	memset(rgsFrame, 0, 2*cwFrame);
	memset(rgsFrameRd, 0, 2*cwFrame);

#ifndef DISABLE_INPUT
	vfSoundIn = 0;
	if (fSoundIn)
		{
		if (areal_internal_FInitAudioRead(nchansIn))
			vfSoundIn = 1;
		// No error message.  Fall back to output-only if input failed.
		}
#endif
	if (!areal_internal_FInitAudioWrite(nchansOut))
		return 0;

	// Cygwin doesn't appear to have pthreads, and says it isn't thread-safe.
	// The faq recommends using _spawnl(), which runs another .exe,
	// as being faster than fork() (uses CreateProcess()).
	// Looks like doing everything in one thread will be better for now.
	//
	// But if we were to spawn a ComputeThread(), we'd do it here.

	// Prime the pump!
	streamcallback(NULL, rgsFrame, cwFrame*2 /*vcbCallback*/, 0);

	return 1;
}

void areal_internal_TermAudio(void)
{
	// quit now: reset, then wait for windows to notice.
#ifndef DISABLE_INPUT
	if (vfSoundIn)
		waveInReset(hWaveRd);
#endif
    for (int iBond=0; iBond<cBond; iBond++)
		{
		waveOutReset(hWaveWr[iBond]);
		// The following waveOutUnprepareHeader() sometimes causes a click.
		// This is particularly noticeable if you wait between the Reset and Unprepare.
		// Multiple waveOutReset()'s don't work around the problem.
		// And the manual says we can't do waveOutUnprepareHeader (or anything
		// with hWaveWr[iBond]) after waveOutClose(hWaveOut).

		// gotta wait until it's finished playing before doing this!
		if (waveOutUnprepareHeader(hWaveWr[iBond], lpWaveHdrWr[iBond], sizeof(WAVEHDR)) != 0)
			{
			// try it again.  if it still fails, what can we do?
			Sleep(500);
			if (waveOutUnprepareHeader(hWaveWr[iBond], lpWaveHdrWr[iBond], sizeof(WAVEHDR)) != 0)
				{
				// Need to handle this case.  globalunlock and globalfree will gpfault.
				cerr <<"vss error closing MMIO audio output: waveOutUnprepareHeader failed\n";
				return;
				}
			}
		if (waveOutUnprepareHeader(hWaveWr[iBond], lpWaveHdr2Wr[iBond], sizeof(WAVEHDR)) != 0)
			{
			cerr <<"vss error closing MMIO audio output: waveOutUnprepareHeader failed\n";
			}

		GlobalUnlock(hWaveHdrWr[iBond]);
		GlobalFree(hWaveHdrWr[iBond]);
		GlobalUnlock(hWaveHdr2Wr[iBond]);
		GlobalFree(hWaveHdr2Wr[iBond]);
		(void)waveOutClose(hWaveWr[iBond]);
		}
	delete CSAreal_SYS_Wr;

#ifndef DISABLE_INPUT
	if (vfSoundIn)
		{
		if (waveInUnprepareHeader(hWaveRd, lpWaveHdrRd, sizeof(WAVEHDR)) != 0)
			{
			cerr <<"vss error closing MMIO audio input: waveInUnprepareHeader failed\n";
			return;
			}
		if (waveInUnprepareHeader(hWaveRd, lpWaveHdr2Rd, sizeof(WAVEHDR)) != 0)
			{
			cerr <<"vss error closing MMIO audio input: waveInUnprepareHeader failed\n";
			}

		GlobalUnlock(hWaveHdrRd);
		GlobalFree(hWaveHdrRd);
		GlobalUnlock(hWaveHdr2Rd);
		GlobalFree(hWaveHdr2Rd);
		(void)waveInClose(hWaveRd);
		delete CSAreal_SYS_Rd;
		}
#endif
}


int areal_internal_FSendNow(void)
{
	int w = 0;
	CSAreal_SYS_Wr->Lock();
    if (cDoneWr > 0)
		{
		cDoneWr--;
		w = 1;
		}
	CSAreal_SYS_Wr->Unlock();
	return w;
}

void areal_internal_Send(short* rgsamp, int csamp)
{
	fWhichWr ^= 1;
	if (cBond == 1)
		{
		const int iBond=0;
			{
			short* lpw=(short*)(fWhichWr ? lpDataWr[iBond] : lpData2Wr[iBond]);
			memcpy(lpw, rgsamp, csamp * sizeof(short));
			(void)waveOutWrite(hWaveWr[iBond],
				fWhichWr ? lpWaveHdrWr[iBond] : lpWaveHdr2Wr[iBond],
				sizeof(WAVEHDR));
			}
		}
	else
		{
		// Tricky:  lpw points to frames of cBond*2 shorts,
		// which need to be split into cBond stereo pairs of shorts.
		// csamp is the # of samples, so csamp/cBond is the # of samps per dev.

		int iBond;
		for (iBond=0; iBond<cBond; iBond++)
			{
			short* lpw=(short*)(fWhichWr ? lpDataWr[iBond] : lpData2Wr[iBond]);

			int iSrc=iBond*2; // *2 for stereo.  Initial offset into the src.
			int iDst=0;
			const int iDstMax = csamp/cBond;
			const int diSrc = (cBond-1)*2;
			while (iDst < iDstMax)
				{
				lpw[iDst++] = rgsamp[iSrc++];
				lpw[iDst++] = rgsamp[iSrc++];
				iSrc += diSrc;
				}
			}
		for (iBond=0; iBond<cBond; iBond++)
			{
			(void)waveOutWrite(hWaveWr[iBond],
				fWhichWr ? lpWaveHdrWr[iBond] : lpWaveHdr2Wr[iBond],
				sizeof(WAVEHDR));
			}
		}
	if (vfdOutput >= 0)
		write(vfdOutput, rgsamp, csamp * sizeof(short));
}

#ifndef DISABLE_INPUT
int areal_internal_FRecvNow(void)
{
	if (!vfSoundIn)
		return 0;

	int w = 0;
	CSAreal_SYS_Rd->Lock();
    if (cDoneRd > 0)
		{
		cDoneRd--;
		w = 1;
		}
	CSAreal_SYS_Rd->Unlock();
	return w;
}
#endif

void areal_internal_Recv(short* rgsamp, int csamp)
{
#ifndef DISABLE_INPUT
	(void)waveInAddBuffer(hWaveRd, !fWhichRd ? lpWaveHdrRd : lpWaveHdr2Rd, sizeof(WAVEHDR));
	fWhichRd ^= 1;
	short* lpw = (short*)(!fWhichRd ? lpData2Rd : lpDataRd);
	for (int i=0; i<csamp; i++)
		rgsamp[i] = lpw[i];
#endif
}


extern void areal_internal_Frame(void)
{
	// get input from the sound hardware
	int fRecvNow = areal_internal_FRecvNow();
	if (fRecvNow)
		areal_internal_Recv(rgsFrameRd, cwFrame);

	static int fFilled = 0;
	if (!fFilled)
		{
		// Ask for another buffer of samples.
		// (vrgsCallback can be NULL during initialization;
		// do nothing, in that case.)
		if (vrgsCallback)
			{
			streamcallback(NULL, rgsFrame, cwFrame*2 /*vcbCallback*/, 0);
			}
		fFilled = 1;
		}

	int fSendNow = areal_internal_FSendNow();
	if (fSendNow)
		{
		areal_internal_Send(rgsFrame, cwFrame);
		fFilled = 0;
		}
}

#endif
