extern const float vssversion = 4.2f;

#ifdef DEBUG
#define VSS_VERSION_NUMBER "4.2 DEBUG"
#else
#define VSS_VERSION_NUMBER "4.2"
#endif

extern "C" const char* GetVssLibVersion(void)
{
	return "VSS Sound Server V" VSS_VERSION_NUMBER

#if 0

#if defined(VSS_LINUX)
#if defined(VSS_LINUX_20ALSA)
		", Linux ALSA"
#elif defined(VSS_LINUX_21ALSA)
		", Linux ALSA"
#elif defined(VSS_LINUX_SONORUS)
		", Linux, Sonorus STUDI/O"
#else
		", Linux"
#endif

#elif defined(VSS_WINDOWS)
		", Win32"

#elif defined(VSS_IRIX)
#if defined(VSS_IRIX_53)
		", Irix 5.3"
#elif defined(VSS_IRIX_62)
		", Irix 6.2"
#elif defined(VSS_IRIX_63)
		", Irix 6.3"
#elif defined(VSS_IRIX_65)
		", Irix 6.5"
#elif defined(VSS_IRIX_65_MIPS3)
		", Irix 6.5 (R4000)"
#else
#error unspecified subplatform of VSS_IRIX
#endif

#elif defined(VSS_FreeBSD)
		", FreeBSD 3.4"

#elif defined(VSS_SOLARIS)
		", Solaris 8"

#else
#error unspecified platform
#endif

#endif
		;
}

extern "C" const char* GetVssLibDate(void)
{
	return __DATE__ ", " __TIME__;
}
