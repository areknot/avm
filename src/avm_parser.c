
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tree_sitter/api.h>

#include "code.h"
#include "avm_parser.h"
#include "tree-sitter-avm/src/tree_sitter/array.h"

#define AVM_LABEL_SIZE   200
#define AVM_CMD0_SIZE     50
#define AVM_LITERAL_SIZE 100

const TSLanguage *tree_sitter_avm(void);

typedef struct {
  char* label;
  int offset;
} location;

static AVM_parse_error error = {};

AVM_parse_error *last_parse_error() { return &error; }

#define REPORT(errno, node, format, ...)                                       \
  do {                                                                         \
    char *_m = malloc(200 * sizeof(char));                                     \
    sprintf(_m, format, __VA_ARGS__);                                          \
    report_error(node, _m);                                                    \
    *errno = 1;                                                                \
    return;                                                                    \
  } while (false)

#define ASSERT_TYPE(errno, node, type)                                         \
  do {                                                                         \
    TSNode _x = node;                                                          \
    if (strcmp(ts_node_type(_x), type) != 0) {                                 \
      if (node_is_error(_x)) {                                              \
        REPORT(errno, _x, "A syntax error%s", "");                 \
      } else {                                                                 \
        REPORT(errno, _x, "Expected <%s>, but found <%s>", type,               \
               ts_node_type(_x));                                              \
      }                                                                        \
    }                                                                          \
  } while (false)

#define SUCCESS(errno)                                                         \
  do {                                                                         \
    *(errno) = 0;                                                              \
    return;                                                                    \
  } while (false)

#define GUARD(errno)                                                           \
  do {                                                                         \
    if (*errno != 0)                                                           \
      return;                                                                  \
  } while (false)

void report_error(TSNode node, char *message) {
  error.message = message;
  error.start_byte = ts_node_start_byte(node);
  error.start_row = ts_node_start_point(node).row;
  error.start_col = ts_node_start_point(node).column;
  error.end_byte = ts_node_end_byte(node);
  error.end_row = ts_node_end_point(node).row;
  error.end_col = ts_node_end_point(node).column;
}

bool node_is_error(TSNode n) {
  return (strcmp(ts_node_type(n), "ERROR") == 0);
}

typedef Array(AVM_instr_t) * AVM_instr_array;

typedef Array(location) * location_array;

///////////////////////////////////////////////////

void parse_tree_read_text(char *source, TSNode node, char *buffer, int size,
                          int *errno) {
  int begin = ts_node_start_byte(node);
  int end = ts_node_end_byte(node);
  int len = end - begin;
  if (len >= size) {
    REPORT(errno, node, "Terminal's length %d is over limit %d", len, size);
  }
  strncpy(buffer, source + begin, len);
  buffer[len] = 0;
}

void parse_tree_read_cmd0(char *cmd0, TSNode node, AVM_instr_kind *kind,
                          int *errno) {
  if (strcmp(cmd0, "let") == 0)
    *kind = AVM_Let;
  else if (strcmp(cmd0, "endlet") == 0)
    *kind = AVM_EndLet;
  else if (strcmp(cmd0, "add") == 0)
    *kind = AVM_Add;
  else if (strcmp(cmd0, "sub") == 0)
    *kind = AVM_Sub;
  else if (strcmp(cmd0, "le") == 0)
    *kind = AVM_Le;
  else if (strcmp(cmd0, "eq") == 0)
    *kind = AVM_Eq;
  else if (strcmp(cmd0, "app") == 0)
    *kind = AVM_Apply;
  else if (strcmp(cmd0, "tapp") == 0)
    *kind = AVM_TailApply;
  else if (strcmp(cmd0, "mark") == 0)
    *kind = AVM_PushMark;
  else if (strcmp(cmd0, "grab") == 0)
    *kind = AVM_Grab;
  else if (strcmp(cmd0, "ret") == 0)
    *kind = AVM_Return;
  else if (strcmp(cmd0, "halt") == 0)
    *kind = AVM_Halt;
  else
    REPORT(errno, node, "Internal error: <%s> is not a <cmd0>", cmd0);
  SUCCESS(errno);
}

