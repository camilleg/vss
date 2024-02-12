const char* GetVssLibVersion()
{
	return "Virtual Sound Server v4.3.0"
#ifdef DEBUG
		" debug"
#endif
		;
}

const char* GetVssLibDate()
{
	return __TIMESTAMP_ISO8601__;
}
