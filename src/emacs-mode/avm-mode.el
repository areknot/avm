;;; avm-mode.el --- The major mode for editing AVM bytecode  -*- lexical-binding: t; -*-

;; Copyright (C) 2025  Zhangfan Li

;; Author: Zhangfan Li <dzangfan.li@gmail.com>
;; Keywords: languages

;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.

;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <https://www.gnu.org/licenses/>.

;;; Commentary:

;; This file provide a major mode, `avm-mode', to edit the bytecode of
;; AVM. Currently, the mode provides the following functionalities.
;;
;;   - Syntax Highlight
;;   - Indentation

;;; Code:

(defvar avm-mode-default-indent 4)

(defconst avm-mode--line-type-0
  "^[[:blank:]]*\\([a-zA-Z_][a-zA-Z0-9_]*[[:blank:]]*:[[:blank:]]*\\)[^[:blank:]]"
  "label : instr")

(defconst avm-mode--line-type-1
  "^[[:blank:]]*[a-zA-Z_][a-zA-Z0-9_]*[[:blank:]]*:[[:blank:]]*$"
  "label :")

(defconst avm-mode--line-type-2
  "^\\([[:blank:]]*\\)[^[:blank:]]"
  "    instr")

(defun avm-mode--compute-indent (prev-line)
  (cond
   ((string-match avm-mode--line-type-0 prev-line)
    (length (match-string-no-properties 1 prev-line)))
   ((string-match avm-mode--line-type-1 prev-line)
    avm-mode-default-indent)
   ((string-match avm-mode--line-type-2 prev-line) 'this)
   ((string-match-p "^[[:blank:]]*$" prev-line) 'continue)
   (t avm-mode-default-indent)))

(defun avm-mode--determine-indent ()
  (let ((n (line-number-at-pos)))
    (if (= n 1) avm-mode-default-indent
      (previous-line)
      (let ((ind (avm-mode--compute-indent
		  (buffer-substring-no-properties
		   (line-beginning-position)
		   (line-end-position)))))
	(cond ((eq 'continue ind)
	       (avm-mode--determine-indent))
	      ((eq 'this ind) (current-indentation))
	      (t ind))))))

(defun avm-mode-indent-function ()
  (let ((line (buffer-substring-no-properties
	       (line-beginning-position)
	       (line-end-position))))
    (if (or (string-match-p avm-mode--line-type-0 line)
	    (string-match-p avm-mode--line-type-1 line))
	(indent-line-to 0)
      (let ((cind (current-indentation))
	    (ind (save-excursion (avm-mode--determine-indent))))
	(indent-line-to ind)))))

(defun avm-mode--initialize-buffer ()
  (setq-local indent-line-function 'avm-mode-indent-function))

(define-generic-mode avm-mode
  '("#")
  '("let" "endlet" "add" "eq" "app"
    "tapp" "mark" "grab" "ret" "halt"
    "load" "acc" "b" "bf" "clos")
  '(("\\<true\\|false\\>" . font-lock-constant-face)
    ("\\<[a-zA-Z_][a-zA-Z0-9_]*\\>\\s-*:" . font-lock-function-name-face)
    ("\\<[a-zA-Z_][a-zA-Z0-9_]*\\>" . font-lock-variable-name-face)
    ("\\(-?[1-9][0-9]*\\)\\|0" . font-lock-constant-face))
  '("\\.avm$")
  '(avm-mode--initialize-buffer))

(provide 'avm-mode)
;;; avm-mode.el ends here
