#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "glk.h"
#include "glkstart.h"
#include "gi_blorb.h"
#include "garversion.h"

#define T_ADRIFT	"scare"
#define T_ADVSYS	"advsys"
#define T_AGT	"agility"
#define T_ALAN2	"alan2"
#define T_ALAN3	"alan3"
#define T_GLULX "git"
#define T_HUGO	"hugo"
#define T_LEV9	"level9"
#define T_MGSR	"magnetic"
#define T_TADS2 "tadsr"
#define T_TADS3 "tadsr"
#define T_ZCODE "frotz"
#define T_ZSIX  "nitfol"

#define ID_ZCOD (giblorb_make_id('Z','C','O','D'))
#define ID_GLUL (giblorb_make_id('G','L','U','L'))

#include <process.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

char argv0[1024];
char dir[1024];
char buf[1024];
char tmp[1024];

char filterlist[] =
"All Games\0*.taf;*.agx;*.d$$;*.acd;*.a3c;*.ulx;*.hex;*.gam;*.t3;*.z?;*.l9;*.sna;*.mag;*.dat;*.blb;*.glb;*.zlb;*.blorb;*.gblorb;*.zblorb\0"
"Adrift Games (*.taf)\0*.taf\0"
"AdvSys Games (*.dat)\0*.dat\0"
"AGT Games (*.agx)\0*.agx;*.d$$\0"
"Alan Games (*.acd,*.a3c)\0*.acd;*.a3c\0"
"Glulxe Games (*.ulx)\0*.ulx;*.blb;*.blorb;*.glb;*.gblorb\0"
"Hugo Games (*.hex)\0*.hex\0"
"Level 9 (*.sna)\0*.sna\0"
"Magnetic Scrolls (*.mag)\0*.mag\0"
"TADS 2 Games (*.gam)\0*.gam;*.t3\0"
"TADS 3 Games (*.t3)\0*.gam;*.t3\0"
"Z-code Games (*.z?)\0*.z?;*.zlb;*.zblorb\0"
"All Files\0*\0"
"\0\0";

void runterp(char *exe, char *flags)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    int res;

    sprintf(tmp, "\"%s\\%s.exe\" %s \"%s\"", dir, exe, flags, buf);

    memset(&si, 0, sizeof si);
    memset(&pi, 0, sizeof pi);

    si.cb = sizeof si;

    res = CreateProcess(
            NULL,	// no module name (use cmd line)
            tmp,	// command line
            NULL,	// security attrs,
            NULL,	// thread attrs,
            FALSE,	// inherithandles
            0,		// creation flags
            NULL,	// environ
            NULL,	// cwd
            &si,	// startupinfo
            &pi		// procinfo
            );

    if (res == 0)
    {
        MessageBoxA(NULL, "Could not start 'terp.\nSorry.", "Gargoyle", MB_ICONERROR);
        exit(1);
    }
    exit(0);
}

void runblorb(void)
{
    char magic[4];
    strid_t file;
    giblorb_result_t res;
    giblorb_err_t err;
    giblorb_map_t *map;

    sprintf(tmp, "Could not load Blorb file:\n%s\n", buf);

    file = glkunix_stream_open_pathname(buf, 0, 0);
    if (!file) {
        MessageBoxA(NULL, tmp, "Gargoyle", MB_ICONERROR);
        exit(1);
    }

    err = giblorb_create_map(file, &map);
    if (err) {
        MessageBoxA(NULL, tmp, "Gargoyle", MB_ICONERROR);
        exit(1);
    }

    err = giblorb_load_resource(map, giblorb_method_FilePos,
            &res, giblorb_ID_Exec, 0);
    if (err) {
        MessageBoxA(NULL, tmp, "Gargoyle", MB_ICONERROR);
        exit(1);
    }

    glk_stream_set_position(file, res.data.startpos, 0);
    glk_get_buffer_stream(file, magic, 4);

    switch (res.chunktype)
    {
    case ID_ZCOD:
        if (magic[0] == 6)
            runterp(T_ZSIX, ""); 
        else
            runterp(T_ZCODE, "");
        break;

    case ID_GLUL:
        runterp(T_GLULX, "");
        break;

    default:
        sprintf(tmp, "Unknown game type in Blorb file:\n%s\n", buf);
        MessageBoxA(NULL, tmp, "Gargoyle", MB_ICONERROR);
        exit(1);
    }
}

