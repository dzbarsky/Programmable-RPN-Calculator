// David Zbarsky
// RPN Calculator
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Error flag for returning proper error code and cleaning up
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
  NONE, //Used for labels and whitespace
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

// Used to hold values on the stack
typedef struct StackValue_tag {
  int value;
  struct StackValue_tag* next;
} StackValue;

// Global stack
StackValue* stack = 0;

// Used to create a table of lables
typedef struct Label_tag {
  int pc;
  char* label;
  struct Label_tag* next;
} LabelList;

// Global table labels
LabelList* labels = 0;

// An Instruction represents one line of a script
typedef struct {
  Command command;
  int reg;
  union {
    int value;
    char* label;
  } u;
} Instruction;

// The array of instructions
Instruction* Instructions = 0;
// The number of instructions
int nInstructions = 0;

// The registers
int R[8];

// Used to parse commands such as PUSH, POP, BRANCH
int ParseRegFromString(char* string) {
  return *(string+1) - '0';
}

// Used to push a value onto the stack
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

// Pops a value from the stack and returns it
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

// Given a label, this function looks up the pc for it in the label table
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

// the main function takes a filename as an argument and executes it.
int main(int argc, char* argv[]) {
  FILE* file;
  size_t nbytes = 20;
  char* line = malloc(nbytes + 1);
  char* command = malloc(9); // the longest command is 8 chars long
  char* regString = malloc(3);
  char* label = malloc(200); // If you want your labels longer than this, then you're outta luck
  if (!line || !command || !regString || !label) {
    printf("Could not allocate memory!\n");
    return -1;
  }
  int reg;
  int value;
  int pc;

  if (argc != 2) {
    printf("Please supply exactly one script to execute!\n");
    goto error;
  }

  // Open the file for reading
  file = fopen(argv[1], "rt");
  if (!file) {
    printf("Error opening file %s!\n", argv[1]);
    goto error;
  }

  // Count the number of instructions, so we can allocate them in contiguous
  // memory
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

  // Read the file line by line, parsing instructions.
  // I chose to use sscanf to parse lines, which allows comments.  For example,
  // when reading the line:
  // CONST R2 5 Set initial bounds
  // we see CONST, so we parse R2 and 5, and ignore everything after that.
  while (getline(&line, &nbytes, file) != -1) {
    if (sscanf(line, "%s", command) != 1) {
      Instructions[nInstructions].command = NONE;
      nInstructions++;
      continue;
    }
    // Parse and store a const instruction
    if (strcmp(command, "CONST") == 0) {
      if (sscanf(line, "%s %s %i", command, regString, &value) != 3) {
        printf("Error reading file: CONST command is malformed!\n");
        goto error;
      }
      Instructions[nInstructions].command = CONST;
      Instructions[nInstructions].u.value = value;
      Instructions[nInstructions].reg = ParseRegFromString(regString);
    } else if (strncmp(command, "PUSH", 4) == 0) {
      // Parse and store a push instruction
      if (sscanf(line, "%s %s", command, regString) != 2) {
        printf("Error reading file: PUSH command is malformed!\n");
        goto error;
      }
      Instructions[nInstructions].command = PUSH;
      Instructions[nInstructions].reg = ParseRegFromString(regString);
    } else if (strncmp(command, "POP", 3) == 0) {
      // Parse and store a pop instruction
      if (sscanf(line, "%s %s", command, regString) != 2) {
        printf("Error reading file: POP command is malformed!\n");
        goto error;
      }
      Instructions[nInstructions].command = POP;
      Instructions[nInstructions].reg = ParseRegFromString(regString);
    } else if (strncmp(command, "PRINTNUM", 8) == 0) {
      // Store a printnum instruction
      Instructions[nInstructions].command = PRINTNUM;
    } else if (strncmp(command, "ADD", 3) == 0) {
      // Store an add instruction
      Instructions[nInstructions].command = ADD;
    } else if (strncmp(command, "SUB", 3) == 0) {
      // Store a sub instruction
      Instructions[nInstructions].command = SUB;
    } else if (strncmp(command, "MPY", 3) == 0) {
      // Store a multiply instruction
      Instructions[nInstructions].command = MPY;
    } else if (strncmp(command, "DIV", 3) == 0) {
      // Store a div instruction
      Instructions[nInstructions].command = DIV;
    } else if (strncmp(command, "MOD", 3) == 0) {
      // Store a mod instruction
      Instructions[nInstructions].command = MOD;
    } else if (strncmp(command, "LABEL", 5) == 0) {
      // Parse a label instruction
      if (sscanf(line, "%s %s", command, label) != 2) {
        printf("Error reading file: LABEL command is malformed!\n");
        goto error;
      }
      Instructions[nInstructions].command = NONE;

      // Add the label to the label list
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
      // Parse a branch instruction
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
      // figure out the exact type of the Branch
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
        // Disallow things like BRANCHnpz
        printf("Unknown BRANCH command encountered!\n");
        goto error;
      }
    } else if (strncmp(command, "JSR", 3) == 0) {
      // Parse a JSR instruction
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
      // Parse a JMPR instruction
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

  // We are done reading the commands, no execute the instructions, starting at
  // pc 0
  for (pc = 0; pc < nInstructions; pc++) {
    switch (Instructions[pc].command) {
      case CONST:
        // Store const in register
        R[Instructions[pc].reg] = Instructions[pc].u.value;
        break;
      case PUSH:
        // Push a register
        PushValue(R[Instructions[pc].reg]);
        break;
      case POP:
        // Pop to a register
        R[Instructions[pc].reg] = PopValue();
        break;
      case PRINTNUM:
      {
        // Print value at top of stack
        if (!stack) {
          printf("Attempting to print from empty stack!\n");
          goto error;
        }
        printf("%i\n", stack->value);
        break;
      }
      // Arithmetic ops
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
      // Skip whitespace and label instructions
      case NONE:
        break;
      // For the branches, subtract one from the pc because the for loop will
      // increment the pc.
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

    // If we encountered an error, we need to clean up and exit
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

  // Return the correct error code
  if (error) {
    return -1;
  }

  return 0;

  // Allows us to goto error instead of setting the error flag everywhere there
  // may be an erro (for example, every malloc call)
error:
  error = 1;
  goto cleanup;
}

