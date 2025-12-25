
#include <stdlib.h>
#include <string.h>
#include <tree_sitter/api.h>

#include "code.h"
#include "parser.h"
#include "tree-sitter-avm/src/tree_sitter/array.h"

#define AVM_LABEL_SIZE   200
#define AVM_CMD0_SIZE     50
#define AVM_LITERAL_SIZE 100

const TSLanguage *tree_sitter_avm(void);

typedef struct {
  char* label;
  int offset;
} location;

typedef Array(ZAM_instr_t) * ZAM_instr_array;

typedef Array(location) * location_array;

///////////////////////////////////////////////////

void parse_tree_read_text(char *source, TSNode node, char *buffer, int size) {
  int begin = ts_node_start_byte(node);
  int end = ts_node_end_byte(node);
  int len = end - begin;
  assert(end - begin < size);
  strncpy(buffer, source + begin, len);
  buffer[len] = 0;
}

ZAM_instr_kind parse_tree_read_cmd0(char *cmd0) {
  if (strcmp(cmd0, "let"))
    return ZAM_Let;
  else if (strcmp(cmd0, "endlet"))
    return ZAM_EndLet;
  else if (strcmp(cmd0, "add"))
    return ZAM_Add;
  else if (strcmp(cmd0, "eq"))
    return ZAM_Eq;
  else if (strcmp(cmd0, "app"))
    return ZAM_Apply;
  else if (strcmp(cmd0, "tapp"))
    return ZAM_TailApply;
  else if (strcmp(cmd0, "mark"))
    return ZAM_PushMark;
  else if (strcmp(cmd0, "grab"))
    return ZAM_Grab;
  else if (strcmp(cmd0, "ret"))
    return ZAM_Return;
  else if (strcmp(cmd0, "halt"))
    return ZAM_Halt;
  assert((cmd0, false));
}

ZAM_instr_t parse_tree_read_cmd1(char* source, TSNode cmd1) {
  if (strcmp(ts_node_type(cmd1), "load") == 0) {
    TSNode param = ts_node_named_child(cmd1, 0);
    char buffer[AVM_LITERAL_SIZE] = {};
    parse_tree_read_text(source, param, buffer, AVM_LITERAL_SIZE);
    ZAM_instr_t instr = {};
    if (strcmp(buffer, "true") == 0) {
      instr.kind = ZAM_Ldb;
      instr.const_bool = true;
    } else if (strcmp(buffer, "false") == 0) {
      instr.kind = ZAM_Ldb;
      instr.const_bool = true;
    } else {
      int value = 0;
      int count = sscanf(buffer, "%d", &value);
      assert(count == 1);
      instr.kind = ZAM_Ldi;
      instr.const_int = value;
    }
    return instr;
  } else if (strcmp(ts_node_type(cmd1), "acc") == 0) {
    TSNode param = ts_node_named_child(cmd1, 0);
    char buffer[AVM_LITERAL_SIZE] = {};
    parse_tree_read_text(source, param, buffer, AVM_LITERAL_SIZE);
    ZAM_instr_t instr = {};
    int value = 0;
    int count = sscanf(buffer, "%d", &value);
    assert(count == 1);
    instr.kind = ZAM_Ldi;
    instr.const_int = value;
    return instr;
  } else {
    ZAM_instr_t instr = {};
    if (strcmp(ts_node_type(cmd1), "b") == 0) {
      instr.kind = ZAM_Jump;
    } else if (strcmp(ts_node_type(cmd1), "bf") == 0) {
      instr.kind = ZAM_CJump;
    } else if (strcmp(ts_node_type(cmd1), "clos") == 0) {
      instr.kind = ZAM_Closure;
    } else {
      assert((ts_node_type(cmd1), false));
    }
    TSNode lab_node = ts_node_named_child(cmd1, 0);
    char *label = malloc(sizeof(char) * AVM_LABEL_SIZE + 10);
    parse_tree_read_text(source, lab_node, label, AVM_LABEL_SIZE + 10);
    instr.payload = label;
    return instr;
  }
}

