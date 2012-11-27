#include <stdio.h>
#include <string.h>
#include <stdlib.h>





int ParseRegFromString(char* string) {
  printf("\n parsing reg! for %s\n, will parse %s", string, string + 1);
  return atoi(string++);
}

int main(int argc, char* argv[]) {
  FILE* file;
  size_t nbytes = 20;
  char* line = malloc(nbytes + 1);
  char* command = malloc(9);
  char* regString = malloc(3);
  int reg;
  int value;

  if (argc != 2) {
    printf("Please supply exactly one script to execute!\n");
    return -1;
  }

  file = fopen(argv[1], "rt");
  if (!file) {
    printf("Error opening file %s!\n", argv[1]);
    return -1;
  }

  while (getline(&line, &nbytes, file) != -1) {
    if (sscanf(line, "%s", command) != 1) {
      printf("Error reading file: Line is missing command!\n");
      return -1;
    }
    printf("\n found command: %s__", command);
    if (strcmp(command, "CONST") == 0) {
      if (sscanf(line, "%s %s %i", command, regString, &value) != 3) {
        printf("Error reading file: CONST command is malformed!\n");
        return -1;
      }
      reg = ParseRegFromString(regString);
      printf("parsed const reg %i", reg);
    } else if (strncmp(command, "PUSH", 4) == 0) {
      if (sscanf(line, "%s %s", command, regString) != 2) {
        printf("Error reading file: PUSH command is malformed!\n");
        return -1;
      }
      reg = ParseRegFromString(regString);
    } else if (strncmp(command, "POP", 3) == 0) {
      if (sscanf(line, "%s %s", command, regString) != 2) {
        printf("Error reading file: POP command is malformed!\n");
        return -1;
      }
      reg = ParseRegFromString(regString);
    } else if (strncmp(command, "PRINTNUM", 8) == 0) {
    } else if (strncmp(command, "ADD", 3) == 0) {
    } else if (strncmp(command, "SUB", 3) == 0) {
    } else if (strncmp(command, "MPY", 3) == 0) {
    } else if (strncmp(command, "DIV", 3) == 0) {
    } else if (strncmp(command, "MOD", 3) == 0) {
    } else if (strncmp(command, "LABEL", 5) == 0) {
     // TODO: implement me
    } else if (strncmp(command, "BRANCH", 5) == 0) {
      //TODO: implement me
    } else if (strncmp(command, "JSR", 3) == 0) {
      //TODO: implement me
    } else if (strncmp(command, "JMPR", 4) == 0) {
      //TODO: implement me
    }
  }

}