void parse_tree_read_cmd1(char* source, TSNode cmd1, AVM_instr_t* ptr_instr, int* errno) {
  if (strcmp(ts_node_type(cmd1), "load") == 0) {
    TSNode param = ts_node_named_child(cmd1, 0);
    char buffer[AVM_LITERAL_SIZE] = {};
    parse_tree_read_text(source, param, buffer, AVM_LITERAL_SIZE, errno);
    GUARD(errno);
    AVM_instr_t instr = {};
    if (strcmp(buffer, "true") == 0) {
      instr.kind = AVM_Ldb;
      instr.const_bool = true;
    } else if (strcmp(buffer, "false") == 0) {
      instr.kind = AVM_Ldb;
      instr.const_bool = false;
    } else {
      int value = 0;
      int count = sscanf(buffer, "%d", &value);
      if (count != 1) {
        REPORT(errno, cmd1, "Cannot load [%s] (only support bool and naturals)",
               buffer);
      }
      instr.kind = AVM_Ldi;
      instr.const_int = value;
    }
    *ptr_instr = instr;
    SUCCESS(errno);
  } else if (strcmp(ts_node_type(cmd1), "acc") == 0) {
    TSNode param = ts_node_named_child(cmd1, 0);
    char buffer[AVM_LITERAL_SIZE] = {};
    parse_tree_read_text(source, param, buffer, AVM_LITERAL_SIZE, errno);
    GUARD(errno);
    AVM_instr_t instr = {};
    int value = 0;
    int count = sscanf(buffer, "%d", &value);
    if (count != 1) {
      REPORT(errno, cmd1, "Cannot load [%s] (only support bool and naturals)",
	     buffer);
    }
    instr.kind = AVM_Access;
    instr.access = value;
    *ptr_instr = instr;
    SUCCESS(errno);
  } else {
    AVM_instr_t instr = {};
    if (strcmp(ts_node_type(cmd1), "b") == 0) {
      instr.kind = AVM_Jump;
    } else if (strcmp(ts_node_type(cmd1), "bf") == 0) {
      instr.kind = AVM_CJump;
    } else if (strcmp(ts_node_type(cmd1), "clos") == 0) {
      instr.kind = AVM_Closure;
    } else {
      REPORT(errno, cmd1,
             "Expected a instruction with a parameter, but found <%s>",
             ts_node_type(cmd1));
    }
    TSNode lab_node = ts_node_named_child(cmd1, 0);
    char *label = malloc(sizeof(char) * AVM_LABEL_SIZE + 10);
    parse_tree_read_text(source, lab_node, label, AVM_LABEL_SIZE + 10, errno);
    GUARD(errno);
    instr.payload = label;
    *ptr_instr = instr;
    SUCCESS(errno);
  }
}

void parse_tree_read_block(char *source, TSNode block_node,
                           AVM_instr_array instr_buffer, location_array locs,
                           int *errno) {
  TSNode instr_node = {};
  if (ts_node_named_child_count(block_node) == 1)
    instr_node = ts_node_named_child(block_node, 0);
  else {
    instr_node = ts_node_named_child(block_node, 1);
    TSNode lab_node = ts_node_named_child(block_node, 0);
    ASSERT_TYPE(errno, lab_node, "lab");
    int offset = instr_buffer->size;
    char *label = malloc(sizeof(char) * AVM_LABEL_SIZE + 10);
    parse_tree_read_text(source, lab_node, label, AVM_LABEL_SIZE + 10, errno);
    GUARD(errno);
    location loc = {label, offset};
    array_push(locs, loc);
  }
  ASSERT_TYPE(errno, instr_node, "inst");
  TSNode cmd_node = ts_node_named_child(instr_node, 0);
  if (strcmp(ts_node_type(cmd_node), "cmd0") == 0) {
    char cmd0_buffer[AVM_CMD0_SIZE] = {};
    parse_tree_read_text(source, cmd_node, cmd0_buffer, AVM_CMD0_SIZE, errno);
    GUARD(errno);
    AVM_instr_kind k = AVM_Halt;
    parse_tree_read_cmd0(cmd0_buffer, cmd_node, &k, errno);
    GUARD(errno);
    AVM_instr_t instr = {};
    instr.kind = k;
    array_push(instr_buffer, instr);
    SUCCESS(errno);
  } else if (strcmp(ts_node_type(cmd_node), "cmd1") == 0) {
    TSNode cmd1_node = ts_node_named_child(cmd_node, 0);
    AVM_instr_t instr = {};
    parse_tree_read_cmd1(source, cmd1_node, &instr, errno);
    GUARD(errno);
    array_push(instr_buffer, instr);
    SUCCESS(errno);
  } else {
    REPORT(errno, cmd_node, "Expected an instruction, but found <%s>",
           ts_node_type(cmd_node));
  }
}

