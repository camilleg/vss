const char* GetVssLibVersion()
{
	return "Virtual Sound Server v4.2.1"
#ifdef DEBUG
		" debug"
#endif
		;
}

const char* GetVssLibDate()
{
	return __TIMESTAMP_ISO8601__;
}
