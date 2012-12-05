#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int error = 0;

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

StackValue* stack = 0;

typedef struct Label_tag {
  int pc;
  char* label;
  struct Label_tag* next;
} LabelList;

LabelList* labels = 0;

typedef struct {
  Command command;
  int reg;
  union {
    int value;
    char* label;
  } u;
} Instruction;

Instruction* Instructions = 0;
int nInstructions = 0;

int R[8];

int ParseRegFromString(char* string) {
  return *(string+1) - '0';
}

void PushValue(int value) {
  StackValue* newTop = malloc(sizeof(StackValue));
  if (!newTop) {
    printf("Could not push value because out of memory!\n");
    error = 1;
    return;
  }
  newTop->value = value;
  newTop->next = stack;
  stack = newTop;
}

int PopValue() {
  int value;
  if (!stack) {
    printf("Attempting to use value from empty stack!\n");
    error = 1;
    return 0;
  }
  value = stack->value;
  StackValue* newTop = stack->next;
  free(stack);
  stack = newTop;
  return value;
}

int GetPcForLabel(char* label) {
  LabelList* currlabel = labels;
  while (currlabel) {
    if (strcmp(currlabel->label, label) == 0) {
      return currlabel->pc;
    }
    currlabel = currlabel->next;
  }
  return -1;
}