void parse_tree_read_top(TSTree *tree, TSNode *node, int *errno) {
  TSNode root_node = ts_tree_root_node(tree);
  ASSERT_TYPE(errno, root_node, "source_file");
  TSNode code_node = ts_node_named_child(root_node, 0);
  ASSERT_TYPE(errno, code_node, "code");
  *node = code_node;
  SUCCESS(errno);
}

void parse_tree_read(char *source, TSNode code_node,
                     AVM_instr_array instr_buffer, location_array locs,
                     int *errno) {
  int count = ts_node_named_child_count(code_node);
  for (int i = 0; i < count; ++i) {
    TSNode block_node = ts_node_named_child(code_node, i);
    ASSERT_TYPE(errno, block_node, "block");
    parse_tree_read_block(source, block_node, instr_buffer, locs, errno);
    GUARD(errno);
  }
}

int parse_tree_backpatch(AVM_instr_array instr_buffer, location_array locs) {
  for (size_t i = 0; i < instr_buffer->size; ++i) {
    AVM_instr_t *instr = array_get(instr_buffer, i);
    assert(instr);
    if (instr->kind == AVM_CJump || instr->kind == AVM_Jump ||
        instr->kind == AVM_Closure) {
      char *label = instr->payload;
      size_t j = 0;
      for (; j < locs->size; ++j) {
        location *loc = array_get(locs, j);
        if (strcmp(label, loc->label) == 0) {
          instr->addr = loc->offset;
	  break;
        }
      }
      if (j == locs->size)
	return i;
    }
  }
  return -1;
}

void parse_tree(char *source, TSTree *tree, AVM_code_t** code, int *errno) {
  TSNode code_node = {};
  parse_tree_read_top(tree, &code_node, errno);
  GUARD(errno);
  location_array locs = malloc(sizeof(Array(location)));
  AVM_instr_array instr_buffer = malloc(sizeof(Array(AVM_instr_t)));
  array_init(locs);
  array_init(instr_buffer);
  parse_tree_read(source, code_node, instr_buffer, locs, errno);
  if (*errno != 0) goto clean;
  int backpatch_result = parse_tree_backpatch(instr_buffer, locs);
  if (backpatch_result < 0) {
    *code = malloc(sizeof(AVM_code_t));
    (*code)->instr_size = instr_buffer->size;
    AVM_instr_t *instrs = malloc(sizeof(AVM_instr_t) * (*code)->instr_size);
    memcpy(instrs, instr_buffer->contents,
           (*code)->instr_size * sizeof(AVM_instr_t));
    (*code)->instr = instrs;
    goto clean;
  }
  char *message = malloc(100 * sizeof(char));
  sprintf(message, "Unable to backpatch the label (offset = %d)",
          backpatch_result);
  report_error(ts_node_named_child(code_node, backpatch_result), message);
  *errno = 1;
clean:
  for (size_t i = 0; i < locs->size; ++i)
    free(array_get(locs, i)->label);
  array_delete(locs);
  array_delete(instr_buffer);  
}

AVM_code_t *parse(char *source, int size) {
  TSParser *parser = ts_parser_new();
  ts_parser_set_language(parser, tree_sitter_avm());
  TSTree *tree = ts_parser_parse_string(parser, NULL, source, size);
  int errno = 0;
  AVM_code_t* code = NULL;
  if (tree == NULL) {
    errno = 1;
    error.message = "Interval error: unknown";
    error.start_byte = 0;
    error.start_row = 0;
    error.start_col = 0;
    error.end_byte = 0;
    error.end_row = 0;
    error.end_col = 0;
  } else {
    parse_tree(source, tree, &code, &errno);
  }
  ts_tree_delete(tree);
  ts_parser_delete(parser);
  if (errno != 0) code = NULL;
  return code;
}
