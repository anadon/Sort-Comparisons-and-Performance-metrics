<h1>Sort Comparisons and Performance metrics</h1>
This is a small tool built to compare the performance of various sorts in a very detailed way.  It determines the number of various kinds of iterator operations and reports them if directed.  Else it only runs the sort in a way which makes the GNU time utility meaningful.

<h2>Building</h2>
Required:
 * glibc
 * madlib

This was designed to run on Linux.

For standard usage, use `$: make `.  For debugging, use `$: make debug`.  These produce an executable named `SCP`.

<h2>Running</h2>
For usage, please refer to `$: SCP --help` at this time.

<h2>Licensing and Credits</h2>
This software was developed by Josh Marshall, with some code repurposed from David Musser.  This code and associated programs are distributed under the Affero General Public License version 3.  Details may be found in the `COPYING` file.
