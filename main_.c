#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <mpi.h>

#define MAX 1000
#define PORT 8080
#define SA struct sockaddr

int counter(int res_buf, int * submit, int * res_arr, int ** arr) {
    float res_time = 0;

    int ** new_arr;

    int *response_arr;
    response_arr = (int*) malloc(2 * sizeof(int*));

    new_arr = (int**) malloc((res_buf) * sizeof(int*));
    for (int j = 0; j < res_buf; j++) {
        new_arr[j] = (int*) malloc((res_buf+1) * sizeof(int));
    }

    for (int i = 0; i < res_buf; i++) {
        new_arr[i][0] = i;
        for (int j = 1; j < res_buf + 1; j++) {
            new_arr[i][j] = arr[i][j-1];
        }
        printf("\n");
    }


    time_t start = clock();
    printf("INFO: Start counting \n");
    int ii;

    int process_Rank, size_Of_Cluster, message_Item;
    MPI_Comm_size(MPI_COMM_WORLD, &size_Of_Cluster);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);

    for (int i = 1; i < 6; i++) {
        MPI_Send(&res_buf, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
    }

    for(ii = 0; ii < res_buf; ii++) {
        MPI_Send(new_arr[ii], res_buf+1, MPI_INT, (ii % 5) + 1, 0, MPI_COMM_WORLD);
    }

    printf("INFO: Sended \n");

    for(ii = 0; ii < res_buf; ii++) {
        MPI_Recv(response_arr, 2, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        res_arr[response_arr[0]] = response_arr[1];
    }

    printf("INFO: finalized \n");

    time_t end = clock();
    res_time = ((float)(end - start) / 1000000.0F ) * 1000;
    printf("INFO: Stop counting \n");
    return res_time;
}

void end_transaction (int res_buf, int * submit, int * res_arr, int ** arr, int sockfd) {
    char buf[MAX];
    float res_time = counter(res_buf, submit, res_arr, arr);

    for (int i = 0; i < res_buf; i++) {
        sprintf(buf, "%d \n", res_arr[i]);
        write(sockfd, buf, sizeof(buf));
    }

    sprintf(buf, "%d", *submit / 1024);
    write(sockfd, buf, sizeof(buf));
    sprintf(buf, "%f", res_time);
    write(sockfd, buf, sizeof(buf));
    write(sockfd, "exit", sizeof("exit"));
    MPI_Finalize();
}

int get_res_buf(char * val) {
    printf("INFO: Принятие результирующего буффера => выделение памяти %d \n", atoi(val));
    return atoi(val);
}

void func(int sockfd)
{
    char buff[MAX];
    int n;
    for (;;) {
        bzero(buff, sizeof(buff));
//        printf("Enter the string from client: ");
        n = 0;
//        while ((buff[n++] = getchar()) != '\n');
        write(sockfd, "start", sizeof("start"));
        bzero(buff, sizeof(buff));
        read(sockfd, buff, MAX);
        printf("INFO: read %s \n", buff);

        if ((strncmp(buff, "start", sizeof("start"))) == 0) {
            int submit = 0;
            int** arr;
            int* res_arr;
            int res_buf = 0;
            int index_row = 0;
            int index_col = 0;

            printf("INFO: Start from server reciever \n");
            char *val = malloc(sizeof(buff));
            while ((strncmp(buff, "end", sizeof("end"))) != 0) {
                read(sockfd, buff, MAX);

                for (int i = 0; i < strlen(buff); i++) {

                    if(buff[i] == 'e') {
                        memset(val, 0, sizeof(buff));
                        end_transaction(res_buf, &submit, res_arr, arr, sockfd);
                        return;
                    }

                    if (buff[i] == ' ') {
                        arr[index_row][index_col] = atoi(val);
                        memset(val, 0, sizeof(buff));
                        index_col++;
                        continue;
                    }

                    if (buff[i] == '\n') {

                        if (res_buf == 0) {
                            res_buf = get_res_buf(val);
                            res_arr = (int*) malloc(res_buf * sizeof(int));
                            arr = (int**) malloc(res_buf * sizeof(int*));
                            for (int j = 0; j < res_buf; j++) {
                                arr[j] = (int*) malloc(res_buf * sizeof(int));
                            }
                            memset(val, 0, sizeof(buff));
                            continue;
                        }

                        arr[index_row][index_col] = atoi(val);
                        memset(val, 0, sizeof(buff));
                        index_row++;
                        index_col = 0;
                        continue;
                    }

                    if (buff[i] != '\t'){
                        sprintf(val, "%s%c", val, buff[i]);
                    }
                }
            }
            continue;
        }

        printf("From Server : %s", buff);
        if ((strncmp(buff, "exit", 4)) == 0) {
            printf("Client Exit...\n");
            break;
        }
    }
}

int create_and_verify_socket() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        printf("ERROR: Socket creation failed...\n");
        exit(0);
    }
    printf("INFO: Socket successfully created..\n");
    return sock_fd;
}

int configure_serv_addr(struct sockaddr_in * serv_addr, int sockfd) {
    bzero(serv_addr, sizeof(*serv_addr));

    serv_addr->sin_family = AF_INET;
    serv_addr->sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr->sin_port = htons(PORT);

    // Binding newly created socket to given IP and verification
    if ((connect(sockfd, (SA*)serv_addr, sizeof(*serv_addr))) != 0) {
        printf("ERROR: Socket bind failed...\n");
        exit(0);
    }
    else
        printf("INFO: Socket successfully binded..\n");

    return sockfd;
}

//сформировать результирующий вектор как среднее по каждой строке исходной квадратной матрицы
int main(int argc, char *argv[]) {

    int sockfd;
    struct sockaddr_in serv_addr;
    int provided;
    int process_Rank = 0;

    MPI_Init_thread(0, 0, MPI_THREAD_FUNNELED,  &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);

    if (process_Rank == 0) {
        sockfd = create_and_verify_socket();
        configure_serv_addr(&serv_addr, sockfd);
        func(sockfd);
        close(sockfd);
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);
    int number;
    int number2;
    int* res_arr;
    int res_arr_size = 0;
    int source;
    int tag;
    int* respones_arr;

    respones_arr = (int*) malloc(2 * sizeof(int));

    for (;;) {

        if (res_arr_size == 0) {
            MPI_Recv(&res_arr_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
            res_arr = (int*) malloc((res_arr_size + 1) * sizeof(int));
        } else {
            MPI_Recv(res_arr, res_arr_size + 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);

            int res = 0;
            for(int jj = 1 ; jj < *res_arr + 1; jj++) {
                res += res_arr[jj];
            }


            respones_arr[0] = res_arr[0];
            respones_arr[1] = res / res_arr_size;

            MPI_Send(respones_arr, 2, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
}
