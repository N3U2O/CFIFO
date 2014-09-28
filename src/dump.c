//- --------------------------------------------------------------------------
//! \file   dump.c
//!
//! \brief  A utility function to dump data in memory at the given address.
//!         Code borrowed from paxdiablo who answered to the Stack Overflow
//!         question here:
//!         http://stackoverflow.com/questions/7775991/
//!                                     how-to-get-hexdump-of-a-structure-data
//- --------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>

void hexDump(const char *desc, void *addr, uint32_t len)
{
    uint32_t i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL) printf ("%s\n", desc);
    // Process every byte in the data.
    for (i = 0; i < len; i++)
    {
        // Multiple of 16 means new line (with line offset).
        if ((i % 16) == 0)
        {
            // Just don't print ASCII for the zeroth line.
            if (i != 0) printf ("  %s\n", buff);
            // Output the offset.
            printf ("  %04X ", i);
        }
        printf (" %02X", pc[i]); // Now the hex code for the specific char.
        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }
    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }
    // And print the final ASCII bit.
    printf ("  %s\n", buff);
}

/**[ Usage ]*****************************************************************

int main (int argc, char *argv[]) {
    char my_str[] = "a char string greater than 16 chars";
    hexDump ("my_str", &my_str, sizeof (my_str));
    return 0;
}

 ****************************************************************************/