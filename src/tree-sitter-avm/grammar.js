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
      source_file: $ => $.code,
      code: $ => repeat1($.block),
      block: $ => seq(optional(seq($.lab, ":")), $.inst),
      inst: $ => choice($.cmd0, $.cmd1),
      cmd0: $ => choice(
	  'let'  , 'endlet' , 'add'  , 'eq'  , 'app' ,
	  'tapp' , 'mark'   , 'grab' , 'ret' , 'halt'
      ),
      cmd1: $ => choice($.load, $.acc, $.b, $.bf, $.clos),
      load: $ => seq('load', choice($.nat, $.bool)),
      acc: $ => seq('acc', $.nat),
      b: $ => seq('b', $.lab),
      bf: $ => seq('bf', $.lab),
      clos: $ => seq('clos', $.lab),
      nat: $ => choice(/[1-9][0-9]*/, '0'),
      bool: $ => choice('true', 'false'),
      lab: $ => /[^:; \t\r\n]+/,
      comment: $ => token(seq("#", /.*/)),
  },
    extras: $ => [/\s/, ';', $.comment]
});
