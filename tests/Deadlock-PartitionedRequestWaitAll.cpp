/* Part of the MUST Project, under BSD-3-Clause License
 * See https://hpc.rwth-aachen.de/must/LICENSE for license information.
 * SPDX-License-Identifier: BSD-3-Clause
 */

// REQUIRES: partitioned-deadlock-support

// RUN: mpicxx -g %s -o %s.exe
// RUN: mustrun -np 3 %s.exe 2>&1 | %filecheck %s

// CHECK: [MUST-REPORT]{{.*}}The application issued a set of MPI calls that can cause a deadlock!
// CHECK: {{(MPI_Waitall)|(MPI_Finalize)}}

/**
 * @file Deadlock-PartitionedRequestWaitAll.cpp
 * Simple test with an MPI_Waitall call and partitioned requests, causes a deadlock (Error).
 *
 * Description:
 * The waitall waits for 4 partitioned requests, of which 3 can complete and one cant,
 * the one that can't complete is associated with a recv from process 0.
 *
 * @author Tobias Hilbrich, Simon Schwitanski
 */

#include <mpi.h>
#include <stdio.h>

#define PARTITIONS 8
#define COUNT 5

int main(int argc, char** argv)
{
    int rank, size;
    MPI_Status statuses[4];
    MPI_Request requests[4] = {MPI_REQUEST_NULL, MPI_REQUEST_NULL, MPI_REQUEST_NULL, MPI_REQUEST_NULL};
    int buf[4];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //Enough tasks ?
    if (size < 3) {
        printf("This test needs at least 3 processes!\n");
        MPI_Finalize();
        return 1;
    }

    //Say hello
    printf("Hello, I am rank %d of %d processes.\n", rank, size);

    if (rank == 0) {
        MPI_Psend_init(&(buf[0]), PARTITIONS, 1, MPI_INT, 1, 42, MPI_COMM_WORLD, MPI_INFO_NULL, &(requests[0]));
        MPI_Precv_init(&(buf[1]), PARTITIONS, 1, MPI_INT, 1, 42, MPI_COMM_WORLD, MPI_INFO_NULL, &(requests[1]));
        MPI_Startall(2, requests);
        for (int i = 0; i < PARTITIONS; ++i) {
            MPI_Pready(i, requests[0]);
        }
        MPI_Waitall(2, requests, statuses);
        MPI_Request_free(&(requests[0]));
        MPI_Request_free(&(requests[1]));
    }

    if (rank == 1) {
        MPI_Precv_init(&(buf[0]), PARTITIONS, 1, MPI_INT, 0, 42, MPI_COMM_WORLD, MPI_INFO_NULL, &(requests[0]));
        MPI_Psend_init(&(buf[1]), PARTITIONS, 1, MPI_INT, 0, 42, MPI_COMM_WORLD, MPI_INFO_NULL, &(requests[1]));
        MPI_Precv_init(&(buf[2]), PARTITIONS, 1, MPI_INT, 2, 123, MPI_COMM_WORLD, MPI_INFO_NULL, &(requests[2]));
        MPI_Precv_init(&(buf[3]), PARTITIONS, 1, MPI_INT, 0, 444, MPI_COMM_WORLD, MPI_INFO_NULL, &(requests[3]));
        MPI_Startall(4, requests);
        for (int i = 0; i < PARTITIONS; ++i) {
            MPI_Pready(i, requests[1]);
        }
        MPI_Waitall(4, requests, statuses);
        MPI_Request_free(&(requests[0]));
        MPI_Request_free(&(requests[1]));
        MPI_Request_free(&(requests[2]));
        MPI_Request_free(&(requests[3]));
    }

    if (rank == 2) {
        MPI_Psend_init(&(buf[1]), PARTITIONS, 1, MPI_INT, 1, 123, MPI_COMM_WORLD, MPI_INFO_NULL, &(requests[0]));
        MPI_Start(&(requests[0]));
        for (int i = 0; i < PARTITIONS; ++i) {
            MPI_Pready(i, requests[0]);
        }
        MPI_Wait(&(requests[0]), MPI_STATUS_IGNORE);
        MPI_Request_free(&(requests[0]));
    }

    //Say bye bye
    printf("Signing off, rank %d.\n", rank);

    MPI_Finalize();

    return 0;
}

