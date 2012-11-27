#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum {
  CONST,
  PUSH,
  POP,
  PRINTNUM,
  ADD,
  SUB,
  MPY,
  DIV,
  MOD,
  LABEL,
  BRANCHn,
  BRANCHz,
  BRANCHp,
  BRANCHnz,
  BRANCHnp,
  BRANCHzp,
  BRANCHnzp,
  JSR,
  JMPR
} Command;

typedef struct StackValue_tag {
  int value;
  struct StackValue_tag* next;
} StackValue;

typedef struct {
  Command command;
  union {
    struct {
      int reg;
      int value;
    } valueReg;
    struct {
      int reg;
      char* value;
    } labelReg;
  } u;
} Instruction;

Instruction* Instructions;

int ParseRegFromString(char* string) {
  return *(string+1) - '0';
}

int main(int argc, char* argv[]) {
  FILE* file;
  size_t nbytes = 20;
  char* line = malloc(nbytes + 1);
  char* command = malloc(9);
  char* regString = malloc(3);
  if (!line || !command || !regString) {
    printf("Could not allocate memory!\n");
    return -1;
  }
  int reg;
  int value;
  int error = 0;
  int nInstructions = 0;

  if (argc != 2) {
    printf("Please supply exactly one script to execute!\n");
    error = 1;
    goto error;
  }

  file = fopen(argv[1], "rt");
  if (!file) {
    printf("Error opening file %s!\n", argv[1]);
    error = 1;
    goto error;
  }

  while (getline(&line, &nbytes, file) != -1) {
    nInstructions++;
  }

  printf("Will read %i lines", nInstructions);
  Instructions = (Instruction*)malloc(sizeof(Instruction) * nInstructions);
  rewind(file);
  nInstructions = 0;

  while (getline(&line, &nbytes, file) != -1) {
    if (sscanf(line, "%s", command) != 1) {
      printf("Error reading file: Line is missing command!\n");
      error = 1;
      goto error;
    }
    printf("found command: %s\n", command);
    if (strcmp(command, "CONST") == 0) {
      if (sscanf(line, "%s %s %i", command, regString, &value) != 3) {
        printf("Error reading file: CONST command is malformed!\n");
        error = 1;
        goto error;
      }
      Instructions[nInstructions].command = CONST;
      Instructions[nInstructions].u.valueReg.value = value;
      Instructions[nInstructions].u.valueReg.reg = ParseRegFromString(regString);
    } else if (strncmp(command, "PUSH", 4) == 0) {
      if (sscanf(line, "%s %s", command, regString) != 2) {
        printf("Error reading file: PUSH command is malformed!\n");
        error = 1;
        goto error;
      }
      Instructions[nInstructions].command = PUSH;
      Instructions[nInstructions].u.valueReg.reg = ParseRegFromString(regString);
    } else if (strncmp(command, "POP", 3) == 0) {
      if (sscanf(line, "%s %s", command, regString) != 2) {
        printf("Error reading file: POP command is malformed!\n");
        error = 1;
        goto error;
      }
      Instructions[nInstructions].command = POP;
      Instructions[nInstructions].u.valueReg.reg = ParseRegFromString(regString);
    } else if (strncmp(command, "PRINTNUM", 8) == 0) {
      Instructions[nInstructions].command = PRINTNUM;
    } else if (strncmp(command, "ADD", 3) == 0) {
      Instructions[nInstructions].command = ADD;
    } else if (strncmp(command, "SUB", 3) == 0) {
      Instructions[nInstructions].command = SUB;
    } else if (strncmp(command, "MPY", 3) == 0) {
      Instructions[nInstructions].command = MPY;
    } else if (strncmp(command, "DIV", 3) == 0) {
      Instructions[nInstructions].command = DIV;
    } else if (strncmp(command, "MOD", 3) == 0) {
      Instructions[nInstructions].command = MOD;
    } else if (strncmp(command, "LABEL", 5) == 0) {
     // TODO: implement me
    } else if (strncmp(command, "BRANCH", 5) == 0) {
      //TODO: implement me
    } else if (strncmp(command, "JSR", 3) == 0) {
      //TODO: implement me
    } else if (strncmp(command, "JMPR", 4) == 0) {
      //TODO: implement me
    } else {
      printf("Unknown command encountered!\n");
      error = 1;
      goto error;
    }
    nInstructions++;
  }

error:
  free(line);
  free(command);
  free(regString);
  fclose(file);

  if (error) {
    return -1;
  }

  return 0;
}

