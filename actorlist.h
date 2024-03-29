// Only for vssSrv.c++'s newActor().
#pragma once

// grep ACTOR_SETUP */*.cpp
#undef ACTOR_SETUP // Override VActor.h's definition with a compatible but typeless one.
#define ACTOR_SETUP(ignored, t) extern VActor* t##_New();
ACTOR_SETUP(addActor, AddActor)
ACTOR_SETUP(analogActor, AnalogActor)
ACTOR_SETUP(AmplActor, AmplitudeAnalyzer)
ACTOR_SETUP(PitchActor, PitchAnalyzer)
ACTOR_SETUP(BasicActor, BasicActor)
ACTOR_SETUP(iter1Actor, BasicIterator)
ACTOR_SETUP(chuaActor, ChuaActor)
ACTOR_SETUP(loopActor, LoopActor)
ACTOR_SETUP(CrowdActor, CrowdActor)
ACTOR_SETUP(PrintfActor, PrintfActor)
ACTOR_SETUP(delayActor, DelayActor)
ACTOR_SETUP(distanceActor, DistanceActor)
ACTOR_SETUP(EnvelopeActor, EnvelopeActor)
ACTOR_SETUP(order1FiltActor, Order1FilterActor)
ACTOR_SETUP(biquadFiltActor, BiQuadFilterActor)
ACTOR_SETUP(fmActor, FmActor)
ACTOR_SETUP(fmmActor, FmmActor)
ACTOR_SETUP(granActor, Granulator)
ACTOR_SETUP(inputActor, InputActor)
ACTOR_SETUP(LaterActor, LaterActor)
ACTOR_SETUP(LedActor, LedActor)
ACTOR_SETUP(logisticActor, LogisticActor)
ACTOR_SETUP(LinearMapActor, LinearMapper)
ACTOR_SETUP(ExpMapActor, ExponentialMapper)
ACTOR_SETUP(SegmentMapActor, SegmentMapper)
ACTOR_SETUP(HidimMapActor, HidimMapper)
ACTOR_SETUP(pianoActor, PianoActor)
ACTOR_SETUP(PlaneMapActor, PlaneMapper)
ACTOR_SETUP(mixerActor, MixerActor)
ACTOR_SETUP(MessageGroup, MessageGroup)
ACTOR_SETUP(noiseActor, NoiseActor)
ACTOR_SETUP(OSCActor, OSCActor)
ACTOR_SETUP(OSCServer, OSCServer)
ACTOR_SETUP(ParticleActor, ParticleActor)
ACTOR_SETUP(PentaActor, PentaActor)
ACTOR_SETUP(pnoiseActor, PseudoNoiseActor)
ACTOR_SETUP(processActor, ProcessActor)
ACTOR_SETUP(reverbActor, ReverbActor)
ACTOR_SETUP(ringmodActor, RingModActor)
ACTOR_SETUP(sampActor, SampleActor)
ACTOR_SETUP(SeqActor, SeqActor)
ACTOR_SETUP(shimmerActor, ShimmerActor)
ACTOR_SETUP(smActor, SmActor)
ACTOR_SETUP(stereoActor, StereoActor)
ACTOR_SETUP(stkActor, StkActor)
ACTOR_SETUP(tb303Actor, Tb303Actor)
ACTOR_SETUP(ThresholdActor, ThresholdActor)
ACTOR_SETUP(SwitchActor, SwitchActor)
#undef ACTOR_SETUP

// Init doesn't warn about duplicate keys.
static const std::map<std::string, VActor*(*)()> m = {

#define FOO(T, t) { T, t##_New },

FOO("AddActor", AddActor)
FOO("AnalogActor", AnalogActor)
FOO("AmplActor", AmplitudeAnalyzer)
FOO("PitchActor", PitchAnalyzer)
FOO("BasicActor", BasicActor)
FOO("Iter1Actor", BasicIterator)
FOO("ChuaActor", ChuaActor)
FOO("LoopActor", LoopActor)
FOO("CrowdActor", CrowdActor)
FOO("PrintfActor", PrintfActor)
FOO("DelayActor", DelayActor)
FOO("DistanceActor", DistanceActor)
FOO("EnvelopeActor", EnvelopeActor)
FOO("Order1FiltActor", Order1FilterActor)
FOO("BiquadFiltActor", BiQuadFilterActor)
FOO("FmActor", FmActor)
FOO("FmmActor", FmmActor)
FOO("GranActor", Granulator)
FOO("InputActor", InputActor)
FOO("LaterActor", LaterActor)
FOO("logisticActor", LogisticActor)
FOO("LedActor", LedActor)
FOO("LinearMapActor", LinearMapper)
FOO("ExpMapActor", ExponentialMapper)
FOO("SegmentMapActor", SegmentMapper)
FOO("HidimMapActor", HidimMapper)
FOO("PlaneMapActor", PlaneMapper)
FOO("MixerActor", MixerActor)
FOO("MessageGroup", MessageGroup)
FOO("NoiseActor", NoiseActor)
FOO("OSCActor", OSCActor)
FOO("OSCServer", OSCServer)
FOO("ParticleActor", ParticleActor)
FOO("PentaActor", PentaActor)
FOO("PianoActor", PianoActor)
FOO("PseudoNoiseActor", PseudoNoiseActor)
FOO("ProcessActor", ProcessActor)
FOO("ReverbActor", ReverbActor)
FOO("RingmodActor", RingModActor)
FOO("SampActor", SampleActor)
FOO("SampleActor", SampleActor)		// backwards compatible
FOO("SeqActor", SeqActor)
FOO("ShimmerActor", ShimmerActor)
FOO("SmActor", SmActor)
FOO("StereoActor", StereoActor)
FOO("StkActor", StkActor)
//FOO("AgogobelActor", AgogoBelActor)
//FOO("BowedActor", BowedActor)
//FOO("BrassActor", BrassActor)
//FOO("ClarinetActor", ClarinetActor)
//FOO("FluteActor", FluteActor)
//FOO("FmVoicesActor", FMVoicesActor)
//FOO("MandolinActor", MandolinActor)
//FOO("MarimbaActor", MarimbaActor)
//FOO("ShakerActor", ShakerActor)
//FOO("TubebellActor", TubeBellActor)
//FOO("VibraphnActor", VibraphnActor)
//FOO("VoicFormActor", VoicFormActor)
FOO("Tb303Actor", Tb303Actor)
FOO("ThresholdActor", ThresholdActor)
FOO("SwitchActor", SwitchActor)
#undef FOO
};
