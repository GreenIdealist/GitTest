#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h> // Windows 소켓 헤더

#define PORT 8080
#define BUFFER_SIZE 8192 // [수정 1] 버퍼 크기를 1024에서 8192로 크게 늘립니다.

int main() {
    WSADATA wsaData;
    SOCKET server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // 0단계: Winsock 초기화 (Windows 필수)
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup 실패\n");
        return 1;
    }

    // 1단계: 서버 소켓 생성 (TCP 통신용)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        printf("소켓 생성 실패\n");
        WSACleanup();
        return 1;
    }

    // 2단계: 소켓에 IP 주소와 포트 번호 할당
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(PORT);
    
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

    // 3단계: 클라이언트의 연결 요청 대기
    listen(server_fd, 3);
    printf("서버가 포트 %d에서 실행 중입니다. http://localhost:%d\n", PORT, PORT);

    // 4단계: 무한 루프를 돌며 클라이언트 요청 처리
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&address, (int*)&addrlen);
        if (client_fd == INVALID_SOCKET) {
            continue;
        }
        
        // 넉넉해진 버퍼(8192)로 클라이언트의 요청을 충분히 읽어들임
        recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        printf("\n[클라이언트 요청]\n%s\n", buffer);

        // 5단계: index.html 파일 읽기
        FILE *file = fopen("index.html", "r");
        if (file) {
            char response_header[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n";
            send(client_fd, response_header, (int)strlen(response_header), 0);

            char file_buffer[BUFFER_SIZE];
            while (fgets(file_buffer, sizeof(file_buffer), file) != NULL) {
                send(client_fd, file_buffer, (int)strlen(file_buffer), 0);
            }
            fclose(file);
        } else {
            char not_found[] = "HTTP/1.1 404 Not Found\r\n\r\n<h1>404 Not Found</h1>";
            send(client_fd, not_found, (int)strlen(not_found), 0);
        }

        // [수정 2] 우아한 종료 (Graceful Close) 
        // 클라이언트에게 "서버가 보낼 데이터는 더 이상 없다"고 명시적으로 알립니다.
        shutdown(client_fd, SD_SEND);

        // 6단계: 클라이언트와 연결 최종 종료
        closesocket(client_fd);
        memset(buffer, 0, BUFFER_SIZE);
    }
    
    closesocket(server_fd);
    WSACleanup();
    return 0;
}