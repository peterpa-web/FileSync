/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2009  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */


#include "..\PrecompiledHeader.h"

#include "IsoFS.h"
#include "IsoFile.h"

//////////////////////////////////////////////////////////////////////////
// IsoDirectory
//////////////////////////////////////////////////////////////////////////

// Used to load the Root directory from an image
IsoDirectory::IsoDirectory(SectorSource& r) :
        internalReader(r)
{
        u8 sector[2448];

        internalReader.readSector(sector,16);		// prim vol descr (ecma 8.4)

		// For a Root Directory:
		// ? the first Directory Record of the Root Directory shall describe the Root Directory and shall have a Directory Identifier consisting of a single (00) byte;
		// ? the second Directory Record of the Root Directory shall describe the Root Directory and shall have a Directory Identifier consisting of a single (01) byte;
		// ? a Directory Record describing the Root Directory shall be contained in the Root Directory field of the volume descriptor that identifies the directory hierarchy.

        IsoFileDescriptor rootDirEntry(sector+156,38);

        Init(rootDirEntry);
}

// Used to load a specific directory from a file descriptor
IsoDirectory::IsoDirectory(SectorSource& r, IsoFileDescriptor directoryEntry) :
        internalReader(r)
{
        Init(directoryEntry);
}

IsoDirectory::~IsoDirectory() throw()
{
}

void IsoDirectory::Init(const IsoFileDescriptor& directoryEntry)
{
        // parse directory sector
        IsoFile dataStream (internalReader, directoryEntry);

        //files.clear();
        files.RemoveAll();

        int remainingSize = directoryEntry.size;

        u8 b[257];

        while(remainingSize>=4) // hm hack :P
        {
                b[0] = dataStream.read<u8>();

                if(b[0]==0)
                {
                        break; // or continue?
                }

                remainingSize -= b[0];

                dataStream.read(b+1, b[0]-1);

                // files.push_back(IsoFileDescriptor(b, b[0]));
                files.Add(IsoFileDescriptor(b, b[0]));
        }

        b[0] = 0;
}

void IsoDirectory::TraceFiles() const
{
	for ( int i=0; i < files.GetCount(); ++i )
	{
		if ( files.GetAt(i).IsFile() )
			TRACE0("  F: ");
		else
			TRACE0("  D: ");
		TRACE1("%s\n", files.GetAt(i).name );
	}
}

const IsoFileDescriptor& IsoDirectory::GetEntry(int index) const
{
        return files[index];
}

int IsoDirectory::GetIndexOf(const wxString& fileName) const
{
        //for(unsigned int i=0;i<files.size();i++)
        for( int i=0; i<files.GetCount(); i++)
        {
                if(files[i].name == fileName) return i;
        }
		wxString fileNameX = fileName + ";1";
        for( int i=0; i<files.GetCount(); i++)
        {
                if(files[i].name == fileNameX) return i;
        }

        //throw Exception::FileNotFound( fileName );
		ASSERT( FALSE );
		AfxThrowFileException(CFileException::fileNotFound, -1, fileName );
}

const IsoFileDescriptor& IsoDirectory::GetEntry(const wxString& fileName) const
{
        return GetEntry(GetIndexOf(fileName));
}

IsoFileDescriptor IsoDirectory::FindFile(const wxString& filePath) const
{
        ASSERT( !filePath.IsEmpty() );

        // wxWidgets DOS-style parser should work fine for ISO 9660 path names.  Only practical difference
        // is case sensitivity, and that won't matter for path splitting.
        //wxFileName parts( filePath, wxPATH_DOS );
		CStringArray parts;
		int nPos=0;
		CString part;
		while (TRUE) {
			part = filePath.Tokenize(_T("/\\"), nPos);
			if (part.IsEmpty())
				break;
			parts.Add( part );
		}

        IsoFileDescriptor info;
        const IsoDirectory* dir = this;
        // ScopedPtr<IsoDirectory> deleteme;
		IsoDirectory* deleteme = NULL;

        // walk through path ("." and ".." entries are in the directories themselves, so even if the
        // path included . and/or .., it still works)

        //for(uint i=0; i<parts.GetDirCount(); ++i)
        for(int i=0; i<parts.GetCount()-1; ++i)
        {
                //info = dir->GetEntry(parts.GetDirs()[i]);
                info = dir->GetEntry(parts[i]);
                if(info.IsFile()) {//throw Exception::FileNotFound( filePath );
					ASSERT( FALSE );
					AfxThrowFileException(CFileException::genericException, -1, filePath );
				}
				if ( deleteme != NULL )
					delete deleteme;
                dir = deleteme = new IsoDirectory(internalReader, info);
        }

//        if( !parts.GetFullName().IsEmpty() )
  //              info = dir->GetEntry(parts.GetFullName());
		if ( !parts[parts.GetCount()-1].IsEmpty() )
			info = dir->GetEntry( parts[parts.GetCount()-1] );

		if ( deleteme != NULL )
			delete deleteme;
        return info;
}

bool IsoDirectory::IsFile(const wxString& filePath) const
{
        if( filePath.IsEmpty() ) return false;
        return (FindFile(filePath).flags&2) != 2;
}

bool IsoDirectory::IsDir(const wxString& filePath) const
{
        if( filePath.IsEmpty() ) return false;
        return (FindFile(filePath).flags&2) == 2;
}

u32 IsoDirectory::GetFileSize( const wxString& filePath ) const
{
        return FindFile( filePath ).size;
}

IsoFileDescriptor::IsoFileDescriptor()
{
        lba = 0;
        size = 0;
        flags = 0;
}

IsoFileDescriptor::IsoFileDescriptor(const u8* data, int length)
{
        lba             = (u32&)data[2];
        size    = (u32&)data[10];

        date.year      = data[18] + 1900;
        date.month     = data[19];
        date.day       = data[20];
        date.hour      = data[21];
        date.minute    = data[22];
        date.second    = data[23];
        date.gmtOffset = data[24];

        flags = data[25];

        int fileNameLength = data[32];

        if(fileNameLength==1)
        {
                u8 c = data[33];

                switch(c)
                {
                        case 0: name = L"."; break;
                        case 1: name = L".."; break;
                        default: name = (wxChar)c;
                }
        }
        else
        {
                // copy string and up-convert from ascii to wxChar

                const u8* fnsrc = data+33;
                const u8* fnend = fnsrc+fileNameLength;

                while( fnsrc != fnend )
                {
                        name += (wxChar)*fnsrc;
                        ++fnsrc;
                }
        }
}

