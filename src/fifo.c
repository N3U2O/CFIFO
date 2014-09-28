//- --------------------------------------------------------------------------
//! \file   fifo.c
//! \brief  A FIFO implementation using circular buffer with stored last
//!         operation. The main file tests the functionality of the FIFO.
//! \author Adrian Gugyin
//! \date   28/09/2014
//- --------------------------------------------------------------------------
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

// Turn on debug messages
#define _DEBUG_

// Version (VERMAJOR.VERMINOR[.BUILD_ID])
#define VERMAJOR 0
#define VERMINOR 1
#define BUILD 1

// Macro contants
#define NFIFOCAP 4
#define NITEMS 6
#define NNAMESIZE 20



//- --------------------------------------------------------------------------
//! \brief  FIFO entry structure definiton
//- --------------------------------------------------------------------------
struct FifoEntry
{
    uint8_t ID;
    char    Name [NNAMESIZE];
    clock_t TimeStamp;
};

// FIFO entry typedef
typedef struct FifoEntry FifoEntry_t;

//- --------------------------------------------------------------------------
//! \brief  FIFO structure definiton
//- --------------------------------------------------------------------------
struct Fifo
{
    FifoEntry_t data [NFIFOCAP];    // statically allocated array for entries
    FifoEntry_t *pRd;               // read pointer
    FifoEntry_t *pWr;               // write pointer
    bool        fRd;                // last operation, true if it was a read
};

// FIFO typedef
typedef struct Fifo Fifo_t;


// Global function prototypes
bool FifoInit(Fifo_t *pFifo);
bool FifoPut(Fifo_t *pFifo, FifoEntry_t item);
bool FifoGet(Fifo_t *pFifo, FifoEntry_t *pItem);
bool testFIFO(void);

#ifdef _DEBUG_
// For debug purposes, compile the file "dump.c" for a hexadecimal dump
// to list the contents of \c length bytes of memory starting from address
// \c addr with an optional constant string description \c desc
extern void hexDump(const char* desc, void* addr, uint32_t length);
#endif




//= ==========================================================================
//!
//!     MAIN
//!
//= ==========================================================================

int main(void)
{
    printf("FIFO demo v%d.%d\n"
           "==============\n\n", VERMAJOR, VERMINOR);

    testFIFO();

    return 0;
}

//= ==========================================================================
//!
//!     END OF MAIN
//!
//= ==========================================================================



// Main test function for the FIFO
bool testFIFO(void)
{
    uint8_t idx;                // run index for the loop to make entries
    Fifo_t  fifo;               // the FIFO structure
    clock_t tick0 = clock();    // reference time [CPU clocks] set here

    FifoInit(&fifo);            // initialize the FIFO

#ifdef _DEBUG_
    printf("[DEBUG]: Size of FIFO: %d\n\n", (uint32_t)sizeof(fifo));
#endif

    for (idx = 0; idx < NITEMS; idx++)
    {
        uint8_t eIdx = idx + 1;
        char    eName [NNAMESIZE];

        // We have to be careful when assigning names, that's why we use the
        // function snprintf to put a formatted literal into the eName string.
        snprintf(eName, NNAMESIZE-1, "( entry [%d] )", eIdx);
        // Create a temporary FIFO entry item which we'll put into the FIFO
        FifoEntry_t item;
        // Set its members, starting with ID:
        item.ID = eIdx;
        // Name (using strncpy to safely assign strings to one another):
        strncpy(item.Name, eName, sizeof(item.Name));
        // and the TimeStamp [in CPU ticks elapsed since the reference time]:
        item.TimeStamp = clock() - tick0;

#ifdef _DEBUG_
        printf("[DEBUG]: timestamp for entry #(%d) is %f seconds.\n",
                eIdx, ((float) item.TimeStamp)/CLOCKS_PER_SEC );
        printf("[DEBUG]: entry #(%d) data:  { %d, \"%s\", %d }\n",
                eIdx, item.ID, item.Name, item.TimeStamp);
#endif

        if (FifoPut(&fifo, item))
            printf("FifoPut successful!\n");
        else
            printf("FifoPut unsuccessful, the FIFO is probably full.\n");
    }

#ifdef _DEBUG_
    #define NMSGLEN 40
    {
        char msg[NMSGLEN];
        snprintf(msg, NMSGLEN-1, "[DEBUG]: FIFO (start address %08X) dump",
                                                        (uint64_t)&fifo);
        hexDump(msg, &fifo, (uint32_t)sizeof(fifo));
    }
    #undef NMSGLEN
#endif

    printf("\n");
    // Get items from FIFO
    for (idx = 0; idx < NITEMS; idx++)
    {
        FifoEntry_t e;
        if (FifoGet(&fifo, &e))
            printf("FifoGet successful! Got { %d, \"%s\", %d }\n",
                                                e.ID, e.Name, e.TimeStamp);
        else
            printf("FifoGet unsuccessful, the FIFO is probably empty.\n");
    }

}


