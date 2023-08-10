/* Part of the MUST Project, under BSD-3-Clause License
 * See https://hpc.rwth-aachen.de/must/LICENSE for license information.
 * SPDX-License-Identifier: BSD-3-Clause
 */


// RUN: mpicxx -g %s -o %s.exe
// RUN: mustrun -np 2 %s.exe 2>&1 | %filecheck  --implicit-check-not \
// RUN: '[MUST-REPORT]{{.*(Error|ERROR|Warning|WARNING)}}' %s

/**
 * @file PreadyListNoError.cpp
 * This is a test for the analysis group PartitionedP2PChecks.
 *
 * Description:
 * Performs a partitioned send and receive test with MPI_Pready_list instead of MPI_Pready.
 * No errors should occur.
 *
 *  @date 02.10.2022
 *  @author Niko Sakic
 */

//#include "mpi_proto.h"
#include <iostream>
#include <mpi.h>
#define PARTITIONS 20
#define COUNT 5
int main(int argc, char** argv)
{
    double message[PARTITIONS * COUNT];
    MPI_Count partitions = PARTITIONS;
    int size, rank;
    int source = 0, dest = 1, tag = 1, flag = 0;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Request request;

    //Enough tasks ?
    if (size < 2) {
        std::cerr << "This test needs at least 2 processes!" << std::endl;
        MPI_Finalize();
        return 1;
    }

    //Say hello
    std::cout << "Hello, I am rank " << rank << " of " << size << " processes." << std::endl;

    if (rank == 0) {
        //MPI_Send (&size, -1, MPI_INT, 1, 42, MPI_COMM_WORLD);
        MPI_Psend_init(
            message,
            partitions,
            COUNT,
            MPI_DOUBLE,
            dest,
            tag,
            MPI_COMM_WORLD,
            MPI_INFO_NULL,
            &request);
        MPI_Start(&request);
        int array[20];
        for (int i = 0; i < 20; ++i) {
            array[i] = i;
        }
        MPI_Pready_list(20, array, request);
        // some more work
        while (!flag) {
            /* do useful work #1 */
            MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
            /* do useful work #2 */
        }

        MPI_Request_free(&request);
    }

    if (rank == 1) {
        //MPI_Recv (&size, 1, MPI_INT, 0, 42, MPI_COMM_WORLD, &status);
        MPI_Precv_init(
            message,
            partitions,
            COUNT,
            MPI_DOUBLE,
            source,
            tag,
            MPI_COMM_WORLD,
            MPI_INFO_NULL,
            &request);
        MPI_Start(&request);
        while (!flag) {
            /* do useful work #1 */
            MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
            /* do useful work #2 */
        }
        MPI_Request_free(&request);
    }

    //Say bye bye
    std::cout << "Signing off, rank " << rank << "." << std::endl;

    MPI_Finalize();

    return 0;
}
