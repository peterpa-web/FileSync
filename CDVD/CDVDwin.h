#pragma once

#define fromUTF8(name) name

// basic types
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned long
#define u64 unsigned long long
#define uint unsigned int
#define s32 long
#define wxString CString
#define wxChar TCHAR
#define btoi(b)         ((b)/16*10 + (b)%16)            /* BCD to u_char */
#define itob(i)         ((i)/10*16 + (i)%10)            /* u_char to BCD */

static __forceinline s32 msf_to_lsn(u8 *Time)
{
        u32 lsn;

        lsn = Time[2];
        lsn +=(Time[1] - 2) * 75;
        lsn += Time[0] * 75 * 60;
        return lsn;
}

static __forceinline s32 msf_to_lba(u8 m, u8 s, u8 f)
{
        u32 lsn;
        lsn = f;
        lsn += (s - 2) * 75;
        lsn += m * 75 * 60;
        return lsn;
}

static __forceinline void lsn_to_msf(u8 *Time, s32 lsn)
{
        lsn += 150;
        Time[2] = (u8)(lsn / 4500);                   // minuten
        lsn = lsn - Time[2] * 4500;					  // minuten rest
        Time[1] = (u8)(lsn / 75);                     // sekunden
        Time[0] = (u8)(lsn - Time[1] * 75);           // sekunden rest
}


static __forceinline void lba_to_msf(s32 lba, u8* m, u8* s, u8* f)
{
        lba += 150;
        *m = (u8)(lba / (60 * 75));
        *s = (u8)((lba / 75) % 60);
        *f = (u8)(lba % 75);
}
