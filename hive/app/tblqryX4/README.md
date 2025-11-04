Form parameters
===============

Input
-----

* objs - input object IDs
* tbl - input object's file name (`_.csv` by default; if name is empty or equals `_.csv` and the table was not found in the object, the code tries to look for `_.tsv` (in tsv-table file objects) or for the first sheet's table (in excel-table objects))
* missing-tbl - whether a missing table in an input object is `fatal` (default) or `nonfatal`
* colsep - column separator when parsing the table (default: `,` for csv, `\t` for tsv)
* commentPrefix - prefix for comment lines to skip when parsing the table (not used by default; can be set to e.g. `#`, `//` etc. for specific input formats)
* parseCnt - max number of lines of the table to parse (not the same as `cnt`!)
* parseStart - number of input lines to skip before starting to parse
* objQry - alternative way to load input data; a query language expression that returns CSV text
* dataCmd - alternative way to load input data; a dna.cgi command that outputs CSV text

Output rows
-----------

* hdr - whether to print the header
* start - first output row to print
* cnt - number of rows to print (or to resample, if `resolution` is specified)
* resolution - number of bins into which to resample rows for printing. (The first and last row of the table will be printed as is. The remaining rows will be resampled into `n-2` bins.)

Output columns
--------------

* cols - range or list of columns to print
* minmaxCols - range or list of columns to min/max if resampling (if `resolution` is specified). This means two rows (with min and max) will be printed for each resample bin.
* minmaxMainCol - column whose values determine whether in the min/max row pair for a single bin the min or the max would get printed first.
* abscissaCol - "position" or "abscissa" column for resampling (if `resolution` is specified); the resampling bins will be evenly spread over the values of this column; values in this column must be monotonically increasing. If not specified, the row index is used as the "position".

Search
------

* search - substring match
* searchRegExp - regexp match
* searchCols - range or list of columns in which to search

TQS parameters
--------------

* tqsId - object ID of TQS file object
* tqs - TQS text; see below for the syntax
* tqsCnt - max number of TQS operations to use (the rest will be ignored)

TQS Syntax
==========

TQS is a JSON array of "operations". Each operation is a JSON object of the following form:

    { "op": "operation name", "arg": { ... } }

`op` specifies the operation name; `arg` is the argument; the remaining top-level keys of the object are ignored by the table query backend and may be used for custom purposes by the  frontend.

Many operations' arguments operate on columns; columns can be specified as:

* number - `"col": 10`
* name - `"col": { "name": "ABC" }` - the first column with name "ABC". More complicated example: `"col": { "name": "ABC", "num": 1, "optional": true }` - this means the second column with name "ABC", and if such a column is not found, that is a non-fatal condition.
* JSON array of numbers/names - `"cols": [ 0, 1, 2, { "name": "Frequency" } ]`

Many operations use query language formulas. The query language for TQS supports the following additional features:

* Dollar syntax for cells in the current row: `$1` is cell in input column 1; `${Frequency}` is cell in input column titled "Frequency"
* `cur_row` - index of current input row
* `input_name` - name of input table
* `input_obj` - ID of input object
* several extra builtin metehods, see classes derived from BuiltinFunction in tblqryX4.cpp

The list of builtin operations and arguments is in comments near the top of tblqryX4.cpp. In addition, each table query plugin has its own TQS operation associated with it.

Code organization
=================

* ExecContext (exec-context.hpp, exec-context.cpp, tblqryX4.cpp) - the  "main application". The key methods:
** `parseForm` - self-explanatory.
** `prepareCommands` - make a list of Commands (see below) from form parameters and TQS. Tell each Command to load its arguments from TQS.
** `loadInput` - load input tables specified by form parameters and TQS. May involve launching and waiting for dna.cgi subrequests.
** `processCommands` - run the table query Commands (see below) to generate the output.
** `saveResult` - save the final table as a CSV file
* InputTableSource (exec-context-files.cpp) - for generic loading of inputs. Since inputs can come not just from form parameters, but from some TQS operations (like `glue` and `load`).
* ParseUtils (utils.hpp) - for parsing column and row TQS arguments (see TQS syntax above)
* Command (tblqryX4_cmd.hpp, tblqryX4.cpp) - roughly (but not exactly) corresponds to a TQS operation. Many of the built-in TQS operations are implemented by the same Command - PrintStage; and the system tries to run as many TQS operations as possible in a single PrintStage for efficiency. Each Command has an `init` function (to parse TQS arguments) and a `compute` (which typically produces an output table from an input table, but may create other output files too). These tables are typically kept in memory, which is less than optimal for large data. Commands may declare that they don't produce a table (`computesOutTable`) or that their output table contains a pointer to the input table and therefore the input cannot be freed before the output (`wrapsInTable`). See the header for more details. Commands may also need their input table to be reinterpreted, in which case a `ReinterpretStage` will be run before them.
* PrintStage (tblqryX4.cpp) - the  most important Command. Unpleasantly complicated logic to handle filtering (on both input and output rows/columns) and resolution resampling. As a result, some of PrintStage's functionality is broken out into separate classes:
* OutputColumns and related classes (rowscols.hpp, rowscols.cpp) - used by PrintStage. The list of columns to print (each one of which may correspond to a physical column in the input table or to the result of a formula). The system tries to keep from recomputing the same formula unnecessarily, since query language is slow and tables can be big.
* OutputBuffer (rowscols.hpp, rowscols.cpp) - used by PrintStage. For efficiency, PrintStage may need to process output cells for a row (or pair of rows) out of order. The cells therefore need to be buffered in OutputBuffer which then prints them in the right order.
* OutputFilter and related classes (rowscols.hpp, rowscols.cpp) - used by PrintStage to implement search, and for `filter` and `colfilter` operations.
* TblQueryContext (qlang.hpp and tblqryX4.cpp) - custom query language dialect used in TQS
* sGluedTabular (glue.hpp) - two tables glued side by side with an sTabular interface; used by the `glue` TQS operation.