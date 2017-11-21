bool __qpluginfactory_is_debug()
{
#ifdef QT_NO_DEBUG
	return false;
#else
	return true;
#endif
}
