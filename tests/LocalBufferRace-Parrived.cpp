/* Part of the MUST Project, under BSD-3-Clause License
 * See https://hpc.rwth-aachen.de/must/LICENSE for license information.
 * SPDX-License-Identifier: BSD-3-Clause
 */

// REQUIRES: partitioned-race-support

// RUN: mpicxx -g %s -o %s.exe
// RUN: mustrun -np 2 %s.exe 2>&1 | %filecheck %s

// CHECK: data race

/**
 * @file LocalBufferRace-Parrived.cpp
 * This is a test for the analysis group PartitionedP2PChecks.
 *
 * Description:
 * Performs partitioned communication with a local buffer access *before*
 * any Parrived has ensured that the data of the partition is available
 * which is a data race.
 *
 *  @date 13.09.2022
 *  @author Niko Sakic, Simon Schwitanski
 */

#include <atomic>
#include <iostream>
#include <mpi.h>
#include <vector>
#include <unistd.h>
#define PARTITIONS 8
#define COUNT 5
int main(int argc, char** argv)
{
    double message[PARTITIONS * COUNT];
    MPI_Count partitions = PARTITIONS;
    int size, rank;
    int source = 0, dest = 1, tag = 1, flag = 0;
    int i;
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

    if (rank == 0) {
        for (int i = 0; i < partitions; i++) {
            message[i] = i+1;
        }
    }

    if (rank == 1) {
        for (int i = 0; i < partitions; i++) {
            message[i] = -1;
        }
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
        for (i = 0; i < partitions; ++i) {
            /* compute and fill partition #i, then mark ready: */
            MPI_Pready(i, request);
        }
        MPI_Wait(&request, MPI_STATUS_IGNORE);
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
        printf("message[0] is %f\n", message[0]); // data race
        MPI_Wait(&request, MPI_STATUS_IGNORE);
        MPI_Request_free(&request);
    }

    //Say bye bye
    std::cout << "Signing off, rank " << rank << "." << std::endl;

    MPI_Finalize();

    return 0;
}