void parse_tree_read_block(char *source, TSNode block_node,
                           ZAM_instr_array instr_buffer,
                           location_array locs) {
  TSNode instr_node = {};
  if (ts_node_named_child_count(block_node) == 1)
    instr_node = ts_node_named_child(block_node, 0);
  else {
    instr_node = ts_node_named_child(block_node, 1);
    TSNode lab_node = ts_node_named_child(block_node, 0);
    int offset = instr_buffer->size;
    char *label = malloc(sizeof(char) * AVM_LABEL_SIZE + 10);
    parse_tree_read_text(source, lab_node, label, AVM_LABEL_SIZE + 10);
    location loc = {label, offset};
    array_push(locs, loc);
  }
  assert(strcmp(ts_node_type(instr_node), "inst") == 0);
  TSNode cmd_node = ts_node_named_child(instr_node, 0);
  if (strcmp(ts_node_type(cmd_node), "cmd0") == 0) {
    char cmd0_buffer[AVM_CMD0_SIZE] = {};
    parse_tree_read_text(source, cmd_node, cmd0_buffer, AVM_CMD0_SIZE);
    ZAM_instr_t instr = {};
    instr.kind = parse_tree_read_cmd0(cmd0_buffer);
    array_push(instr_buffer, instr);
    return;
  }
  assert(strcmp(ts_node_type(cmd_node), "cmd1") == 0);
  TSNode cmd1_node = ts_node_named_child(cmd_node, 0);
  ZAM_instr_t instr = parse_tree_read_cmd1(source, cmd1_node);
  array_push(instr_buffer, instr);
  return;
}

ZAM_instr_array parse_tree_read(char *source, TSTree *tree,
                                location_array locs) {
  ZAM_instr_array instr_buffer = malloc(sizeof(Array(ZAM_instr_t)));
  array_init(instr_buffer);
  TSNode root_node = ts_tree_root_node(tree);
  assert(strcmp(ts_node_type(root_node), "source_file") == 0);
  TSNode code_node = ts_node_named_child(root_node, 0);
  assert(strcmp(ts_node_type(code_node), "code") == 0);
  int count = ts_node_named_child_count(code_node);
  for (int i = 0; i < count; ++i) {
    TSNode block_node = ts_node_named_child(code_node, i);
    if (strcmp(ts_node_type(block_node), "newline") == 0)
      continue;
    printf("type = %s\n", ts_node_type(block_node));
    assert(strcmp(ts_node_type(block_node), "block") == 0);
    parse_tree_read_block(source, block_node, instr_buffer, locs);
  }
  return instr_buffer;
}

int parse_tree_backpatch(ZAM_instr_array instr_buffer, location_array locs) {
  for (int i = 0; i < instr_buffer->size; ++i) {
    ZAM_instr_t *instr = array_get(instr_buffer, i);
    assert(instr);
    if (instr->kind == ZAM_CJump || instr->kind == ZAM_Jump ||
        instr->kind == ZAM_Closure) {
      char *label = instr->payload;
      int j = 0;
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

ZAM_code_t *parse_tree(char *source, TSTree *tree, int *errcod) {
  location_array locs = malloc(sizeof(Array(location)));
  array_init(locs);
  ZAM_instr_array instr_buffer = parse_tree_read(source, tree, locs);
  int backpatch_result = parse_tree_backpatch(instr_buffer, locs);
  ZAM_code_t * code = NULL;
  if (backpatch_result >= 0)
    *errcod = 2;
  else {
    *errcod = 0;
    code = malloc(sizeof(ZAM_code_t));
    code->instr_size = instr_buffer->size;
    ZAM_instr_t *instrs = malloc(sizeof(ZAM_instr_t) * code->instr_size);
    memcpy(instrs, instr_buffer->contents, code->instr_size);
  }
  for (int i = 0; i < locs->size; ++i)
    free(array_get(locs, i)->label);
  array_delete(locs);
  array_delete(instr_buffer);
  return code;
}

ZAM_code_t *parse(char *source, int size, int *errno) {
  TSParser *parser = ts_parser_new();
  ts_parser_set_language(parser, tree_sitter_avm());
  TSTree *tree = ts_parser_parse_string(parser, NULL, source, size);
  int errno_ = 0;
  ZAM_code_t* code = NULL;
  if (tree == NULL) {
    errno_ = 1;
  } else {
    code = parse_tree(source, tree, &errno_);
  }
  if (errno != NULL)
    *errno = errno_;
  ts_tree_delete(tree);
  ts_parser_delete(parser);
  return code;
}
