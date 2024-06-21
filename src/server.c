#include <stdio.h>
#include <libwebsockets.h>
#include <cjson/cJSON.h>
#include <stdbool.h>

#define MAX_MESSAGE_LEN 512
#define USER_MAX 30
#define USERNAME_LEN_MAX 32

struct UserConnection {
    char username[USERNAME_LEN_MAX];

}

struct UserConnection user_list[USER_MAX];
int user_count = 0;

static int callback_chat(struct lws *wsi, enum lws_callback_reasons reason,
                         void *user, void *in, size_t len)
{
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            printf("WebSocket connection established\n");
            break;

        case LWS_CALLBACK_RECEIVE:
            // Handle received data (in points to the message)
            printf("Received: %.*s\n", (int)len, (char *)in);
            char recvd[MAX_MESSAGE_LEN];
            snprintf(recvd, MAX_MESSAGE_LEN, "%.*s", (int)len, (char *)in);
            cJSON *send = cJSON_CreateObject(); 
            char *json_str;

            printf("Bruh\n%s\n",recvd);
            cJSON *username = cJSON_Parse(recvd); 
            if (username == NULL) { 
                const char *error_ptr = cJSON_GetErrorPtr(); 
                if (error_ptr != NULL) { 
                    printf("Error: %s\n", error_ptr); 
                } 
                cJSON_Delete(username); 
                return 1; 
            }

            cJSON *type = cJSON_GetObjectItemCaseSensitive(username, "type"); 
            cJSON *un = cJSON_GetObjectItemCaseSensitive(username, "username"); 
            if (cJSON_IsString(type) && !strcmp(type->valuestring, "login")) { 
                printf("Adding user: %s\n", un->valuestring); 
                if (user_count >= USER_MAX) {
                    printf("Couldn't add user: server is full"); 
                    cJSON_AddStringToObject(send, "type", "error"); 
                    cJSON_AddStringToObject(send, "error", "Server is full, please try again later");
                    json_str = cJSON_Print(send);
                    lws_write(wsi, json_str, strlen(json_str), LWS_WRITE_TEXT);
                }
                else {
                    bool taken = false;
                    for (int i = 0; i < user_count; i++) {
                        if (!strcmp(user_list[i],un->valuestring)) {
                            taken = true;
                            break;
                        }
                    }
                    user_list[user_count] = malloc(USERNAME_LEN_MAX + 1);
                    strncpy(user_list[user_count],un->valuestring,USERNAME_LEN_MAX);
                    cJSON_AddStringToObject(send, "type", "userList"); 
                    cJSON *users = cJSON_CreateArray();
                    cJSON_AddItemToObject(send, "users", users);
                    for (int i = 0; i < user_count; i++) {
                        cJSON_AddItemToArray(users, cJSON_CreateString(user_list[i]));
                        printf("User #%i:%s\n",i,user_list[i]);
                    }
                    if (taken) {
                        printf("Couldn't add user: username taken\n"); 
                        cJSON_AddStringToObject(send, "type", "error"); 
                        cJSON_AddStringToObject(send, "error", "Username taken, please choose another");
                        json_str = cJSON_Print(send);
                        lws_write(wsi, json_str, strlen(json_str), LWS_WRITE_TEXT);
                    }
                    else {
                        json_str = cJSON_Print(send);
                        printf("JSON:\n%s\n",json_str);
                        lws_write(wsi, json_str, strlen(json_str), LWS_WRITE_TEXT);
                        user_count++;
                    }
                }
            }
            cJSON_Delete(send); 
            cJSON_Delete(username); 
            break;

        case LWS_CALLBACK_CLOSED:
            printf("Connection closed\n");
            // remove_user(wsi);
            break;

        default:
            break;
    }

    return 0;
}

static struct lws_protocols protocols[] = {
    { "chat-protocol", callback_chat, 0, MAX_MESSAGE_LEN },
    { NULL, NULL, 0, 0 }
};

int main() {

    for (int i = 0; i < USER_MAX; i++) {
        user_list[i] = NULL;
    }

    struct lws_context_creation_info info;
    struct lws_context *context;

    memset(&info, 0, sizeof(info));
    info.port = 8080;
    info.protocols = protocols;

    context = lws_create_context(&info);
    if (!context) {
        printf("Error creating WebSocket context\n");
        return 1;
    }

    printf("WebSocket server listening on port 8080...\n");

    while (1) {
        lws_service(context, 50);
    }

    lws_context_destroy(context);
    return 0;
}