//- --------------------------------------------------------------------------
//! \brief  FIFO initialization
//! \desc   Sets a default entry, then sets the read and write pointers to
//!         match the inital address of the FIFO's circular buffer that is
//!         at the beginning of the statically allocated arrays RAM location.
//!         It order to start with an empty FIFO, the function sets the last
//!         operation to "read" (fRd <- true).
//! \param  [in]
//!     pFifo       : FIFO address
//! \return
//!     Type        : bool
//!     Value       : true, if initalization was successful
//- --------------------------------------------------------------------------
bool FifoInit(Fifo_t *pFifo)
{
    FifoEntry_t defaultEntry = {0, "DEFAULT", clock()};
    pFifo->pRd = pFifo->data;
    pFifo->pWr = pFifo->data;
    // Set last operation to "read" to indicate FIFO is empty.
    pFifo->fRd = true;
    return true;
}



//- --------------------------------------------------------------------------
//! \brief  Put an entry to the FIFO end (pointed by pWr)
//! \param  [in]
//!     pFifo       : FIFO address
//!     item        : FIFO entry item
//! \return
//!     Type        : bool
//!     Value       : true, if write was successful (ie. the FIFO wasn't full)
//- --------------------------------------------------------------------------
bool FifoPut(Fifo_t *pFifo, FifoEntry_t item)
{
    // If the last operation wasn't a read and the read and write pointers
    // coincide, the FIFO is full, we have to reject the 'put' request.
    if (!pFifo->fRd && pFifo->pWr==pFifo->pRd) return false;
    // Copy the item to the current write location and update the pointer
    *pFifo->pWr++ = item;
    pFifo->fRd = false;     // update last operation
    // If we've reached the end of the buffer, reset pWr to the start location.
    if (pFifo->pWr >= pFifo->data + NFIFOCAP) pFifo->pWr = pFifo->data;
    return true;
}



//- --------------------------------------------------------------------------
//! \brief  Get an entry from the FIFO start (pointed by pRd)
//! \param  [in]
//!     pFifo       : FIFO address
//! \param  [out]
//!     pItem       : FIFO entry item
//! \return
//!     Type        : bool
//!     Value       : true, if read was successful (ie. the FIFO wasn't empty)
//- --------------------------------------------------------------------------
bool FifoGet(Fifo_t *pFifo, FifoEntry_t *pItem)
{
    // If the last operation was a read and the read and write pointers
    // coincide, the FIFO is empty, no item to get, so reject the request.
    if (pFifo->fRd && pFifo->pWr==pFifo->pRd) return false;
    // Copy the item from current read location and update the pointer
    *pItem = *pFifo->pRd++;
    pFifo->fRd = true;      // update last operation
    // If we've reached the end of the buffer, reset pRd to the start location.
    if (pFifo->pRd >= pFifo->data + NFIFOCAP) pFifo->pRd = pFifo->data;
    return true;
}
