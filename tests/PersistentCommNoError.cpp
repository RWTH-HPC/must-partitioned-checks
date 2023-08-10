/* Part of the MUST Project, under BSD-3-Clause License
 * See https://hpc.rwth-aachen.de/must/LICENSE for license information.
 * SPDX-License-Identifier: BSD-3-Clause
 */


// RUN: mpicxx -g %s -o %s.exe
// RUN: mustrun -np 2 %s.exe 2>&1 | %filecheck  --implicit-check-not \
// RUN: '[MUST-REPORT]{{.*(Error|ERROR|Warning|WARNING)}}' %s

/**
 * @file PersistentCommNoError.cpp
 * This is a test for the analysis group PartitionedP2PChecks.
 *
 * Description:
 * Performs a persistent send and receive test.
 * No errors should occur.
 *
 *  @date 13.09.2022
 *  @author Niko Sakic
 */

#include <iostream>
#include <mpi.h>
int main(int argc, char** argv)
{
    int size, rank;
    int tag = 1;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Request request;
    MPI_Status status;

    //Enough tasks ?
    if (size < 2) {
        std::cerr << "This test needs at least 2 processes!" << std::endl;
        MPI_Finalize();
        return 1;
    }

    //Say hello
    std::cout << "Hello, I am rank " << rank << " of " << size << " processes." << std::endl;

    if (rank == 0) {
        MPI_Send_init(&size, 1, MPI_INT, 1, tag, MPI_COMM_WORLD, &request);
        MPI_Start(&request);
    }

    if (rank == 1) {
        MPI_Recv_init(&size, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &request);
        MPI_Start(&request);
    }

    MPI_Wait(&request, &status);
    if (request != MPI_REQUEST_NULL)
        MPI_Request_free(&request);

    //Say bye bye
    std::cout << "Signing off, rank " << rank << "." << std::endl;

    MPI_Finalize();

    return 0;
}
