/**
 * @file The parser for AVM
 * @author Zhangfan Li <dzangfan.li@gmail.com>
 * @license Apache-2.0
 */

/// <reference types="tree-sitter-cli/dsl" />
// @ts-check

export default grammar({
  name: "avm",

  rules: {
      source_file: $ => field("code", $.code),
      code: $ => repeat1($.block),
      block: $ => seq(optional(seq(field("lab", $.lab), ":")),
		      field("inst", $.inst)),
      inst: $ => field("cmd", choice($.cmd0, $.cmd1)),
      cmd0: $ => choice(
	  'let'  , 'endlet' , 'add'  , 'sub', 'le', 'eq'  , 'app' ,
	  'tapp' , 'mark'   , 'grab' , 'ret' , 'halt', "push"
      ),
      cmd1: $ => field("cmd1",
		       choice($.load, $.acc, $.b, $.bf, $.clos, $.dum, $.upd)),
      load: $ => seq('load', field("value", choice($.integer, $.bool))),
      acc: $ => seq('acc', field("index", $.nat)),
      b: $ => seq('b', field("addr", $.lab)),
      bf: $ => seq('bf', field("addr", $.lab)),
      clos: $ => seq('clos', field("addr", $.lab)),
      dum: $ => seq('dum', field("index", $.nat)),
      upd: $ => seq('upd', field("index", $.nat)),
      nat: $ => choice(/[1-9][0-9]*/, '0'),
      integer: $ => choice(/-?[1-9][0-9]*/, '0'),
      bool: $ => choice('true', 'false'),
      lab: $ => /[a-zA-Z_][a-zA-Z0-9_]*/,
      comment: $ => token(seq(';', /.*/)),
  },
    extras: $ => [/\s/, $.comment]
});
