#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const int PORT_NUMBER = 18000;

int main(int argc, char** argv) {
  char* buffer = malloc(500);

  // RUN ./WTFtest

  memset(buffer, 0, 500);
  sprintf(buffer, "./WTFserver %d &", PORT_NUMBER);
  system(buffer);

  memset(buffer, 0, 500);
  sprintf(buffer, "make");
  system(buffer);

  memset(buffer, 0, 500);
  sprintf(buffer, "./WTF configure 127.0.0.1 %d", PORT_NUMBER);
  system(buffer);

  memset(buffer, 0, 500);
  sprintf(buffer, "./WTF create example_project");
  system(buffer);

  memset(buffer, 0, 500);
  sprintf(buffer, "touch ./example_project/file1");
  system(buffer);

  memset(buffer, 0, 500);
  sprintf(buffer, "echo some text > ./example_project/file1");
  system(buffer);

  memset(buffer, 0, 500);
  sprintf(buffer, "./WTF add example_project example_project/file1");
  system(buffer);

  memset(buffer, 0, 500);
  sprintf(buffer, "./WTF commit example_project");
  system(buffer);

  memset(buffer, 0, 500);
  sprintf(buffer, "./WTF push example_project");
  system(buffer);

  memset(buffer, 0, 500);
  sprintf(buffer, "touch ./example_project/file2");
  system(buffer);

  memset(buffer, 0, 500);
  sprintf(buffer, "echo some text > ./example_project/file2");
  system(buffer);

  memset(buffer, 0, 500);
  sprintf(buffer, "./WTF add example_project example_project/file2");
  system(buffer);

  memset(buffer, 0, 500);
  sprintf(buffer, "./WTF update example_project");
  system(buffer);

  memset(buffer, 0, 500);
  sprintf(buffer, "./WTF upgrade example_project");
  system(buffer);

  memset(buffer, 0, 500);
  sprintf(buffer, "./WTF rollback example_project 1");
  system(buffer);

  memset(buffer, 0, 500);
  sprintf(buffer, "pkill WTFserver");
  system(buffer);

  free(buffer);

  return 0;
}