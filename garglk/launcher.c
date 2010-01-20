/******************************************************************************
 *                                                                            *
 * Copyright (C) 2006-2009 by Tor Andersson.                                  *
 * Copyright (C) 2008-2009 by Ben Cressey.                                    *
 * Copyright (C) 2009 by Baltasar Garc�a Perez-Schofield.                     *
 *                                                                            *
 * This file is part of Gargoyle.                                             *
 *                                                                            *
 * Gargoyle is free software; you can redistribute it and/or modify           *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 2 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * Gargoyle is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with Gargoyle; if not, write to the Free Software                    *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA *
 *                                                                            *
 *****************************************************************************/

#include "glk.h"
#include "glkstart.h"
#include "gi_blorb.h"
#include "launcher.h"

#define T_ADRIFT    "scare"
#define T_ADVSYS    "advsys"
#define T_AGT       "agility"
#define T_ALAN2     "alan2"
#define T_ALAN3     "alan3"
#define T_GLULX     "git"
#define T_HUGO      "hugo"
#define T_JACL      "jacl"
#define T_LEV9      "level9"
#define T_MGSR      "magnetic"
#define T_QUEST     "geas"
#define T_TADS2     "tadsr"
#define T_TADS3     "tadsr"
#define T_ZCODE     "frotz"
#define T_ZSIX      "nitfol"

#define ID_ZCOD (giblorb_make_id('Z','C','O','D'))
#define ID_GLUL (giblorb_make_id('G','L','U','L'))

#define MaxBuffer 1024
char tmp[MaxBuffer];

int runblorb(char *path, char *game)
{
    char magic[4];
    strid_t file;
    giblorb_result_t res;
    giblorb_err_t err;
    giblorb_map_t *map;

    sprintf(tmp, "Could not load Blorb file:\n%s\n", game);

    file = glkunix_stream_open_pathname(game, 0, 0);
    if (!file) {
        winmsg(tmp);
        return FALSE;
    }

    err = giblorb_create_map(file, &map);
    if (err) {
        winmsg(tmp);
        return FALSE;
    }

    err = giblorb_load_resource(map, giblorb_method_FilePos,
            &res, giblorb_ID_Exec, 0);
    if (err) {
        winmsg(tmp);
        return FALSE;
    }

    glk_stream_set_position(file, res.data.startpos, 0);
    glk_get_buffer_stream(file, magic, 4);

    switch (res.chunktype)
    {
    case ID_ZCOD:
        if (magic[0] == 6)
            return winterp(path, T_ZSIX, "", game);
        else
            return winterp(path, T_ZCODE, "", game);
        break;

    case ID_GLUL:
            return winterp(path, T_GLULX, "", game);
        break;

    default:
        sprintf(tmp, "Unknown game type in Blorb file:\n%s\n", game);
        winmsg(tmp);
    }
    
    return FALSE;
}

int rungame(char *path, char *game)
{
    char *ext = strrchr(game, '.');

    if (ext)
        ext++;
    else
        ext = "";

    if (!strcasecmp(ext, "blb"))
        return runblorb(path, game);
    if (!strcasecmp(ext, "blorb"))
        return runblorb(path, game);
    if (!strcasecmp(ext, "glb"))
        return runblorb(path, game);
    if (!strcasecmp(ext, "gbl"))
        return runblorb(path, game);
    if (!strcasecmp(ext, "gblorb"))
        return runblorb(path, game);
    if (!strcasecmp(ext, "zlb"))
        return runblorb(path, game);
    if (!strcasecmp(ext, "zbl"))
        return runblorb(path, game);
    if (!strcasecmp(ext, "zblorb"))
        return runblorb(path, game);

    if (!strcasecmp(ext, "dat"))
        return winterp(path, T_ADVSYS, "", game);

    if (!strcasecmp(ext, "d$$"))
        return winterp(path, T_AGT, "-gl", game);
    if (!strcasecmp(ext, "agx"))
        return winterp(path, T_AGT, "-gl", game);

    if (!strcasecmp(ext, "acd"))
        return winterp(path, T_ALAN2, "", game);

    if (!strcasecmp(ext, "a3c"))
        return winterp(path, T_ALAN3, "", game);

    if (!strcasecmp(ext, "taf"))
        return winterp(path, T_ADRIFT, "", game);

    if (!strcasecmp(ext, "ulx"))
        return winterp(path, T_GLULX, "", game);

    if (!strcasecmp(ext, "hex"))
        return winterp(path, T_HUGO, "", game);

    if (!strcasecmp(ext, "jacl"))
        return winterp(path, T_JACL, "", game);
    if (!strcasecmp(ext, "j2"))
        return winterp(path, T_JACL, "", game);

    if (!strcasecmp(ext, "gam"))
        return winterp(path, T_TADS2, "", game);

    if (!strcasecmp(ext, "t3"))
        return winterp(path, T_TADS3, "", game);

    if (!strcasecmp(ext, "z1"))
        return winterp(path, T_ZCODE, "", game);
    if (!strcasecmp(ext, "z2"))
        return winterp(path, T_ZCODE, "", game);
    if (!strcasecmp(ext, "z3"))
        return winterp(path, T_ZCODE, "", game);
    if (!strcasecmp(ext, "z4"))
        return winterp(path, T_ZCODE, "", game);
    if (!strcasecmp(ext, "z5"))
        return winterp(path, T_ZCODE, "", game);
    if (!strcasecmp(ext, "z7"))
        return winterp(path, T_ZCODE, "", game);
    if (!strcasecmp(ext, "z8"))
        return winterp(path, T_ZCODE, "", game);

    if (!strcasecmp(ext, "z6"))
        return winterp(path, T_ZSIX, "", game);

    if (!strcasecmp(ext, "l9"))
        return winterp(path, T_LEV9, "", game);
    if (!strcasecmp(ext, "sna"))
        return winterp(path, T_LEV9, "", game);
    if (!strcasecmp(ext, "mag"))
        return winterp(path, T_MGSR, "", game);

    if (!strcasecmp(ext, "asl"))
        return winterp(path, T_QUEST, "", game);
    if (!strcasecmp(ext, "cas"))
        return winterp(path, T_QUEST, "", game);

    sprintf(tmp, "Unknown file type: \"%s\"\nSorry.", ext);
    winmsg(tmp);
    
    return FALSE;
}
