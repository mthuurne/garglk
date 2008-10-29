// $Id: savefile.c,v 1.6 2003/10/20 16:05:06 iain Exp $

#include "git.h"

static void writeWord (git_sint32 word)
{
    char buffer [4];
    write32 (buffer, word);
    glk_put_buffer (buffer, 4);
}

static git_uint32 readWord (strid_t file)
{
    char buffer [4];
    glk_get_buffer_stream (file, buffer, 4);
    return (git_uint32) read32 (buffer);
}

git_sint32 restoreFromFile (git_sint32 * base, git_sint32 id,
    git_uint32 protectPos, git_uint32 protectSize)
{
    git_uint32 protectEnd = protectPos + protectSize;
    git_uint32 i;
    strid_t file;
    glui32 fileSize, fileStart;

    int gotIdent = 0;
    int gotMemory = 0;
    int gotStack = 0;

    // Find out what stream they want to use, and make sure it's valid.
    file = git_find_stream_by_id (id);
    if (file == 0)
        return 1;

    // Read IFF header.
    if (readWord (file) != read32("FORM"))
        return 1; // Not an IFF file.
    
    fileSize = readWord (file);
    fileStart = glk_stream_get_position (file);
    
    if (readWord (file) != read32("IFZS"))
        return 1; // Not a Quetzal file.

    // Read all the chunks.
    
    while (glk_stream_get_position(file) < fileStart + fileSize)
    {
        git_uint32 chunkType, chunkSize, chunkStart;
        chunkType = readWord (file);
        chunkSize = readWord (file);
        chunkStart = glk_stream_get_position (file);

        if (chunkType == read32("IFhd"))
        {
            if (gotIdent)
                return 1;

            gotIdent = 1;

            if (chunkSize != 128)
                return 1;

            for (i = 0 ; i < 128 ; ++i)
            {
                glui32 c = glk_get_char_stream (file);
                if (gRom [i] != c)
                    return 1;
            }
        }
        else if (chunkType == read32("Stks"))
        {
            if (gotStack)
                return 1;

            gotStack = 1;

            if (chunkSize & 3)
                return 1;

            gStackPointer = base;
            for ( ; chunkSize > 0 ; chunkSize -= 4)
                *gStackPointer++ = readWord(file);
        }
        else if (chunkType == read32("CMem"))
        {
            git_uint32 bytesRead = 0;
            if (gotMemory)
                return 1;

            gotMemory = 1;

            if (readWord(file) != gEndMem)
                return 1;

            bytesRead = 4;
            i = gRamStart;
            while (i < gExtStart && bytesRead < chunkSize)
            {
                int mult = 0;
                char c = (char) glk_get_char_stream(file);
                ++bytesRead;
                
                if (c == 0)
                {
                    mult = (unsigned char) glk_get_char_stream(file);
                    ++bytesRead;
                }
                
                for (++mult ; mult > 0 ; --mult, ++i)
                    if (i >= protectEnd || i < protectPos)
                        gRam [i] = gRom [i] ^ c;
            }

            while (i < gEndMem && bytesRead < chunkSize)
            {
                int mult = 0;
                char c = (char) glk_get_char_stream(file);
                ++bytesRead;
                
                if (c == 0)
                {
                    mult = (unsigned char) glk_get_char_stream(file);
                    ++bytesRead;
                }
                
                for (++mult ; mult > 0 ; --mult, ++i)
                    if (i >= protectEnd || i < protectPos)
                        gRam [i] = c;
            }

            while (i < gExtStart)
                if (i >= protectEnd || i < protectPos)
                    gRam [i] = gRom [i], ++i;

            while (i < gEndMem)
                if (i >= protectEnd || i < protectPos)
                    gRam [i] = 0, ++i;

            if (bytesRead != chunkSize)
                return 1; // Too much data!

            if (chunkSize & 1)
                glk_get_char_stream (file);
        }
        else if (chunkType == read32("MAll"))
        {
            if (chunkSize != 0)
            {
                fatalError ("Sorry, this saved game can't be loaded, because it uses "
                    "dynamic memory allocation which Git does not currently support.");
            }
        }
        else
        {
            // Unknown chunk type -- just skip it.
            glk_stream_set_position (file, (chunkSize + 1) & ~1, seekmode_Current);
        }
    }

    // Make sure we have all the chunks we need.

    if (!gotIdent)
        fatalError ("No ident chunk in save file");

    if (!gotStack)
        fatalError ("No stack chunk in save file");

    if (!gotMemory)
        fatalError ("No memory chunk in save file");

    // If we reach this point, we restored successfully.

    return 0;
}

git_sint32 saveToFile (git_sint32 * base, git_sint32 * sp, git_sint32 id)
{
    git_uint32 n, zeroCount;
    glui32 fileSize, fileSizePos;
    glui32 memSize, memSizePos;

    strid_t file, oldFile;

    // Find out what stream they want to use, and make sure it's valid.
    file = git_find_stream_by_id (id);
    if (file == 0)
        return 1;

    // Make the given stream the default.
    oldFile = glk_stream_get_current ();
    glk_stream_set_current (file);

    // Write Quetzal header.
    glk_put_string ("FORM");

    fileSizePos = glk_stream_get_position (file);
    writeWord (0);

    glk_put_string ("IFZS");

    // Header chunk.
    glk_put_string ("IFhd");
    writeWord (128);
    glk_put_buffer ((char *) gRom, 128);

    // Stack chunk.
    glk_put_string ("Stks");
    writeWord ((sp - base) * 4);
    for (n = 0 ; n < (git_uint32) (sp - base) ; ++n)
        writeWord (base [n]);

    // Memory chunk.
    glk_put_string ("CMem");
    memSizePos = glk_stream_get_position (file);
    writeWord (0);

    writeWord (gEndMem);
    for (zeroCount = 0, n = gRamStart ; n < gEndMem ; ++n)
    {
        unsigned char c = ((git_uint32) gRom[n]) ^ ((git_uint32) gRam[n]);
        if (c == 0)
            ++zeroCount;
        else
        {
            for ( ; zeroCount > 256 ; zeroCount -= 256)
            {
                glk_put_char (0);
                glk_put_char (0xff);
            }

            if (zeroCount > 0)
            {
                glk_put_char (0);
                glk_put_char ((char) (zeroCount - 1));
                zeroCount = 0;
            }

            glk_put_char (c);
        }
    }
    // Note: we don't bother writing out any remaining zeroes,
    // because the memory is padded out with zeroes on restore.

    memSize = glk_stream_get_position (file) - memSizePos - 4;
    if (memSize & 1)
       glk_put_char (0);

    // Back up and fill in the lengths.
    fileSize = glk_stream_get_position (file) - fileSizePos - 4;

    glk_stream_set_position (file, fileSizePos, seekmode_Start);
    writeWord (fileSize);

    glk_stream_set_position (file, memSizePos, seekmode_Start);
    writeWord (memSize);

    // Restore the previous default stream.
    glk_stream_set_current (oldFile);

    // And we're done.
    return 0;
}
