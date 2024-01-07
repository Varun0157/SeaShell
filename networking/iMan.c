#include "../utils/headers.h"

#define MAX_RESPONSE_LENGTH ((int)1e6)

void iMan(char* args[], int argsCount) {
  if (argsCount != 2) {
    fprintf(stderr, TCBRED "Invalid usage: " RESET);
    fprintf(stderr, TCNRED "Enter iMan <command_name>\n" RESET);
    return;
  }

  /*
  DNS Resolution
  https://www.geeksforgeeks.org/c-program-display-hostname-ip-address/
  */
  struct hostent* hostInfo = gethostbyname("man.he.net");
  if (hostInfo == NULL) {
    perror(TCBRED "DNS resolution failed" TCNRED);
    fprintf(stderr, RESET);
    return;
  }

  /*
  Open a TCP Socket to the IP address
  https://aticleworld.com/socket-programming-in-c-using-tcpip/
  */
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    perror(TCBRED "Socket creation failed" TCNRED);
    fprintf(stderr, RESET);
    return;
  }

  struct in_addr address;
  memcpy(&address, hostInfo->h_addr_list[0], hostInfo->h_length);

  struct sockaddr_in server = {0};
  server.sin_family = AF_INET;
  server.sin_addr = address;
  server.sin_port = htons(80);

  if (connect(sockfd, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) == -1) {
    perror(TCBRED "Connection failed" TCNRED);
    fprintf(stderr, RESET);
    return;
  }

  /*
  Send a GET request to the website
  https://stackoverflow.com/a/11208699/22348094
  */
  char request[MAX_NAME_LENGTH + 1];
  snprintf(request, MAX_NAME_LENGTH,
           "GET /?topic=%s&section=all HTTP/1.1\r\nHost: man.he.net\r\n\r\n", args[1]);
  if (write(sockfd, request, strlen(request)) == -1) {
    perror(TCBRED "GET Request Failed" TCNRED);
    fprintf(stderr, RESET);
    close(sockfd);
    return;
  }

  char response[MAX_RESPONSE_LENGTH + 1] = {'\0'};
  int currResponseSize = 0;

  char buffer[MAX_NAME_LENGTH + 1];
  int charsRead;
  while ((charsRead = read(sockfd, buffer, MAX_NAME_LENGTH)) > 0) {
    if (currResponseSize + charsRead < MAX_RESPONSE_LENGTH) {
      strncpy(response + currResponseSize, buffer, charsRead);
      currResponseSize += charsRead;
    } else if (currResponseSize != MAX_RESPONSE_LENGTH) {
      strncpy(response + currResponseSize, buffer,
              (MAX_RESPONSE_LENGTH - currResponseSize));
      currResponseSize += (MAX_RESPONSE_LENGTH - currResponseSize);
    } else {
      break;
    }
  }

  // printf("%s\n\n", response);

  // Can't think of a GET request taht wouldn't return 200, but might as well be safe if
  // possible
  char strStatusCode[4];
  strncpy(strStatusCode, response + strlen("HTTP/1.0 "), 3);
  strStatusCode[3] = '\0';
  char* endPtr;
  /*
  Ensuring the status code is 200 before proceeding. A somewhat risky check,
    but I'm hoping strtol ensures I'm actually parsing the status code and not
    text I don't want to parse.
   */
  long statusCode = strtol(strStatusCode, &endPtr, 10);
  if (*endPtr == '\0') {
    if (statusCode != 200) {
      fprintf(stderr, TCBRED "Unsuccesful request, status: %ld\n" RESET, statusCode);
      close(sockfd);
      return;
    }
  }

  if (strstr(response, "No matches for \"")) {
    printf(TCBRED "No such command\n" RESET);
    close(sockfd);
    return;
  }

  int startIndex = 0;
  // int endIndex = strlen(response);
  char* strLoc;

  /*
  To find a start index:
  - First, try to match STRONG>\n\nNAME
  - Second, try to match NAME\n(spaces)(command name)
  I am hoping one of these work, if neither do, then starting at 0 after a last desperate
  search
  */
  if ((strLoc = strstr(response, "STRONG>\n\nNAME"))) {
    startIndex = (strLoc + strlen("STRONG\n\n")) - response;
  }

  char searchName[MAX_NAME_LENGTH];
  snprintf(searchName, MAX_NAME_LENGTH, "NAME\n       %s", args[1]);
  if ((strLoc = strstr(response, searchName))) {
    startIndex = strLoc - response;
  }
  /*
  Last, desperate search
  */
  if (startIndex == 0) {
    if ((strLoc = strstr(response, "NAME\n"))) {
      startIndex = strLoc - response;
    }
  }

  int endIndex = strlen(response);
  int lastDoubleNewLine = strlen(response);
  int offset = startIndex;
  while ((offset < endIndex) && (strLoc = strstr(response + offset, "\n\n"))) {
    lastDoubleNewLine = strLoc - response;
    offset = lastDoubleNewLine + 1;
  }

  if (lastDoubleNewLine > startIndex) {
    endIndex = lastDoubleNewLine;
  }

  // /*
  // To find end index:
  // Try to match SEE ALSO, COPYRIGHT, REPORTING BUGS, AUTHOR in that order
  // If none match, matching to the end of the response
  //  */
  // char* potentialMatches[] = {"SEE ALSO\n",
  //                             "COPYRIGHT\n",
  //                             "REPORTING BUGS\n",
  //                             "AUTHORS\n",
  //                             "EXAMPLES\n",
  //                             "BUGS\n",
  //                             "CAVEATS\n",
  //                             "NOTES\n",
  //                             "HISTORY\n",
  //                             "STANDARDS\n",
  //                             "VERSIONS\n",
  //                             "ATTRIBUTES\n",
  //                             "FILES\n",
  //                             "ENVIRONMENT\n",
  //                             "ERRORS\n",
  //                             "RETURN VALUE\n",
  //                             "EXIT STATUS\n",
  //                             "OPTIONS\n",
  //                             "CC0 1.0 Universal\n",
  //                             "Copyright",
  //                             "copyright"};
  // int numMatches = 21;
  // for(int i = 0; i < numMatches; i++){
  //   if((strLoc = strstr(response, potentialMatches[i])) && (strLoc - response >
  //   startIndex)){
  //     endIndex = strLoc - response;
  //     break;
  //   }
  // }

  response[endIndex] = '\0';
  printf("%s\n\n", response + startIndex);

  /*
  Close socket
  */
  close(sockfd);
}