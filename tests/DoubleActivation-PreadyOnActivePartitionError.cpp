/* Part of the MUST Project, under BSD-3-Clause License
 * See https://hpc.rwth-aachen.de/must/LICENSE for license information.
 * SPDX-License-Identifier: BSD-3-Clause
 */


// RUN: mpicxx -g %s -o %s.exe
// RUN: mustrun -np 2 %s.exe 2>&1 | %filecheck --check-prefix=CHECK-STANDARD %s

// RUN: mpicxx -g %s -o %s.exe
// RUN: mustrun --must:stacktrace backward --must:partitioned_verbose_report -np 2 %s.exe 2>&1 | %filecheck --check-prefix=CHECK-ST %s

// CHECK-STANDARD: [MUST-REPORT]{{.*Calling MPI_Pready_[*] on active partition[(]s[)][(].*[)] is erroneous.*}}
// CHECK-ST: [MUST-REPORT]{{.*Found the following already active partitions, which is erroneous.*}}

/**
 * @file DoubleActivation-PreadyOnActivePartitionError.cpp
 * This is a test for the analysis group PartitionedP2PChecks.
 *
 * Description:
 * Performs a partitioned send and recv where MPI_Pready is called twice on one partition,
 * which is a usage error.
 *
 *  @date 13.09.2022
 *  @author Niko Sakic
 */

#include <iostream>
#include <mpi.h>
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

    //Say hello
    std::cout << "Hello, I am rank " << rank << " of " << size << " processes." << std::endl;

    if (rank == 0) {
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
            // CHECK-ST: PreadyOnActivePartitionError.cpp:[[@LINE+1]]
            MPI_Pready(i, request);
        }
        MPI_Pready(0, request);
        while (!flag) {
            /* do useful work #1 */
            MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
            /* do useful work #2 */
        }
        MPI_Request_free(&request);
    }

    if (rank == 1) {
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
