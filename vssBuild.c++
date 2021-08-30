extern const float vssversion = 4.2f;

extern "C" const char* GetVssLibVersion()
{
	return "Virtual Sound Server v4.2.1"
#ifdef DEBUG
		" debug"
#endif
		;
}

extern "C" const char* GetVssLibDate()
{
	return __TIMESTAMP_ISO8601__;
}
