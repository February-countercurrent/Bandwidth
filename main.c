#include <stdio.h>
#include <unistd.h>
#include "C:\Program Files (x86)\Microsoft SDKs\MPI\Include\mpi.h"

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Define message sizes (in bytes) to test
    int message_sizes[] = {1024, 2048, 4096, 8192};
    int num_sizes = sizeof(message_sizes) / sizeof(message_sizes[0]);

    int num_iterations = 100; // Number of iterations for stable results

    for (int i = 0; i < num_sizes; i++) {
        int msg_size = message_sizes[i];
        double total_time = 0.0;

        // Create message buffer
        char* data = (char*)malloc(msg_size);

        // Non-blocking send test
        if (rank == 0) {
            double start_time, end_time;
            MPI_Request request;

            for (int iter = 0; iter < num_iterations; iter++) {
                start_time = MPI_Wtime();
                MPI_Isend(data, msg_size, MPI_BYTE, 1, 0, MPI_COMM_WORLD, &request);
                MPI_Wait(&request, MPI_STATUS_IGNORE);
                end_time = MPI_Wtime();
                total_time += (end_time - start_time);
            }
        } else if (rank == 1) {
            for (int iter = 0; iter < num_iterations; iter++) {
                MPI_Recv(data, msg_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }

        // Calculate bandwidth in Mbps
        double bandwidth = (msg_size * num_iterations * 8.0) / (total_time * 1e6);

        if (rank == 0) {
            printf("Message size: %d bytes | Bandwidth (Non-blocking Send): %lf Mbps\n", msg_size, bandwidth);
        }

        // Reset time
        total_time = 0.0;

        // Blocking send test
        if (rank == 0) {
            double start_time, end_time;

            for (int iter = 0; iter < num_iterations; iter++) {
                start_time = MPI_Wtime();
                MPI_Send(data, msg_size, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
                end_time = MPI_Wtime();
                total_time += (end_time - start_time);
            }
        } else if (rank == 1) {
            for (int iter = 0; iter < num_iterations; iter++) {
                MPI_Recv(data, msg_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }

        // Calculate bandwidth in Mbps
        bandwidth = (msg_size * num_iterations * 8.0) / (total_time * 1e6);

        if (rank == 0) {
            printf("Message size: %d bytes | Bandwidth (Blocking Send): %lf Mbps\n", msg_size, bandwidth);
        }

        free(data);
    }

    MPI_Finalize();
    return 0;
}