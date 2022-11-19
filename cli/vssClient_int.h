#ifndef __CLIENT_H__
#define __CLIENT_H__

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C" {
#endif

extern int fdMidi; /* for MIDI input */
typedef struct
{
	int cb;
	char rgb[4];
} VSSMDevent;

VSSMDevent* GetMidiMessages(float* pcmsg, float hMidiActor);

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
}
#endif

#endif /* __CLIENT_H__ */