int main(int argc, char* argv[]) {
  FILE* file;
  size_t nbytes = 20;
  char* line = malloc(nbytes + 1);
  char* command = malloc(9);
  char* regString = malloc(3);
  char* label = malloc(100);
  if (!line || !command || !regString || !label) {
    printf("Could not allocate memory!\n");
    return -1;
  }
  int reg;
  int value;
  int pc;
  int hadBlankLine = 0;

  if (argc != 2) {
    printf("Please supply exactly one script to execute!\n");
    goto error;
  }

  file = fopen(argv[1], "rt");
  if (!file) {
    printf("Error opening file %s!\n", argv[1]);
    goto error;
  }

  while (getline(&line, &nbytes, file) != -1) {
    nInstructions++;
  }

  Instructions = (Instruction*)malloc(sizeof(Instruction) * nInstructions);
  if (!Instructions) {
    printf("Could not allocate needed memory!\n");
    goto error;
  }
  rewind(file);
  nInstructions = 0;

  while (getline(&line, &nbytes, file) != -1) {
    if (sscanf(line, "%s", command) != 1) {
      hadBlankLine = 1;
      continue;
    }
    printf("found command: %s\n", command);
    if (hadBlankLine) {
      printf("Error reading file: Commands cannot follow blank line\n");
      goto error;
    }
    if (strcmp(command, "CONST") == 0) {
      if (sscanf(line, "%s %s %i", command, regString, &value) != 3) {
        printf("Error reading file: CONST command is malformed!\n");
        goto error;
      }
      Instructions[nInstructions].command = CONST;
      Instructions[nInstructions].u.value = value;
      Instructions[nInstructions].reg = ParseRegFromString(regString);
    } else if (strncmp(command, "PUSH", 4) == 0) {
      if (sscanf(line, "%s %s", command, regString) != 2) {
        printf("Error reading file: PUSH command is malformed!\n");
        goto error;
      }
      Instructions[nInstructions].command = PUSH;
      Instructions[nInstructions].reg = ParseRegFromString(regString);
    } else if (strncmp(command, "POP", 3) == 0) {
      if (sscanf(line, "%s %s", command, regString) != 2) {
        printf("Error reading file: POP command is malformed!\n");
        goto error;
      }
      Instructions[nInstructions].command = POP;
      Instructions[nInstructions].reg = ParseRegFromString(regString);
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
      if (sscanf(line, "%s %s", command, label) != 2) {
        printf("Error reading file: LABEL command is malformed!\n");
        goto error;
      }
      Instructions[nInstructions].command = LABEL;

      LabelList* newLabel = malloc(sizeof(LabelList));
      if (!newLabel) {
        printf("Could not allocate needed memory!\n");
        goto error;
      }
      newLabel->label = strdup(label);
      if (!newLabel->label) {
        printf("Could not allocate needed memory!\n");
        goto error;
      }
      newLabel->pc = nInstructions;
      newLabel->next = labels;
      labels = newLabel;
    } else if (strncmp(command, "BRANCH", 5) == 0) {
      if (sscanf(line, "%s %s %s", command, regString, label) != 3) {
        printf("Error reading file: BRANCH command is malformed!\n");
        goto error;
      }
      Instructions[nInstructions].reg = ParseRegFromString(regString);
      Instructions[nInstructions].u.label = strdup(label);
      if (!Instructions[nInstructions].u.label) {
        printf("Could not allocate needed memory!\n");
        goto error;
      }
      if (strcmp(command, "BRANCHn") == 0) {
        Instructions[nInstructions].command = BRANCHn;
      } else if (strcmp(command, "BRANCHz") == 0) {
        Instructions[nInstructions].command = BRANCHz;
      } else if (strcmp(command, "BRANCHp") == 0) {
        Instructions[nInstructions].command = BRANCHp;
      } else if (strcmp(command, "BRANCHnz") == 0) {
        Instructions[nInstructions].command = BRANCHnz;
      } else if (strcmp(command, "BRANCHzp") == 0) {
        Instructions[nInstructions].command = BRANCHzp;
      } else if (strcmp(command, "BRANCHnp") == 0) {
        Instructions[nInstructions].command = BRANCHnp;
      } else if (strcmp(command, "BRANCHnzp") == 0) {
        Instructions[nInstructions].command = BRANCHnzp;
      } else {
        printf("Unknown BRANCH command encountered!\n");
        goto error;
      }
    } else if (strncmp(command, "JSR", 3) == 0) {
      if (sscanf(line, "%s %s", command, label) != 2) {
        printf("Error reading file: JSR command is malformed!\n");
        goto error;
      }
      Instructions[nInstructions].command = JSR;
      Instructions[nInstructions].u.label = strdup(label);
      if (!Instructions[nInstructions].u.label) {
        printf("Could not allocate needed memory!\n");
        goto error;
      }
    } else if (strncmp(command, "JMPR", 4) == 0) {
      if (sscanf(line, "%s %s", command, regString) != 2) {
        printf("Error reading file: JMPR command is malformed!\n");
        goto error;
      }
      Instructions[nInstructions].command = JMPR;
      Instructions[nInstructions].reg = ParseRegFromString(regString);
    } else {
      printf("Unknown command encountered!\n");
      goto error;
    }
    nInstructions++;
  }

  for (pc = 0; pc < nInstructions; pc++) {
    switch (Instructions[pc].command) {
      case CONST:
        R[Instructions[pc].reg] = Instructions[pc].u.value;
        break;
      case PUSH:
        PushValue(R[Instructions[pc].reg]);
        break;
      case POP:
        R[Instructions[pc].reg] = PopValue();
        break;
      case PRINTNUM:
      {
        if (!stack) {
          printf("Attempting to print from empty stack!\n");
          goto error;
        }
        printf("%i\n", stack->value);
        break;
      }
      case ADD:
        PushValue(PopValue() + PopValue());
        break;
      case SUB:
        PushValue(PopValue() - PopValue());
        break;
      case MPY:
        PushValue(PopValue() * PopValue());
        break;
      case DIV:
      {
        int numer = PopValue();
        int denom = PopValue();
        if (denom == 0) {
          printf("Error!  Attempting to divide by 0!\n");
          goto error;
        }
        PushValue(numer / denom);
        break;
      }
      case MOD:
      {
        int numer = PopValue();
        int denom = PopValue();
        if (denom == 0) {
          printf("Error!  Attempting to mod by 0!\n");
          goto error;
        }
        PushValue(numer % denom);
        break;
      }
      case LABEL:
        break;
      case BRANCHn:
        pc = R[Instructions[pc].reg] < 0 ? GetPcForLabel(Instructions[pc].u.label) - 1 : pc;
        break;
      case BRANCHz:
        pc = R[Instructions[pc].reg] == 0 ? GetPcForLabel(Instructions[pc].u.label) - 1 : pc;
        break;
      case BRANCHp:
        pc = R[Instructions[pc].reg] > 0 ? GetPcForLabel(Instructions[pc].u.label) - 1 : pc;
        break;
      case BRANCHnz:
        pc = R[Instructions[pc].reg] <= 0 ? GetPcForLabel(Instructions[pc].u.label) - 1 : pc;
        break;
      case BRANCHnp:
        pc = R[Instructions[pc].reg] != 0 ? GetPcForLabel(Instructions[pc].u.label) - 1 : pc;
        break;
      case BRANCHzp:
        pc = R[Instructions[pc].reg] >= 0 ? GetPcForLabel(Instructions[pc].u.label) - 1 : pc;
        break;
      case BRANCHnzp:
        pc = GetPcForLabel(Instructions[pc].u.label) - 1;
        break;
      case JSR:
      {
        PushValue(pc + 1);
        pc = GetPcForLabel(Instructions[pc].u.label) - 1; //The for loop will increment the PC at the end
        if (pc < 0) {
          printf("Could not find label %s", Instructions[pc].u.label);
          goto error;
        }
        break;
      }
      case JMPR:
      {
        pc = R[Instructions[pc].reg] - 1; //The for loop will increment the PC at the end
        if (pc < 0 || pc >= nInstructions) {
          printf("Attempting to JMPR to %i, which is out of range", pc);
          goto error;
        }
        break;
      }
    }

    if (error) {
      goto cleanup;
    }
  }

// Yeah, I know.  Goto considered harmful.  This is the cleanest way to make
// sure we clean up in case of errors.
cleanup:
  free(line);
  free(command);
  free(regString);
  free(label);

  // Free the labels in the instructions
  for (pc = 0; pc < nInstructions; pc++) {
    switch(Instructions[pc].command) {
      case BRANCHn:
      case BRANCHz:
      case BRANCHp:
      case BRANCHnz:
      case BRANCHnp:
      case BRANCHzp:
      case BRANCHnzp:
      case JSR:
        free(Instructions[pc].u.label);
      default:
        break;
    }
  }

  free(Instructions);
  fclose(file);

  // Free any elements left on the stack
  while (stack) {
    PopValue();
  }

  // Free the label data structure we're holding
  while (labels) {
    free(labels->label);
    LabelList* next = labels->next;
    free(labels);
    labels = next;
  }

  if (error) {
    return -1;
  }

  return 0;

error:
  error = 1;
  goto cleanup;
}