int main(int argc, char **argv)
{
    char *title = "Gargoyle " VERSION;

    OPENFILENAME ofn;
    char *ext;

    /* get name of executable */
    strcpy(argv0, argv[0]);

    /* get dir of executable */
    strcpy(dir, argv[0]);
    strrchr(dir, '\\')[0] = '\0';

    if (argc == 2)
    {
        strcpy(buf, argv[1]);
    }
    else
    {
        memset(&ofn, 0, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = NULL;
        ofn.lpstrFile = buf;
        ofn.nMaxFile = sizeof buf;
        ofn.lpstrInitialDir = NULL;
        ofn.lpstrTitle = title;
        ofn.lpstrFilter = filterlist;
        ofn.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
        if (!GetOpenFileName(&ofn))
            strcpy(buf, "");
    }

    if (strlen(buf) == 0)
        exit(0);

    ext = strrchr(buf, '.');
    if (ext)
        ext++;
    else
        ext = "";

    if (!strcasecmp(ext, "blb"))
        runblorb();
    if (!strcasecmp(ext, "blorb"))
        runblorb();
    if (!strcasecmp(ext, "glb"))
        runblorb();
    if (!strcasecmp(ext, "gbl"))
        runblorb();
    if (!strcasecmp(ext, "gblorb"))
        runblorb();
    if (!strcasecmp(ext, "zlb"))
        runblorb();
    if (!strcasecmp(ext, "zbl"))
        runblorb();
    if (!strcasecmp(ext, "zblorb"))
        runblorb();

    if (!strcasecmp(ext, "dat"))
        runterp(T_ADVSYS, "");

    if (!strcasecmp(ext, "d$$"))
        runterp(T_AGT, "-gl");
    if (!strcasecmp(ext, "agx"))
        runterp(T_AGT, "-gl");

    if (!strcasecmp(ext, "acd"))
        runterp(T_ALAN2, "");

    if (!strcasecmp(ext, "a3c"))
        runterp(T_ALAN3, "");

    if (!strcasecmp(ext, "taf"))
        runterp(T_ADRIFT, "");

    if (!strcasecmp(ext, "ulx"))
        runterp(T_GLULX, "");

    if (!strcasecmp(ext, "hex"))
        runterp(T_HUGO, "");

    if (!strcasecmp(ext, "gam"))
        runterp(T_TADS2, "");

    if (!strcasecmp(ext, "t3"))
        runterp(T_TADS3, "");

    if (!strcasecmp(ext, "z1"))
        runterp(T_ZCODE, "");
    if (!strcasecmp(ext, "z2"))
        runterp(T_ZCODE, "");
    if (!strcasecmp(ext, "z3"))
        runterp(T_ZCODE, "");
    if (!strcasecmp(ext, "z4"))
        runterp(T_ZCODE, "");
    if (!strcasecmp(ext, "z5"))
        runterp(T_ZCODE, "");
    if (!strcasecmp(ext, "z7"))
        runterp(T_ZCODE, "");
    if (!strcasecmp(ext, "z8"))
        runterp(T_ZCODE, "");

    if (!strcasecmp(ext, "z6"))
        runterp(T_ZSIX, "");

    if (!strcasecmp(ext, "l9"))
        runterp(T_LEV9, "");
    if (!strcasecmp(ext, "sna"))
        runterp(T_LEV9, "");
    if (!strcasecmp(ext, "mag"))
        runterp(T_MGSR, "");

    sprintf(tmp, "Unknown file type: \"%s\"\nSorry.", ext);
    MessageBoxA(NULL, tmp, "Gargoyle", MB_ICONERROR);

    return 1;
}

