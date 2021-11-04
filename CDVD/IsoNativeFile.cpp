#include "PrecompiledHeader.h"
#include "IsoNativeFile.h"

IsoNativeFile::IsoNativeFile()
{
	m_iso = NULL;
}

IsoNativeFile::IsoNativeFile( const TCHAR *pszFilePath )
{
	Open( pszFilePath );
}

void IsoNativeFile::Open( const TCHAR *pszFilePath )
{
	m_iso = isoOpen( CStringA(pszFilePath) );
	ASSERT( m_iso != NULL );
	ASSERT( m_iso->type == ISOTYPE_CD || m_iso->type == ISOTYPE_DVD );
}

IsoNativeFile::~IsoNativeFile()
{
	isoClose(m_iso);
}

int  IsoNativeFile::getNumSectors()
{
	return (int)m_iso->blocks;
}
        
bool IsoNativeFile::readSector(unsigned char* buffer, int lba)
{
	return isoReadBlock(m_iso, buffer, lba);
}

bool IsoDirectory::Extract( const wxString& filePath, const wxString& filePathDest,
	LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData )
{
//	IsoFileDescriptor fd = FindFile( filePath );
//	if ( !fd.IsFile() )
//		return false;
	IsoFile isoFile( *this, filePath );
	wxString pathDest = filePathDest + "\\" + filePath;
	u32 len = isoFile.getLength();
	LARGE_INTEGER nTotal;
	nTotal.QuadPart = len;
	void *handle = _openfile( CStringA(pathDest), O_WRONLY | O_CREAT );
	if ( handle == NULL )
		return false;

	DWORD dwStreamNumber = 1;
	DWORD dwReason = CALLBACK_STREAM_SWITCH;
	s32 bufsz = 0x4000;
	void *buf = malloc( bufsz );
	s32 lenRead = bufsz;
	while ( len != 0 )
	{
		if ( len < bufsz )
			lenRead = len;
		s32 rd = isoFile.read( buf, lenRead );
		if ( rd < 1 )
			break;
		u32 wr = _writefile( handle, buf, rd );
		len -= wr;
		if ( lpProgressRoutine != NULL ) {
			LARGE_INTEGER nTransferred;
			nTransferred.QuadPart = nTotal.QuadPart - len;
			LARGE_INTEGER nStreamSize;
			nStreamSize.QuadPart = rd;
			LARGE_INTEGER nStreamTransf;
			nStreamTransf.QuadPart = wr;
			DWORD rc = (*lpProgressRoutine)( nTotal, nTransferred, nStreamSize, nStreamTransf, dwStreamNumber, dwReason, NULL, handle, lpData );
			if ( rc == PROGRESS_CANCEL )
				break;
			++dwStreamNumber;
			dwReason = CALLBACK_CHUNK_FINISHED;
		}
		if ( wr != rd )
			break;
	}
	free( buf );
	_closefile( handle );
	if ( len != 0 ) {	// remove partial file
		DeleteFile( pathDest );
	}
	return ( len == 0 );
}

