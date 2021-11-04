#pragma once
#include "CDVDwin.h"
#include "IsoFS/IsoFS.h"
#include "IsoFS/SectorSource.h"
#include "IsoFileFormats.h"

class IsoNativeFile : public SectorSource
{
public:
	IsoNativeFile();
	IsoNativeFile( const TCHAR *pszFilePath );
	void Open( const TCHAR *pszFilePath );
    virtual ~IsoNativeFile() throw();
    virtual int getNumSectors(); 
    virtual bool readSector(unsigned char* buffer, int lba);

protected:
	isoFile *m_iso;
};